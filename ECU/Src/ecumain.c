/*
 * ecumain.c
 *
 *  Created on: 14 Mar 2019
 *      Author: Visa Harvey
 *
 */

/**
 ******************************************************************************
 * @file           : ecumain.c
 * @brief          : Actual Main program body
 ******************************************************************************

 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "ecumain.h"
#include "dwt_delay.h"
#include "watchdog.h"
#include "errors.h"
#include "eeprom.h"
#include "powernode.h"
#include "analognode.h"
#include "node.h"
#include "input.h"
#include "output.h"
#include "adcecu.h"
#include "ivt.h"
#include "bms.h"
#include "timerecu.h"
#include "imu.h"
#include "inverter.h"
#include "debug.h"
#include "configuration.h"
#include "operationalprocess.h"
#include "uartecu.h"
#include "taskpriorities.h"

#include "dma.h"
#include "gpio.h"
#include "rng.h"


//Hello worldS

HAL_StatusTypeDef SystemClock_Config(void);
static int HardwareInit(void);

int __io_putchar(int ch) {
	ITM_SendChar(ch);
	return ch;
}

void SWD_Init(void) {
	*(__IO uint32_t*) (0x5C001004) |= 0x00700000; // DBGMCU_CR D3DBGCKEN D1DBGCKEN TRACECLKEN

	//UNLOCK FUNNEL
	*(__IO uint32_t*) (0x5C004FB0) = 0xC5ACCE55; // SWTF_LAR
	*(__IO uint32_t*) (0x5C003FB0) = 0xC5ACCE55; // SWO_LAR

	//SWO current output divisor register
	//This divisor value (0x000000C7) corresponds to 400Mhz
	//To change it, you can use the following rule
	// value = (CPU Freq/sw speed )-1
	*(__IO uint32_t*) (0x5C003010) = ((SystemCoreClock / 2000000) - 1); // SWO_CODR

	//SWO selected pin protocol register
	*(__IO uint32_t*) (0x5C0030F0) = 0x00000002; // SWO_SPPR

	//Enable ITM input of SWO trace funnel
	*(__IO uint32_t*) (0x5C004000) |= 0x00000001; // SWFT_CTRL

	//RCC_AHB4ENR enable GPIOB clock
	*(__IO uint32_t*) (0x580244E0) |= 0x00000002;

	// Configure GPIOB pin 3 as AF
	*(__IO uint32_t*) (0x58020400) = (*(__IO uint32_t*) (0x58020400)
			& 0xffffff3f) | 0x00000080;

	// Configure GPIOB pin 3 Speed
	*(__IO uint32_t*) (0x58020408) |= 0x00000080;

	// Force AF0 for GPIOB pin 3
	*(__IO uint32_t*) (0x58020420) &= 0xFFFF0FFF;
}

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

volatile CarStateType CarState;
volatile DeviceStateType DeviceState;

#define MAINTASKSTACK_SIZE 128*24
#define MAINTASKTASKNAME  "MainTaskTask"
StaticTask_t xMAINTASKTaskBuffer;
StackType_t xMAINTASKStack[MAINTASKSTACK_SIZE];

TaskHandle_t MainTaskTaskHandle = NULL;

EventGroupHandle_t xStartupSync;

EventGroupHandle_t xCycleSync;

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
void configureTimerForRunTimeStats(void) {

}

unsigned long getRunTimeCounterValue(void) {
	return DWT->CYCCNT;
}

void MainTask(void *argument) {
	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	HardwareInit();

	initERRORState();

	ResetStateData();

	TickType_t xLastWakeTime;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	OperationalState = StartupState;

	uint8_t watchdogBit = registerWatchdogBit("MainTask");

#ifdef LEDTEST
	setOutput(LED4, On);

	vTaskDelay(1000);
	blinkOutput(LED4, BlinkMed, 1);


	setOutput(LED5, Off);
	blinkOutput(LED5, Timed, 800);

	blinkOutput(LED7, BlinkFast, 1500);
#endif

	xEventGroupSync(xStartupSync, 1, 1, 100);

	CarState.allowtsactivation = true;

	while (1) {
		TickType_t startloop = xLastWakeTime;
		OperationalProcess();
		vTaskDelayUntil(&xLastWakeTime, CYCLETIME);
		setWatchdogBit(watchdogBit);
		xEventGroupSync(xCycleSync, 1, 1, 0);
		// send sync at end of loop. set up a syncronisation group.
		CAN_NMTSyncRequest();

		if (xLastWakeTime - startloop > CYCLETIME)
			DebugMsg("Long process loop!");
	}
	// shouldn't get here, but terminate thread gracefully if do somehow.
	vTaskDelete(NULL);
}

void HAL_Delay(volatile uint32_t millis) {
	/* replace HAL library blocking delay function
	 * with FreeRTOS thread aware equivalent */
	vTaskDelay(millis);
}

