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
#include "output.h"
#include "timerecu.h"
#include "power.h"
#include "errors.h"

#include "brake.h"

/* Private includes ----------------------------------------------------------*/

#include "ecumain.h"

int TSActiveProcess(uint32_t OperationLoops) {
	static uint8_t readystate;

	static uint32_t prechargetimer = 0;
	static uint32_t nextprechargemsg = 0;


	if (OperationLoops == 0) // reset state on entering/rentering.
			{

		CAN_SendDebug(ETSAS_ID); // Entering TS Active State

		ShutdownCircuitSet(true);
			
		prechargetimer = gettimer();
		nextprechargemsg = 0;
		InverterAllowTorqueAll(false);

		CarState.AllowRegen = false;

		resetOutput(TSLED, Off);
		resetOutput(RTDMLED, Off);
	}

	// check if all inverters are ready.
	////////////////////////////
	for(int i = 0; i < MOTORCOUNT; i++)
	{
		if (getInvState(i)->Device == OPERATIONAL)
		{
			readystate |= (0x1 << i);
		}
		else
		{
			readystate &= ~(0x1 << i);
		}
	}
		
	////////////////////////////
	bool waiting_btn = false;
	if( readystate > 0 && BPPS.data > 20 && gettimer() - prechargetimer > 5000) 	{
		waiting_btn = true;
		//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
		// inverters are ready, so allow torque.
	
	}


	/* EV 4.11.6
	 * After the TS has been activated, additional actions must be required by the driver
	 * to set the vehicle to ready-to-drive mode (e.g. pressing a dedicated start button).
	 *
	 * One of these actions must include the actuation of the mechanical brakes while ready-to-drive mode is entered.
	 */

	if (waiting_btn && BTN3.data) // if inverters ready, rtdm pressed, and brake held down.
		{

			return RunningState;
		}

	return TSActiveState;
}
