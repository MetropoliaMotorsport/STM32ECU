/**
  ******************************************************************************
  * @file           : operation.c
  * @brief          : Operational Loop and related function
  ******************************************************************************

  ******************************************************************************
  */

#include "main.h"
#include <stdlib.h>

/* Private includes ----------------------------------------------------------*/

#include "ecumain.h"
#include "ecu.h"
#include "configuration.h"
#include "output.h"
#include "input.h"
#include "canecu.h"

#ifdef LCD
  #include "vhd44780.h"
#endif


void setDriveMode(void)
{
	switch ( ADCState.DrivingMode )
	{

		SetupNormalTorque();

		CarState.LimpDisable = 0;
		CarState.DrivingMode = ADCState.DrivingMode;

		case 1: // 5nm  5 , 5,    0,     5,   5,    10,    15,    20,   25,     30,    64,    65,   0
			CarState.Torque_Req_Max = 5;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 0;
#endif
			break;
		case 2: // 10nm
			CarState.Torque_Req_Max = 25;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 0;
#endif
			break;
		case 3: // 15nm
			CarState.Torque_Req_Max = 25;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 1;
#endif
			break;
		case 4: // 20nm
			CarState.Torque_Req_Max = 35;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 0;
#endif
			break;
		case 5: // 25nm
			CarState.Torque_Req_Max = 35;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 1;
#endif
			break;
		case 6: // 30nm
			CarState.Torque_Req_Max = 65;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 0;
#endif
			SetupLargeLowRangeTorque();
			break;
		case 7: // 65nm Track
			CarState.Torque_Req_Max = 65;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 1;
#endif
			SetupLargeLowRangeTorque();
			break;
		case 8: // 65nm Accel
			CarState.Torque_Req_Max = 65;
			CarState.LimpDisable = 1;
			SetupLowTravelTorque();
			break;

	}

	CarState.Torque_Req_CurrentMax = CarState.Torque_Req_Max;

}


static uint16_t DevicesOnline( uint16_t returnvalue )
{
// external devices ECU expects to be present on CAN
//	InverterOnline
//	PDMOnline
//	FLeftSpeedOnline
//	FRightSpeedOnline
//	PedalsADC = 1;
	if ( returnvalue == 0xFFFF ) // first loop, set what devices expecting.
	{
		 returnvalue =
	#ifdef HPF19
						  (0x1 << FLeftSpeedReceived) + // initialise return value to all devices in error state ( bit set )
						  (0x1 << FRightSpeedReceived) +
	#endif
						  (0x1 << Inverter1Received)+
#ifdef TWOINVERTERMODULES
						  (0x1 << Inverter2Received)+
#endif
						  (0x1 << BMSReceived)+
						  (0x1 << PDMReceived)+
						  (0x1 << PedalADCReceived)+
						  (0x1 << IVTReceived);
	}

#ifdef STMADC
#endif
	if ( DeviceState.ADC == OFFLINE )
	{
		DeviceState.ADC = OPERATIONAL;
		returnvalue &= ~(0x1 << PedalADCReceived); // using local adc, already established online in initialisation.
	} else
	{
		if ( CheckADCSanity() == 0 )
		   returnvalue &= ~(0x1 << PedalADCReceived);
		else
		   returnvalue |= 0x1 << PedalADCReceived;
	}

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


	if ( receiveINVNMT(&CarState.Inverters[Inverter1]))
	{
	//		if ( ( CarState.LeftInvState != 0xFF || CarState.RightInvState != 0xFF ) )
			if ( GetInverterState(CarState.Inverters[Inverter1].InvState) >= 0 || GetInverterState(CarState.Inverters[Inverter1+1].InvState) >= 0 )
			{
				returnvalue &= ~(0x1 << Inverter1Received);
			} else returnvalue |= 0x1 << Inverter1Received;

	}

#ifdef TWOINVERTERMODULES
	if ( receiveINVNMT(CarState.Inverters[Inverter2]))
	{
	//		if ( ( CarState.LeftInvState != 0xFF || CarState.RightInvState != 0xFF ) )
			if ( GetInverterState(CarState.Inverters[Inverter2].InvState) >= 0 || GetInverterState(CarState.Inverters[Inverter2+1].InvState) >= 0 )
			{
				returnvalue &= ~(0x1 << Inverter2Received);
			} else returnvalue |= 0x1 << Inverter2Received;

	}
#endif

	if ( receivePDM() )// && !errorPDM() )
	{
		returnvalue &= ~(0x1 << PDMReceived);
	}
	else
	{
		returnvalue |= 0x1 << PDMReceived;
	}

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
	//	returnvalue |= 0x1 << IVTReceived;
	}

	// check datalogger response?   --

	return returnvalue; // should be 0 when everything ready.
}

