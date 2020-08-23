/*
 * interrupts.c
 *
 *  Created on: 8 Jan 2019
 *      Author: drago
 */

#include "ecumain.h"
#include "interrupts.h"

void initInterrupts( void )
{
#ifndef RTOS
	InButtonpress = 1; // stops random button events triggering till interrupts are properly enabled.

	// enable and start LED timer interrupt
	HAL_NVIC_SetPriority(TIM3_IRQn, 6, 5);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
#endif

#ifdef HPF19
	// enable button interrupts -- handled in GPIO setup, set to low priority, human scale input, not priority.
	HAL_NVIC_SetPriority(USER_Btn_EXTI_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(USER_Btn_EXTI_IRQn);

	HAL_NVIC_SetPriority(Input1_EXTI_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(Input1_EXTI_IRQn);

	HAL_NVIC_SetPriority(Input2_EXTI_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(Input2_EXTI_IRQn);

	HAL_NVIC_SetPriority(Input3_EXTI_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(Input3_EXTI_IRQn);
#endif

#ifdef HPF20

// setup in GPIO init already.
// EXTI2_IRQn
// EXTI3_IRQn
// EXTI4_IRQn
// EXTI9_5_IRQn
// EXTI15_10_IRQn
#endif

#ifndef RTOS
	if ( HAL_TIM_Base_Start_IT(&htim3) != HAL_OK){
		  Error_Handler();
	}
	InButtonpress = 0;

	// start LCD update Timer.
	if ( HAL_TIM_Base_Start_IT(&htim6) != HAL_OK){
		Error_Handler();
	}

#endif



#ifdef PARALLELLCD
	  HAL_NVIC_SetPriority(TIM17_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(TIM17_IRQn);

	if ( HAL_TIM_Base_Start_IT(&htim17) != HAL_OK){
		Error_Handler();
	}
#endif
}
