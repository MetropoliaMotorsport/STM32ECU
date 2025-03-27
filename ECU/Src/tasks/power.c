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
#define FIXEDTEMP 0	 //To use the PI controllers, you need to change the value of FIXEDTEMP to 1.
#define Kp_fan 5.0   // Proportional coefficient for the fans
#define Ki_fan 0.1   // Integral coefficient for the fans

#define Kp_pump 7.0  // // Proportional coefficient for the pumps
#define Ki_pump 0.2  // Integral coefficient for the pumps
#define DT 0.1

float fan_integral = 0;
float pump_integral = 0;

void temp_ctl(){

	uint16_t WaterGoalTemp = 50;  // Target water temp (°C)
	uint16_t MaxGoalTemp = 70;    // Target temp for motor or inverter (°C)

	int16_t WaterTemp = CarState.WaterTemp;  // current water temp
	uint16_t InvTemp = CarState.InvTemp;      // current inverter temp
	int16_t MotorTemp = CarState.MotorTemp;  // current motor temp

	uint16_t MaxTemp = (MotorTemp > InvTemp) ? MotorTemp : InvTemp;

	/*uint16_t InvGoalTemp = 60;
	int16_t MotorGoalTemp = 70;
	uint8_t SetPWM = 0;*/

#if FIXEDTEMP

	//PI controller for fans (based on water temperature)
	float fan_error = WaterTemp - WaterGoalTemp;

	if (fan_error < 0) {
		fan_error = 0;
		fan_integral *= 0.9;
	} else {
		fan_integral += fan_error * DT;
	}

	if (fan_integral > 100) fan_integral = 100;

	float fan_PWM = (Kp_fan * fan_error) + (Ki_fan * fan_integral);

	if (fan_PWM > 100) fan_PWM = 100;
	if (fan_PWM < 10) fan_PWM = 10;


	//PI controller for pumps (based on MaxTemp temperature)
	float pump_error = MaxTemp - MaxGoalTemp;

	if (pump_error < 0) {
		pump_error = 0;
		pump_integral *= 0.9;
	} else {
		pump_integral += pump_error * DT;
	}

	if (pump_integral > 100) pump_integral = 100;

	float pump_PWM = (Kp_pump * pump_error) + (Ki_pump * pump_integral);

	if (pump_PWM > 100) pump_PWM = 100;
	if (pump_PWM < 30) pump_PWM = 30;

	setNodeDevicePWM(SideFans, (uint8_t)fan_PWM);
	setNodeDevicePWM(RightPump, (uint8_t)pump_PWM);
	setNodeDevicePWM(LeftPump, (uint8_t)pump_PWM);


	// The code in the comment section can be removed once the PI controllers are tested

	/*if (CarState.InvTemp < InvGoalTemp && CarState.MotorTemp < MotorGoalTemp)
	{
		SetPWM = 10;

		setNodeDevicePWM(SideFans, SetPWM);
		setNodeDevicePWM(RightPump, SetPWM * 3 );
		setNodeDevicePWM(LeftPump, SetPWM * 3) ;
	}
	else
	{
		int SetPWM1 = (CarState.InvTemp - InvGoalTemp) * 7;
		int SetPWM2 = (CarState.MotorTemp - MotorGoalTemp) * 15;

		SetPWM = SetPWM1 > SetPWM2 ? SetPWM1 : SetPWM2;

		SetPWM = SetPWM > 100 ? 100 : SetPWM;

		setNodeDevicePWM(SideFans, SetPWM);

		SetPWM = SetPWM + 15 > 90 ? 90 : SetPWM;
		setNodeDevicePWM(RightPump, SetPWM);
		setNodeDevicePWM(LeftPump, SetPWM) ;
	}*/

#endif
	
		setNodeDevicePWM(SideFans, 20);
		setNodeDevicePWM(RightPump, 90);
		setNodeDevicePWM(LeftPump, 90) ;

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

		//CheckDeviceState();

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
		//DebugMsg("BMS input PIN");
	}
	if (DeviceState.BMS != OPERATIONAL) {
		//DebugMsg("BMS NOT operational");
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
		//DebugMsg("IMD input PIN");
	}
	if (DeviceState.BMS != OPERATIONAL) {
		//DebugMsg("BMS NOT operational in IMD check");
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
	//DebugPrintf("Stopping buzzer\n");
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

