/*
 * powerloss.c
 *
 *  Created on: 20 Feb 2020
 *      Author: Visa
 */


#include "ecumain.h"
#include "powernode.h"
#include "output.h"
#include "power.h"

#include "comp.h"

volatile bool powerlost = false;

// callback gets called when power lost, to allow for setting up reaction.


void resetPowerLost( void )
{
    powerlost = false;
}

void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp) {
// send lost power indicator to canbus.
//	CAN_SendStatus( 50, 100, 0xFFFF ); // TODO send loosing power error, choose message.
	setOutput(LED2,On);
	blinkOutput(LED2, On, 1000);
	if ( !powerlost )
		PowerLogError( 0, 0xFFFF);
	powerlost = true;
}

int initPowerLossHandling( void )
{
	  MX_COMP1_Init();
	  HAL_COMP_Start_IT(&hcomp1);// need to reset interrupts before starting. triggers during startup.
	  vTaskDelay(10);
	  powerlost = false;
	  return 0;
}
