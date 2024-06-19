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
#define POWERTASKNAME  "PowerTask"
StaticTask_t xPOWERTaskBuffer;
StackType_t xPOWERStack[POWERSTACK_SIZE];

#define PowerQUEUE_LENGTH    20
#define PowerErrorQUEUE_LENGTH    20
#define PowerITEMSIZE		sizeof( Power_msg )
#define PowerErrorITEMSIZE		sizeof( Power_Error_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t PowerStaticQueue;
static StaticQueue_t PowerErrorStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
 uxQueueLength * uxItemSize bytes. */
uint8_t PowerQueueStorageArea[PowerQUEUE_LENGTH * PowerITEMSIZE];
uint8_t PowerErrorQueueStorageArea[PowerErrorQUEUE_LENGTH * PowerErrorITEMSIZE];

QueueHandle_t PowerQueue, PowerErrorQueue;

ShutdownState Shutdown;

bool HVLost;

// task shall take power handling request, and forward them to nodes.
// ensure contact is kept with brake light board as brake light is SCS.

// how to ensure power always enabled?

static SemaphoreHandle_t waitStr = NULL;

char PNodeWaitStr[20] = "";
uint32_t curpowernodesOnline = 0;


void ClearHVLost(void) {
	if (HVLost) {
		HVLost = false;
		DebugMsg("Clearing HV Lost");
	}
}

bool CheckHVLost(void) {
	// if ( Shutdown.AIRm == 0 || Shutdown.AIRp == 0 || Shutdown.TS_OFF || HVLost ||
	if (CarState.VoltageINV < 60) {
		return true;
	}
	return false;
}

void PowerTask(void *argument) {
	xEventGroupSync(xStartupSync, 0, 1, portMAX_DELAY);

	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT(PowerQueue);

	Power_msg msg;
	Power_Error_msg errormsg;

	char str[MAXERROROUTPUT];

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
	return Shutdown.TS_OFF;
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

// request a power state for a device.
bool setDevicePower(DevicePower device, bool state) {
	Power_msg msg;

	msg.cmd = DirectPowerCmd;
	msg.power = device;
	msg.enabled = state;
	return (xQueueSend(PowerQueue, &msg, 0));

}

bool getDevicePower(DevicePower device) {
	return getNodeDevicePower(device);
}

// reset a device's power channel
bool resetDevicePower(DevicePower device) {
	Power_msg msg;

	msg.cmd = PowerErrorReset;
	msg.power = device;
	msg.enabled = false;
	return (xQueueSend(PowerQueue, &msg, 0));

}


void FanPWMControl(uint8_t leftduty, uint8_t rightduty) {
//	for example: [7][2][5][255][128] will set the lowest numbered output (DI3)
//to have a 100% duty cycle and the third output (DI5) to have a 50% duty cycle (if configured as PWM output)
	Power_msg msg;

	msg.cmd = FanPowerCmd;
	msg.PWMLeft = leftduty;
	msg.PWMRight = rightduty;
	xQueueSend(PowerQueue, &msg, 0);

}


xTimerHandle timerHndlBuzzer;

bool soundBuzzer(void) {
	DebugPrintf("Sounding buzzer\n");
	resetDevicePower(Buzzer);
	setDevicePower(Buzzer, true);
	xTimerStart(timerHndlBuzzer, 0);
	return true;
}

static void stopBuzzer(xTimerHandle pxTimer) {
	DebugPrintf("Stopping buzzer\n");
	setDevicePower(Buzzer, false);
}

bool getPowerHVReady(void) {
	return true; // TODO implement rather than dummy.
}

bool PowerLogError(uint8_t nodeid, uint32_t errorcode) {
	Power_Error_msg msg;

	msg.nodeid = nodeid;
	msg.error = errorcode;

	if (xPortIsInsideInterrupt())
		return xQueueSendFromISR(PowerErrorQueue, &msg, NULL);
	else
		return (xQueueSend(PowerErrorQueue, &msg, 0));
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

