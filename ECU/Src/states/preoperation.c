/**
  ******************************************************************************
  * @file           : operation.c
  * @brief          : Operational Loop and related function
  ******************************************************************************

  ******************************************************************************
  */

#include "ecumain.h"
#include "runningprocess.h"
#include "preoperation.h"
#include "configuration.h"
#include "errors.h"
#include "power.h"
#include "adcecu.h"
#include "bms.h"
#include "lcd.h"
#include "input.h"
#include "output.h"
#include "inverter.h"
#include "powernode.h"
#include "timerecu.h"
#include "debug.h"
#include "ivt.h"
#include "eeprom.h"

//#define PRINTDEBUGRUNNING

static uint16_t DevicesOnline( uint16_t returnvalue )
{
	if ( returnvalue == 0xFFFF ) // first loop, set what devices expecting.
	{
		 returnvalue =
	#ifdef HPF19
						  (0x1 << FLeftSpeedReceived) + // initialise return value to all devices in error state ( bit set )
						  (0x1 << FRightSpeedReceived) +
	#endif
						  (0x1 << InverterReceived)+
						  (0x1 << BMSReceived)+
#ifndef POWERNODES
						  (0x1 << PDMReceived)+
#endif
						  (0x1 << PedalADCReceived)+
						  (0x1 << IVTReceived);
	}

	if ( DeviceState.ADCSanity == 0 )
	   returnvalue &= ~(0x1 << PedalADCReceived); // ensures even if analogue nodes online, input needs to be sane for bootup.
	else
	   returnvalue |= 0x1 << PedalADCReceived;


#ifdef HPF19
	if ( DeviceState.FrontSpeedSensors == DISABLED ) // if we've decided to operate without speed sensors, don't process them
	{
		returnvalue &= ~(0x1 << FLeftSpeedReceived);
		returnvalue &= ~(0x1 << FRightSpeedReceived);
	}
	else // check for speed sensor presence.
	{
		if ( sickState(FLSpeed_COBID) != OPERATIONAL ) returnvalue &= ~(0x1 << FLeftSpeedReceived);
		if ( sickState(FRSpeed_COBID) != OPERATIONAL ) returnvalue &= ~(0x1 << FRightSpeedReceived);

//		receiveSick(FLSpeed_COBID);
//		receiveSick(FRSpeed_COBID);
	}
#endif

	static bool first = false;
	if ( 1 ) //DeviceState.Inverter != OFFLINE ) // && GetInverterState() != INERROR )
	{
		if ( !first )
		{
			first = true;
//			DebugMsg("Inverters online in startup.");
		}
	   returnvalue &= ~(0x1 << InverterReceived);
	}
	else
	   returnvalue |= 0x1 << InverterReceived;

#ifndef POWERNODES


//	if ( receivePDM() )// && !errorPDM() )
	{
		returnvalue &= ~(0x1 << PDMReceived);
	}
	else
	{
		returnvalue |= 0x1 << PDMReceived;
	}
#endif

	if ( receiveBMS() ) // ensure heard from BMS
	{
		returnvalue &= ~(0x1 << BMSReceived);
	}
	else
	{
		returnvalue |= 0x1 << BMSReceived;
	}

	if ( receiveIVT() ) // ensure heard from IVT
	{
		returnvalue &= ~(0x1 << IVTReceived);
	}
	else
	{
		returnvalue |= 0x1 << IVTReceived; // why was this commented out?
	}

	// check datalogger response?   --

	return returnvalue; // should be 0 when everything ready.
}

static volatile bool testmotors = false;
static bool testmotorslast = false;

void setTestMotors( bool state )
{
	testmotors = state;
}

#define READYCONFIGBIT		0
//#define READYSDCBIT     	1
#define READYDEVBIT			2
#define READYINVBIT			3
#define READYSENSBIT		4
#define READYPOWERBIT		5
#define READYTSALBIT		6
#define READYTESTING		7

