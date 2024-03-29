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
	static uint32_t nextprechargemsg = 0;

	char str[80] = "";

	if ( OperationLoops == 0) // reset state on entering/rentering.
	{
		CheckHVLost();
		DebugMsg("Entering TS Active State");
		ShutdownCircuitSet(true);
		CarState.allowtsactivation = false;
		 	 	 	 	 	 //12345678901234567890
		lcd_clear();
		//lcd_settitle("TS Active");
		prechargetimer = gettimer();
		nextprechargemsg = 0;
		InverterAllowTorqueAll(false);

	    CarState.AllowRegen = false;

	    resetOutput(TSLED, Off);
	    resetOutput(RTDMLED, Off);
	}

	readystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, TSActiveState, readystate );
	}

	if ( CarState.VoltageINV < TSACTIVEV )
		PrintRunning("TS:LoV");
	else
		PrintRunning("TS:On");

	uint8_t ReceiveNonCriticalError = 0;

	uint32_t received = OperationalReceive();

	uint32_t curtime = gettimer();

	uint8_t prechargedone = 0;
	if ( prechargetimer+MS1000*6 > curtime ) // !Shutdown.PRE &&
	{
		blinkOutput(RTDMLED, LEDBLINK_TWO, 100);
		prechargedone = 0;
		if ( prechargetimer+MS1000*6 > curtime )
		{
			lcd_send_stringline(1,"Precharge Wait.", 255);
		}
		// check SDC now, should be powered.
#if 0
		strcat(str, "SDC(" );

			strcat(str, ShutDownOpenStr());

		strcat(str, ") " );

		lcd_send_stringline(2, str, 255);
#endif
	} else
	{
		if (  CarState.VoltageINV > TSACTIVEV && ( prechargetimer+MS1000*6 <= curtime ) ) // Shutdown.PRE ||
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

	if ( !prechargedone )
	{
		if ( nextprechargemsg < curtime )
		{
			nextprechargemsg = curtime + 200;
			DebugPrintf("TS at %lu invV:%d", curtime, CarState.VoltageINV);
		}
	}

	if ( prechargedone && CarState.VoltageINV <= TSACTIVEV)
	{
		lcd_send_stringline(1,"TS Lost.", 255);
		DebugPrintf("TS failure at %lu invV:%d Shutdown.Pre:%d, prechargetimer:%d", gettimer(), CarState.VoltageINV, Shutdown.PRE, prechargetimer);
		prechargedone = 0;
		return IdleState;
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
		Errors.ErrorReason = ReceivedCriticalError | ( CheckCriticalError() << 8 );
		return OperationalErrorState; // something has triggered an error, drop to error state to deal with it.
	}

	// allow APPS checking before RTDM

	int16_t pedalreq;
	CarState.Torque_Req = PedalTorqueRequest(&pedalreq); // no actual request, merely being calculated.

	vectoradjust adj;
	speedadjust spd;

	doVectoring( CarState.Torque_Req, &adj, &spd, pedalreq );

	InverterSetTorque(&adj, 0);

	/* EV 4.11.6
	 * After the TS has been activated, additional actions must be required by the driver
	 * to set the vehicle to ready-to-drive mode (e.g. pressing a dedicated start button).
	 *
	 * One of these actions must include the actuation of the mechanical brakes while ready-to-drive mode is entered.
	 */


	if ( readystate == 0
		&& CheckRTDMActivationRequest() ) // if inverters ready, rtdm pressed, and brake held down.
	{
		if ( getBrakeRTDM() )
			return RunningState;
		else
			DebugPrintf("RTDM activation attempt with no braking");
	}

	if ( CheckActivationRequest() )
	{
		if ( !prechargedone )
		{
			lcd_send_stringline(1,"Wait for Precharge", 254);
			DebugPrintf("Wait for Precharge\n");
		} else
		{
			DebugPrintf("Returning to idle state at request.");
			return IdleState;  // if requested disable TS drop state
		}
	}


	return TSActiveState;
}
