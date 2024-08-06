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
#include "output.h"
#include "inverter.h"

#include "node_device.h"
#include "imu.h"
#include "eeprom.h"


int RunningProcess(uint32_t OperationLoops, uint32_t targettime) {
	// EV4.11.3 RTDM Check
	// Closing the shutdown circuit by any part defined in EV 6.1.2 must not (re-)activate the TS.
	// Additional action must be required.

	if (OperationLoops == 0) // reset state on entering/rentering.
	{
		
	
		CAN_SendDebug(ERDTM_ID); // Entering Running State
		/* EV 4.12.1
		 * The vehicle must make a characteristic sound, continuously for at least one second and a maximum of three seconds when it enters ready-to-drive mode.
		 */
		// send buzzer as entering RTDM
		soundBuzzer();
		
		InverterAllowTorqueAll(true);		

	}

	if (OperationLoops < 100){
		setNodeDevicePower(Buzzer, 1, 0);
	}
	else{
		setNodeDevicePower(Buzzer, 0, 0);
	}
	
	
	



	// if drop status due to error, check if recoverable, or if limp mode possible.

	return RunningState;
}
