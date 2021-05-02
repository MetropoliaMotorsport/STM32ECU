/*
 * watchdog.c
 *
 *      Author: Visa
 */

#include "ecumain.h"
#include "wwdg.h"
#include "event_groups.h"


osThreadId_t WatchdogTaskHandle;
const osThreadAttr_t WatchdogTask_attributes = {
  .name = "WatchdogTask",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 128*4
};

bool watchdogreboot = false;

EventGroupHandle_t xWatchdog, xWatchdogActive;

void setWatchdogBit(uint8_t bit)
{
	xEventGroupSetBits(xWatchdog, ( 1 << bit ) );
}

void registerWatchdogBit(uint8_t bit)
{
	xEventGroupSetBits(xWatchdogActive, ( 1 << bit ) );
	setWatchdogBit(bit);
}


bool watchdogRebooted( void )
{
	return watchdogreboot;
}

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

	vTaskDelay(20);

	while ( 1 )
	{
		count++;

		EventBits_t activeBits = xEventGroupGetBits(xWatchdogActive);

		EventBits_t uxBits = xEventGroupGetBits(xWatchdog);

		if( ( uxBits & activeBits ) == ( activeBits ) )
		{
			// only kick the watchdog if all expected bits are set.
			HAL_WWDG_Refresh(&hwwdg1);
		}

		xEventGroupClearBits(xWatchdog, 0xFF );

		vTaskDelay(20);
	}
}

int initWatchdog( void )
{
	volatile uint8_t resetflag = __HAL_RCC_GET_FLAG(RCC_FLAG_WWDG1RST);

	/*##-1- Check if the system has resumed from WWDG reset ####################*/
	if (resetflag != RESET)
	{
		  // system has been reset by WWDG, do any special initialisation here.
		watchdogreboot = true;
	} else
	{
		watchdogreboot = false;
	}
	/* Clear reset flags in any case */
	__HAL_RCC_CLEAR_RESET_FLAGS();

	/* Enable system wide reset */
	HAL_RCCEx_WWDGxSysResetConfig(RCC_WWDG1);


	// TODO allocate static?
	xWatchdog = xEventGroupCreate();
	xWatchdogActive = xEventGroupCreate();

	    /* Was the event group created successfully? */
	    if( xWatchdog == NULL )
	    {
	        /* The event group was not created because there was insufficient
	        FreeRTOS heap available. */
	    }
	    else
	    {
	        /* The event group was created. */
	    }

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

	hwwdg1.Init.Prescaler = WWDG_PRESCALER_16;
	hwwdg1.Init.Window    = 96;
	hwwdg1.Init.Counter   = 127;

	// 16

	hwwdg1.Init.EWIMode = WWDG_EWI_ENABLE;

	if (HAL_WWDG_Init(&hwwdg1) != HAL_OK)
	{
		Error_Handler();
	}

	// manually setup stop watchdog when debugging.
	// otherwise J-Link is non workable without disabling watchdog.
	// no option in Cube IDE to disable like for ST-Link.

	DBGMCU->APB3FZ1 = ( 1 << 6 ); //Bit6 WWDG1:WWDG1stopindebug;

	HAL_NVIC_EnableIRQ(WWDG_IRQn);


	volatile uint32_t windowend = TimeoutCalculation((hwwdg1.Init.Counter) + 1);
	volatile uint32_t windowstart = windowend/2+TimeoutCalculation((hwwdg1.Init.Counter-hwwdg1.Init.Window) - 1);


	volatile int counter = 0;

	// min 8 // should now be double.
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
