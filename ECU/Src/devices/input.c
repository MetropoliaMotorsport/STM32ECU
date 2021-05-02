/*
 * input.c
 *
 *  Created on: 14 Apr 2019
 *      Author: Visa
 */

#include "ecumain.h"
#include "input.h"

// PWM Pin needs capacitor taken off to deactivate low pass filter.

typedef struct ButtonData {
	GPIO_TypeDef * port;
	uint16_t pin;
	bool logic; // 0 for press on low, 1 for press on high, allow either pull low or high buttons to work. default is pull to ground.
	bool state; // current state
	uint32_t statecount; // how long this state has maintained.
	uint32_t lastpressed; // time stamp of last time state was determined to be an actual button press
	uint32_t lastreleased; // when was button last let go.
	uint32_t count; // how many times has it been pressed.
	bool pressed; // has button been pressed since last check.
	bool held; // is button being held down currently, for e.g. scrolling.
	// define the hardware button for passing button data including reading it
} ButtonData;


#ifdef HPF20
static volatile ButtonData Input[NO_INPUTS] = {
		{ DI2_GPIO_Port, DI2_Pin}, //0  pin 9
		{ DI3_GPIO_Port, DI3_Pin}, //1  pin 17
		{ DI4_GPIO_Port, DI4_Pin}, //2  pin 24
		{ DI5_GPIO_Port, DI5_Pin}, //3  pin 25
		{ DI6_GPIO_Port, DI6_Pin}, //4  pin 34 pin Also potential PWM Pin.
		{ NULL,0},//{ DI7_GPIO_Port, DI7_Pin}, //5 pin 33 being used for PWM.
		{ DI8_GPIO_Port, DI8_Pin}, //6
		{ DI10_GPIO_Port, DI10_Pin}, //7
		{ DI11_GPIO_Port, DI11_Pin}, //8
		{ DI13_GPIO_Port, DI13_Pin}, //9
		{ DI14_GPIO_Port, DI14_Pin}, //10
		{ DI15_GPIO_Port, DI15_Pin} //11
};
#endif

#ifdef HPF19
volatile ButtonData Input[NO_INPUTS] = {
	{ USER_Btn_GPIO_Port, USER_Btn_Pin},
	{ Input1_GPIO_Port, Input1_Pin},
	{ Input2_GPIO_Port, Input2_Pin},
	{ Input3_GPIO_Port, Input3_Pin},
	{ Input4_GPIO_Port, Input4_Pin},
	{ Input5_GPIO_Port, Input5_Pin},
	{ Input6_GPIO_Port, Input6_Pin},
	{ Input7_GPIO_Port, Input7_Pin},
	{ Input8_GPIO_Port, Input8_Pin}
};
#endif


#define InputQUEUE_LENGTH    20
#define InputITEMSIZE		sizeof( struct input_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t InputStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t InputQueueStorageArea[ InputQUEUE_LENGTH * InputITEMSIZE ];

QueueHandle_t InputQueue;


bool receiveCANInput(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle);

CANData CANButtonInput = { 0, AdcSimInput_ID+2, 8, receiveCANInput, NULL, 0, 0 };

// PWM Code.

/* Captured Value */
volatile uint32_t            uwIC2Value = 0;
/* Duty Cycle Value */
volatile uint32_t            uwDutyCycle = 0;
/* Frequency Value */
volatile uint32_t            uwFrequency = 0;

volatile uint32_t reading;
volatile uint32_t PWMtime;

osThreadId_t InputTaskHandle;
const osThreadAttr_t InputTask_attributes = {
  .name = "InputTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128*2
};

void InputTask(void *argument)
{
	TickType_t xLastWakeTime;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	while( 1 )
	{
		for ( int i = 0;i<NO_INPUTS;i++)
		{
			if ( Input[i].port == NULL) continue; // don't bother processing an input being used for PWM

			bool curstate = HAL_GPIO_ReadPin(Input[i].port, Input[i].pin );

#ifndef BUTTONPULLUP
			curstate = !curstate;
#else

#endif
			if ( curstate != Input[i].state) // current state of button has changed reset duration count.
			{
				Input[i].statecount = 0;
				Input[i].held = false;
				Input[i].state = curstate;
			}

			Input[i].statecount++;

			if ( Input[i].statecount == 0 ) Input[i].statecount = 0xFFFFFFFF; // don't allow wrap;
			else if ( Input[i].statecount == 3 ) // button held for long enough to count as a change of state, register it.
			{
				if ( curstate )
				{
					Input[i].pressed = true;
					Input[i].lastpressed=gettimer();
					Input[i].count++;
					// add button press to queue here.

				} else
				{
					Input[i].lastreleased=gettimer();
				}
			} else if ( Input[i].statecount > 3 )
			{
				if ( curstate )
				{
					Input[i].held = true;
				}
			}

		}

		vTaskDelayUntil( &xLastWakeTime, CYCLETIME );
	}

	osThreadTerminate(NULL);
}


