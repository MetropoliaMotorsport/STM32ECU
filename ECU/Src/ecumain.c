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
#include "debug.h"
#include "watchdog.h"

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

	initERRORState();
	initDebug();

    ResetStateData();

	TickType_t xLastWakeTime;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	lcd_settitle("Starting Main Loop");

	OperationalState = StartupState;

	registerWatchdogBit(0);

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
		setWatchdogBit(0);
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

//In addition, call the HAL_IncTick() function from the FreeRTOS TickHook:

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

	MX_DMA_Init();

	static int enteredcount = 0;
	enteredcount++;
	ErrorCode = 0;
	//  HAL_StatusTypeDef status;
	int returnval = 0;

	// run through  the cubemx generated init functions.

	vTaskDelay(50); // delay to allow debugger to hopefully latch. // have some input pin act as a boot stopper.

	/* Initialize all configured peripherals */

	MX_GPIO_Init(); // no failure return value
	MX_RNG_Init();

	// startup LCD first
#ifdef HPF20
	ShutdownCircuitSet( false ); // ensure shutdown circuit is closed at start

	initLCD();

	if ( watchdogRebooted() )
	  lcd_send_stringline(0, "Watchdog Rebooted...", 0);

	lcd_startscroll();
	lcd_setscrolltitle("Startup...");


	initTimer();

#ifdef PWMSTEERING
	initPWM();
#endif

#elif
	DeviceState.LCD = DISABLED;
#endif
	lcd_send_stringscroll("Start CANBUS");

	initCAN();
	initRTC();

	initPower();
	initConfig();
    initInv();
	initIMU();
	initPDM();
	initIVT();
	initBMS();
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

	initADC();

	lcd_send_stringscroll("Enable LEDS");
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
		i++;
	}
#endif

#ifdef SCROLLTEST
	char str[20];
	int i = 0;
	for ( i=0;i<10;i++ ){
		vTaskDelay(100);
		sprintf(str,"%.8d", i);
		lcd_send_stringscroll(str);
	}

	while ( 1 )
	{
		lcd_processscroll(GetUpDownPressed());
		vTaskDelay(50);
	}
#endif

	lcd_send_stringscroll("Hardware init done.");

//	lcd_send_stringscroll("Shutdown closed.");
//	ShutdownCircuitSet( true );
//	while ( 1 ){};

	vTaskDelay(200);

	lcd_endscroll();
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

//    int MainState = 0;

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Init scheduler */
	osKernelInitialize();  /* Call init function for freertos objects (in freertos.c) */

	// init the watchdog before anything else so that it will catch any startup hang.

	DWT_Init();
	initWatchdog();

	MainTaskHandle = osThreadNew(MainTask, NULL, &MainTask_attributes);

	// MX_FREERTOS_Init();

	/* Start scheduler */
	osKernelStart();

	while ( 1 )
	{};

 // should never get here.
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
