/*
 * interrupts.c
 *
 *  Created on: 8 Jan 2019
 *      Author: drago
 */

#include "ecumain.h"
#include "interrupts.h"

void setupInterrupts( void )
{
	InButtonpress = 1; // stops random button events triggering till interrupts are properly enabled.

	// enable and start timer interrupt
	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 5);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	// enable button interrupts
	HAL_NVIC_SetPriority(USER_Btn_EXTI_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(USER_Btn_EXTI_IRQn);

	HAL_NVIC_SetPriority(Input1_EXTI_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(Input1_EXTI_IRQn);

	HAL_NVIC_SetPriority(Input2_EXTI_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(Input2_EXTI_IRQn);

	HAL_NVIC_SetPriority(Input3_EXTI_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(Input3_EXTI_IRQn);


	if ( HAL_TIM_Base_Start_IT(&htim3) != HAL_OK){
		  Error_Handler();
	}

	InButtonpress = 0;

	// start LCD update Timer.
	if ( HAL_TIM_Base_Start_IT(&htim6) != HAL_OK){
		Error_Handler();
	}
#ifdef LCD
	  HAL_NVIC_SetPriority(TIM17_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(TIM17_IRQn);

	if ( HAL_TIM_Base_Start_IT(&htim17) != HAL_OK){
		Error_Handler();
	}
#endif
}
