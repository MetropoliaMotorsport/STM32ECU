/*
 * ecumain.c
 *
 *  Created on: 14 Mar 2019
 *      Author: Visa Harvey
 *
 *
 *      -- use interrupt for sending torque request?
 *      -- semaphores for can receive?
 */

/**
  ******************************************************************************
  * @file           : ecumain.c
  * @brief          : Actual Main program body
  ******************************************************************************

  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

//#ifdef NOHARDWARE

#include <operationalprocess.h>
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fdcan.h"
#include "i2c.h"
#include "sdmmc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "wwdg.h"
#include "gpio.h"
#include "canecu.h"
#include "interrupts.h"

//#endif

#include <stdlib.h>

/* Private includes ----------------------------------------------------------*/

#include "operationalprocess.h"
#include "ecumain.h"
#include "ecu.h"
#include "output.h"
#include "input.h"

#include "dwt_delay.h"

HAL_StatusTypeDef SystemClock_Config(void);

// Initialise the ECU's internal features.
static int HardwareInit( void )
{
	static int enteredcount = 0;
	enteredcount++;
	ErrorCode = 0;
	//  HAL_StatusTypeDef status;
	int returnval = 0;

	// run through  the cubemx generated init functions.

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	if ( HAL_Init() != HAL_OK )
	{
		returnval = 99; // If error in basic hardware initialisation, something very bad wrong.
	  	  	  	  	  	  //should possibly hang here, nothing yet initialised to communicate status.
	}

	/* Configure the system clock */
	if ( SystemClock_Config() != HAL_OK )
	{

		returnval = 99; // if error in setting system clock something very bad wrong.
		  	  	  	  	  //should possibly hang here, nothing yet initialised to communicate status.
	}

	DWT_Init();

	/* Initialize all configured peripherals */

	MX_GPIO_Init(); // no failure return value
	MX_TIM3_Init(); // at this point LED status should work.

	MX_FDCAN1_Init();
#ifndef ONECAN
	MX_FDCAN2_Init();
#endif

	FDCAN1_start(); // sets up can ID filters and starts can bus 1
	FDCAN2_start(); // starts up can bus 2

	CarState.HighVoltageReady = 0;
	sendPDM( 0 ); // send high voltage off request to PDM.

	CAN_SendStatus(0,0,1); // earliest possible place to send can message signifying wakeup.

#ifdef STMADC
	MX_DMA_Init();

	MX_ADC1_Init();
	MX_ADC3_Init();
#endif

//	MX_I2C2_Init();
	MX_TIM7_Init();

	MX_TIM6_Init();

	setupInterrupts(); // start timers etc

#ifdef WATCHDOG
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
#endif

	setupLEDs(); // set default led states and start life indicator LED blinking.


/*	delay = TimeoutCalculation((hwwdg1.Init.Counter) + 1); // actual reset, 40ms.

	HAL_Delay(delay);

	delay = TimeoutCalculation((hwwdg1.Init.Counter)-64 + 1); // early warning, 20ms

	HAL_Delay(delay); */

	// after cubemx hardware inits, run our own initialisations to start up essential function.
#ifdef LCD
	MX_TIM17_Init(); // lcd timer.
	hd44780_Init(); // initialise LCD for debug helper information, remove for live code?
#endif

	// should also read in defaults for calibrations, power levels etc.

#ifdef STMADC
	if ( startADC() == 0 )  //  starts the ADC dma processing.
	{
		DeviceState.ADC = 1;
	} else returnval = 99;
#endif
	//  startupLEDs(); // run little startup LED animation to indicate powered on.
	setupButtons(); // moved later, during startup sequence inputs were being triggered early

	//send can state to show hardware initialized.

//	  uint16_t volatile * const power = (uint16_t *) PWR_D3CR;




	return returnval;
}

/* Captured Value */
__IO uint32_t            uwIC2Value = 0;
/* Duty Cycle Value */
__IO uint32_t            uwDutyCycle = 0;
/* Frequency Value */
__IO uint32_t            uwFrequency = 0;


/**
  * @brief  Input Capture callback in non blocking mode
  * @param  htim: TIM IC handle
  * @retval None
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    /* Get the Input Capture value */
    uwIC2Value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

    if (uwIC2Value != 0)
    {
      /* Duty cycle computation */
      uwDutyCycle = ((HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1)) * 100) / uwIC2Value;

      /* uwFrequency computation
      TIM4 counter clock = (RCC_Clocks.HCLK_Frequency) */
      uwFrequency = (HAL_RCC_GetHCLKFreq()) / uwIC2Value;
    }
    else
    {
      uwDutyCycle = 0;
      uwFrequency = 0;
    }
  }
}


