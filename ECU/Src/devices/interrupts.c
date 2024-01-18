/*
 * interrupts.c
 *
 *  Created on: 8 Jan 2019
 *      Author: Visa
 */

#include "ecumain.h"
#include "interrupts.h"

void initInterrupts(void) {
#ifndef RTOS
	InButtonpress = 1; // stops random button events triggering till interrupts are properly enabled.

	// enable and start LED timer interrupt
	HAL_NVIC_SetPriority(TIM3_IRQn, 6, 5);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
#endif

// button reads from wheel, interrupt triggers
#ifdef HPF2023
	HAL_NVIC_SetPriority(WHLINT_EXTI_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(WHLINT_EXTI_IRQn);
#endif

#ifndef RTOS
	if ( HAL_TIM_Base_Start_IT(&htim3) != HAL_OK){
		  Error_Handler();
	}
	InButtonpress = 0;
#endif
}