// get external hardware upto state to allow entering operational state on request.
int PreOperationState( uint32_t OperationLoops  )
{
//	static int OperationLoops = 0;
	static uint16_t preoperationstate;
	static uint16_t ReadyToStart;

	char str[80] = "";

#ifdef PRINTDEBUGRUNNING
	PrintRunning( "Boot" );
#else

	sprintf(str,"Boot   %8.8s %.3liv",getCurTimeStr(), CarState.VoltageBMS); // lcd_geterrors());

	lcd_send_stringline(0,str, 255);
#endif
    if ( OperationLoops == 0 )
	{
 //   	setOutput(LED2,On);
    	DebugMsg("Entering Pre Operation State");
    	// pre operation state is to allow hardware to get ready etc, no point in logging errors at this point.
    	// the user can see operational state.
    	SetErrorLogging( false );
		preoperationstate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state.
		InverterAllowTorqueAll(false);

		setOutput(TSLED, On);
		setOutput(RTDMLED, Off);

		ReadyToStart = 0xFFFF;
#ifdef STMADC
		minmaxADCReset();
#endif

		// set startup powerstates to bring devices up.

		setDevicePower( Front1, true );
		setDevicePower( Front2, true );
		setDevicePower( TSAL, true );
		setDevicePower( Current, true );

//		setDevicePower(Buzzer, 0);

//		setDevicePower(IVT, 1);
//		setDevicePower(Telemetry, 1);

		initVectoring();

 //   	NMTReset(); //send NMT reset when first enter state to catch any missed boot messages, see if needed or not.
		// send to individual devices rather than reset everything if needed.
	}
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, PreOperationalState, preoperationstate );

		// do power request

		// test power error checking.
    	if ( DeviceState.IVTEnabled && DeviceState.IVT == OFFLINE )
    	{
    		if ( !powerErrorOccurred( IVT ) )
    			setDevicePower(IVT, true);
    		else
    		{
				Errors.ErrorPlace = 0xAA;
				Errors.ErrorReason = 0;//TODO error code for lost power.
    			return OperationalErrorState;
    		}
    	}

		// generate device waiting string.

		if ( preoperationstate != 0 || ReadyToStart != 0 ){
			// TODO add a checker that if all devices on canbus missing, show this instead of individual.

			strcpy(str, "Wait:");

			char * nodewait = "";

#ifdef POWERNODES
			nodewait = getPNodeWait();
			if ( nodewait[0] != 0)
			{
				sprintf(&str[strlen(str)],"P%s ", nodewait );
			}
#endif

#ifdef ANALOGNODES


			if ( DeviceState.Sensors != OPERATIONAL )
			{
				nodewait = getADCWait();
				if ( nodewait[0] != 0)
				{
					sprintf(&str[strlen(str)],"A%s ", nodewait );
				}
			}
#endif

#ifdef HPF19
			if (preoperationstate & (0x1 << FLeftSpeedReceived) )  { strcat(str, "FSL " ); }
			if (preoperationstate & (0x1 << FRightSpeedReceived) )  {strcat(str, "FSR " );  }
#endif
			if (preoperationstate & (0x1 << InverterReceived) ) {
				if ( getEEPROMBlock(0)->InvEnabled )
					strcat(str, "INV " );
				else
					strcat(str, "INVDIS " );
			}

			if (preoperationstate & (0x1 << BMSReceived) ) { strcat(str, "BMS " );  }
#ifndef POWERNODES
			if (preoperationstate & (0x1 << PDMReceived) ) { strcat(str, "PDM " ); }
#endif
#ifndef STMADC // ADC is onboard, not waiting for it.
			if (preoperationstate & (0x1 << PedalADCReceived) ) {strcat(str, "ADC " );  }
#endif
			if (preoperationstate & (0x1 << IVTReceived) ) { strcat(str, "IVT " );  }
			if ( str[strlen(str)] == 32 ) { str[strlen(str)] = 0 ; }

			strpad(str,20, true);

#ifndef PRINTDEBUGRUNNING
			lcd_send_stringline(1,str, 255);
#endif

			if ( ReadyToStart != 0 )
			{
				strcpy(str, "Err:");


//				if (ReadyToStart & (0x1 << READYTESTING ) ) {	strcat(str, "TST " ); }

#ifdef STMADC // ADC is onboard, any issues with it are an error not a wait.
				if (preoperationstate & (0x1 << PedalADCReceived) ) { strcat(str, "ADC " ); }
#endif

#if 0
				if (ReadyToStart & (0x1 << READYSDCBIT ) ) {

					strcat(str, "SDC(" );

						strcat(str, ShutDownOpenStr());

					strcat(str, ") " );
				}
#endif

				if (ReadyToStart & (0x1 << READYTSALBIT ) ) { strcat(str, "TSAL " );  }

				if (ReadyToStart & (0x1 << READYCONFIGBIT ) ) {	strcat(str, "CFG " ); }

				if (ReadyToStart & (0x1 << READYDEVBIT ) ) {	strcat(str, "DEV " ); }

				if (ReadyToStart & (0x1 << READYSENSBIT ) ) {	strcat(str, "SNS " ); }

				if (ReadyToStart & (0x1 << READYPOWERBIT ) ) {	strcat(str, "PWR " ); }

#ifndef POWERNODES
				if (! ( preoperationstate & (0x1 << PDMReceived) ) ) {
				// only show as PDM error if pdm is on bus.
					if (preoperationstate & (0x1 << PDMReceived) ) { strcat(str, "PDM " ); }

					if (ReadyToStart & (0x1 << READYPOWERBIT ) ) { strcat(str, "PDM " );  } // ?
				}
#endif
				if (ReadyToStart & (0x1 << READYINVBIT ) ) {

					if ( getEEPROMBlock(0)->InvEnabled )
						strcat(str, "INV " );
					else
						strcat(str, "INVDIS " );
				}


				strpad(str,20, true);
#ifndef PRINTDEBUGRUNNING
				lcd_send_stringline(2,str,255);
#endif
			}

		} else
		{
			// TODO print any non critical warning still.

			lcd_send_stringpos(1,0,"                    ", 255);
			lcd_send_stringpos(2,0,"   Ready To Start   ", 255);
			CheckCriticalError(); // clear any pending critical errors from startup procedure.
		}

	}

	lcd_send_stringline(3, getConfStr(), 255);

	ReadyToStart = 0;

	if ( !inConfig() )
	{
		if ( CheckButtonPressed(Config_Input) )
		{
			if ( !testmotors )
			{
				DebugPrintf("Enter Config\r\n");
				ConfigInput( 0xFFFF );
				ReadyToStart |= (1<<READYCONFIGBIT); // we're probably entering config, don't allow startup this cycle.
			}
		}

		static bool showbrakebal = false;

		static bool showadc = false;

		static bool showcurrent = false;

		switch ( GetLeftRightPressed() )
		{
			case -1 : if (!showadc && !showcurrent) showbrakebal = !showbrakebal; break;
			case 1 : if (!showbrakebal && !showcurrent) showadc = !showadc; break;
		}

		switch ( GetUpDownPressed() )
		{
			case -1 : if ( showcurrent )
				{
					clearRunningData();
					lcd_send_stringline( 0, "Clear runningval", 3);
				}
				break;
			case 1 :  if (!showadc && !showbrakebal) showcurrent = !showcurrent; break;
		}

		if ( showcurrent )
		{
			sprintf(str, "IVT %3dA  0%3dA",
					runtimedata_p->maxIVTI,
					runtimedata_p->maxMotorI[0]
			);

			lcd_send_stringline(0,str, 255);

			sprintf(str, "1%3dA 2%3dA 3%3dA ",
				runtimedata_p->maxMotorI[1],
				runtimedata_p->maxMotorI[2],
				runtimedata_p->maxMotorI[3] );

			lcd_send_stringline(1,str, 255);

		}

		if ( showbrakebal ) PrintBrakeBalance( );

		if ( showadc )
		{
#ifdef ADC
			sprintf(str,"A1 %.5lu %.5lu %.5lu", ADC_Data[0], ADC_Data[1], ADC_Data[2]);
			lcd_send_stringline(1,str, 255);

			sprintf(str,"A3 %.5lu %.5lu %.5lu", ADC_Data[3], ADC_Data[4], ADC_Data[5]);
			lcd_send_stringline(2,str, 255);
#else
			sprintf(str, "L%3d%% R%3d%% B%3d%% ",
					    ADCState.Torque_Req_L_Percent/10,
					    ADCState.Torque_Req_R_Percent/10,
					    ADCState.Regen_Percent/10);

			lcd_send_stringline(0,str, 254);
#endif
		}
	} else
	{
		ReadyToStart |= (1<<READYCONFIGBIT);  // being in config means not ready to start.
		// process config input.

		if ( CheckButtonPressed(Config_Input) )
		{
			DebugPrintf("Enter\r\n");
			ConfigInput( KEY_ENTER );
		}

		switch ( GetLeftRightPressed() )
		{
			case -1 :
				DebugPrintf("Left\r\n");
				ConfigInput( KEY_LEFT ); break;
			case 1 :
				DebugPrintf("Right\r\n");
				ConfigInput( KEY_RIGHT ); break;
		}

		switch ( GetUpDownPressed() )
		{
			case -1 :
				DebugPrintf("Up\r\n");
				ConfigInput( KEY_UP ); break;
			case 1 :
				DebugPrintf("Down\r\n");
				ConfigInput( KEY_DOWN ); break;
		}

	}

	vTaskDelay(5);

	preoperationstate = DevicesOnline(preoperationstate);

	// set drive mode

	setCurConfig();

	// allow APPS checking before RTDM
	int16_t Torque_Req = PedalTorqueRequest();

	vectoradjust adj;

	doVectoring(Torque_Req, &adj);

