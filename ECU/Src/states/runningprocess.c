/*
 * operationreadyness.c
 *
 *  Created on: 13 Apr 2019
 *      Author: Visa
 */

#include "ecumain.h"
#include "node_device.h"
#include "operationalprocess.h"
#include "idleprocess.h"
#include "errors.h"
#include "power.h"
#include "timerecu.h"
#include "brake.h"
#include "torquecontrol.h"
#include "input.h"
#include "output.h"
#include "inverter.h"
#include "debug.h"
#include "node_device.h"
#include "imu.h"
#include "eeprom.h"


uint8_t buz_timer = 0;
int RunningProcess(uint32_t OperationLoops, uint32_t targettime) {
	// EV4.11.3 RTDM Check
	// Closing the shutdown circuit by any part defined in EV 6.1.2 must not (re-)activate the TS.
	// Additional action must be required.

	static uint16_t readystate;
	static uint32_t standstill;
	static uint8_t allowstop;
	static uint32_t limpcounter;
	static uint32_t nextmsg = 0;

	if (OperationLoops == 0) // reset state on entering/rentering.
			{
		DebugMsg("Entering RTDM State");
		CAN_SendDebug(ERDTM_ID);
		/* EV 4.12.1
		 * The vehicle must make a characteristic sound, continuously for at least one second and a maximum of three seconds when it enters ready-to-drive mode.
		*/

		InverterAllowTorqueAll(true);
		CarState.InvRunning = true;
	}
	////////////////////////////
	if(CarState.PRE_Done && buz_timer < 58)
	{
		buz_timer++;
		setNodeDevicePower(Buzzer, (buz_timer < 56 ? true : false), 0);
	}
	////////////////////////

	PedalTorqueRequest(NULL);

	{
		CAN_SendStatus(1, RunningState, readystate);
	}

	uint32_t curtick = gettimer();

	CAN_SendDebug(RDTMTA_ID);

	if(!CarState.HV_on){
		ShutdownCircuitSet(false);
		return TestingState;
	}
	

	// if drop status due to error, check if recoverable, or if limp mode possible.

	return RunningState;
}
