/*
 * shutdown.c
 *
 *  Created on: 15 Jul 2020
 *      Author: Visa
 */

#include "ecumain.h"

void ShutdownCircuitSet( bool state )
{
	HAL_GPIO_WritePin( Shutdown_GPIO_Port, Shutdown_Pin, state);
}

int ShutdownCircuitCurrent( void )
{
	// check if ADC ok
	return ADC_Data[2] * 1.22; // ~780 count ~= 0.95A ~820=1A   1.22 multiplication factor for approx mA calibrated.
}

int ShutdownCircuitState( void )
{
	return HAL_GPIO_ReadPin(Shutdown_GPIO_Port, Shutdown_Pin);
}

