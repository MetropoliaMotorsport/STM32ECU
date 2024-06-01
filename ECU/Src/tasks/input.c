/*
 * input.c
 *
 *  Created on: 14 Apr 2019
 *      Author: Visa
 */

#include "ecumain.h"
#include "input.h"
#include "timerecu.h"
#include "configuration.h"
#include "tim.h"
#include "debug.h"
#include "taskpriorities.h"

// PWM Pin needs capacitor taken off to deactivate low pass filter.

typedef struct ButtonData {
	GPIO_TypeDef *port;
	uint16_t pin;
	bool logic; // 0 for press on low, 1 for press on high, allow either pull low or high buttons to work. default is pull to ground.
	bool state; // current state
	uint32_t statecount; // how long this state has maintained.
	uint32_t lastpressed; // time stamp of last time state was determined to be an actual button press
	uint32_t lastreleased; // when was button last let go.
	uint32_t count; // how many times has it been pressed.
	bool pressed; // has button been pressed since last check.
	uint32_t held; // is button being held down currently, for e.g. scrolling.
	bool first;
// define the hardware button for passing button data including reading it
} ButtonData;

#ifdef HPF20
static volatile ButtonData Input[NO_INPUTS] = { { DI2_GPIO_Port, DI2_Pin }, //0  pin 9
		{ DI3_GPIO_Port, DI3_Pin }, //1  pin 17
		{ DI4_GPIO_Port, DI4_Pin }, //2  pin 24
		{ DI5_GPIO_Port, DI5_Pin }, //3  pin 25
		{ NULL, 0 }, //4  pin 34 pin Also potential PWM Pin.

#ifdef HPF2023
		{ NULL, 0 }, // DI7 used for wheel input interrupt signal
#else
		{ DI7_GPIO_Port, DI7_Pin},//{ DI7_GPIO_Port, DI7_Pin}, //5 pin 33 being used for PWM.
#endif
		{ DI8_GPIO_Port, DI8_Pin }, //6 // PWM signal.
		{ DI10_GPIO_Port, DI10_Pin }, //7
		{ DI11_GPIO_Port, DI11_Pin }, //8
		{ DI13_GPIO_Port, DI13_Pin }, //9
		{ DI14_GPIO_Port, DI14_Pin }, //10
		{ DI15_GPIO_Port, DI15_Pin } //11
};
#endif

#define InputQUEUE_LENGTH    20
#define InputITEMSIZE		sizeof( struct input_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t InputStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
 uxQueueLength * uxItemSize bytes. */
uint8_t InputQueueStorageArea[InputQUEUE_LENGTH * InputITEMSIZE];

QueueHandle_t InputQueue;

bool receiveCANInput(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle);

CANData CANButtonInput =
		{ 0, AdcSimInput_ID + 2, 8, receiveCANInput, NULL, 0, 0 };

// PWM Code.

/* Captured Value */
volatile uint32_t uwIC2Value = 0;
/* Duty Cycle Value */
volatile uint32_t uwDutyCycle = 0;
/* Frequency Value */
volatile uint32_t uwFrequency = 0;

volatile uint32_t reading;
volatile uint32_t PWMtime;

#define INPUTSTACK_SIZE 128*2
#define INPUTTASKNAME  "InputTask"
StaticTask_t xINPUTTaskBuffer;
StackType_t xINPUTStack[INPUTSTACK_SIZE];

TaskHandle_t InputTaskHandle;

