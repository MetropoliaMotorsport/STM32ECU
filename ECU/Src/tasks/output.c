/*
 * output.c
 *
 *  Created on: 14 Apr 2019
 *      Author: Visa
 */

#include "ecumain.h"
#include "output.h"
#include "timerecu.h"
#include "debug.h"
#include "power.h"
#include "taskpriorities.h"
#include "queue.h"

#define OUTPUTSTACK_SIZE 128*6
#define OUTPUTTASKNAME  "OutputTask"
StaticTask_t xOUTPUTTaskBuffer;
StackType_t xOUTPUTStack[ OUTPUTSTACK_SIZE ];

TaskHandle_t OutputTaskHandle;

#define OutputQUEUE_LENGTH    20
#define OutputITEMSIZE		sizeof( struct output_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t OutputStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t OutputQueueStorageArea[ OutputQUEUE_LENGTH * OutputITEMSIZE ];

QueueHandle_t OutputQueue;

typedef volatile struct OutputType
{
	GPIO_TypeDef * port;
	uint16_t pin;
    output output; // this is only really here to make seeing which led using in debugger.
    output_state state;
    uint16_t ontime;
    uint16_t offtime;
    uint32_t start;
    uint32_t duration;
    bool debug;
} OutputType;

#ifdef HPF20
OutputType Output[OUTPUTCount] = {
		 { DO1_GPIO_Port, DO1_Pin},
		 { DO2_GPIO_Port, DO2_Pin},
		 { DO3_GPIO_Port, DO3_Pin},
		 { DO4_GPIO_Port, DO4_Pin},
		 { DO5_GPIO_Port, DO5_Pin},
		 { DO6_GPIO_Port, DO6_Pin},
		 { DO7_GPIO_Port, DO7_Pin},
		 { DO11_GPIO_Port, DO11_Pin},
		 { DO12_GPIO_Port, DO12_Pin},
		 { DO13_GPIO_Port, DO13_Pin},
		 { DO14_GPIO_Port, DO14_Pin},
		 { DO15_GPIO_Port, DO15_Pin},
		 { LD7_GPIO_Port, LD7_Pin},
		 { LD8_GPIO_Port, LD8_Pin},
		 { LD9_GPIO_Port, LD9_Pin},
		 { LD0_GPIO_Port, LD0_Pin},
		 { LD1_GPIO_Port, LD1_Pin},
		 { LD3_GPIO_Port, LD3_Pin},
		 { LD4_GPIO_Port, LD4_Pin}
};
#endif

#ifdef HPF19

OutputType Output[OUTPUTCount] = {
		 { Output1_GPIO_Port, Output1_Pin},
		 { Output2_GPIO_Port, Output2_Pin},
		 { Output3_GPIO_Port, Output3_Pin},
		 { Output4_GPIO_Port, Output4_Pin},
		 { Output5_GPIO_Port, Output5_Pin},
		 { Output6_GPIO_Port, Output6_Pin},
		 { Output7_GPIO_Port, Output7_Pin},
		 { Output8_GPIO_Port, Output8_Pin},
		 { LD1_GPIO_Port, LD1_Pin},
		 { LD2_GPIO_Port, LD2_Pin},
		 { LD3_GPIO_Port, LD3_Pin}

};
#endif

#define BLINKSTACK_SIZE 64

StaticTask_t xBlinkTaskBuffer[OUTPUTCount];

TaskHandle_t xBlinkHandle[OUTPUTCount] = { NULL };


StackType_t xStack[OUTPUTCount][ BLINKSTACK_SIZE ];


void BlinkTask( void * pvParameters )
{
	OutputType * params = &Output[ (uint32_t) pvParameters ]; // parameter is int indicating which output this thread controls.

	while ( 1 )
    {
		bool Suspend = false;
		if ( params->duration > 0 )
		{
			HAL_GPIO_WritePin(params->port, params->pin, On);
			vTaskDelay(params->ontime);
			if ( params->offtime > 0 )
			{
				HAL_GPIO_WritePin(params->port, params->pin, Off);
				vTaskDelay(params->offtime);
			}
			if ( params->duration == 1 ) // if duration set to 1, only do blink cycle once.
			{
				Suspend = true;
			} else
			if ( params->duration != 0xFFFF )
			{
				if ( gettimer() - params->start > params->duration )
				{
					Suspend = true;
				}
			}
		} else
		{
			HAL_GPIO_WritePin(params->port, params->pin, params->state); // stopping blinking, set state.
			Suspend = true;
		}

		// if determined this is the last blink cycle then suspend the task to wait for next blink.
		if ( Suspend )
		{
			HAL_GPIO_WritePin(params->port, params->pin, params->state);
			params->duration = 0;
			//osThreadTerminate(NULL); // bug? apparently this does nothing in heap model 1.
			vTaskSuspend(NULL);
		}
    }

	vTaskDelete(NULL);
}

