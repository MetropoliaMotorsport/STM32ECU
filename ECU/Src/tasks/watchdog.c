/*
 * watchdog.c
 *
 *      Author: Visa
 *
 *      Last resort, if something like a stack overflow happens
 *      and critical thread stops running, should reboot the board
 *      and allow indicating to driver an error happened.
 *
 */

#include "ecumain.h"
#include "watchdog.h"
#include "errors.h"
#include "wwdg.h"
#include "event_groups.h"
#include "semphr.h"
#include "taskpriorities.h"
#include "debug.h"

#define MAXWATCHDOGTASKS			10
#define MAXWATCHDOGTASKNAMELENGTH 	20

#define WATCHDOGSTACK_SIZE 128*4
#define WATCHDOGTASKNAME  "WatchdogTask"
#define WATCHDOGTASKPRIORITY 1
StaticTask_t xWATCHDOGTaskBuffer;
StackType_t xWATCHDOGStack[WATCHDOGSTACK_SIZE];

TaskHandle_t WatchdogTaskHandle = NULL;

bool watchdogreboot = false;

EventGroupHandle_t xWatchdog, xWatchdogActive;
SemaphoreHandle_t debugAllocate;

struct watchdogTaskInfo {

};

char watchdogTasks[MAXWATCHDOGTASKS][MAXWATCHDOGTASKNAMELENGTH] = { 0 };

void setWatchdogBit(uint8_t bit) {
	xEventGroupSetBits(xWatchdog, (1 << bit));
}

uint8_t registerWatchdogBit(char *taskname) {
	// grab semaphore to prevent another task being able to conflict with assignment during startup if a context switch occurs.
	xSemaphoreTake(debugAllocate, portMAX_DELAY);
	uint8_t allocation = 0;

	if (taskname[0] == 0) {
		LogError("Must give a taskname to watchdog");

		while (1) {
// tried to allocate too many watchdog bits, force a hang, which will cause a reset.

		}
	}

	// find first non zero allocation, or a pre existing one matching name.
	while ((watchdogTasks[allocation][0] != 0
			&& strncmp(taskname, watchdogTasks[allocation],
					MAXWATCHDOGTASKNAMELENGTH)) != 0
			&& allocation < MAXWATCHDOGTASKS) {
		allocation++;
	}

	// make sure we are within allocation size.
	if (allocation >= MAXWATCHDOGTASKS) {
		LogError("Too many watchdog tasks");

		while (1) {
// tried to allocate too many watchdog bits, force a hang, which will cause a reset.

		}
	}
	// copy the taskname into buffer
	strncpy(watchdogTasks[allocation], taskname, MAXWATCHDOGTASKNAMELENGTH);

	xEventGroupSetBits(xWatchdogActive, (1 << allocation));

	setWatchdogBit(allocation);
	xSemaphoreGive(debugAllocate);
	return allocation;
}

bool watchdogRebooted(void) {
	return watchdogreboot;
}

/**
 * @brief  Timeout calculation function.
 *         This function calculates any timeout related to
 *         WWDG with given prescaler and system clock.
 * @param  timevalue: period in term of WWDG counter cycle.
 * @retval None
 */
static uint32_t TimeoutCalculation(uint32_t timevalue) {
	uint32_t timeoutvalue = 0;
	uint32_t pclk1 = 0;
	uint32_t wdgtb = 0;

	/* Get PCLK1 value */
	pclk1 = HAL_RCC_GetPCLK1Freq();

	/* get prescaler */
	switch (hwwdg1.Init.Prescaler) {
	case WWDG_PRESCALER_1:
		wdgtb = 1;
		break;
	case WWDG_PRESCALER_2:
		wdgtb = 2;
		break;
	case WWDG_PRESCALER_4:
		wdgtb = 4;
		break;
	case WWDG_PRESCALER_8:
		wdgtb = 8;
		break;
	case WWDG_PRESCALER_16:
		wdgtb = 16;
		break;
	case WWDG_PRESCALER_32:
		wdgtb = 32;
		break;
	case WWDG_PRESCALER_64:
		wdgtb = 64;
		break;
	case WWDG_PRESCALER_128:
		wdgtb = 128;
		break;

	default:
		Error_Handler();
		break;
	}

	/* calculate timeout */
	timeoutvalue = ((4096 * wdgtb * timevalue) / ((pclk1 * 2) / 1000)); // not sure if right.

	return timeoutvalue;
}

