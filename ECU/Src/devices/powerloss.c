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

bool checkPowerState( void )
{
	// pin pb0, if it's high, we have power?
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0 );
}

void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp) {
	timeOutput(LED2, 1000);
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