void OutputTask(void *argument)
{
	xEventGroupSync( xStartupSync, 0, 1, portMAX_DELAY );

	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT( OutputQueue );

	// T 11.9.5Indicators according to T 11.9.1 with safe state “illuminated”
	// (e.g. absence of failures is not actively indicated) must be illuminated
	// for 1 s to 3 s for visible check after power cycling the LVMS.


	for ( int i=0;i<19;i++)
	{
		HAL_GPIO_WritePin(getGpioPort(i), getGpioPin(i), On);
	}

	vTaskDelay(2000);

	for ( int i=0;i<19;i++)
	{
		HAL_GPIO_WritePin(getGpioPort(i), getGpioPin(i), Off);
	}

#if 0
	for ( int i=0;i<10;i++)
	{
		HAL_GPIO_WritePin(getGpioPort(i), getGpioPin(i), On);
		vTaskDelay(1000);
		HAL_GPIO_WritePin(getGpioPort(i), getGpioPin(i), Off);
	}

	vTaskDelay(1000);

	HAL_GPIO_WritePin(getGpioPort(3), getGpioPin(3), On); // ok white button
	HAL_GPIO_WritePin(getGpioPort(4), getGpioPin(4), On); // ok red button

	HAL_GPIO_WritePin(getGpioPort(6), getGpioPin(6), On); // ok middle left

	HAL_GPIO_WritePin(getGpioPort(7), getGpioPin(7), On); // left left

	HAL_GPIO_WritePin(getGpioPort(8), getGpioPin(8), On); // right left

	vTaskDelay(5000);

	HAL_GPIO_WritePin(getGpioPort(3), getGpioPin(3), Off);

	HAL_GPIO_WritePin(getGpioPort(4), getGpioPin(4), Off);

	HAL_GPIO_WritePin(getGpioPort(6), getGpioPin(6), Off);

	HAL_GPIO_WritePin(getGpioPort(7), getGpioPin(7), Off);

	HAL_GPIO_WritePin(getGpioPort(8), getGpioPin(8), Off);
#endif

//	blinkOutput(IMDLED, Timed, 2000);
//	blinkOutput(BMSLED, Timed, 2000);

	TickType_t xLastWakeTime;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

//	unsigned long counter;

	struct output_msg msg;

	bool debugmode = false;

	while( 1 )
	{

		while ( uxQueueMessagesWaiting( OutputQueue ) )
		{
        // UBaseType_t uxQueueMessagesWaiting( QueueHandle_t xQueue );
			if ( xQueueReceive(OutputQueue,&msg,0) )
			{
				#if 0
				if ( msg.debug )
				{
					if ( msg.state == Stopdebug)
						debugmode = false;
					else
						debugmode = true;

				}

				if ( debugmode && !msg.debug)
				{
					continue;
				}
				#endif

				if ( msg.state == Toggle )
				{
					Output[msg.output].state = !Output[msg.output].state;
					Output[msg.output].debug = msg.debug;
					toggleOutputMetal(msg.output);

				} else if ( msg.state > On )
				{
					Output[msg.output].output = msg.output;
					Output[msg.output].debug = msg.debug;

					int ontime = 0;
					int offtime = 0;

					switch ( msg.state )
					{
						case BlinkVerySlow : ontime = 1000; offtime = 1000; break;
						case BlinkSlow : ontime = 500; offtime = 500; break;
						case BlinkMed : ontime = 200; offtime = 200; break;
						case BlinkFast : ontime = 50; offtime = 100; break;
						case BlinkVeryFast : ontime = 10; offtime = 10; break;
						case Timed : ontime = 10; offtime = 0; break;
						case Nochange : break;
						default :
						ontime = 500; offtime = 500;
					}

					if ( msg.state != Nochange )
					{
						Output[msg.output].ontime = ontime;
						Output[msg.output].offtime = offtime;
					}
					Output[msg.output].start = gettimer();

					if ( msg.time != 1)
					{
						if ( msg.time < ontime+offtime ) // ensure that one complete cycle will show
							msg.time = ontime+offtime;
					}

					Output[msg.output].duration = msg.time;

					// only create blink task when it's actually needed for first time

					// should keep task list a little cleaner if only led's called to blink actually get populated.

					// only actually create task first time LED is requested to be blinking.
					if ( ( xBlinkHandle[msg.output] == NULL	|| eTaskGetState(xBlinkHandle[msg.output]) == eDeleted )
							&& msg.time > 0 )
					{
						xBlinkHandle[msg.output]  = xTaskCreateStatic(
									  BlinkTask,       /* Function that implements the task. */
									  "BlinkTask",     /* Text name for the task. */
									  BLINKSTACK_SIZE,      /* Number of indexes in the xStack array. */
									  ( void * ) msg.output,    /* Parameter passed into the task. */
									  2,/* Priority at which the task is created. */
									  xStack[msg.output],          /* Array to use as the task's stack. */
									  &xBlinkTaskBuffer[msg.output] );  /* Variable to hold the task's data structure. */
					} else if ( msg.time > 0 && eTaskGetState(xBlinkHandle[msg.output]) == eSuspended )
					{
						vTaskResume(xBlinkHandle[msg.output]);
					}
				} else if ( msg.state == On )
				{
				//	if ( xBlinkHandle[msg.output] != NULL )
				//	vTaskSuspend( xBlinkHandle[msg.output] );
					HAL_GPIO_WritePin(getGpioPort(msg.output), getGpioPin(msg.output), On);
					Output[msg.output].state = On;
					Output[msg.output].debug = msg.debug;
				} else
				{
					HAL_GPIO_WritePin(getGpioPort(msg.output), getGpioPin(msg.output), Off);
					Output[msg.output].state = Off;
					Output[msg.output].debug = msg.debug;
					//Output[msg.output].ontime = Off;
//					if ( xBlinkHandle[msg.output] != NULL )
//						vTaskSuspend( xBlinkHandle[msg.output] );
				}
			}
		}

		// input polling here?

		vTaskDelayUntil( &xLastWakeTime, CYCLETIME );
	}

	vTaskDelete(NULL);
}

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

