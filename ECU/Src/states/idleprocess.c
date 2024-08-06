/*
 * operationreadyness.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#include "ecumain.h"
#include "idleprocess.h"
#include "runningprocess.h"
#include "operationalprocess.h"
#include "configuration.h"
#include "errors.h"
#include "inverter.h"
#include "timerecu.h"
#include "output.h"
#include "power.h"

#include "node_device.h"
#include "canecu.h"

uint32_t OperationalReceive(void) {
uint32_t returnvalue = 0;

if( DeviceState.Inverter == OPERATIONAL )
	returnvalue |= (0x1 << InverterReceived);

	int invcount = 0;
	for (int i = 0; i < MOTORCOUNT; i++) {
		if (getInvState(i)->Device == OFFLINE) {
			invcount++;
		}
	}
	return returnvalue;
}

int IdleProcess(uint32_t OperationLoops) // idle, inverters on.
{
	static uint16_t readystate;
	static uint8_t TSRequested;

	static uint32_t HVEnableTimer;

	static uint32_t nextmsg;

	// request ready states from devices.

	if (OperationLoops == 0) // reset state on entering/rentering.
			{
		readystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.

		CAN_SendDebug(EIS_ID); // Entering Idle State

		ShutdownCircuitSet( false);

		InverterAllowTorqueAll(false);

		HVEnableTimer = gettimer();
		TSRequested = 0;
	}

	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
	if(BTN2.data == 1) {

		//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
		ShutdownCircuitSet( true);
		return TSActiveState;
	}

	return IdleState;
	}
