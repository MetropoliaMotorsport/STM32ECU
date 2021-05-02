/*
 * watchdog.c
 *
 *      Author: Visa
 */

#include "ecumain.h"
#ifdef WATCHDOG
#include "wwdg.h"
#endif



osThreadId_t WatchdogTaskHandle;
const osThreadAttr_t WatchdogTask_attributes = {
  .name = "WatchdogTask",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 128*4
};



/**
  * @brief  Timeout calculation function.
  *         This function calculates any timeout related to
  *         WWDG with given prescaler and system clock.
  * @param  timevalue: period in term of WWDG counter cycle.
  * @retval None
  */
static uint32_t TimeoutCalculation(uint32_t timevalue)
{
  uint32_t timeoutvalue = 0;
  uint32_t pclk1 = 0;
  uint32_t wdgtb = 0;

  /* Get PCLK1 value */
  pclk1 = HAL_RCC_GetPCLK1Freq();

  /* get prescaler */
  switch(hwwdg1.Init.Prescaler)
  {
    case WWDG_PRESCALER_1:   wdgtb = 1;   break;
    case WWDG_PRESCALER_2:   wdgtb = 2;   break;
    case WWDG_PRESCALER_4:   wdgtb = 4;   break;
    case WWDG_PRESCALER_8:   wdgtb = 8;   break;
    case WWDG_PRESCALER_16:  wdgtb = 16;  break;
    case WWDG_PRESCALER_32:  wdgtb = 32;  break;
    case WWDG_PRESCALER_64:  wdgtb = 64;  break;
    case WWDG_PRESCALER_128: wdgtb = 128; break;

    default: Error_Handler(); break;
  }

  /* calculate timeout */
  timeoutvalue = ((4096 * wdgtb * timevalue) / ((pclk1*2) / 1000)); // not sure if right.

  return timeoutvalue;
}


static void WatchdogTask(void *pvParameters)
{
	volatile int count = 0;

	while ( 1 )
	{
		count++;
		vTaskDelay(10);
		HAL_WWDG_Refresh(&hwwdg1);

	}
}

int initWatchdog( void )
{
	volatile uint8_t resetflag = __HAL_RCC_GET_FLAG(RCC_FLAG_WWDG1RST);

	/*##-1- Check if the system has resumed from WWDG reset ####################*/
	if (resetflag != RESET)
	{
		  // system has been reset by WWDG, do any special initialisation here.
	//	__BKPT(1);
	}
	/* Clear reset flags in any case */
	__HAL_RCC_CLEAR_RESET_FLAGS();

	/* Enable system wide reset */
	HAL_RCCEx_WWDGxSysResetConfig(RCC_WWDG1);

	/*##-2- Init & Start WWDG peripheral ######################################*/
	/*
	  Timing calculation:
	  a) WWDG clock counter period (in ms) = (4096 * PRESCALER) / (PCLK1 / 1000)
        ~ 0.1365ms

	  b) WWDG timeout (in ms) = (counter + 1) * period

	  ~17ms

	  c) Time to enter inside window
	  Window timeout (in ms) = (counter - window + 1) * period

 */

	hwwdg1.Instance = WWDG1;

	hwwdg1.Init.Prescaler = WWDG_PRESCALER_8;
	hwwdg1.Init.Window    = 96;
	hwwdg1.Init.Counter   = 127;

	// 16

	hwwdg1.Init.EWIMode = WWDG_EWI_ENABLE;

	if (HAL_WWDG_Init(&hwwdg1) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_NVIC_EnableIRQ(WWDG_IRQn);


	volatile uint32_t windowend = TimeoutCalculation((hwwdg1.Init.Counter) - 1);
	volatile uint32_t windowstart = TimeoutCalculation((hwwdg1.Init.Counter-hwwdg1.Init.Window) + 1);


	volatile int counter = 0;

	// min 8
	// max 16

/*	while ( 1 )
	{
		counter++;
		HAL_Delay(16); // ensure that first trigger will be within window.
		HAL_WWDG_Refresh(&hwwdg1);

	}
*/
	WatchdogTaskHandle = osThreadNew(WatchdogTask, NULL, &WatchdogTask_attributes);
	return 0;
}
