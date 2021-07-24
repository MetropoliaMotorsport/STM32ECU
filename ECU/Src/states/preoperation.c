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
			DebugMsg("Inverters online in startup.");
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
#define READYSDCBIT     	1
#define READYDEVBIT			2
#define READYINVBIT			3
#define READYSENSBIT		4
#define READYPOWERBIT		5

// get external hardware upto state to allow entering operational state on request.
int PreOperationState( uint32_t OperationLoops  )
{
//	static int OperationLoops = 0;
	static uint16_t preoperationstate;
	static uint16_t ReadyToStart;

	char RequestState = PreOperationalState; // initialise to PreOperationalState as default requested next state.

	char str[80] = "";

#ifdef PRINTDEBUGRUNNING
	PrintRunning( "Boot" );
#else

	sprintf(str,"Boot   %8.8s %.3liv",getCurTimeStr(), CarState.VoltageBMS); // lcd_geterrors());

	lcd_send_stringline(0,str, 255);
#endif
    if ( OperationLoops == 0 )
	{
    	// pre operation state is to allow hardware to get ready etc, no point in logging errors at this point.
    	// the user can see operational state.
    	SetErrorLogging( false );
		preoperationstate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state.
		InverterAllowTorqueAll(false);

		ReadyToStart = 0xFFFF;
#ifdef STMADC
		minmaxADCReset();
#endif

		// set startup powerstates to bring devices up.

		setDevicePower( Front1, true );
		setDevicePower( Front2, true );
		setDevicePower( TSAL, true );

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

		if ( testmotors != testmotorslast)
		{
			InverterAllowTorqueAll(testmotors);
			testmotorslast = testmotors;
		}

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
			if (preoperationstate & (0x1 << InverterReceived) ) { strcat(str, "INV " ); }

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
#ifdef STMADC // ADC is onboard, any issues with it are an error not a wait.
				if (preoperationstate & (0x1 << PedalADCReceived) ) { strcat(str, "ADC " ); }
#endif

				if (ReadyToStart & (0x1 << READYSDCBIT ) ) {

					strcat(str, "SDC(" );

						strcat(str, ShutDownOpenStr());

					strcat(str, ") " );
				}

#ifndef POWERNODES
				if (! ( preoperationstate & (0x1 << PDMReceived) ) ) {
				// only show as PDM error if pdm is on bus.
					if (preoperationstate & (0x1 << PDMReceived) ) { strcat(str, "PDM " ); }

					if (ReadyToStart & (0x1 << READYPOWERBIT ) ) { strcat(str, "PDM " );  } // ?
				}
#endif
				if (ReadyToStart & (0x1 << READYINVBIT ) ) { strcat(str, "INV " );  }


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
		}

	}

	lcd_send_stringline(3, getConfStr(), 255);

	// TODO this variable is going to be done away with.
	RequestState = OperationalReadyState; // nothing happening in config, assume normal operation.

	ReadyToStart = 0;

	if ( !inConfig() )
	{
		if ( CheckButtonPressed(Config_Input) )
		{
			DebugPrintf("Enter Config\r\n");
			ConfigInput( 0xFFFF );
			ReadyToStart |= (1<<READYCONFIGBIT); // we're probably entering config, don't allow startup this cycle.
		}

		static bool showbrakebal = false;

		static bool showadc = false;

		switch ( GetLeftRightPressed() )
		{
			case -1 : showbrakebal = !showbrakebal; break;
			case 1 : showadc = !showadc; break;
		}

		switch ( GetUpDownPressed() )
		{
			case -1 : showbrakebal = !showbrakebal; break;
			case 1 : showadc = !showadc; break;
		}

		if ( showbrakebal ) PrintBrakeBalance( );

	#ifdef ADC
		if ( showadc )
		{
			sprintf(str,"A1 %.5lu %.5lu %.5lu", ADC_Data[0], ADC_Data[1], ADC_Data[2]);
			lcd_send_stringline(1,str, 255);

			sprintf(str,"A3 %.5lu %.5lu %.5lu", ADC_Data[3], ADC_Data[4], ADC_Data[5]);
			lcd_send_stringline(2,str, 255);
		}
	#endif
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

	setRunningPower( false, false );

	vTaskDelay(5);

	preoperationstate = DevicesOnline(preoperationstate);

	// set drive mode

	setCurConfig();

	// allow APPS checking before RTDM
	int Torque_Req = PedalTorqueRequest();

	vectoradjust adj;

	doVectoring(Torque_Req, &adj);

//	if ( testmotorslast ) InverterSetTorque(&adj, 1000);

	if ( CarState.APPSstatus )
		setOutput(RTDMLED,On);
	else
		setOutput(RTDMLED,Off);

	// Check startup requirements.

	if ( !CheckShutdown() )
	{
		blinkOutput(TSOFFLED, BlinkMed, 1);
#ifdef SHUTDOWNSWITCHCHECK
	    ReadyToStart += 2;
#endif
	}  else
	{
		blinkOutput(TSOFFLED, On, 0);
		setOutput(TSOFFLED,On);
	}


	if ( CheckRTDMActivationRequest() ) // manual startup power request whilst in testing phases, allows to reset if error occurred.
	{
//		setDevicePower( Front1, true );
//		setDevicePower( Front2, true );
//		setDevicePower( TSAL, true );
		setDevicePower( Inverters, true );
		setDevicePower( RightPump, true );
		setDevicePower( LeftPump, true );
		DebugPrintf("Setting startup power.");
		lcd_send_stringline( 3, "Setting startup power.", 3);
	}

//	if ( errorPower() ) { ReadyToStart += 1; }

	if ( preoperationstate != 0 ) { ReadyToStart |= (1<<READYDEVBIT); }
	if ( GetInverterState() < BOOTUP  ) { ReadyToStart |= (1<<READYINVBIT);} // require inverters to be online
	if ( DeviceState.CriticalSensors != OPERATIONAL ) { ReadyToStart |= (1<<READYSENSBIT); } // require critical sensor nodes online for startup.
	if ( DeviceState.PowerNodes != OPERATIONAL ) { ReadyToStart |= (1<<READYPOWERBIT);}
//	if ( !getDevicePower(TSAL) ) { ReadyToStart += 64; } // require TSAL power to allow startup.

	if ( ReadyToStart == 0 )
	{
		blinkOutput(TSLED, LEDBLINK_ONE, 1);
			// devices are ready and in pre operation state.
			// check for request to move to active state.

			if ( ( RequestState == TestingState ) )
			{
				return PreOperationalState;
			}

			if ( CheckActivationRequest() && RequestState != TestingState ) // check if driver has requested activation and if so proceed
			{
				OperationLoops = 0;

				setOutput(RTDMLED,Off);
				return OperationalReadyState; // normal operational state on request

			}

	} else
	{ // hardware not ready for active state
		if ( OperationLoops == 50 ) // 0.5 seconds, send reset nmt, try to get inverters online if not online at startup.
		{

		}

		if ( CheckActivationRequest() == 1 )
		{
			if ( 1 ) // calculate this to max time for expecting everything online
			{
				// user pressed requesting startup sequence before devices ready
				blinkOutput(TSLED, LEDBLINK_FOUR, 1);
				CAN_SendStatus(1,PowerOnRequestBeforeReady,0);

				lcd_send_stringline( 3, "Not ready.", 3);

				// send NMT.
			} else
			{
				OperationLoops = 0;

				CAN_SendStatus(1,PowerOnRequestTimeout,0);
				blinkOutput(TSLED, LEDBLINK_FOUR, 1);

				if ( ( RequestState == TestingState ) )
				{   // should allow testing mode regardless of all hardware being initialised
					OperationLoops = 0;
					return TestingState; // an alternate mode ( testing requested in config for next state.
				}

				// check if limp state possible?
				// return LimpState; // either testing or limp mode requested requested. - not seperate state in current config.

				return OperationalErrorState; // quit right away to error handler state if no possible special state requested on timeout
			}
		}
	}

	return PreOperationalState; // nothing caused entry to a different state, continue in current state.
}



