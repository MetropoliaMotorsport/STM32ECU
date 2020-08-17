/*
 * output.c
 *
 *  Created on: 14 Apr 2019
 *      Author: drago
 */

#include "ecumain.h"

/**
 * returns gpio port for given output number.
 */

osThreadId_t OutputTaskHandle;
const osThreadAttr_t OutputTask_attributes = {
  .name = "OutputTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128*2
};

/* The queue is to be created to hold a maximum of 10 uint64_t
variables. */
#define OutputQUEUE_LENGTH    20
#define OutputITEMSIZE		sizeof( struct output_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t OutputStaticQueue;


/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t OutputQueueStorageArea[ OutputQUEUE_LENGTH * OutputITEMSIZE ];

QueueHandle_t OutputQueue;


typedef volatile struct BlinkParam
{
    output output;
    output_state state;
    uint16_t ontime;
    uint16_t offtime;
    uint32_t start;
    uint32_t duration;
} BlinkParam;


BlinkParam BlinkParams[OUTPUTCount] = { 0 };

#define BLINKSTACK_SIZE 32

StaticTask_t xBlinkTaskBuffer[OUTPUTCount];

TaskHandle_t xBlinkHandle[OUTPUTCount] = { NULL };


StackType_t xStack[OUTPUTCount][ BLINKSTACK_SIZE ];


void BlinkTask( void * pvParameters )
{
	BlinkParam * params = (BlinkParam *) pvParameters;

	while ( 1 )
    {
		bool terminate = false;
		if ( params->duration > 0 )
		{
			HAL_GPIO_WritePin(getGpioPort(params->output), getGpioPin(params->output), On);
			vTaskDelay(params->ontime);
			if ( params->offtime > 0 )
			{
				HAL_GPIO_WritePin(getGpioPort(params->output), getGpioPin(params->output), Off);
				vTaskDelay(params->offtime);
			}
			if ( params->duration == 1 ) // if duration set to 1, only do blink cycle once.
			{
				terminate = true;
			}
			if ( params->duration != 0xFFFF )
			{
				if ( gettimer() - params->start > params->duration )
				{
					terminate = true;
				}
			}
		} else
			HAL_GPIO_WritePin(getGpioPort(params->output), getGpioPin(params->output), params->state);

		if ( terminate )
		{
			HAL_GPIO_WritePin(getGpioPort(params->output), getGpioPin(params->output), params->state);
			params->duration = 0;
			osThreadTerminate(NULL);
		}
    }

	osThreadTerminate(NULL);
}

void OutputTask(void *argument)
{

	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT( OutputQueue );

	TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

//	unsigned long counter;

	struct output_msg msg;

	while( 1 )
	{

		while ( uxQueueMessagesWaiting( OutputQueue ) )
		{
        // UBaseType_t uxQueueMessagesWaiting( QueueHandle_t xQueue );
			if ( xQueueReceive(OutputQueue,&msg,0) )
			{
				if ( msg.state == Toggle )
				{
					toggleOutputMetal(msg.output);
				} else if ( msg.state > On )
				{

					BlinkParams[msg.output].output = msg.output;

					int ontime = 0;
					int offtime = 0;

					switch ( msg.state )
					{
						case BlinkVerySlow : ontime = 1000; offtime = 1000; break;
						case BlinkSlow : ontime = 500; offtime = 500; break;
						case BlinkMed : ontime = 200; offtime = 200; break;
						case BlinkFast : ontime = 50; offtime = 50; break;
						case BlinkVeryFast : ontime = 10; offtime = 10; break;
						case Timed : ontime = 10; offtime = 0; break;
						default :
						ontime = 500; offtime = 500;
					}

					BlinkParams[msg.output].ontime = ontime;
					BlinkParams[msg.output].offtime = offtime;
					BlinkParams[msg.output].start = gettimer();

					if ( msg.time != 1)
					{
						if ( msg.time < ontime+offtime ) // ensure that one complete cycle will show
							msg.time = ontime+offtime;
					}

					BlinkParams[msg.output].duration = msg.time;

					// only create blink task when it's actually needed for first time

					// should keep task list a little cleaner if only led's called to blink actually get populated.

					if ( xBlinkHandle[msg.output] == NULL && msg.time > 0 )
					{
						xBlinkHandle[msg.output]  = xTaskCreateStatic(
									  BlinkTask,       /* Function that implements the task. */
									  "BlinkTask",     /* Text name for the task. */
									  BLINKSTACK_SIZE,      /* Number of indexes in the xStack array. */
									  ( void * ) &BlinkParams[msg.output],    /* Parameter passed into the task. */
									  tskIDLE_PRIORITY,/* Priority at which the task is created. */
									  xStack[msg.output],          /* Array to use as the task's stack. */
									  &xBlinkTaskBuffer[msg.output] );  /* Variable to hold the task's data structure. */
					}
				} else if ( msg.state == On )
				{
					HAL_GPIO_WritePin(getGpioPort(msg.output), getGpioPin(msg.output), On);
					BlinkParams[msg.output].state = On;
				} else
				{
					HAL_GPIO_WritePin(getGpioPort(msg.output), getGpioPin(msg.output), Off);
					BlinkParams[msg.output].ontime = Off;
//					if ( xBlinkHandle[msg.output] != NULL )
//						vTaskSuspend( xBlinkHandle[msg.output] );
				}
			}
		}

		vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}

	osThreadTerminate(NULL);
}