int NMTReset( void )
{
	// SetSensors
	// send NMT reset message incase a node had sent bootup message before ECU saw it.

	 if ( CAN_NMT( 0x82, 0x0 ) )  //0x81 reset nodes, 0x82 reset comms. // 0x0 being sent, not correct protocol
	 {
//		 return OperationalErrorState;
	 }

	return 0;
}

// get external hardware upto state to allow entering operational state on request.
int PreOperation( uint32_t OperationLoops  )
{
//	static int OperationLoops = 0;
	static uint16_t preoperationstate;
	static uint16_t ReadyToStart;

	char RequestState = PreOperationalState; // initialise to PreOperationalState as default requested next state.

    if ( OperationLoops == 0 )
	    {
    		lcd_setscrolltitle("Pre Operation");
    		lcd_send_stringpos(3,0,"  <Red for config>");
	    	preoperationstate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state.
	    	CarState.HighVoltageReady = 0;
	    	ReadyToStart = 0xFFFF;
	    	minmaxADCReset();
	 //   	NMTReset(); //send NMT reset when first enter state to catch any missed boot messages, see if needed or not.
	    	// send to individual devices rather than reset everything if needed.
	    }
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, PreOperationalState, preoperationstate );

		// generate device waiting string.

		if ( preoperationstate != 0 || ReadyToStart != 0 ){
 //   		lcd_send_stringpos(1,0,"Waiting for:");

			char str[80] = "Wait:";

#ifdef HPF19
			if (preoperationstate & (0x1 << FLeftSpeedReceived) )  { strcat(str, "FSL" ); }
			if (preoperationstate & (0x1 << FRightSpeedReceived) )  {strcat(str, "FSR" );  }
#endif
			if (preoperationstate & (0x1 << Inverter1Received) ) { strcat(str, "IV1 " ); }
#ifdef TWOINVERTERMODULES
			if (preoperationstate & (0x1 << Inverter2Received) )  { strcat(str, "IV2" );  }
#endif
			if (preoperationstate & (0x1 << BMSReceived) ) { strcat(str, "BMS " );  }
			if (preoperationstate & (0x1 << PDMReceived) ) { strcat(str, "PDM " ); }
			if (preoperationstate & (0x1 << PedalADCReceived) ) {strcat(str, "ADC " );  }
			if (preoperationstate & (0x1 << IVTReceived) ) { strcat(str, "IVT " );  }
			if ( str[strlen(str)] == 32 ) { str[strlen(str)] = 0 ; }
			if ( strlen(str) > 20 ) { str[18] = '.'; str[19] = '.'; str[20] = 0; };
			lcd_send_stringpos(1,0,str);

			if ( ReadyToStart != 0 ){
				strcpy(str, "Err:");

				if (ReadyToStart & (0x1 << 0 ) ) { strcat(str, "PDM " );  }
				if (ReadyToStart & (0x1 << 2 ) ) { strcat(str, "INV " );  }
				if (ReadyToStart & (0x1 << 3 ) ) { strcat(str, "ShutdownSW " );  }

				if ( strlen(str) > 20 ) { str[18] = '.'; str[19] = '.'; str[20] = 0; };
				lcd_send_stringpos(2,0,str);
			}

		} else
		{
			lcd_clearscroll();

				lcd_send_stringpos(1,0,"Ready To Start      ");
				lcd_send_stringpos(2,0,"                    ");
				lcd_send_stringpos(3,0,"   <Press Yellow>   ");

//			char str[20] ="";
//			lcd_send_stringpos(2,0,str);
		}

	}

