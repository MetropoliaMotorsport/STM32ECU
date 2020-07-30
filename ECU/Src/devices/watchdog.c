/*
 * memorator.c
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#include "ecumain.h"
#include "wwdg.h"


int initWatchdog( void )
{
	/*##-1- Check if the system has resumed from WWDG reset ####################*/
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDG1RST) != RESET)
	{
	/* Insert 4s delay */
		HAL_Delay(4000);
	} else
	{
	  // system has been reset by WWDG, do any special initialisation here.
	}

	/* Clear reset flags in any case */
	__HAL_RCC_CLEAR_RESET_FLAGS();

	/* Enable system wide reset */
	HAL_RCCEx_WWDGxSysResetConfig(RCC_WWDG1);

	//	MX_WWDG1_Init();

	hwwdg1.Instance = WWDG1;
	hwwdg1.Init.Prescaler = WWDG_PRESCALER_8; // 8-19ms window.
	hwwdg1.Init.Window = 102;
	hwwdg1.Init.Counter =127;
	hwwdg1.Init.EWIMode = WWDG_EWI_ENABLE;

	if (HAL_WWDG_Init(&hwwdg1) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_NVIC_EnableIRQ(WWDG_IRQn);

/*	 while ( 1 )
	 {
	 uint32_t delay = TimeoutCalculation((hwwdg1.Init.Counter-hwwdg1.Init.Window) + 1); // start of window, 9ms.

		HAL_Delay(19); // ensure that first trigger will be within window.
		if (HAL_WWDG_Refresh(&hwwdg1) != HAL_OK)
		{
		  Error_Handler();
		}
	 } */
	return 0;
}
