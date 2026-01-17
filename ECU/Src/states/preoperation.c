/**
 ******************************************************************************
 * @file           : operation.c
 * @brief          : Operational Loop and related function
 ******************************************************************************

 ******************************************************************************
 */

#include "ecumain.h"
#include "runningprocess.h"
#include "preoperation.h"

#include "errors.h"
#include "power.h"
#include "node_device.h"
#include "bms.h"
#include "input.h"
#include "output.h"
#include "inverter.h"
#include "powernode.h"
#include "timerecu.h"

#include "ivt.h"
#include "eeprom.h"
#include "canecu.h"

//#define PRINTDEBUGRUNNING

static uint16_t DevicesOnline(uint16_t returnvalue) {

	//TODO update function

	returnvalue = 0;
	return returnvalue; // should be 0 when everything ready.
}

static volatile bool testmotors = false;

void setTestMotors( bool state) {
	testmotors = state;
}

#define READYCONFIGBIT		0
//#define READYSDCBIT     	1
#define READYDEVBIT			2
#define READYINVBIT			3
#define READYSENSBIT		4
#define READYPOWERBIT		5
#define READYTSALBIT		6
#define READYTESTING		7

// get external hardware upto state to allow entering operational state on request.
int PreOperationState(uint32_t OperationLoops) {
//	static int OperationLoops = 0;

	if (OperationLoops == 0) {

		CAN_SendDebug(EPOS_ID);

		initVectoring();

		setNodeDevicePower(Inverters, true, 0);
		vTaskDelay(5);

		setNodeDevicePower(RightPump, true, 0);
		setNodeDevicePower(LeftPump, true, 0);
	
	}
	setNodeDevicePower(Inverters, true, 0);
	vTaskDelay(2);
	setNodeDevicePower(LeftPump, true, 0);
	
	PedalTorqueRequest(NULL);

	return IdleState;

	return PreOperationalState; // nothing caused entry to a different state, continue in current state.
}