//	ResetCanReceived();

	uint32_t loopstart = gettimer();
	uint32_t looptimer = 0;

	// check if received configuration requests, or mode change -> testing state.
	switch ( CheckConfigurationRequest() ) // allows RequestState to be set to zero to prevent mode changing mid config, or request a different mode.
	{
		case TestingState :
			RequestState = TestingState; // Testing state requested from Configuration.
			break;
/*
		case LimpState :
			RequestState = LimpState; // Limpmode state requested from Configuration, set as requested next state.
			break; */

		case ReceivingConfig :
			preoperationstate = 2^15;
			RequestState = PreOperationalState; // don't allow leaving pre operation whilst
												// in middle of processing a configuration / testing request.
			break;
		case 0:
		default:
			RequestState = OperationalReadyState; // nothing happening in config, assume normal operation.
			// do nothing
	}

	sendPDM( 0 );

	// checks if we have heard from other necessary connected devices for operation.

	while (  looptimer < PROCESSLOOPTIME-50 ) {
		looptimer = gettimer() - loopstart;
		__WFI(); // sleep till interrupt, avoid loading cpu doing nothing.
	}; // check

	preoperationstate |= DevicesOnline(preoperationstate);

	uint16_t error = CheckErrors();
	// check error state.
	if ( error )
	{
		// print error reasons preventing startup.
//		return OperationalErrorState; // if a device is in error state, quit to error handler to decide how to proceed.
	}

	// set drive mode

	setDriveMode();

	// allow APPS checking before RTDM
	int Torque_Req = PedalTorqueRequest();

	for ( int i=0;i<INVERTERCOUNT;i++){
		CarState.Inverters[i].Torque_Req = Torque_Req;
	}


#ifdef TORQUEVECTOR
		TorqueVectorProcess( Torque_Req );
#endif

	if ( CarState.APPSstatus ) setOutput(RTDMLED_Output,LEDON); else setOutput(RTDMLED_Output,LEDOFF);

#ifdef FANCONTROL
		FanControl();
#endif

	if ( !CarState.TestHV )
	{
		for ( int i=0;i<INVERTERCOUNT;i++)
		{
			receiveINVStatus(&CarState.Inverters[i]);
			receiveINVSpeed(&CarState.Inverters[i]);
			receiveINVTorque(&CarState.Inverters[i]);
		}
	}

	if ( !CarState.ShutdownSwitchesClosed )
	{
		blinkOutput(TSOFFLED_Output, LEDBLINK_FOUR, 1);
	}  else
	{
		blinkOutput(TSOFFLED_Output, LEDON, 0);
		setOutput(TSOFFLED_Output,LEDON);
	}

	bool invonline = true;
	for ( int i=0;i<INVERTERCOUNT;i++)
	{
		if ( CarState.Inverters[i].InvState == 0xFF ) invonline = false; // if any inverter has yet to be put in a status, all inverters are not ready.
	}


	ReadyToStart = 0;

	if ( errorPDM() ) { ReadyToStart += 1; }
	if ( preoperationstate != 0 ) { ReadyToStart += 2; }
	if ( !invonline ) { ReadyToStart += 4; }
#ifdef SHUTDOWNSWITCHCHECK
	if ( CarState.ShutdownSwitchesClosed )  { ReadyToStart += 8; }
#endif

/*	if ( !errorPDM()
			&& preoperationstate == 0
			&& invonline // everything is ready to move to next state.
#ifdef SHUTDOWNSWITCHCHECK
            && CarState.ShutdownSwitchesClosed // only allow startup procedure if shutdown switches open.
#endif
	   )
	{
		ReadyToStart = true;
	} else
	{
		ReadyToStart = false;
	}
*/

	if ( ReadyToStart == 0 )
	{
		blinkOutput(TSLED_Output, LEDBLINK_ONE, 1);
			// devices are ready and in pre operation state.
			// check for request to move to active state.

			if ( ( RequestState == TestingState ) )
			{
//				OperationLoops = 0;
//				return TestingState; // an alternate mode ( testing requested in config for next state.
				return PreOperationalState;
			}

			if ( CheckActivationRequest() && RequestState != TestingState ) // check if driver has requested activation and if so proceed
			{
				OperationLoops = 0;

				for ( int i=0;i<INVERTERCOUNT;i++){
					CarState.Inverters[i].Torque_Req = 0;
				}

				setOutput(RTDMLED_Output,LEDOFF);
				return OperationalReadyState; // normal operational state on request

			}

	} else
	{ // hardware not ready for active state
		if ( OperationLoops == 50 ) // 0.5 seconds, send reset nmt, try to get inverters online if not online at startup.
		{
#ifdef HPF19 // unsue on expected HPF20 behaviour yet.
			if ( !CarState.TestHV )	NMTReset();
#endif
		}

		if ( CheckActivationRequest() == 1 )
		{
			if ( 1 ) // calculate this to max time for expecting everything online
			{
				// user pressed requesting startup sequence before devices ready
				blinkOutput(TSLED_Output, LEDBLINK_FOUR, 1);
				CAN_SendStatus(1,PowerOnRequestBeforeReady,0);

				// send NMT.
			} else
			{
				OperationLoops = 0;

				CAN_SendStatus(1,PowerOnRequestTimeout,0);
				blinkOutput(TSLED_Output, LEDBLINK_FOUR, 1);

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



