/*
 * debug.c
 *
 *  Created on: 30 Apr 2021
 *      Author: visa
 */

#include "debug.h"
#include "ecumain.h"
#include "semphr.h"
#include "usart.h"
#include <stdio.h>
#include <stdarg.h>



osThreadId_t DebugTaskHandle;
const osThreadAttr_t DebugTask_attributes = {
  .name = "DebugTask",
  .priority = (osPriority_t) osPriorityIdle,
  .stack_size = 128*6
};

#define DebugQUEUE_LENGTH    20
#define DebugITEMSIZE		sizeof( Debug_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t DebugStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t DebugQueueStorageArea[ DebugQUEUE_LENGTH * DebugITEMSIZE ];

QueueHandle_t DebugQueue;

SemaphoreHandle_t DebugUARTTxDone;
SemaphoreHandle_t DebugUARTRxDone;


#define DEBUGPROMPT    "DebugCmd: "


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
		// receive complete
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	xSemaphoreGiveFromISR(DebugUARTTxDone, &xHigherPriorityWoken);
	portEND_SWITCHING_ISR(xHigherPriorityWoken);

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
		// receive complete
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	xSemaphoreGiveFromISR(DebugUARTRxDone, &xHigherPriorityWoken);
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}


int PRINT_MESG_UART(const char * format, ... )
{
	va_list ap;
	uint8_t buffer [128];
	int n;
	va_start(ap, format);
	n = vsnprintf ((char*)buffer, 128, format, ap);
	va_end(ap);
	//notify_uart(buffer, n);
	if(HAL_UART_Transmit_IT(&huart2, (uint8_t*)buffer, n) != HAL_OK) {
		return 0;
	}

	// set some realistic timeout
	if( xSemaphoreTake(DebugUARTTxDone, 100) == pdTRUE) {
		return 1;
	} else
		return 0;
}

int uartwritech(char ch)
{
	if(HAL_UART_Transmit_IT(&huart2, (uint8_t *)&ch, 1) != HAL_OK) {
		return 0;
	}

	// set some realistic timeout
	if( xSemaphoreTake(DebugUARTTxDone, 100) == pdTRUE) {
		return 1;
	} else
		return 0;

}



void uartwrite(char *str)
{
	PRINT_MESG_UART(str);
}


char uartread( char *ch )
{
	if(HAL_UART_Receive_IT(&huart2, (uint8_t *)ch, 1) != HAL_OK) {
		return 0;
	}

	// set some realistic timeout
	if( xSemaphoreTake(DebugUARTRxDone, portMAX_DELAY) == pdTRUE) {
		return 1;
	} else
		return 0;
	return 0;
}

#ifdef configSUPPORT_DYNAMIC_ALLOCATION

#else

#endif

static char str[40*100] = { 0 };


static void DebugTask(void *pvParameters)
{
	uint8_t charcount = 0;

	int inputpos = 0;
	uartwrite(DEBUGPROMPT);

	char ch = 0;

	while (1) {
		// just to be on safe side then.
		char ch = 0;
		int read = uartread(&ch);

		// didn't receive a char, skip.
		if ( read == 0 )
		{
			taskYIELD();
			continue; // nothing to do this loop, return to start.
		}

		inputpos += read;

		bool endline = false;

		if ( !( ch == '\n' || ch == '\r') )
		{
			str[charcount] = ch;
			str[charcount+1] = 0;
			uartwritech(ch);
			++charcount;
		} else
		{
			endline = true;
			uartwrite("\r\n");
		}

		if ( charcount == 60 || endline )
		{
			if ( strcmp( str, "stat") == 0 )
			{
				// print stats.
				uartwrite("\r\nStatistics output:\r\n");
				vTaskGetRunTimeStats( str ); // fills ringbuffer, need to split into multiple transmissions

				uartwrite(str);

				uartwrite("\r\n");

			} else 	if ( strcmp( str, "list") == 0 )
			{
				// print list.
				uartwrite("\r\nTask List\r\n");
				vTaskList( str );
				uartwrite(str);
				uartwrite("\r\n");
			}

			charcount = 0;
			str[0] = 0;
			// print prompt to request further input.
			uartwrite(DEBUGPROMPT);
		}
	}

	osThreadTerminate(NULL);
}


int initDebug( void )
{
	MX_USART2_UART_Init();

	DebugUARTTxDone = xSemaphoreCreateBinary();
	DebugUARTRxDone = xSemaphoreCreateBinary();

	DebugQueue = xQueueCreateStatic( DebugQUEUE_LENGTH,
							  DebugITEMSIZE,
							  DebugQueueStorageArea,
							  &DebugStaticQueue );

	vQueueAddToRegistry(DebugQueue, "DebugQueue" );

	DebugTaskHandle = osThreadNew(DebugTask, NULL, &DebugTask_attributes);
	return 0;
}