#ifdef HPF19
GPIO_TypeDef* getGpioPort(output output)
{
	switch ( output ) { // set gpio values for requested port
		case 0 :
			return Output1_GPIO_Port;
		case 1 :
			return Output2_GPIO_Port;
		case 2 :
			return Output3_GPIO_Port;
		case 3 :
			return Output4_GPIO_Port;
		case 4 :
			return Output5_GPIO_Port;
		case 5 :
			return Output6_GPIO_Port;
		case 6 :
			return Output7_GPIO_Port;
		case 7 :
			return Output8_GPIO_Port;
		case 8 :
			return LD1_GPIO_Port;
		case 9 :
			return LD2_GPIO_Port;
		case 10 :
			return LD3_GPIO_Port;
		default :
			return 0;
	}
}

/**
 * returns GPIO pin for given output number.
 */

int getGpioPin(output output)
{
	switch ( output )
	{ // set gpio values for requested port
		case 0 :
			return Output1_Pin;
		case 1 :
			return Output2_Pin;
		case 2 :
			return Output3_Pin;
		case 3 :
			return Output4_Pin;
		case 4 :
			return Output5_Pin;
		case 5 :
			return Output6_Pin;
		case 6 :
			return Output7_Pin;
		case 7 :
			return Output8_Pin;
		case 8 :
			return LD1_Pin;
		case 9 :
			return LD2_Pin;
		case 10 :
			return LD3_Pin;
		default :
			return 0;
	}
}
#endif

#ifdef HPF20
GPIO_TypeDef* getGpioPort(output output)
{

	switch ( output ) { // set gpio values for requested port
		case 0 :
			return DO1_GPIO_Port;
		case 1 :
			return DO2_GPIO_Port;
		case 2 :
			return DO3_GPIO_Port;
		case 3 :
			return DO4_GPIO_Port;
		case 4 :
			return DO5_GPIO_Port;
		case 5 :
			return DO6_GPIO_Port;
		case 6 :
			return DO7_GPIO_Port;
		case 7 :
			return DO11_GPIO_Port;
		case 8 :
			return DO12_GPIO_Port;
		case 9 :
			return DO13_GPIO_Port;
		case 10 :
			return DO14_GPIO_Port;
		case 11 :
			return DO15_GPIO_Port;

		case 12 :
			return LD7_GPIO_Port;
		case 13 :
			return LD8_GPIO_Port;
		case 14 :
			return LD9_GPIO_Port;
		case 15 :
			return LD0_GPIO_Port;
		case 16 :
			return LD1_GPIO_Port;
		case 17 :
			return LD3_GPIO_Port;
		case 18 :
			return LD4_GPIO_Port;

		default :
			return 0;
	}
}

/**
 * returns GPIO pin for given output number.
 */

