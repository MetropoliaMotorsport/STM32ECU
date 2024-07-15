/*
 * power.c
 *
 *  Created on: Jul 17, 2020
 *      Author: Visa
 */

#include "ecumain.h"
#include "limits.h"
#include "task.h"
#include "power.h"
#include "powerloss.h"
#include "powernode.h"
#include "errors.h"
#include "eeprom.h"
#include "inverter.h"
#include "taskpriorities.h"
#include "debug.h"
#include "timerecu.h"
#include "semphr.h"
#include "output.h"
#include "input.h"

TaskHandle_t PowerTaskHandle = NULL;

#define POWERSTACK_SIZE 128*6
#define PowerITEMSIZE		sizeof( Power_msg )
#define POWERTASKNAME  "PowerTask"
StaticTask_t xPOWERTaskBuffer;
StackType_t xPOWERStack[POWERSTACK_SIZE];

#define PowerQUEUE_LENGTH    20
#define PowerErrorQUEUE_LENGTH    20
#define PowerErrorITEMSIZE		sizeof( Power_Error_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t PowerStaticQueue;
static StaticQueue_t PowerErrorStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
 uxQueueLength * uxItemSize bytes. */
uint8_t PowerQueueStorageArea[PowerQUEUE_LENGTH * PowerITEMSIZE];
uint8_t PowerErrorQueueStorageArea[PowerErrorQUEUE_LENGTH * PowerErrorITEMSIZE];

QueueHandle_t PowerQueue, PowerErrorQueue;

bool HVLost;

// task shall take power handling request, and forward them to nodes.
// ensure contact is kept with brake light board as brake light is SCS.

// how to ensure power always enabled?

static SemaphoreHandle_t waitStr = NULL;

char PNodeWaitStr[20] = "";
uint32_t curpowernodesOnline = 0;
/*
Function makes sure that devices are in the state they are expected to be in.
*/
void CheckDeviceState(){
	for(int i = 0; i < DEVICE_COUNT; i++) {
		if (&DevicePowerList[i] != NULL){
			if (DevicePowerList[i].expectedstate != DevicePowerList[i].actualstate) {
				setNodeDevicePower(DevicePowerList[i].device, DevicePowerList[i].expectedstate, false);
				vTaskDelay(1);
			}
		}
	}
}

void temp_ctl(){
	uint16_t GoalTemp = 30;
	uint8_t SetPWM = 0;

	if (CarState.InvTemp < GoalTemp) 
	{
		SetPWM = 10;

		setNodeDevicePWM(SideFans, SetPWM);
		setNodeDevicePWM(RightPump, SetPWM * 3 );
		setNodeDevicePWM(LeftPump, SetPWM * 3) ;
	}
	else
	{
		SetPWM = (CarState.InvTemp - GoalTemp) * 5;
		SetPWM = SetPWM > 100 ? 100 : SetPWM;

		setNodeDevicePWM(SideFans, SetPWM);
		
		SetPWM = SetPWM + 15 > 100 ? 100 : SetPWM;
		setNodeDevicePWM(RightPump, SetPWM);
		setNodeDevicePWM(LeftPump, SetPWM) ;
	}
	
}
	

uint32_t PowerReceived = 0;

void PowerTask(void *argument) {
	xEventGroupSync(xStartupSync, 0, 1, portMAX_DELAY);

	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT(PowerQueue);


	resetPowerLost();
	xQueueReset(PowerErrorQueue);

	uint32_t powernodesOnline = 0;
	uint32_t lastpowernodesOnline = 0;
	uint32_t powernodesOnlineSince = 0;
	uint32_t count = 0;
	uint32_t lastseenHV = 0;
	uint32_t lastseenpumpR = 0;
	uint32_t lastseenpumpL = 0;
	uint32_t restartpumpR = 0;
	uint32_t restartpumpL = 0;
	bool HVactive = false;

	uint32_t lastseenall = 0;

	uint32_t oldestcritical = 0;

	bool fanssent = false;

	bool LVdown = false;
	uint32_t LVdowntime = 0;

	bool IMDset = false;
	bool TSOFFset = true;
	bool BMSset = false;

	while(1){

		CheckDeviceState();

		temp_ctl();


		xEventGroupSync(xCycleSync, 0, 1, portMAX_DELAY); // wait for main cycle.
		// after synced, send current state for next cycle. Higher priority task, so should be received first.
		xTaskNotifyWait( pdFALSE, ULONG_MAX, &PowerReceived, 0);
	}


	vTaskDelete(NULL);
}