//In addition, call the HAL_IncTick() function from the FreeRTOS TickHook:

void vApplicationTickHook(void) {
	HAL_IncTick();
}

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

// Initialise the ECU's internal features.
static int HardwareInit(void) {
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

	initUART();

	initDebug();

	// startup LCD first
#ifdef HPF20
	ShutdownCircuitSet( false); // ensure shutdown circuit is closed at start

	initOutput(); // set default led states and start life indicator LED blinking. needed for LCD powering.

	if (watchdogRebooted()) {
		DebugMsg("Watchdog Rebooted!");
	}

	initTimer();

#ifdef PWMSTEERING
	initPWM();
#endif

#elif
	DeviceState.LCD = DISABLED;
#endif

	initCAN();
	initRTC();

	initPower();

	initIMU();
#ifdef PDM
	initPDM();
#endif
	initIVT();
	initBMS();
	initInput();
	initECU();

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

#ifdef EEPROMSTORAGE
	initEEPROM();
#endif

	initConfig(); // config relies on eeprom data, was too early in process!

	eepromdata *data = getEEPROMBlock(0);

	DebugPrintf("Apps Calib L: %5d - %5d ( %5d %5d )\r\n",
			data->ADCTorqueReqLInput[0], data->ADCTorqueReqLInput[1],
			data->ADCTorqueReqLInput[2], data->ADCTorqueReqLInput[3]);

	DebugPrintf("Apps Calib R: %5d - %5d ( %5d %5d )\r\n",
			data->ADCTorqueReqRInput[0], data->ADCTorqueReqRInput[1],
			data->ADCTorqueReqRInput[2], data->ADCTorqueReqRInput[3]);

	DebugPrintf("Regen Calib:  %5d - %5d ( %5d %5d )\r\n",
			data->ADCBrakeTravelInput[0], data->ADCBrakeTravelInput[1],
			data->ADCBrakeTravelInput[2], data->ADCBrakeTravelInput[3]);

	for (int i = 0; i < 100; i += 10) {

		//	int range =

		//	100/(data->ADCTorqueReqLInput[1] - data->ADCTorqueReqLInput[0]) * ;

		//	DebugPrintf(getTorqueReqPercR();
	}

	// Moved inverters after eeprom so that config value can be used.

	initInv();

	if (getEEPROMBlock(0)->alwaysHV) {
		DebugMsg("Shutdowncircuit Closed");
		ShutdownCircuitSet(true);
	} else {
		DebugMsg("Shutdowncircuit Open");
		ShutdownCircuitSet(false);
	}

	// after cubemx hardware inits, run our own initialisations to start up essential function.

	// should also read in defaults for calibrations, power levels etc.

// initButtons was here moved later, during startup sequence inputs were being triggered early

#ifdef SCROLLTEST
	char str[20];
	int i = 0;
	for ( i=0;i<10;i++ ){
		vTaskDelay(100);
		sprintf(str,"%.8d", i);
	}

	while ( 1 )
	{
		vTaskDelay(50);
	}
#endif

	vTaskDelay(200);

	return returnval;
}

/*
 * Primary operating function entered into after initialisation.
 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int realmain(void) {
	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	// init the watchdog before anything else so that it will catch any startup hang.

	DWT_Init();
	initWatchdog();

	xStartupSync = xEventGroupCreate();
	xCycleSync = xEventGroupCreate();

	MainTaskTaskHandle = xTaskCreateStatic(MainTask,
	MAINTASKTASKNAME,
	MAINTASKSTACK_SIZE, (void*) 1,
	MAINTASKTASKPRIORITY, xMAINTASKStack, &xMAINTASKTaskBuffer);

	vTaskStartScheduler();

	while (1) {
	};

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
