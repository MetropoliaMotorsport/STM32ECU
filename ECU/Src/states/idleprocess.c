/*
 * operationreadyness.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */


#include "main.h"
#include "ecumain.h"

bool CheckHV = false;

char IdleRequest( void )   // request data / invalidate existing data to ensure we have fresh data from this cycle.
{
	// Inverter Pre Operation preparedness.

	// reset can data before operation to ensure we aren't checking old data from previous cycle.

//	ResetCanReceived(); // reset can data before operation to ensure we aren't checking old data from previous cycle.
	CAN_NMTSyncRequest();

	setRunningPower( CheckHV, false );

	// request ready states from devices.

	invRequestState( PREOPERATIONAL ); // request to go into ready for HV

	return 0;
}

char OperationalReceive( uint16_t returnvalue )
{
	if (returnvalue == 0xFF)
	{ returnvalue =
#ifdef HPF19
			(0x1 << FLeftSpeedReceived) + // initialise return value to all devices in error state ( bit set )
				  (0x1 << FRightSpeedReceived) +
#endif
				  (0x1 << BMSReceived)+
#ifndef POWERNODES
				  (0x1 << PDMReceived)+
#endif
				  (0x1 << InverterReceived)+ // TODO inverter receive
				  (0x1 << PedalADCReceived);
#ifdef HPF20

//				  (0x1 << InverterLReceived)+
//				  (0x1 << InverterRReceived);
#endif
	}

	// change order, get status from pdo3, and then compare against pdo2?, 2 should be more current being higher priority

	if ( DeviceState.Inverter > OFFLINE )
		returnvalue &= ~(0x1 << (InverterReceived));

#ifdef HPF19
	if (DeviceState.FrontSpeedSensors == ENABLED )
	{
		if ( receiveSick(FLSpeed_COBID) ) returnvalue &= ~(0x1 << FLeftSpeedReceived);
		if ( receiveSick(FRSpeed_COBID) ) returnvalue &= ~(0x1 << FRightSpeedReceived);
	} else
	{
		returnvalue &= ~(0x1 << FLeftSpeedReceived);
		returnvalue &= ~(0x1 << FRightSpeedReceived);
	}
#endif

#ifndef POWERNODES
	if ( receivePDM() ) returnvalue &= ~(0x1 << PDMReceived);
#endif
	if ( receiveBMS() )
		returnvalue &= ~(0x1 << BMSReceived);

	 receiveIVT();
	// if ( receiveIVT() )
		returnvalue &= ~(0x1 << IVTReceived); // assume IVT is present, don't go to error state if missing.

		// need new function to check for ADC input, so that more workable with a CAN node.
    if ( DeviceState.ADCSanity == 0 ) returnvalue &= ~(0x1 << PedalADCReceived); // change this to just indicate ADC received in some form.

    return returnvalue;
}

char OperationalReceiveLoop( void )
{
//	LastCarState = CarState;
//	uint32_t loopstart = gettimer();
//	uint32_t looptimer = 0;
	uint8_t received = 0xFF;

	static int errorcount = 0; // keep count of how many consecutive errors had.
	static uint32_t errorfree = 0;

	// loop for upto 5ms before end process loop time waiting for data or all data received.
	// allows time to process incoming data, should be ~5ms.

	vTaskDelay(5);
//	do
//	{
		// check for resanityceived data and set states
        // check for incoming data, break when all received.
//		looptimer = gettimer() - loopstart;
//		__WFI(); // sleep till interrupt, avoid loading cpu doing nothing.
//	} while ( /* received != 0 && */ looptimer < PROCESSLOOPTIME-50 ); // check

	received = OperationalReceive( received );

	// check what not received here, only error for inverters

	if ( received != 0 ) // not all expected data received in window.
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
							 //12345678901234567890
		lcd_clear();
		//lcd_settitle("Ready to activate TS");
		CarState.HighVoltageReady = false;
		InverterAllowTorque(false);
		CheckHV = false;
		HVEnableTimer = 0;
		TSRequested = 0;
		setOutput(RTDMLED, Off);
		blinkOutput(RTDMLED, Off, 0);
	}
	readystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, IdleState, readystate );
	}

	PrintRunning("Ready");

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

	if ( invertersStateCheck(STOPPED) // returns true if all inverters match state
	  && !ReceiveNonCriticalError
	  && CarState.VoltageBMS > MINHV
#ifdef IVTEnable
	#ifndef NOTSAL
	  && CarState.VoltageINV > -5 // 18
	#endif
#endif
#ifdef SHUTDOWNSWITCHCHECK
	  && CheckShutdown() // only allow TS enabling if shutdown switches are all closed.
#endif
	  ) // minimum accumulator voltage to allow TS, set a little above BMS limit, so we can
	{
		readystate = 0;
		 if ( !TSRequested )
			 blinkOutput(TSLED,LEDBLINK_FOUR,LEDBLINKNONSTOP); // start blinking to indicate ready.
	}

#ifdef SETDRIVEMODEINIDLE
	setCurConfig();
#endif


// allow APPS checking before RTDM
	vectoradjust adj;

	if ( CarState.TorqueVectoring )
		doVectoring( CarState.Torque_Req, &adj );

	if ( CarState.APPSstatus ) setOutput(TSLED,On); else setOutput(TSLED,Off);

	InverterSetTorque(&adj, 0);

	uint8_t InvHVPresent = 0;

	if ( DeviceState.IVTEnabled )
	{
		if ( CarState.VoltageINV > 60 ) InvHVPresent = 1;
	} else InvHVPresent = 1; // just assume HV present after request if IVT not enabled.

	if ( readystate == 0 && TSRequested && CarState.VoltageBMS > MINHV && InvHVPresent )
	{
		return TSActiveState;
	}

	if ( TSRequested == 1
		&& HVEnableTimer+MS1000*9 < gettimer()
		&& !InvHVPresent // make this optional so IVT can be disabled.
		)
	{
        // error enabling high voltage, stop trying and alert.
		CheckHV = false;
//		CarState.HighVoltageReady = 0;
//		blinkOutput(TSLED_Output,LEDBLINK_FOUR,1);
		TSRequested = 0;

		// SHOW ERROR.

		if ( CheckShutdown() )
		{
			lcd_send_stringline(1,"Error Activating TS", 255);
			lcd_send_stringline(2,"Check TSMS & HVD.", 255);
		}
	}

	if ( readystate == 0 && CheckTSActivationRequest() )
	{
		TSRequested = 1;
		HVEnableTimer = gettimer();
		CheckHV=true;
//		CarState.HighVoltageReady = 1; // start timer, go to error state after 1am
	} else
	{

	}

	return IdleState;
}