/*
 * Primary operating function entered into after initialisation.
 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int realmain(void)
{
#ifdef PWMSTEERINGTEST

	MX_TIM8_Init(); // pwm

	  if(HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_2) != HAL_OK)
	  {
	    /* Starting Error */
	    Error_Handler();
	  }

	  /*##-5- Start the Input Capture in interrupt mode ##########################*/
	  if(HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_1) != HAL_OK)
	  {
	    /* Starting Error */
	    Error_Handler();
	  }

	  /* Infinite loop */
	  while (1)
	  {
	  }
#endif


  /* MCU Configuration--------------------------------------------------------*/

 int MainState = 0;

 // uint8_t CANTxData[8];

  while (1) // primary state loop.
  {
	  switch ( MainState )
	  {
		case 0 : // initialisation, no external communication barring a turn on message.

			switch ( HardwareInit() ) // call initialisation function.
			{
				case 0 :  // init successful, move to operational state.
					MainState = 1;
					break;
/*					case 1 :  // init not successful in non fatal manner.
					InitAttempts++; // mark failed init, leave state at current to try again if not overrun attempts
					break; */
				case 99 : // unrecoverable init failure, full shutdown.
				default :
					MainState = 99;  // unrecoverable fault.

			}
			break;

		case 1  : // operational state.
			switch ( OperationalProcess() )
			{
				case 0 : // shouldn't need to return except for a full shutdown?
					break;
				default	: // any other state assumed to be error also.
					MainState = 99;
			}
			break;
		case 99 : //  error state
			// trigger error LED's, try to send error can bus message.
		default : // shouldn't be here, treat also as error state.
			MainState = 99;
			receivePDM();


			setOutputNOW(LED1_Output,LEDON);

			while( 1 ) // enter do nothing loop of failure.
			{

			};
	}

  }
}




int testmain(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
//  MX_FDCAN1_Init();
  MX_TIM3_Init();
//  MX_FDCAN2_Init();
  MX_TIM7_Init();
//  MX_USB_OTG_FS_PCD_Init();
//  MX_SDMMC1_SD_Init();
//  MX_SPI5_Init();
//  MX_USART2_UART_Init();
 // MX_SPI2_Init();
 // MX_WWDG1_Init();
  MX_TIM6_Init();
  MX_TIM17_Init();
//  MX_ADC3_Init();
  /* USER CODE BEGIN 2 */

  setupInterrupts();

//	FDCAN1_start(); // sets up can ID filters and starts can bus 1
	// first point from which can messages can be sent.
//	FDCAN2_start();

//  startADC();
  int i = 0;
   setupButtons();

  setOutput(LED2_Output,LEDOFF);
  setOutput(LED3_Output,LEDON);
  blinkOutput(LED1_Output, LEDON, 5);
  blinkOutput(LED2_Output, LEDON, 4);
  blinkOutput(LED3_Output, LEDBLINK_TWO, 255);

  while (1)
  {
	  HAL_Delay(500);
//	  int j = ADC_Data[SteeringADC];
//	  CAN_SendStatus( i, 0, 0 );
//	  CANSendInverter( 255-i, 0, 0 );
	  i++;
//	  int adcstuff = aADCxConvertedDataADC3;

	  if ( Input[UserBtn].pressed )
	  {
		  Input[UserBtn].pressed = 0;
	/*	  toggleOutput(0);
		  toggleOutput(1);
		  toggleOutput(2);
		  toggleOutput(3);
		  toggleOutput(4);
		  toggleOutput(5);
		  toggleOutput(6);
		  toggleOutput(7); */
		  setOutput(LED2_Output,LEDOFF);
		  setOutput(LED1_Output,LEDON);
		//  toggleOutput(8);
		//  toggleOutput(9);
		//  toggleOutput(10);

//		  blinkOutput(9, LEDBLINK_FOUR, 1);
//		  blinkOutput(10, LEDBLINK_FOUR, 1);
	  }


//	  LEDs[10].blinkingrate=1;
//	  LEDs[8].blinkingrate=LED_BLINK_ONE;
//	  LEDs[9].blinkingrate=LED_ON;
//	  LEDs[10].blinkingrate=LED_OFF;
//	  blinkOutput(8, LED_BLINK_FOUR, 0);

//	  blinkOutput(10, LED_BLINK_FOUR, 4);

//	  toggleOutput(9);
//	  toggleOutput(10);
//	  toggleOutput(11);
//	  setOutput(13,0);
//	  setOutput(14,1);
//	  toggleOutput(15);*/

  }
  /* USER CODE END 3 */
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
