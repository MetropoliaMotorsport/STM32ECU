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
		DebugMsg("Entering TS Active State");
		CAN_SendDebug(ETSAS_ID);

		ShutdownCircuitSet(true);
		CarState.allowtsactivation = false;

	}

	PedalTorqueRequest(NULL);

	if(BTN2.data && BPPS.data > 10){
		return RunningState;
	}

	if(!CarState.HV_on){
		ShutdownCircuitSet(false);
		return TestingState;
	}

	return TSActiveState;
}
