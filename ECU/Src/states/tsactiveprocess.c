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
#include "node_device.h"
#include "inverter.h"
#include "input.h"
#include "output.h"
#include "timerecu.h"
#include "power.h"
#include "errors.h"
#include "debug.h"
#include "brake.h"

/* Private includes ----------------------------------------------------------*/

#include "ecumain.h"

int TSActiveProcess(uint32_t OperationLoops) {
	static uint16_t readystate;

	static uint32_t prechargetimer = 0;
	static uint32_t nextprechargemsg = 0;

	char str[80] = "";

	if (OperationLoops == 0) // reset state on entering/rentering.
			{
		CheckHVLost();
		DebugMsg("Entering TS Active State");
		CAN_SendDebug(ETSAS_ID);

		ShutdownCircuitSet(true);
		CarState.allowtsactivation = false;
		//12345678901234567890
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
		CAN_SendStatus(1, TSActiveState, readystate);
	}

	if (CarState.VoltageINV < TSACTIVEV){
		CAN_SendDebug(TSLOV);
	}
	else{
		CAN_SendDebug(TSON);
	}

	uint8_t ReceiveNonCriticalError = 0;

	uint32_t received = OperationalReceive();

	uint32_t curtime = gettimer();

	uint8_t prechargedone = 0;
	if (prechargetimer + MS1000 * 6 > curtime) { // !Shutdown.PRE &&
		blinkOutput(RTDMLED, LEDBLINK_TWO, 100);
		prechargedone = 0;
		// check SDC now, should be powered.
	} else {
		if (CarState.VoltageINV > TSACTIVEV
				&& (prechargetimer + MS1000 * 6 <= curtime)) // Shutdown.PRE ||
				{
			setOutput(RTDMLED, On);
			prechargedone = 1;
			invRequestState(OPERATIONAL);
		} else {
			DebugPrintf(
					"TS Activation failure at %lu invV:%d Shutdown.Pre:%d, prechargetimer:%d",
					gettimer(), CarState.VoltageINV, Shutdown.PRE,
					prechargetimer); // TODO make CAN message.
			prechargedone = 0;
			return IdleState;
		}
	}

	if (!prechargedone) {
		if (nextprechargemsg < curtime) {
			nextprechargemsg = curtime + 200;
			DebugPrintf("TS at %lu invV:%d", curtime, CarState.VoltageINV); // TODO make CAN message.
		}
	}

	if (prechargedone && CarState.VoltageINV <= TSACTIVEV) {
		DebugPrintf(	
				"TS failure at %lu invV:%d Shutdown.Pre:%d, prechargetimer:%d",	// TODO make CAN message.
				gettimer(), CarState.VoltageINV, Shutdown.PRE, prechargetimer);
		prechargedone = 0;
		return IdleState;
	}

	if ( // invertersStateCheck(PREOPERATIONAL)
	!ReceiveNonCriticalError && prechargedone) // ensure can't enter RTDM till given time for precharge to take place.
			{
		readystate = 0;
//	  blinkOutput(RTDMLED,LEDBLINK_THREE,LEDBLINKNONSTOP); // start blinking RTDM to indicate ready to enable.
	} else {
		setOutput(RTDMLED, Off);
	}

	if (CheckCriticalError()) {
		Errors.ErrorPlace = 0xDB;
		Errors.ErrorReason = ReceivedCriticalError
				| (CheckCriticalError() << 8);
		return OperationalErrorState; // something has triggered an error, drop to error state to deal with it.
	}

	// allow APPS checking before RTDM

	int16_t pedalreq;
	CarState.Torque_Req = PedalTorqueRequest(&pedalreq); // no actual request, merely being calculated.

	vectoradjust adj;
	speedadjust spd;

	doVectoring(CarState.Torque_Req, &adj, &spd, pedalreq);

	InverterSetTorque(&adj, 0);

	/* EV 4.11.6
	 * After the TS has been activated, additional actions must be required by the driver
	 * to set the vehicle to ready-to-drive mode (e.g. pressing a dedicated start button).
	 *
	 * One of these actions must include the actuation of the mechanical brakes while ready-to-drive mode is entered.
	 */

	if (readystate == 0) // if inverters ready, rtdm pressed, and brake held down.
			{

			return RunningState;

		}

	return TSActiveState;
}
