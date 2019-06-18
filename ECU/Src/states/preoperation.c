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
	 returnvalue = (0x1 << FLeftSpeedReceived) + // initialise return value to all devices in error state ( bit set )
					  (0x1 << FRightSpeedReceived) +
					  (0x1 << InverterReceived)+
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

	if ( receiveINVNMT(LeftInverter) )
	{
//		if ( ( CarState.LeftInvState != 0xFF || CarState.RightInvState != 0xFF ) )
		if ( GetInverterState(CarState.LeftInvState) >= 0 || GetInverterState(CarState.RightInvState) >= 0 )
		{
			returnvalue &= ~(0x1 << InverterReceived);
		} else returnvalue |= 0x1 << InverterReceived;

	}

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

	char RequestState = PreOperationalState; // initialise to PreOperationalState as default requested next state.

    if ( OperationLoops == 0 )
	    {
	    	preoperationstate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state.
	    	CarState.HighVoltageReady = 0;
	    	minmaxADCReset();
	 //   	NMTReset(); //send NMT reset when first enter state to catch any missed boot messages, see if needed or not.
	    	// send to individual devices rather than reset everything if needed.
	    }
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, PreOperationalState, preoperationstate );
	}

//	ResetCanReceived();

	sendPDM( 0 );

	uint32_t loopstart = gettimer();
	uint32_t looptimer = 0;

	// check if received configuration requests, or mode change -> testing state.
	switch ( CheckConfigurationRequest() ) // allows RequestState to be set to zero to prevent mode changing mid config, or request a different mode.
	{
/*		case TestingState :
			RequestState = TestingState; // Testing state requested from Configuration.
			break;

		case LimpState :
			RequestState = LimpState; // Limpmode state requested from Configuration, set as requested next state.
			break; */

		case ReceivingConfig :
			RequestState = PreOperationalState; // don't allow leaving pre operation whilst
												// in middle of processing a configuration / testing request.
		case 0:
		default:
			RequestState = OperationalReadyState; // nothing happening in config, assume normal operation.
			// do nothing
	}

	// checks if we have heard from other necessary connected devices for operation.

	while (  looptimer < PROCESSLOOPTIME-50 ) {	looptimer = gettimer() - loopstart;}; // check

	preoperationstate  = DevicesOnline(preoperationstate);

	uint16_t error = CheckErrors();
	// check error state.
	if ( error )
	{
//		return OperationalErrorState; // if a device is in error state, quit to error handler to decide how to proceed.
	}

	CarState.Torque_Req_Max = ADCState.DrivingMode;
	CarState.Torque_Req_CurrentMax = ADCState.DrivingMode;
	CarState.Torque_Req_L = PedalTorqueRequest();  // allow APPS checking before startup
	CarState.Torque_Req_R = CarState.Torque_Req_L;
	if ( CarState.APPSstatus ) setOutput(RTDMLED_Output,LEDON); else setOutput(RTDMLED_Output,LEDOFF);

	receiveINVStatus(LeftInverter);
	receiveINVStatus(RightInverter);

	receiveINVSpeed(LeftInverter);
	receiveINVSpeed(RightInverter);

	receiveINVTorque(LeftInverter);
	receiveINVTorque(RightInverter);

	if ( !errorPDM() && preoperationstate == 0 && CarState.LeftInvState != 0xFF && CarState.RightInvState != 0xFF) // everything is ready to move to next state.
	{
		blinkOutput(TSLED_Output, LEDBLINK_ONE, 1);
			// devices are ready and in pre operation state.
			// check for request to move to active state.

			if ( ( RequestState == TestingState ) )
			{
				OperationLoops = 0;
				return TestingState; // an alternate mode ( testing requested in config for next state.
			}

			if ( CheckActivationRequest() ) // check if driver has requested activation and if so proceed
			{
				OperationLoops = 0;
				CarState.Torque_Req_L = 0;
				CarState.Torque_Req_R = 0;
				setOutput(RTDMLED_Output,LEDOFF);
				return OperationalReadyState; // normal operational state on request
			}

	} else
	{ // hardware not ready for active state
		if ( OperationLoops == 50 ) // 0.5 seconds, send reset nmt, try to get inverters online if not online at startup.
		{
		NMTReset(); // chec±k
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
				// return LimpState; // either testing or limp mode requested requested.

				return OperationalErrorState; // quit right away to error handler state if no possible special state requested on timeout
			}
		}
	}

	return PreOperationalState; // nothing caused entry to a different state, continue in current state.
}