void InputTask(void *argument) {
	xEventGroupSync(xStartupSync, 0, 1, portMAX_DELAY);

	TickType_t xLastWakeTime;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	while (1) {
		for (int i = 0; i < NO_INPUTS; i++) {
			if (Input[i].port == NULL)
				continue; // don't bother processing an input being used for PWM

			bool curstate = HAL_GPIO_ReadPin(Input[i].port, Input[i].pin);

//#define BUTTONPULLUP
			curstate = !curstate;

			if (curstate != Input[i].state) // current state of button has changed reset duration count.
					{
				Input[i].statecount = 0;
				Input[i].held = 0;
				Input[i].state = curstate;
			}

			Input[i].statecount++;

			if (Input[i].statecount == 0)
				Input[i].statecount = 0xFFFFFFFF; // don't allow wrap;
			else if (Input[i].statecount == 3) // button held for long enough to count as a change of state, register it.
					{
				if (Input[i].first == true) {
					Input[i].first = false;
				} else {
					char str[40];
					if (curstate) {
						snprintf(str, 40, "Button %d pressed (%lu)", i,
								gettimer());
						DebugMsg(str);

						Input[i].pressed = true;
						Input[i].lastpressed = gettimer();
						Input[i].count++;
						// add button press to queue here.

					} else {
						snprintf(str, 40, "Button %d released (%lu)", i,
								gettimer());
						DebugMsg(str);
						Input[i].lastreleased = gettimer();
					}
				}
			} else if (Input[i].statecount > 3) {
				if (curstate) {
					Input[i].held += 1; // add one for each cycle held.
				}
			}

		}

		// TODO add a reset procedure via long button press as another last resort.

		vTaskDelayUntil(&xLastWakeTime, CYCLETIME);
	}

	vTaskDelete(NULL);
}


int checkReset(void) {
	if (checkConfigReset()) {
		return 1;
	}

	if (Input[StartStop_Input].pressed != 0) {
		Input[StartStop_Input].pressed = 0;
		//		blinkOutput(TSOFFLED_Output,LEDBLINK_FOUR,1);
		return 1;
	}

	return 0;
}

int CheckBrakeBalRequest(void) // this should be a push-hold, so not a single toggle read.
{
#ifdef ONBOARDBUTTON
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 0;
	}
#endif

	if (Input[StartStop_Input].pressed != 0) {
		Input[StartStop_Input].pressed = 0;
		//		blinkOutput(TSOFFLED_Output,LEDBLINK_FOUR,1);
		return 1;
	}

	return 0;
}

int CheckButtonPressed(uint8_t In) {
	if (Input[In].pressed != 0) {
		Input[In].pressed = 0;
		return 1;
	}
	return 0;
}

int GetUpDownPressed(void) {
	if (Input[Up_Input].pressed != 0) {
		Input[Up_Input].pressed = 0;
		return -1;
	} else if (Input[Down_Input].pressed != 0) {
		Input[Down_Input].pressed = 0;
		return 1;
	}
#if 0
	else 	if ( Input[Up_Input].held != 0 ){
		if (gettimer() - Input[Up_Input].lastpressed > REPEATRATE )
		{
			Input[Up_Input].lastpressed = gettimer();
			return -1;
		}
	} else 	if ( Input[Down_Input].held != 0 ){
		if (gettimer() - Input[Down_Input].lastpressed > REPEATRATE )
		{
			Input[Down_Input].lastpressed = gettimer();
			return 1;
		}
	}
#endif

	return 0;
}

void setInput(input input) {
	Input[input].pressed = true;
}

int GetLeftRightPressed(void) {
	if (Input[Left_Input].pressed != 0) {
		Input[Left_Input].pressed = 0;
		return -1;
	} else if (Input[Right_Input].pressed != 0) {
		Input[Right_Input].pressed = 0;
		return 1;
	} else
		return 0;
}

int CheckActivationRequest(void) {
#ifdef ONBOARDBUTTON
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 0;
	}
#endif

	if (Input[StartStop_Input].pressed != 0) { //StartStop_Switch
		Input[StartStop_Input].pressed = 0;
		//		blinkOutput(TSOFFLED_Output,LEDBLINK_FOUR,1);
		return 1;
	}

	return 0;
}

int CheckLimpActivationRequest(void) {
#ifdef ONBOARDBUTTON
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 0;
	}
#endif

	// driven by BMS.

	// could be a can message or other source of activation request also.

	// eg, in future for driverless.

	return 0;
}

