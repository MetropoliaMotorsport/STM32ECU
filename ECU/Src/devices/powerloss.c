/*
 * powerloss.c
 *
 *  Created on: 20 Feb 2020
 *      Author: Visa
 */

#include "ecumain.h"
#include "powerloss.h"

volatile bool powerlost = false;

// callback gets called when power lost, to allow for setting up reaction.

void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp) {
	powerlost = true;
// send lost power indicator to canbus.
	CAN_SendErrorStatus( 50, 100, 0xFFFF );
	setOutput(LED7_Output,LEDON);
}