/**
 * returns gpio port for given output number.
 */
GPIO_TypeDef* getGpioPort(output output)
{
	return Output[output].port;
}

/**
 * returns GPIO pin for given output number.
 */

int getGpioPin(output output)
{
	return Output[output].pin;
}


/**
 * @brief sets specific output state of the state of specified GPIO output using programs defined input numbering
 */
void setOutput(output output, output_state state)
{
	output_msg msg;

	msg.debug = false;
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
}

void setOutputDebug(output output, output_state state)
{
	output_msg msg;

	msg.output = output;
	msg.debug = true;
	if ( state >= On)
		msg.state = On;
	else
		msg.state = Off;
	msg.time = 0;

	if ( xPortIsInsideInterrupt() )
		xQueueSendFromISR( OutputQueue, ( void * ) &msg, NULL );
	else
		xQueueSend( OutputQueue, ( void * ) &msg, ( TickType_t ) 0 );
}

void setOutputNOW(output output, output_state state)
{
	//output_msg msg;

	//msg.debug = false;
	//msg.output = output;
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
	//msg.time = 0;

	//if ( xPortIsInsideInterrupt() )
	//	xQueueSendFromISR( OutputQueue, ( void * ) &msg, NULL );
	//else
	//	xQueueSend( OutputQueue, ( void * ) &msg, ( TickType_t ) 0 );
}

/**
 * @brief Toggles the state of specified GPIO output using programs defined input numbering
 */
void toggleOutput(output output)
{
	output_msg msg;

	msg.debug = false;
	msg.output = output;
	msg.state = Toggle;
	msg.time = 0;

	if ( xPortIsInsideInterrupt() )
		xQueueSendFromISR( OutputQueue, ( void * ) &msg, NULL );
	else
		xQueueSend( OutputQueue, ( void * ) &msg, ( TickType_t ) 0 );
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
	output_msg msg;

	msg.debug = false;
	msg.output = output;
	msg.state = blinkingrate;
	msg.time = time;

	if ( xPortIsInsideInterrupt() )
		xQueueSendFromISR( OutputQueue, ( void * ) &msg, NULL );
	else
		xQueueSend( OutputQueue, ( void * ) &msg, ( TickType_t ) 0 );
}

void blinkOutputDebug(output output, output_state blinkingrate, uint32_t time) // max 30 seconds if timed.
{
	output_msg msg;

	msg.output = output;
	msg.state = blinkingrate;
	msg.time = time;
	msg.debug = true;

	if ( xPortIsInsideInterrupt() )
		xQueueSendFromISR( OutputQueue, ( void * ) &msg, NULL );
	else
		xQueueSend( OutputQueue, ( void * ) &msg, ( TickType_t ) 0 );
}


void stopBlinkOutput(output output)
{
	 blinkOutput(output, Nochange, 0);
}

void timeOutput(output output, uint32_t time)
{
	blinkOutput(output, Timed, time);
}

int initOutput( void )
{
	OutputQueue = xQueueCreateStatic( OutputQUEUE_LENGTH,
							  OutputITEMSIZE,
							  OutputQueueStorageArea,
							  &OutputStaticQueue );

	vQueueAddToRegistry(OutputQueue, "OutputQueue" );


	OutputTaskHandle = xTaskCreateStatic(
						  OutputTask,
						  OUTPUTTASKNAME,
						  OUTPUTSTACK_SIZE,
						  ( void * ) 1,
						  OUTPUTTASKPRIORITY,
						  xOUTPUTStack,
						  &xOUTPUTTaskBuffer );

	blinkOutput(LED1, BlinkVerySlow, LEDBLINKNONSTOP); // startup board activity blinker/power light.

	return 0;
}
