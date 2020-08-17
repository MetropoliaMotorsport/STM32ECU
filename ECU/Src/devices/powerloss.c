/*
 * powerloss.c
 *
 *  Created on: 20 Feb 2020
 *      Author: Visa
 */

#include <powernode.h>
#include "ecumain.h"

volatile bool powerlost = false;

// callback gets called when power lost, to allow for setting up reaction.

void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp) {
	powerlost = true;
// send lost power indicator to canbus.
	CAN_SendStatus( 50, 100, 0xFFFF ); // TODO send loosing power error, choose message.
	setOutput(LED7,On);


}

int initPowerLossHandling( void )
{
	  MX_COMP1_Init();
	  HAL_COMP_Start_IT(&hcomp1);// need to reset interrupts before starting. triggers during startup.
	  HAL_Delay(10);
	  powerlost = false;
	  return 0;
}
