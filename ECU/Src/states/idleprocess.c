/*
 * operationreadyness.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */


#include "main.h"
#include "ecumain.h"

char IdleRequest( void )   // request data / invalidate existing data to ensure we have fresh data from this cycle.
{
//	uint16_t command;
	// Inverter Pre Operation preparedness.

	// reset can data before operation to ensure we aren't checking old data from previous cycle.

//	ResetCanReceived(); // reset can data before operation to ensure we aren't checking old data from previous cycle.
	CAN_NMTSyncRequest();
	sendPDM( 0 );

	// request ready states from devices.

	CarState.LeftInvCommand = InverterStateMachine( LeftInverter );

	if ( GetInverterState( CarState.LeftInvState ) !=  INVERTERREADY ) // should be in ready state, if not, run state machine.
	{
	     // request left inv state machine to On from ready.
		CANSendInverter( CarState.LeftInvCommand, 0, LeftInverter );
	} // else command = set command to current state request.

	CarState.RightInvCommand = InverterStateMachine( RightInverter );

	if ( GetInverterState( CarState.RightInvState ) != INVERTERREADY )
	{
		CANSendInverter( CarState.RightInvCommand, 0, RightInverter );
	}



	// if ( DeviceState.FLeftSpeed == OPERATIONAL ) should eventually respond by sync.

	// send FLeftSpeed, currently no sync request, just sending data.
	// send BMS - currently no sync request, just sending data

	// send PDM - currently no sync request, just sending data.
	return 0;
}

char OperationalReceive( uint16_t returnvalue )
{
	if (returnvalue == 0xFF)
	{ returnvalue = (0x1 << FLeftSpeedReceived) + // initialise return value to all devices in error state ( bit set )
				  (0x1 << FRightSpeedReceived) +
				  (0x1 << InverterLReceived)+
				  (0x1 << InverterRReceived)+
				  (0x1 << BMSReceived)+
				  (0x1 << PDMReceived)+
				  (0x1 << PedalADCReceived);
	}

	// change order, get status from pdo3, and then compare against pdo2?, 2 should be more current being higher priority

//	if ( OperationalState == RunningState )
	{
		receiveINVStatus(LeftInverter);
		receiveINVStatus(RightInverter);

		if ( receiveINVSpeed(LeftInverter) )
		{
			returnvalue &= ~(0x1 << InverterLReceived); // speed should always be seen every cycle in RTDM
		}

		if ( receiveINVSpeed(RightInverter) )
		{
			returnvalue &= ~(0x1 << InverterRReceived);
		}

		receiveINVTorque(LeftInverter);
		receiveINVTorque(RightInverter);

	}

	if (DeviceState.FrontSpeedSensors == ENABLED )
	{
		if ( receiveSick(FLSpeed_COBID) ) returnvalue &= ~(0x1 << FLeftSpeedReceived);
		if ( receiveSick(FRSpeed_COBID) ) returnvalue &= ~(0x1 << FRightSpeedReceived);
	} else
	{
		returnvalue &= ~(0x1 << FLeftSpeedReceived);
		returnvalue &= ~(0x1 << FRightSpeedReceived);
	}

	if ( receivePDM() ) returnvalue &= ~(0x1 << PDMReceived);

	if ( receiveBMS() )
		returnvalue &= ~(0x1 << BMSReceived);

	 receiveIVT();
	// if ( receiveIVT() )
		returnvalue &= ~(0x1 << IVTReceived); // assume IVT is present, don't go to error state if missing.

		// need new function to check for ADC input, so that more workable with a CAN node.
    if ( CheckADCSanity() == 0 ) returnvalue &= ~(0x1 << PedalADCReceived); // change this to just indicate ADC received in some form.

    return returnvalue;
}

char OperationalReceiveLoop( void )
{
	LastCarState = CarState;
	uint32_t loopstart = gettimer();
	uint32_t looptimer = 0;
	uint8_t received = 0xFF;

	static int errorcount = 0; // keep count of how many consecutive errors had.
	static uint32_t errorfree = 0;

	// loop for upto 5ms before end process loop time waiting for data or all data received.
	// allows time to process incoming data, should be ~5ms.
	do
	{
		// check for resanityceived data and set states
        // check for incoming data, break when all received.
		looptimer = gettimer() - loopstart;
	} while ( /* received != 0 && */ looptimer < PROCESSLOOPTIME-50 ); // check

	received = OperationalReceive( received );


	// check what not received here, only error for inverters

	if ( received != 0 ) //  PROCESSLOOPTIME-50 ) // spent too long waiting for data.
	{
		CAN_SendStatus(1, OperationalState, received);
		Errors.ErrorPlace = 0x9A;
		Errors.OperationalReceiveError = received;
		Errors.State = OperationalState;
		return OperationalErrorState;
	}

	switch ( CheckErrors() )
	{
		case 0 :
			errorfree++;
			if ( errorfree >= ReduceErrorCountRate )
			{
				if ( errorcount > 0 ) errorcount--;
			}
			return 0;

/*		case 1 : // non critical error.
			errorcount++;
			errorfree = 0;
			if ( errorcount > MaxRunningErrorCount )
			{
				ErrorCarState = CarState;
				return 99;
			}
//			CarState = LastCarState; // restore car state to last known good state.
			return 1; */
			break;

		case 99 : // critical error.
		default :
			Errors.ErrorPlace = 0x9B;
			return OperationalErrorState; // something has triggered an unacceptable error ( inverter error state etc ), drop to error state to deal with it.
	}
}