int getGpioPin(output output)
{
	switch ( output ) { // set gpio values for requested port
		case 0 :
			return DO1_Pin;
		case 1 :
			return DO2_Pin;
		case 2 :
			return DO3_Pin;
		case 3 :
			return DO4_Pin;
		case 4 :
			return DO5_Pin;
		case 5 :
			return DO6_Pin;
		case 6 :
			return DO7_Pin;
		case 7 :
			return DO11_Pin;
		case 8 :
			return DO12_Pin;
		case 9 :
			return DO13_Pin;
		case 10 :
			return DO14_Pin;
		case 11 :
			return DO15_Pin;
		case 12 :
			return LD7_Pin;
		case 13 :
			return LD8_Pin;
		case 14 :
			return LD9_Pin;
		case 15 :
			return LD0_Pin;
		case 16 :
			return LD1_Pin;
		case 17 :
			return LD3_Pin;
		case 18 :
			return LD4_Pin;
		default :
			return 0;
	}
}
#endif


/**
 * @brief sets specific output state of the state of specified GPIO output using programs defined input numbering
 */
void setOutput(output output, output_state state)
{
#ifdef RTOS
	output_msg msg;

	msg.output = output;
	if ( state >= On)
		msg.state = On;
	else
		msg.state = Off;
	msg.time = 0;

	if ( xPortIsInsideInterrupt() )
		xQueueSendFromISR( OutputQueue, ( void * ) &msg, NULL );
	else
		xQueueSend( OutputQueue, ( void * ) &msg, ( TickType_t ) 0 );
#else
	if ( output < OUTPUTCount ){
		if ( state == Off )
		  LEDs[output].state = 0;
		else
		  LEDs[output].state = 9;
	}
//	LEDs[output].blinktime = 0;
//	LEDs[output].blinkingrate = 0;
#endif
}

void setOutputNOW(output output, output_state state)
{
#ifdef RTOS
	output_msg msg;

	msg.output = output;
	if ( state >= On)
	{
		HAL_GPIO_WritePin(getGpioPort(output), getGpioPin(output), On);
		msg.state = On;
	}
	else
	{
		HAL_GPIO_WritePin(getGpioPort(output), getGpioPin(output), Off);
		msg.state = Off;
	}
	msg.time = 0;

	if ( xPortIsInsideInterrupt() )
		xQueueSendFromISR( OutputQueue, ( void * ) &msg, NULL );
	else
		xQueueSend( OutputQueue, ( void * ) &msg, ( TickType_t ) 0 );
#else
	if ( output < OUTPUTCount ){
		if ( state == Off )
		{
			HAL_GPIO_WritePin(getGpioPort(output), getGpioPin(output), 0);
			LEDs[output].state = 0;
		} else
		{
			HAL_GPIO_WritePin(getGpioPort(output), getGpioPin(output), 1);
			LEDs[output].state = 9;
		}
	}
#endif
}

/**
 * @brief Toggles the state of specified GPIO output using programs defined input numbering
 */
void toggleOutput(output output)
{
#ifdef RTOS
	output_msg msg;

	msg.output = output;
	msg.state = Toggle;
	msg.time = 0;

	if ( xPortIsInsideInterrupt() )
		xQueueSendFromISR( OutputQueue, ( void * ) &msg, NULL );
	else
		xQueueSend( OutputQueue, ( void * ) &msg, ( TickType_t ) 0 );
#else

	if ( output < OUTPUTCount ){
		if ( LEDs[output].state == 0 ) LEDs[output].state = 9; else LEDs[output].state = 0;
	}
#endif
}

void toggleOutputMetal(output output)
{
	if ( output < OUTPUTCount ){
		if(getGpioPin(output) != 0)
		{
			HAL_GPIO_TogglePin(getGpioPort(output), getGpioPin(output));
		}
	}
}

void blinkOutput(output output, output_state blinkingrate, uint32_t time) // max 30 seconds if timed.
{
#ifdef RTOS
	output_msg msg;

	msg.output = output;
	msg.state = blinkingrate;
	msg.time = time;

	if ( xPortIsInsideInterrupt() )
		xQueueSendFromISR( OutputQueue, ( void * ) &msg, NULL );
	else
		xQueueSend( OutputQueue, ( void * ) &msg, ( TickType_t ) 0 );
#else
	if ( output < OUTPUTCount ){
		switch ( blinkingrate )
		{
			case Off : LEDs[output].blinkingrate = 0; break;
			case On : LEDs[output].blinkingrate = 9; break;
			case BlinkVerySlow : LEDs[output].blinkingrate = 8; break;
			case BlinkSlow : LEDs[output].blinkingrate = 4; break;
			case BlinkMed : LEDs[output].blinkingrate = 2; break;
			case BlinkFast : LEDs[output].blinkingrate = 1; break;
			default :
				LEDs[output].blinkingrate = 9;
				break;
		}

		if ( time == 255 ){
			LEDs[output].blinktime = LEDBLINKNONSTOP;
		} else
		{
			LEDs[output].blinktime = time*8;
		}
	}
#endif
}