static void WatchdogTask(void *pvParameters) {
//	xEventGroupSync( xStartupSync, 0, 1, portMAX_DELAY );
	volatile int count = 0;

	vTaskDelay(CYCLETIME * 2);

	while (1) {
		count++;

		volatile EventBits_t activeBits = xEventGroupGetBits(xWatchdogActive);

		volatile EventBits_t uxBits = xEventGroupGetBits(xWatchdog);

		if ((uxBits & activeBits) == (activeBits)) {
			// only kick the watchdog if all expected bits are set.
#ifdef WATCHDOG
			HAL_WWDG_Refresh(&hwwdg1);
#endif
		} else {
			if (activeBits > 0) {
				volatile int watchdognotkicked = 1;
				char str[40] = "";
				snprintf(str, 40, "Watchdog kicked bits: %4X",
						(uint32_t) uxBits);
#ifdef WATCHDOG
				UART_Transmit(DEBUGUART, str, strlen(str));
#else
				DebugMsg(str);
#endif
			}
		}

		xEventGroupClearBits(xWatchdog, 0xFF);

		vTaskDelay(CYCLETIME * 2);
	}
}

int initWatchdog(void) {

	for (int i = 0; i < MAXWATCHDOGTASKS; i++)
		watchdogTasks[i][0] = 0;

	// TODO allocate static?
	xWatchdog = xEventGroupCreate();
	xWatchdogActive = xEventGroupCreate();

	debugAllocate = xSemaphoreCreateMutex();

#ifdef WATCHDOG

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

        ~0.546

	  b) WWDG timeout (in ms) = (counter + 1) * period

	  ~17ms

	  c) Time to enter inside window
	  Window timeout (in ms) = (counter - window + 1) * period

 */

	hwwdg1.Instance = WWDG1;

	if ( CYCLETIME == 20 )
	{
		hwwdg1.Init.Prescaler = WWDG_PRESCALER_32; // approx ~68ms for full 63 unit window.
		hwwdg1.Init.Window    = 75; // approx 35ms
		hwwdg1.Init.Counter   = 106; // approx 45ms, giving +/- 5ms on 40ms to kick WD.∑
	}
	else
	{
		hwwdg1.Init.Prescaler = WWDG_PRESCALER_16;
		hwwdg1.Init.Window    = 96;
		hwwdg1.Init.Counter   = 127;
	}


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
#endif

	// min 8 // should now be double.
	// max 16

#ifdef TESTWATCHDOGTIMING
	volatile uint32_t windowend = TimeoutCalculation((hwwdg1.Init.Counter) + 1);
	volatile uint32_t windowstart = windowend/2+TimeoutCalculation((hwwdg1.Init.Counter-hwwdg1.Init.Window) - 1);


	volatile int counter = 0;

	volatile int wdtime = 32;

	while ( 1 )
	{
		counter++;
		DWT_Delay(36*1000); // ensure that first trigger will be within window.
		HAL_WWDG_Refresh(&hwwdg1);
//		DebugPrintf("Watchdog call %d time: %d", counter, wdtime );

	}
#endif

	WatchdogTaskHandle = xTaskCreateStatic(WatchdogTask,
	WATCHDOGTASKNAME,
	WATCHDOGSTACK_SIZE, (void*) 1,
	WATCHDOGTASKPRIORITY, xWATCHDOGStack, &xWATCHDOGTaskBuffer);

	return 0;
}