int IdleProcess( uint32_t OperationLoops ) // idle, inverters on.
{
	static uint16_t readystate;
	static uint8_t TSRequested;

	static uint32_t HVEnableTimer;

	if ( OperationLoops == 0) // reset state on entering/rentering.
	{
		CarState.HighVoltageReady = 0;
		HVEnableTimer = 0;
		TSRequested = 0;
		setOutput(RTDMLED_Output, LEDOFF);
		blinkOutput(RTDMLED_Output, LEDOFF, LEDOFF);
	}
	readystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, IdleState, readystate );
	}

	IdleRequest(); // process requests

	//readystate = OperationalReceiveLoop();

	uint8_t ReceiveNonCriticalError = 0;

	uint8_t OperationalError = OperationalReceiveLoop();

	switch ( OperationalError )
	{
		case 0 : break;
		case 1 : ReceiveNonCriticalError = 1; break;
		case OperationalErrorState :
			Errors.ErrorPlace = 0xCA;
			Errors.ErrorReason = OperationalError;
			return OperationalErrorState;
	}

	// check for error messages -> drop state

	// at this state, everything is ready to be powered up.

	if ( // !CheckErrors() && // this is done in receive loop already.
			GetInverterState( CarState.LeftInvState ) == INVERTERREADY
	  && GetInverterState( CarState.RightInvState ) == INVERTERREADY
		&& !ReceiveNonCriticalError
	  && CarState.VoltageBMS > MINHV
	  && CarState.VoltageINV > 18
	  ) // minimum accumulator voltage to allow TS, set a little above BMS limit, so we can
	{
		readystate = 0;
		 if ( !TSRequested )
			 blinkOutput(TSLED_Output,LEDBLINK_FOUR,LEDBLINKNONSTOP); // start blinking to indicate ready.
	} /* else
	{
	  setOutput(TSLED_Output,LEDOFF);
	} */

	// fail process, inverters go from 33->60->68  when no HV supplied and request startup.

/*	if ( GetInverterState( CarState.LeftInvState ) == INVERTERERROR
		  || GetInverterState( CarState.RightInvState ) == INVERTERERROR )
	{
		return OperationalErrorState;
	} */ // checked earlier in checkerrors.

	uint8_t InvHVPresent = 0;

	if ( DeviceState.IVTEnabled )
	{
		if ( CarState.VoltageINV > 60 ) InvHVPresent = 1;
	} else InvHVPresent = 1; // just assume HV present after request if IVT not enabled.

	if ( readystate == 0 && TSRequested && CarState.VoltageBMS > MINHV && InvHVPresent )
	{
	//    setOutput(TSOFFLED_Output,LEDOFF);
		return TSActiveState;
	}

	if ( TSRequested == 1
		&& HVEnableTimer+9000 < gettimer()
		&& !InvHVPresent // make this optional so IVT can be disabled.
		)
	{
        // error enabling high voltage, stop trying and alert.
		CarState.HighVoltageReady = 0;
//		setOutput(TSOFFLED_Output,LEDON);
//		blinkOutput(TSLED_Output,LEDBLINK_FOUR,LEDBLINKNONSTOP);
		TSRequested = 0;
	}

	if ( readystate == 0 && CheckTSActivationRequest() )
	{
		TSRequested = 1;
		HVEnableTimer = gettimer();
		CarState.HighVoltageReady = 1; // start timer, go to error state after 1am
//		setOutput(TSOFFLED_Output,LEDOFF);
//		setOutput(TSLED_Output,LEDOFF);
//		blinkOutput(TSLED_Output,LEDOFF,0);
//		blinkOutput(RTDMLED_Output,LEDBLINK_THREE,LEDBLINKNONSTOP);

/*		CarState.HighVoltageReady = 1;

		CarState.HighVoltageAllowedL = 1;
		CarState.HighVoltageAllowedR = 1;

		while ( 1 )
		{
			HAL_Delay(10);
			sendPDM( 0 );
		}  */
		// enable HV, then if successfull, enter TS, otherwise flash error.
	//	return TSActiveState;
	}

	return IdleState;
}
