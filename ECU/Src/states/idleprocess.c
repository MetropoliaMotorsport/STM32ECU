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

#include "errors.h"
#include "input.h"
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

uint8_t sht_timer = 0;
bool sht_timer_on = false;
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
		//DebugMsg("Entering Idle State");
		CAN_SendDebug(EIS_ID);

		ShutdownCircuitSet( false);
		InverterAllowTorqueAll(false);
	}

	{
		CAN_SendStatus(1, IdleState, readystate);
	}

	PedalTorqueRequest(NULL);

	if(BTN3.data){
		ShutdownCircuitSet(true);
		sht_timer_on = true;
	}

	if(sht_timer_on){
		sht_timer++;
	}

	if(sht_timer > 50 && CarState.VoltageIVTAccu < 60000){
		ShutdownCircuitSet(false);
		sht_timer = 0;
		sht_timer_on = false;
		
	}

	if(CarState.PRE_Done){
		return TSActiveState;
	}



	return IdleState;
}
