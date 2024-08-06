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
#include "configuration.h"
#include "errors.h"
#include "power.h"
#include "node_device.h"
#include "bms.h"
#include "output.h"
#include "inverter.h"
#include "powernode.h"
#include "timerecu.h"

#include "ivt.h"
#include "eeprom.h"
#include "canecu.h"


static volatile bool testmotors = false;

void setTestMotors( bool state) {
	testmotors = state;
}

#define READYCONFIGBIT		0
#define READYDEVBIT			2
#define READYINVBIT			3
#define READYSENSBIT		4
#define READYPOWERBIT		5
#define READYTSALBIT		6
#define READYTESTING		7

uint8_t buz_timer = 0;
// get external hardware upto state to allow entering operational state on request.
int PreOperationState(uint32_t OperationLoops) {

	if (OperationLoops == 0) {
		
		CAN_SendDebug(EPOS_ID);

		initVectoring();

		setNodeDevicePower(Inverters, true, 0);

	}
		
	PedalTorqueRequest(NULL);

	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); //Set first LED on
	if(BTN1.data == 1)
	{
		Set_LV_Devices_On();
		//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); //Set first LED off
		return OperationalReadyState;
	}


	return PreOperationalState; // nothing caused entry to a different state, continue in current state.
}

