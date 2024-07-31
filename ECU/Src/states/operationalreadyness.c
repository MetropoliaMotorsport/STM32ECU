/*
 * operationreadyness.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#include "ecumain.h"
#include "operationalreadyness.h"
#include "inverter.h"
#include "output.h"
#include "power.h"
#include "errors.h"
#include "debug.h"
#include "powernode.h"

bool ReadyReceive() {
	
	bool States_Maching = true;

	for(int i = 0; i < DEVICE_COUNT; i++) {
		if (&DevicePowerList[i] != NULL){
			if (DevicePowerList[i].expectedstate != DevicePowerList[i].actualstate) {
				States_Maching = false;
				}
		}
	}

	return States_Maching;
}

// 1.	Testing the functionality/readings from the different sensors
// 2.	Checking that all expected CAN bus messages were received within specified time-interval
//      (Both Inverters, both encoders, both accelerator pedal sensors, brake pressure sensor/s, BMS
//      IVT-MOD, steering angle sensor, acceleration/yaw sensor�.)
// 3.	Checking that the HV and LV batteries voltages and temperatures are within defined limits

int OperationReadyness(uint32_t OperationLoops) // process function for operation readyness state
{

	static uint16_t received;

	if (OperationLoops == 0) // reset state on entering/rentering.
			{
		DebugMsg("Entering Readyness check State");
		CAN_SendDebug(ERCS_ID);
		//SetErrorLogging(true);
		received = 0xFFFF;

	}



	CAN_SendStatus(1, OperationalReadyState, received);

	if (OperationLoops > 50) // 500 )	// how many loops allow to get all data on time?, failure timeout.
			{
		DebugMsg("Errorplace 0xBA Too many loops.");
		CAN_SendDebug(ERRTL_ID);
		Errors.ErrorPlace = 0xBA;
		return OperationalErrorState; // error, too long waiting for data. Go to error state to inform and allow restart of process.
	}

	if(ReadyReceive){
		
		return IdleState;
	}


	// this state is just to allow devices to get ready, don't need to check any data.

	

	return OperationalReadyState;
}
