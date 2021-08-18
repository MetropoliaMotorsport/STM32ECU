/*
 * operationreadyness.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#include "ecumain.h"
#include "tsactiveprocess.h"
#include "runningprocess.h"
#include "idleprocess.h"
#include "adcecu.h"
#include "inverter.h"
#include "input.h"
#include "output.h"
#include "lcd.h"
#include "timerecu.h"
#include "power.h"
#include "errors.h"
#include "debug.h"
#include "brake.h"

/* Private includes ----------------------------------------------------------*/

#include "ecumain.h"

int TSActiveProcess( uint32_t OperationLoops )
{
	static uint16_t readystate;

	static uint32_t prechargetimer = 0;

	if ( OperationLoops == 0) // reset state on entering/rentering.
	{
		DebugMsg("Entering TS Active State");
		ShutdownCircuitSet(true);
		 	 	 	 	 	 //12345678901234567890
		lcd_clear();
		//lcd_settitle("TS Active");
		prechargetimer = gettimer();
		InverterAllowTorqueAll(false);

		setOutput(TSLED, Off);
		setOutput(RTDMLED, Off);
	}

	readystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, TSActiveState, readystate );
	}

	if ( CarState.VoltageINV < 400 )
		PrintRunning("TS:LoV");
	else
		PrintRunning("TS:On");

	uint8_t ReceiveNonCriticalError = 0;

	uint32_t received = OperationalReceive();

	uint32_t curtime = gettimer();

	uint8_t prechargedone = 0;
	if ( !Shutdown.PRE && prechargetimer+MS1000*6 > curtime )
	{
		blinkOutput(RTDMLED, LEDBLINK_TWO, LEDBLINKNONSTOP);
		prechargedone = 0;
		if ( prechargetimer+MS1000*6 > curtime )
		{
			lcd_send_stringline(1,"Precharge Wait.", 255);
		}
	} else
	{
		if (  CarState.VoltageINV > 400 && ( Shutdown.PRE || prechargetimer+MS1000*6 <= curtime ) )
		{
			setOutput(RTDMLED, On);
			lcd_send_stringline(1,"Precharge Done.", 255);
			prechargedone = 1;
#ifdef HPF19
			invRequestState(PREOPERATIONAL);
#else
			invRequestState(OPERATIONAL);
#endif

		} else
		{
			lcd_send_stringline(1,"TS Activate Fail.", 255);
			DebugPrintf("TS Activation failure at %lu invV:%d Shutdown.Pre:%d, prechargetimer:%d", gettimer(), CarState.VoltageINV, Shutdown.PRE, prechargetimer);
			prechargedone = 0;
			return IdleState;
		}
	}


#if 0
		&& HVEnableTimer+MS1000*9 < gettimer()
		&& // make this optional so IVT can be disabled.
		)
	{
        // error enabling high voltage, stop trying and alert.
		blinkOutput(RTDMLED,LEDBLINK_FOUR, 1);
		TSRequested = 0;

		// SHOW ERROR.

		DebugMsg("Timeout activating TS, check TSMS & HVD?");
		lcd_send_stringline(1,"Error Activating TS", 255);
		lcd_send_stringline(2,"Check TSMS & HVD.", 255);

		if ( CheckShutdown() )
		{
			// TODO stick shutdown circuit check string here.

		}
	}

	uint8_t InvHVPresent = 0;

	if ( DeviceState.IVTEnabled )
	{
		if ( CarState.VoltageINV > 60 )
			InvHVPresent = 1;
	} else InvHVPresent = 1; // just assume HV present after request if IVT not enabled. // TODO read from inverters.



#endif

	if ( // invertersStateCheck(PREOPERATIONAL)
		!ReceiveNonCriticalError && prechargedone ) // ensure can't enter RTDM till given time for precharge to take place.
	{
	  readystate = 0;
//	  blinkOutput(RTDMLED,LEDBLINK_THREE,LEDBLINKNONSTOP); // start blinking RTDM to indicate ready to enable.
	} else
	{
	  setOutput(RTDMLED,Off);
	}

	if ( CheckCriticalError() )
	{
		Errors.ErrorPlace = 0xDB;
		return OperationalErrorState; // something has triggered an error, drop to error state to deal with it.
	}

	// allow APPS checking before RTDM

	CarState.Torque_Req = PedalTorqueRequest(); // no actual request, merely being calculated.

	vectoradjust adj;

	doVectoring(CarState.Torque_Req , &adj);

	InverterSetTorque(&adj, 0);

	/* EV 4.11.6
	 * After the TS has been activated, additional actions must be required by the driver
	 * to set the vehicle to ready-to-drive mode (e.g. pressing a dedicated start button).
	 *
	 * One of these actions must include the actuation of the mechanical brakes while ready-to-drive mode is entered.
	 */

	if ( readystate == 0
		&& CheckRTDMActivationRequest()
	  	&& getBrakeHigh()) // if inverters ready, rtdm pressed, and brake held down.
	{
	    return RunningState;
	}

	if ( CheckActivationRequest() )
	{
		if ( !prechargedone )
		{
			lcd_send_stringline(1,"Wait for Precharge", 254);
		} else
		{
			DebugPrintf("Returning to idle state at request.");
			return IdleState;  // if requested disable TS drop state
		}
	}

	return TSActiveState;
}
