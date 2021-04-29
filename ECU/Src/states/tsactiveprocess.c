/*
 * operationreadyness.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#include "main.h"
#include <stdlib.h>

/* Private includes ----------------------------------------------------------*/

#include "ecumain.h"

int TSActiveRequest( void )
{
//	ResetCanReceived(); // reset can data before operation to ensure we aren't checking old data from previous cycle.
	CAN_NMTSyncRequest();

	setHV( true, false ); // will enable HV if inverters in ready status and HV enabled flag set.

#ifdef POWERNODES
	setDevicePower(IVT, 1);
#endif

	invRequestState(PREOPERATIONAL);

	return 0;
}

int TSActiveProcess( uint32_t OperationLoops )
{
	static uint16_t readystate;

	static uint32_t prechargetimer = 0;

	if ( OperationLoops == 0) // reset state on entering/rentering.
	{
		 	 	 	 	 	 //12345678901234567890
		lcd_clear();
		lcd_settitle("TS Active");
		prechargetimer = gettimer();
		CarState.HighVoltageReady = true; // only enable if reading high enough voltage.
		CarState.AllowTorque = false;
		setOutput(TSLED,On);
		blinkOutput(TSLED,On,0);
		setOutput(RTDMLED,Off);
		blinkOutput(RTDMLED,Off,0);
//		setOutput(TSOFFLED,LEDOFF);
	}

	readystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, TSActiveState, readystate );
	}

	TSActiveRequest();

	uint8_t ReceiveNonCriticalError = 0;

	uint8_t OperationalError = OperationalReceiveLoop();

	switch ( OperationalError )
	{
		case 0 : break;
		case 1 : ReceiveNonCriticalError = 1; break;
		case OperationalErrorState :
			Errors.ErrorPlace = 0xDA;
			Errors.ErrorReason = OperationalError;
			return OperationalErrorState;
	}

	uint32_t curtime = gettimer();

	uint8_t prechargedone = 0;
	if ( prechargetimer+MS1000*6 < curtime )
	{
		prechargedone = 1;
		lcd_send_stringline(1,"Precharge Done.", 255);
	}

	if ( invertersStateCheck(PREOPERATIONAL)
		&& !ReceiveNonCriticalError && prechargedone ) // ensure can't enter RTDM till given time for precharge to take place.
	{
	  readystate = 0;
	  blinkOutput(RTDMLED,LEDBLINK_THREE,LEDBLINKNONSTOP); // start blinking RTDM to indicate ready to enable.
	} else
	{
	  setOutput(RTDMLED,Off);
	}

	uint16_t errors = CheckErrors();

	if ( errors )
	{
		Errors.ErrorPlace = 0xDB;
		return OperationalErrorState; // something has triggered an error, drop to error state to deal with it.
	}

	// allow APPS checking before RTDM
	int Torque_Req = PedalTorqueRequest();

	for ( int i=0;i<MOTORCOUNT;i++)  // set all wheels to same torque request
	{
		CarState.Inverters[i].Torque_Req = Torque_Req;
	} //

#ifdef SIMPLETORQUEVECTOR
	TorqueVectorProcess( Torque_Req );
#endif

/*	uint16_t sanity = CheckADCSanity();
	if ( sanity )
	{
		CAN_SendStatus(5, TSActiveState, sanity);
		Errors.Errorreason = 0xEC;
		return OperationalErrorState;
	} */

	/* EV 4.11.6
	 * After the TS has been activated, additional actions must be required by the driver
	 * to set the vehicle to ready-to-drive mode (e.g. pressing a dedicated start button).
	 *
	 * One of these actions must include the actuation of the mechanical brakes while ready-to-drive mode is entered.
	 */

	if ( readystate == 0
		&& CheckRTDMActivationRequest()
	  	&& ( ADCState.BrakeR >= RTDMBRAKEPRESSURE || ADCState.BrakeF >= RTDMBRAKEPRESSURE ) ) // if inverters ready, rtdm pressed, and brake held down.
	{
	    return RunningState;
	}

	if ( CheckActivationRequest() )
	{
		if ( !prechargedone )
		{
			lcd_send_stringline(1,"Wait for Precharge", 255);
		} else return IdleState;  // if requested disable TS drop state
	}

	return TSActiveState;
}