//	if ( testmotorslast ) InverterSetTorque(&adj, 1000);

	if ( CarState.APPSstatus )
		setOutput(RTDMLED,On);
	else
		setOutput(RTDMLED,Off);

	// Check startup requirements.

#if 0
	if ( !CheckShutdown() )
	{
		blinkOutput(TSOFFLED, BlinkMed, 1);
#ifdef SHUTDOWNSWITCHCHECK
	    ReadyToStart |= (0x1 << READYSDCBIT );
#endif
	}  else
	{
		blinkOutput(TSOFFLED, On, 0);
		setOutput(TSOFFLED,On);
	}
#endif

	static int percR = -1;
	static int32_t requestNm = 0;
	static bool waitprecharge = false;
	static bool doneprecharge = false;

	if ( !getDevicePower(Front1) ||
		 !getDevicePower(Front2) ||
		 !getDevicePower(Inverters) )
	{
		blinkOutput(TSLED, On, LEDBLINKNONSTOP);
	}

	static bool powerset = false;

	if ( CheckTSActivationRequest() )
	{
		if ( !powerset )
		{
			invRequestState( BOOTUP );
			resetDevicePower(Inverters);
			setDevicePower( Inverters, true );
			resetDevicePower(Front1);
			setDevicePower( Front1, true );
			resetDevicePower(Front2);
			setDevicePower( Front2, true );
			setDevicePower( RightPump, true );
			setDevicePower( LeftPump, true );
			setDevicePower( RightFans, true );
			setDevicePower( LeftFans, true );
			lcd_send_stringline( 3, "Power requested!", 3);
			DebugMsg("Power requested.");
			powerset = true;
		}
		else
		{
			setDevicePower( Inverters, false );
			lcd_send_stringline( 3, "Power Inv off!", 3);
			DebugMsg("Power off to inverters requested.");
			powerset = false;
		}

	}