void stopBlinkOutput(output output)
{
	 blinkOutput(output, Nochange, 0);
}

void timeOutput(output output, uint32_t time)
{
#ifdef RTOS
	blinkOutput(output, Timed, time);

#else
	if ( output < OUTPUTCount ){
		LEDs[output].blinkingrate = 0;

		if ( time == 255 ){
			LEDs[output].blinktime = LEDBLINKNONSTOP;
		} else
		{
			LEDs[output].blinktime = time*8;
		}
	}
#endif
}

/**
 * @brief setup start state of LED's to off.
 */
int initOutput( void )
{
#ifndef RTOS
	for ( int i = 0; i < OUTPUTCount; i++)
	{
		LEDs[i].blinkingrate = 0;
		LEDs[i].state = 0;
		LEDs[i].blinktime = 0; // set default state of LED's to off, no blinking
	}

#else
	OutputQueue = xQueueCreateStatic( OutputQUEUE_LENGTH,
							  OutputITEMSIZE,
							  OutputQueueStorageArea,
							  &OutputStaticQueue );

	vQueueAddToRegistry(OutputQueue, "OutputQueue" );

	OutputTaskHandle = osThreadNew(OutputTask, NULL, &OutputTask_attributes);

#endif

	blinkOutput(LED1, BlinkVerySlow, LEDBLINKNONSTOP); // startup board activity blinker/power light.

	return 0;
}

/**
 * @brief set the current state of LED's as defined by carstate variables.
 */
void __setLEDs( void )
{
	// Check. 10 second delay for IMD led in simulink. IMD Light power off delay. missed earlier, significance?

	setOutput(BMSLED, CarState.Shutdown.BMS);
	setOutput(IMDLED, CarState.Shutdown.IMD);
	setOutput(BSPDLED, CarState.Shutdown.BSPDBefore);

#ifndef RTOS
//	if ( CarState.TSALLeftInvLED == 1 && CarState.TSALRightInvLED == 1 )
	{
		LEDs[TSLED_Output].blinkingrate = 2; // cockpit led

   } //else if ( CarState.TSALLeftInvLED >= 2 && CarState.TSALRightInvLED >= 2)
	{
		LEDs[TSLED_Output].blinkingrate = 0;

	}// else
	{
		LEDs[TSLED_Output].blinkingrate = 0;
	}

//	if ( CarState.RtdmLeftInvLED == 1 && CarState.RtdmRightInvLED == 1 )
	{
		LEDs[RTDMLED_Output].blinkingrate = 2;
	}// else if ( CarState.RtdmLeftInvLED >= 2 && CarState.RtdmRightInvLED >= 2)
	{
		LEDs[RTDMLED_Output].blinkingrate = 0;

	} //else
	{
		LEDs[RTDMLED_Output].blinkingrate = 0;
	}
#endif

//	LEDs[TSOFFLED_Output].blinkingrate = CarState.StopLED;
}


void startupLEDs(void)
{
	 //small onboard led display to indicate board startup
	  setOutputNOW(LED1,On);
	  setOutputNOW(LED2,On);
	  setOutputNOW(LED3,On);
	  HAL_Delay(300);
	  setOutputNOW(LED1,Off);
	  HAL_Delay(300);
	  setOutputNOW(LED2,Off);
	  HAL_Delay(300);
	  setOutputNOW(LED3,Off);

#ifdef OLDPOWEROn
	  // display status LED's for two seconds to indicate power on.
	  setOutput(1,On);
	  setOutput(2,On);
	  setOutput(3,On);
	  // HAL_Delay(2000);
	  HAL_Delay(500);
#endif
	  setOutput(BMSLED,On);
	  setOutput(IMDLED,On);
	  setOutput(BSPDLED,On);
	  // HAL_Delay(2000);
	  HAL_Delay(500);

	  for(int i=0;i<=OUTPUTCount;i++){ // turn off all LED's
		  setOutput(i, Off);
	  }
}
