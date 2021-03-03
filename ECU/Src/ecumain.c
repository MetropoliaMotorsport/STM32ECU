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

#include <operationalprocess.h>
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fdcan.h"
#include "i2c.h"
#ifdef HPF19
	#include "sdmmc.h"
	#include "usb_otg.h"
	#include "wwdg.h"
#endif
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "rng.h"
#include "gpio.h"
#include "canecu.h"
#include "interrupts.h"
#ifdef HPF20
	#include "eeprom.h"
	#include <powernode.h>
	#include "i2c-lcd.h"
	#include <stdio.h>
#endif

#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "cmsis_os.h"
#include "operationalprocess.h"
#include "ecumain.h"
#include "ecu.h"
#include "output.h"
#include "input.h"

#include "dwt_delay.h"



HAL_StatusTypeDef SystemClock_Config(void);
static int HardwareInit( void );


int __io_putchar(int ch)
{
	ITM_SendChar(ch);
	return ch;
}


void SWD_Init(void)
{
  *(__IO uint32_t*)(0x5C001004) |= 0x00700000; // DBGMCU_CR D3DBGCKEN D1DBGCKEN TRACECLKEN

  //UNLOCK FUNNEL
  *(__IO uint32_t*)(0x5C004FB0) = 0xC5ACCE55; // SWTF_LAR
  *(__IO uint32_t*)(0x5C003FB0) = 0xC5ACCE55; // SWO_LAR

  //SWO current output divisor register
  //This divisor value (0x000000C7) corresponds to 400Mhz
  //To change it, you can use the following rule
  // value = (CPU Freq/sw speed )-1
   *(__IO uint32_t*)(0x5C003010) = ((SystemCoreClock / 2000000) - 1); // SWO_CODR

  //SWO selected pin protocol register
   *(__IO uint32_t*)(0x5C0030F0) = 0x00000002; // SWO_SPPR

  //Enable ITM input of SWO trace funnel
   *(__IO uint32_t*)(0x5C004000) |= 0x00000001; // SWFT_CTRL

  //RCC_AHB4ENR enable GPIOB clock
   *(__IO uint32_t*)(0x580244E0) |= 0x00000002;

  // Configure GPIOB pin 3 as AF
   *(__IO uint32_t*)(0x58020400) = (*(__IO uint32_t*)(0x58020400) & 0xffffff3f) | 0x00000080;

  // Configure GPIOB pin 3 Speed
   *(__IO uint32_t*)(0x58020408) |= 0x00000080;

  // Force AF0 for GPIOB pin 3
   *(__IO uint32_t*)(0x58020420) &= 0xFFFF0FFF;
}


/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */



osThreadId_t MainTaskHandle;
const osThreadAttr_t MainTask_attributes = {
  .name = "MainTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 50
};





/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
void configureTimerForRunTimeStats(void)
{

}

unsigned long getRunTimeCounterValue(void)
{
	return DWT->CYCCNT;
}

void MainTask(void *argument)
{
	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	HardwareInit();

    ResetStateData();

//	vTaskDelay(500);

	TickType_t xLastWakeTime;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

//	struct AMessage *pxMessage;

//	struct lcd_msg msg;

	uint32_t counter = 0;

//	BaseType_t xHigherPriorityTaskWoken = false;
//	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

	lcd_setscrolltitle("Starting Main Loop");

	OperationalState = StartupState;

#ifdef LEDTEST
	setOutput(LED4, On);

	vTaskDelay(1000);
	blinkOutput(LED4, BlinkMed, 1);


	setOutput(LED5, Off);
	blinkOutput(LED5, Timed, 800);

	blinkOutput(LED7, BlinkFast, 1500);
#endif
	for(;;)
	{
		OperationalProcess();
		counter++;
//		msg.type = LCD_Cmd;
//		msg.data.count = counter;

//		xQueueSend( LCDQueue, ( void * ) &msg, ( TickType_t ) 0 );

//		blinkOutput(LED7, BlinkVeryFast, 1);

		uint32_t random;

		HAL_RNG_GenerateRandomNumber(&hrng, &random);

		if ( random % 100 == 0 )
			blinkOutput(LED6, BlinkVeryFast, 10);

		vTaskDelayUntil( &xLastWakeTime, CYCLETIME );
	}
	// shouldn't get here, but terminate thread gracefully if do somehow.
	osThreadTerminate(NULL);
}


void HAL_Delay(  volatile uint32_t millis )
{
/* replace HAL library blocking delay function
* with FreeRTOS thread aware equivalent */
	vTaskDelay(millis);
}

//In addition, I call the HAL_IncTick() function from the FreeRTOS TickHook:

void vApplicationTickHook( void )
{
	HAL_IncTick();
}

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */


// Initialise the ECU's internal features.
static int HardwareInit( void )
{
//	SWD_Init();

	/* Enable I-Cache---------------------------------------------------------*/
//	SCB_InvalidateICache();
	SCB_EnableICache();

#ifdef CACHE
	/* Enable D-Cache---------------------------------------------------------*/
	SCB_EnableDCache();
#endif
	//  SCB_CleanInvalidateDCache_by_Addr	(	uint32_t * 	addr,
	//int32_t 	dsize
//	)	 DMA

	static int enteredcount = 0;
	enteredcount++;
	ErrorCode = 0;
	//  HAL_StatusTypeDef status;
	int returnval = 0;

	// run through  the cubemx generated init functions.
#ifndef RTOS
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

#endif

	DWT_Init();

	HAL_Delay(50); // delay to allow debugger to hopefully latch. // have some input pin act as a boot stopper.

	/* Initialize all configured peripherals */

	MX_GPIO_Init(); // no failure return value
	MX_RNG_Init();

	// startup LCD first
#ifdef HPF20
	ShutdownCircuitSet( false ); // ensure shutdown circuit is closed at start

	initLCD();

	initTimer();

#ifdef PWMSTEERING
	initPWM();
#endif

#elif
	DeviceState.LCD = DISABLED;
#endif
	lcd_send_stringscroll("Start CANBUS");
	lcd_update();

	initPower();
	initConfig();
    initInv();
	initIMU();
	initPDM();
	initIVT();
	initBMS();
	initMemorator();
	initInput();
	initECU();

#ifdef HPF19
	initCANADC();
#endif

#ifdef POWERNODES
	initNodes();
	initPowerNodes();
#else
	initPDM();
#endif
#ifdef ANALOGNODES
	initAnalogNodes();
#endif

	initCAN();

	initADC();

#ifdef WATCHDOG
	initWatchdog();
#endif

	lcd_send_stringscroll("Enable LEDS");
	lcd_update();
    initOutput(); // set default led states and start life indicator LED blinking.

#ifdef POWERLOSSDETECT
    initPowerLossHandling()
#endif

#ifdef EEPROMSTORAGE
    initEEPROM();
#endif

	// after cubemx hardware inits, run our own initialisations to start up essential function.

	// should also read in defaults for calibrations, power levels etc.

// initButtons was here moved later, during startup sequence inputs were being triggered early

//	  uint16_t volatile * const power = (uint16_t *) PWR_D3CR;

#ifdef TEST
	char str[20];
	i = 0;
	while ( 1 ) {
		sprintf(str,"%.8d", i);
		lcd_send_stringpos(0,0,str);
		lcd_update();
		i++;
	}
#endif

#ifdef SCROLLTEST
	char str[20];
	int i = 0;
	for ( i=0;i<10;i++ ){
		HAL_Delay(100);
		sprintf(str,"%.8d", i);
		lcd_send_stringscroll(str);
		lcd_update();
	}

	while ( 1 )
	{

		lcd_processscroll(GetUpDownPressed());
		lcd_update();
		HAL_Delay(50);

	}
#endif

	lcd_send_stringscroll("Hardware init done.");
	lcd_clearscroll();
	lcd_update();

	lcd_send_stringscroll("Shutdown closed.");
	lcd_update();
//	ShutdownCircuitSet( true );
//	while ( 1 ){};
	return returnval;
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
  /* MCU Configuration--------------------------------------------------------*/

 int MainState = 0;

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/*
	initLCD();

	HAL_Delay(20);

//	unsigned long counter;

	struct lcd_message msg;

	uint32_t counter = 0;

	for(;;)
	{
		char str[LCDCOLUMNS+1] = "";

		sprintf(str,"Count: %.10u", counter );
		counter++;

		lcd_send_stringline(3,str, 255);
//		printf("value received on queue: %lu \n",counter);

		lcd_updatedisplay();
		HAL_Delay(20);
	}
*/

#ifdef RTOS

 /* Init scheduler */
 osKernelInitialize();  /* Call init function for freertos objects (in freertos.c) */

 MainTaskHandle = osThreadNew(MainTask, NULL, &MainTask_attributes);

// MX_FREERTOS_Init();

 /* Start scheduler */
 osKernelStart();

 while ( 1 )
 {};

 // should never get here.

#else

 // uint8_t CANTxData[8];
  while (1) // primary state loop.
  {
	  switch ( MainState )
	  {
		case 0 : // initialisation, no external communication barring a turn on message.

			switch ( HardwareInit() ) // call initialisation function.
			{
				case 0 :  // init successful, move to operational state.
					ResetStateData(); // set values configured.
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


			setOutputNOW(LED1,On);

			while( 1 ) // enter do nothing loop of failure.
			{

			};
	}

  }
#endif
}




int testmain(void)
{
  HAL_Init();
  SystemClock_Config();

//  InButtonpress = 1; // prevent any button presses registering until setup.

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

//  MX_ADC3_Init();
  /* USER CODE BEGIN 2 */

  initInterrupts();

//	FDCAN1_start(); // sets up can ID filters and starts can bus 1
	// first point from which can messages can be sent.
//	FDCAN2_start();

//  startADC();
  int i = 0;
  initInput(); // clears any errant processed button presses.

  setOutput(LED2,Off);
  setOutput(LED3,On);
  blinkOutput(LED1, On, 5);
  blinkOutput(LED2, On, 4);
  blinkOutput(LED3, LEDBLINK_TWO, 255);

  while (1)
  {
	  HAL_Delay(500);
//	  int j = ADC_Data[SteeringADC];
//	  CAN_SendStatus( i, 0, 0 );
//	  CANSendInverter( 255-i, 0, 0 );
	  i++;
//	  int adcstuff = aADCxConvertedDataADC3;

#ifdef HPF19
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
#endif

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