int CheckTSActivationRequest(void) {
#ifdef ONBOARDBUTTON
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 0;
	}
#endif

	if (Input[TS_Input].pressed) {
		Input[TS_Input].pressed = 0;
		//		blinkOutput(TSLED_Output,LEDBLINK_FOUR,1);
		return 1;
	}

	// could be a can message or other source of activation request also.

	// eg, in future for driverless.

	else
		return 0;
}

int CheckRTDMActivationRequest(void) {
#ifdef ONBOARDBUTTON
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 0;
	}
#endif
	if (Input[RTDM_Input].pressed) {
		Input[RTDM_Input].pressed = 0;
		//		blinkOutput(RTDMLED_Output,LEDBLINK_FOUR,1);
		return 1;
	}

	// could be a can message or other source of activation request also.

	// eg, in future for driverless.

	else
		return 0;
}

/**
 * @brief set status of a button to not activated
 */
void resetButton(uint8_t i) {
	Input[i].lastpressed = 0;
	Input[i].lastreleased = 0;
	Input[i].count = 0;
	Input[i].pressed = false;
	Input[i].held = false;
	Input[i].first = true;
}

void clearButtons(void) {
	for (int i = 0; i < NO_INPUTS; i++) {
		Input[i].pressed = 0;
	}
}

bool receiveCANInput(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle) {
	if (CANRxData[0] == 1) {
		Input[StartStop_Input].pressed = 1;  // StartStop_Switch
		Input[StartStop_Input].lastpressed = gettimer();
	}
	if (CANRxData[1] == 1) {
		Input[TS_Input].pressed = 1; // TS_Switch
		Input[TS_Input].lastpressed = gettimer();
	}
	if (CANRxData[2] == 1) {
		Input[RTDM_Input].pressed = 1; // TS_Switch
		Input[RTDM_Input].lastpressed = gettimer();
	}

	switch (CANRxData[3]) {
	case 1:
		Input[Center_Input].pressed = 1;
		Input[Center_Input].lastpressed = gettimer();
		break;
	case 2:
		Input[Left_Input].pressed = 1;
		Input[Left_Input].lastpressed = gettimer();
		break;
	case 4:
		Input[Right_Input].pressed = 1;
		Input[Right_Input].lastpressed = gettimer();
		break;
	case 8:
		Input[Up_Input].pressed = 1;
		Input[Up_Input].lastpressed = gettimer();
		break;
	case 16:
		Input[Down_Input].pressed = 1;
		Input[Down_Input].lastpressed = gettimer();
		break;
	}

#ifdef debug
//REMOVE FROM LIVE CODE.
	if(CANRxData[6] == 0xFE){
// debug id to induce a hang state, for testing watchdog.
		while ( 1 ){
			// do nothing, don't return.
		}
	}
#endif
	return true;
}
#ifdef HPF2023
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == WHLINT_Pin) {
//		wheel_interrupt();
	}
}
#endif

// TODO make stop button interrupt trigger to make more robust?

/**
 * startup routine to ensure all buttons are initialised to unset state.
 */
void resetInput(void) {
	// TODO send flag to reset input task itself.
	{
		for (int i = 0; i < NO_INPUTS; i++) {
			resetButton(i);
		}
	}
}

int initInput(void) {
	InputQueue = xQueueCreateStatic(InputQUEUE_LENGTH, InputITEMSIZE,
			InputQueueStorageArea, &InputStaticQueue);

	vQueueAddToRegistry(InputQueue, "InputQueue");

	// make sure queue registered before the functions that could call it are used.

	RegisterResetCommand(resetInput);

	resetInput();

	RegisterCan1Message(&CANButtonInput);

	InputTaskHandle = xTaskCreateStatic(InputTask,
	INPUTTASKNAME,
	INPUTSTACK_SIZE, (void*) 1,
	INPUTTASKPRIORITY, xINPUTStack, &xINPUTTaskBuffer);

	return 0;
}