#if 0

	if ( CheckRTDMActivationRequest() || testmotors != testmotorslast ) // manual startup power request whilst in testing phases, allows to reset if error occurred.
	{
		if ( testmotors )
		{
			lcd_send_stringline( 3, "Ending test mode.", 3);
			DebugMsg("Ending test mode.");
			percR = -1;
			requestNm = 0;
			testmotors = false;
			InverterAllowTorqueAll( false );
			ShutdownCircuitSet(false);
			setDevicePower( Buzzer, false );
			invRequestState( BOOTUP );
			// don't disable power other than HV.
		} else
		{
			if ( ADCState.Torque_Req_R_Percent == 0 )
			{
				lcd_send_stringline( 3, "Starting test mode.", 3);
				DebugMsg("Starting test mode.");
				testmotors = true;
#if 1
				setDevicePower( Inverters, true );
				setDevicePower( RightPump, true );
				setDevicePower( LeftPump, true );
				InverterAllowTorqueAll( true );
				invRequestState( OPERATIONAL );

				// TODO set a timer to turn off buzzer.
//				setDevicePower( Buzzer, true );
#endif
				ShutdownCircuitSet(true);
				waitprecharge = false;
				doneprecharge = false;
			} else
			{
				lcd_send_stringline( 3, "Release pedal!", 3);
				DebugMsg("Can't start test mode, pedal not at 0.");
			}
		}

		testmotorslast = testmotors;
	}

	if ( testmotors )
	{
		int16_t speed = getEEPROMBlock(0)->maxRpm;
		int32_t maxNm = getEEPROMBlock(0)->MaxTorque;

		uint32_t motorsenabled = getEEPROMBlock(0)->EnabledMotors;

		if ( percR != ADCState.Torque_Req_R_Percent ) // only update if value changes.
		{
			if ( DeviceState.CriticalSensors == OPERATIONAL )
			{
				percR = ADCState.Torque_Req_R_Percent;
			} else
			{
				percR = 0;
				lcd_send_stringline(3,"Pedal timeout", 3);
			}

			requestNm = ((percR*maxNm)*0x4000)/1000;

			if ( Shutdown.PRE )
			{
				if ( ! doneprecharge )
				{
					DebugMsg("Precharge done, torque request active.");
					doneprecharge = true;
					waitprecharge = false;
				}

				for ( int i=0;i<MOTORCOUNT;i++)
				{
	#if 1
					if ( requestNm > 0 && ( ( 1 << i ) & motorsenabled ) )
						InverterSetTorqueInd( i, requestNm, speed);
					else
	#endif
						InverterSetTorqueInd( i, 0, 0);
				}
			} else
			{
				if ( ! waitprecharge )
				{
					waitprecharge = true;
					DebugMsg("Waiting for precharge");
				}
				lcd_send_stringline( 2, "Waiting for PRECh", 3);
			}


			DebugPrintf("Pedal: r%d%%, reqNm %d speed %d, maxNm %d, to MC[%s] 0[I%dc M%dc] 1[I%dc M%dc] 2[I%dc M%dc] 3[I%dc M%dc]\r\n ",
						percR/10, requestNm/0x4000, speed, maxNm, getMotorsEnabledStr(),
						getInvState(0)->InvTemp, getInvState(0)->MotorTemp,
						getInvState(1)->InvTemp, getInvState(1)->MotorTemp,
						getInvState(2)->InvTemp, getInvState(2)->MotorTemp,
						getInvState(3)->InvTemp, getInvState(3)->MotorTemp
					);
		}

		lcd_send_stringline(0,"Motor test.", 254);
		snprintf(str, 80, "P%3d%% Nm %lu %drpm", percR/10, requestNm/0x4000, speed);

		lcd_send_stringline(1,str,254);

		int highesti = 0;
		int highestin = -1;
		int highestm = 0;
		int highestmn = -1;

		for ( int i=0;i<MOTORCOUNT;i++)
		{
			if (getInvState(i)->InvTemp >= highesti )
			{
				highesti = getInvState(i)->InvTemp;
				highestin = i;
			}
			if (getInvState(i)->MotorTemp >= highesti )
			{
				highestm = getInvState(i)->MotorTemp;
				highestmn = i;
			}
		}

		sprintf(str, "I%d %dc  M%d %dc", highestin, highesti, highestmn, highestm);

		lcd_send_stringline(2,str,254);
	}