bool CheckBMS(void) // returns true if shutdown circuit other than ECU is closed
{

	if (HAL_GPIO_ReadPin(BMS_Input_Port, BMS_Input_Pin)) {
		DebugMsg("BMS input PIN");
	}
	if (DeviceState.BMS != OPERATIONAL) {
		DebugMsg("BMS NOT operational");
	}
	return (!(HAL_GPIO_ReadPin(BMS_Input_Port, BMS_Input_Pin)
			|| DeviceState.BMS != OPERATIONAL));

}

bool CheckTSOff(void) // returns true if shutdown circuit other than ECU is closed
{
	//return Shutdown.TS_OFF;
}

bool CheckIMD(void) // returns true if shutdown circuit other than ECU is closed
{

	if (HAL_GPIO_ReadPin(IMD_Input_Port, IMD_Input_Pin)) {
		DebugMsg("IMD input PIN");
	}
	if (DeviceState.BMS != OPERATIONAL) {
		DebugMsg("BMS NOT operational in IMD check");
	}
	return (HAL_GPIO_ReadPin(IMD_Input_Port, IMD_Input_Pin)
			|| DeviceState.BMS != OPERATIONAL);
}

#define MAXSHUTDOWNSTR	40


void ShutdownCircuitSet( bool state) {
	HAL_GPIO_WritePin( DO15_GPIO_Port, DO15_Pin, state);
	HAL_GPIO_WritePin( Shutdown_GPIO_Port, Shutdown_Pin, state);
}

int ShutdownCircuitState(void) {
	return HAL_GPIO_ReadPin(Shutdown_GPIO_Port, Shutdown_Pin);
}

xTimerHandle timerHndlBuzzer;

bool soundBuzzer(void) {
	setNodeDevicePower(Buzzer, true, false);
	xTimerStart(timerHndlBuzzer, 0);
	return true;
}

static void stopBuzzer(xTimerHandle pxTimer) {
	DebugPrintf("Stopping buzzer\n");
	setNodeDevicePower(Buzzer, false, false);
}

int initPower(void) {

	HVLost = false;

	waitStr = xSemaphoreCreateMutex();

	PowerQueue = xQueueCreateStatic(PowerQUEUE_LENGTH, PowerITEMSIZE,
			PowerQueueStorageArea, &PowerStaticQueue);

	vQueueAddToRegistry(PowerQueue, "PowerQueue");

	PowerErrorQueue = xQueueCreateStatic(PowerErrorQUEUE_LENGTH,
			PowerErrorITEMSIZE, PowerErrorQueueStorageArea,
			&PowerErrorStaticQueue);

	vQueueAddToRegistry(PowerErrorQueue, "PowerErrorQueue");

	timerHndlBuzzer = xTimerCreate("buzzertimer", /* name */
	pdMS_TO_TICKS(1000), /* period/time */
	pdFALSE, /* auto reload */
	(void*) 0, /* timer ID */
	stopBuzzer); /* callback */

	PowerTaskHandle = xTaskCreateStatic(PowerTask,
	POWERTASKNAME,
	POWERSTACK_SIZE, (void*) 1,
	POWERTASKPRIORITY, xPOWERStack, &xPOWERTaskBuffer);

	initPowerLossHandling();

	return 0;
}