int initPWM( void )
{

#ifdef PWMSTEERING
	MX_TIM8_Init(); // pwm
#endif

	if(HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_2) != HAL_OK)
	{
		Error_Handler();
	}

	if(HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}
	PWMtime = gettimer();

	DeviceState.PWM = OPERATIONAL;
	return 0;
}

/**
  * @brief  Input Capture callback in non blocking mode
  * @param  htim: TIM IC handle
  * @retval None
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if ( htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) // only get one update per loop?
  {
	PWMtime = gettimer(); // use for timing out if interrupt not getting triggered.

	if ( reading == 0 )
	{
		// this should be called ~ 8 times per loop
		/* Get the Input Capture value */
		uwIC2Value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

		if (uwIC2Value != 0)
		{
		  /* Duty cycle computation */
		  uwDutyCycle = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1) * 10000 / uwIC2Value;
		  /* uwFrequency computation
		  TIM4 counter clock = (RCC_Clocks.HCLK_Frequency) */
		  uwFrequency = (HAL_RCC_GetHCLKFreq()) / uwIC2Value / 5;

		  if ( uwFrequency > 850 || uwFrequency < 830 )
			  DeviceState.PWM = INERROR;
		  // ensure that frequenc is within expected range
		  else
			  reading = 1;
		}
		else
		{
		  uwDutyCycle = 0;
		  uwFrequency = 0;
		}
	}
  }
}

bool receivePWM( void )
{
	return (PWMtime > gettimer()-MS1*10 )? true : false;
}


int getPWMDuty( void ) // returns duty cycle as %*100
{
	reading = 0;
	return uwDutyCycle;
}

int checkReset( void )
{
#ifdef HPF19 // only got a UserBtn on Nucleo board.
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 1;
	}
#endif
	if ( checkConfigReset() )
	{
		return 1;
	}

	if(Input[StartStop_Input].pressed != 0){
			Input[StartStop_Input].pressed = 0;
	//		blinkOutput(TSOFFLED_Output,LEDBLINK_FOUR,1);
			return 1;
	}

	return 0;
}


int CheckBrakeBalRequest( void ) // this should be a push-hold, so not a single toggle read.
{
#ifdef ONBOARDBUTTON
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 0;
	}
#endif

	if(Input[StartStop_Input].pressed != 0){
			Input[StartStop_Input].pressed = 0;
	//		blinkOutput(TSOFFLED_Output,LEDBLINK_FOUR,1);
			return 1;
	}

	return 0;
}

int CheckButtonPressed( uint8_t In )
{
	if ( Input[In].pressed != 0 ){
			Input[In].pressed = 0;
			return 1;
	}
	return 0;
}

int GetUpDownPressed( void )
{
	if ( Input[Up_Input].pressed != 0 ){
			Input[Up_Input].pressed = 0;
			return -1;
	} else if ( Input[Down_Input].pressed != 0)
	{
		Input[Down_Input].pressed = 0;
		return 1;
	} else 	if ( Input[Up_Input].held != 0 ){
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

	return 0;
}


int GetLeftRightPressed( void )
{
	if(Input[Left_Input].pressed != 0){
			Input[Left_Input].pressed = 0;
			return -1;
	} else if(Input[Right_Input].pressed != 0)
	{
		Input[Right_Input].pressed = 0;
		return 1;
	} else
	return 0;
}


int CheckActivationRequest( void )
{
#ifdef ONBOARDBUTTON
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 0;
	}
#endif

	if(Input[StartStop_Input].pressed != 0){ //StartStop_Switch
			Input[StartStop_Input].pressed = 0;
	//		blinkOutput(TSOFFLED_Output,LEDBLINK_FOUR,1);
			return 1;
	}

	return 0;
}


int CheckLimpActivationRequest( void )
{
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


int CheckTSActivationRequest( void )
{
#ifdef ONBOARDBUTTON
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 0;
	}
#endif

	if(Input[TS_Input].pressed){
				Input[TS_Input].pressed = 0;
		//		blinkOutput(TSLED_Output,LEDBLINK_FOUR,1);
				return 1;
	}


	// could be a can message or other source of activation request also.

	// eg, in future for driverless.

	else return 0;
}


int CheckRTDMActivationRequest( void )
{
#ifdef ONBOARDBUTTON
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 0;
	}
#endif
	if(Input[RTDM_Input].pressed){
			Input[RTDM_Input].pressed = 0;
	//		blinkOutput(RTDMLED_Output,LEDBLINK_FOUR,1);
			return 1;
	}

	// could be a can message or other source of activation request also.

	// eg, in future for driverless.

	else return 0;
}

/**
 * @brief set status of a button to not activated
 */
void resetButton( uint8_t i )
{
	Input[i].lastpressed = 0;
	Input[i].lastreleased = 0;
	Input[i].count = 0;
	Input[i].pressed = false;
	Input[i].held = false;
}


void clearButtons(void)
{
	for ( int i=0; i < NO_INPUTS; i++)
	{
		Input[i].pressed = 0;
	}
}