#endif

//	if ( DeviceState.CriticalSensors != OPERATIONAL ) { ReadyToStart |= (1<<READYTESTING); }

//	if ( errorPower() ) { ReadyToStart += 1; }

	if ( preoperationstate != 0 ) { ReadyToStart |= (1<<READYDEVBIT); }


	for ( int i=0;i<MOTORCOUNT; i++)
	{
		if ( getInvState(i)->Device == OFFLINE )
		{
			ReadyToStart |= (1<<READYINVBIT);
		}
	}

	if ( DeviceState.CriticalSensors != OPERATIONAL ) { ReadyToStart |= (1<<READYSENSBIT); } // require critical sensor nodes online for startup.
	if ( DeviceState.PowerNodes != OPERATIONAL ) { ReadyToStart |= (1<<READYPOWERBIT); }
	if ( !getDevicePower(TSAL) ) { ReadyToStart |= (1<<READYTSALBIT); } // require TSAL power to allow startup.

	if ( ReadyToStart == 0 )
	{
		setOutput(STARTLED, On);
			// devices are ready and in pre operation state.
			// check for request to move to active state.

		if ( testmotors )
		{
			return PreOperationalState;
		}

		if ( CheckActivationRequest() ) // check if driver has requested activation and if so proceed
		{
			OperationLoops = 0;
			return OperationalReadyState; // normal operational state on request
		}
	} else
	{ // hardware not ready for active state
		blinkOutput(STARTLED, LEDBLINK_TWO, 1);
		if ( OperationLoops == 50 ) // 0.5 seconds, send reset nmt, try to get inverters online if not online at startup.
		{

		}

		if ( CheckActivationRequest() == 1 )
		{
			if ( 1 ) // calculate this to max time for expecting everything online
			{
				// user pressed requesting startup sequence before devices ready
				blinkOutput(TSLED, LEDBLINK_FOUR, 1000);
				CAN_SendErrorStatus(1,PowerOnRequestBeforeReady,0);

				lcd_send_stringline( 3, "Not ready.", 3);
			}
		}
	}

	return PreOperationalState; // nothing caused entry to a different state, continue in current state.
}



