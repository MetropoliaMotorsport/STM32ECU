/**
 ******************************************************************************
 * @file           : operation.c
 * @brief          : Operational Loop and related function
 ******************************************************************************

 ******************************************************************************
 */

#include "ecumain.h"
#include "operationalprocess.h"
#include "idleprocess.h"
#include "runningprocess.h"
#include "tsactiveprocess.h"
#include "preoperation.h"
#include "operationalreadyness.h"
#include "errors.h"
#include "output.h"
#include "timerecu.h"
#include "power.h"
#include "inverter.h"


static int LastOperationalState = 0;
static int NewOperationalState = 0;
int OperationalState = StartupState;
uint32_t loopcount = 0;
uint32_t totalloopcount = 0;

//	The driver must be able to activate and deactivate the TS, see EV 4.10.2 and EV 4.10.3,
//	from within the cockpit without the assistance of any other person. - deactivation not handled atm?

//	Closing the shutdown circuit by any part defined in EV 6.1.2 must not (re-)activate the TS.
//	Additional action must be required.

// detect shutdown, move back to pre operation state

ResetCommand *ResetCommands[64] = { NULL };

uint32_t ResetCount = 0;

int RegisterResetCommand(ResetCommand *Handler) {
	if (Handler != NULL) {
		ResetCommands[ResetCount] = Handler;
		ResetCount++;
		return 0;
	} else
		return 1;
}

void ResetStateData(void) // set default startup values for global state values.
{
	for (int i = 0; i < ResetCount; i++) {
		if (ResetCommands[i] != NULL)
			(*ResetCommands[i])();
	}

	DeviceState.timeout = false;

	DeviceState.FrontSpeedSensors = DISABLED;
	DeviceState.FLSpeed = OPERATIONAL;
	DeviceState.FRSpeed = OPERATIONAL;

	CarState.PedalProfile = 0;
	CarState.DrivingMode = 0;
	CarState.AllowRegen = false;

	Errors.ErrorPlace = 0;
	Errors.ErrorReason = 0;
	Errors.CANSendError1 = 0;
	Errors.CANSendError2 = 0;
	Errors.ADCSent = false;
}

int Startup(uint32_t OperationLoops) {

	ShutdownCircuitSet( false);
	setNodeDevicePower(Inverters, false, 0); // turn off inverters
	CarState.MaxTorque = 60;
	CarState.PowerBalance = 60;
	CarState.AllowTorque = false;
	HAL_GPIO_WritePin(BMS_Output_Pin, BMS_Output_Port, SET);
		HAL_GPIO_WritePin(IMD_Output_Pin, IMD_Output_Port, SET);


	Set_LV_Devices_Off(); // turn off all low voltage devices

 
	return PreOperationalState;
}

int LimpProcess(uint32_t OperationLoops) {
	CAN_SendStatus(1, LimpState, 0);
	return LimpState;
}

int TestingProcess(uint32_t OperationLoops) {
	CAN_SendStatus(1, TestingState, 0);
	return TestingState;
}

int OperationalProcess(void) {
	// initialise loop timer variable.
	uint32_t looptimer = gettimer(); // was volatile, doesn't need to be

	static uint16_t loopoverrun = 0;

	if (NewOperationalState != OperationalState) // state has changed.
			{
		LastOperationalState = OperationalState;
		OperationalState = NewOperationalState;
		loopcount = 0;
		loopoverrun = 0; // reset over run counter.
	}


	uint32_t currenttimer = gettimer();

	// check how much past 10ms timer is, if too far, soft error. Allow a few times, but not too many before entering an error state?
	int lastlooplength = currenttimer - looptimer;

	looptimer = gettimer(); // start timing loop

	// check loop timing.
	if (lastlooplength > CYCLETIME * 1.1) {

		loopoverrun++; // bms

		if ((loopcount % 100) == 0) // allow one loop overrun every 100 by decrementing overrun counter if over 0
				{
			if (loopoverrun > 0) {
				loopoverrun--;
			}
		}

		if (loopoverrun > 10) {
			// if too many overruns,  do something?
		}
	}

	switch (OperationalState) {
	case StartupState: // NMT, initial startup.
		NewOperationalState = Startup(loopcount); // run NMT state machine till got responses.
		break;

	case PreOperationalState: // pre operation - configuration, wait for device presence announcements in pre operation state.
		NewOperationalState = PreOperationState(loopcount);
		break;

	case OperationalReadyState: // operation has been requested, get all devices to operational ready state and check sanity.
		NewOperationalState = OperationReadyness(loopcount);
		break;

	case IdleState: // idle, inverters on. Ready to enter TS, everything should be ready to go at this stage.
		NewOperationalState = IdleProcess(loopcount);
		break;

	case TSActiveState: // TS active state OperationalState, 0); // can return to state 3 ( stop button ) or go to 5
		NewOperationalState = TSActiveProcess(loopcount);
		break;

	case RunningState: // Running    // can return to state 3
		NewOperationalState = RunningProcess(loopcount, looptimer + CYCLETIME);
		break;

	case TestingState: // testing state, can only enter from state 1.
		NewOperationalState = TestingProcess(loopcount);
		break;

	case LimpState: // limping state, allow car to operate with limited input/motor power in slow mode.
		NewOperationalState = LimpProcess(loopcount);
		break;

	case OperationalErrorState: // critical error or unknown state.
		CAN_SendStatus(1, OperationalState, OperationalErrorState);
	default: // unknown state, assume it's an error and go into error
		NewOperationalState = OperationalErrorHandler(loopcount);
		break;
	}

#ifndef BENCH
	if (CheckCanError()) {
		NewOperationalState = OperationalErrorState;
	}
#endif

	loopcount++;
	totalloopcount++;

	return 0;
}