bool receiveCANInput(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle)
{
	if(CANRxData[0] == 1){
		Input[StartStop_Input].pressed = 1;  // StartStop_Switch
		Input[StartStop_Input].lastpressed = gettimer();
	}
	if(CANRxData[1] == 1){
		Input[TS_Input].pressed = 1; // TS_Switch
		Input[TS_Input].lastpressed = gettimer();
	}
	if(CANRxData[2] == 1){
		Input[RTDM_Input].pressed = 1; // TS_Switch
		Input[RTDM_Input].lastpressed = gettimer();
	}

	switch ( CANRxData[3])
	{
	case 1 :
		Input[Center_Input].pressed = 1;
		Input[Center_Input].lastpressed = gettimer();
		break;
	case 2 :
		Input[Left_Input].pressed = 1;
		Input[Left_Input].lastpressed = gettimer();
		break;
	case 4 :
		Input[Right_Input].pressed = 1;
		Input[Right_Input].lastpressed = gettimer();
		break;
	case 8 :
		Input[Up_Input].pressed = 1;
		Input[Up_Input].lastpressed = gettimer();
		break;
	case 16 :
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


// TODO make stop button interrupt trigger to make more robust?
#ifndef RTOS

/**
 * @brief Interrupt handler fired when a button input line is triggered
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (!InButtonpress ) { // do nothing if already in middle of processing a previous button input
		// should ideally be able to manage multiple at once, with seperate timer channels, or timers.
		InButtonpress = 1; // stop processing further button presses till debounce time given
		ButtonpressPin = GPIO_Pin; // assign the button input being handled so it can be referenced in from timer interrupt

		HAL_StatusTypeDef status = HAL_TIM_Base_Start_IT(&htim7);
		if ( status != HAL_OK){
		//	Error_Handler();
			InButtonpress = 0;
		}
	}
}


void InputTimerCallback( void )
{
	  // timer7, being used for button input debouncing
		switch ( ButtonpressPin )
		{  // process the button that was pressed to start the debounce timer.
#ifdef HPF19
			case USER_Btn_Pin :
				debouncebutton(&Input[0]);
				break;
			case Input1_Pin:
				debouncebutton(&Input[1]);
				break;
			case Input2_Pin:
				debouncebutton(&Input[2]);
				break;
			case Input3_Pin:
				debouncebutton(&Input[3]);
				break;
			case Input4_Pin:
				debouncebutton(&Input[4]);
				break;
			case Input5_Pin:
				debouncebutton(&Input[5]);
				break;
			case Input6_Pin:
				debouncebutton(&Input[6]);
				break;
			case Input7_Pin:
				debouncebutton(&Input[7]);
				break;
			case Input8_Pin:
				debouncebutton(&Input[8]);
				break;
#endif

#ifdef HPF20
			case DI2_Pin:
				debouncebutton(&Input[DI2]);
				break;
			case DI3_Pin:
				debouncebutton(&Input[DI3]);
				break;
			case DI4_Pin:
				debouncebutton(&Input[DI4]);
				break;
			case DI5_Pin:
				debouncebutton(&Input[DI5]);
				break;
			case DI6_Pin:
				debouncebutton(&Input[DI6]);
				break;
#ifndef PWMSTEERING
			case DI7_Pin:
				debouncebutton(&Input[DI7]);
				break;
#endif
			case DI8_Pin:
				debouncebutton(&Input[DI8]);
				break;
			case DI10_Pin:
				debouncebutton(&Input[DI10]);
				break;
			case DI11_Pin:
				debouncebutton(&Input[DI11]);
				break;
			case DI13_Pin:
				debouncebutton(&Input[DI13]);
				break;
			case DI14_Pin:
				debouncebutton(&Input[DI14]);
				break;
			case DI15_Pin:
				debouncebutton(&Input[DI15]);
				break;
#endif
			default : // shouldn't get here, but catch and ignore any other input
				break;
		}

		// work around a bug? new cube mx version does not seem to clear busy flag on one shot timer.
		HAL_TIM_Base_Stop_IT(&htim7);
		InButtonpress = 0;  // reset status of button processing after timer elapsed,
		// to indicate not processing input to allow triggering new timer.
}
#endif

/**
 * startup routine to ensure all buttons are initialised to unset state.
 */
void resetInput( void )
{
	{
		for ( int i=0; i < NO_INPUTS; i++)
		{
			resetButton(i);
		}
	}
}

int initInput( void )
{
	InputQueue = xQueueCreateStatic( InputQUEUE_LENGTH,
							  InputITEMSIZE,
							  InputQueueStorageArea,
							  &InputStaticQueue );

	vQueueAddToRegistry(InputQueue, "InputQueue" );

	// make sure queue registered before the functions that could call it are used.

	RegisterResetCommand(resetInput);

	resetInput();

	RegisterCan1Message(&CANButtonInput);

	InputTaskHandle = osThreadNew(InputTask, NULL, &InputTask_attributes);

	return 0;
}
