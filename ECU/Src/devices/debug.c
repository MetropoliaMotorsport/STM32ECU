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

void uartwritetwoline(char *str, char *str2)
{
	PRINT_MESG_UART(str);
	PRINT_MESG_UART(str2);
	PRINT_MESG_UART("\r\n");
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
	uartwrite("\r\nBooting ECU...\r\n\r\n");


	uartwrite(DEBUGPROMPT);

	char ch = 0;

	while (1) {
		// just to be on safe side then.
		int read = uartread(&ch);

		// didn't receive a char, skip.
		if ( read == 0 )
		{
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

#define TOKENLENGTH   12


		if ( charcount == 60 || endline )
		{
			if ( charcount > 0 )
			{
				char tkn1[TOKENLENGTH];
				char tkn2[TOKENLENGTH];
				char tkn3[TOKENLENGTH];

				uint8_t tokens = 0;

				int val1;
				int val2;

				// parse the input string into tokens to be processed.

				char *s=str;

				if (*(s += strspn(s, " ")) != '\0') {
					size_t tknlen = strcspn(s, " ");
					if ( tknlen < TOKENLENGTH )
					{
						strncpy(tkn1, s, tknlen);
						tkn1[tknlen] = '\0';

						s += tknlen;
						tokens++;
					}
				}

				if (*(s += strspn(s, " ")) != '\0') {
					size_t tknlen = strcspn(s, " ");
					if ( tknlen < TOKENLENGTH )
					{
						strncpy(tkn2, s, tknlen);
						tkn2[tknlen] = '\0';

						s += tknlen;
						tokens++;
					}
				}

				if ( strlen(tkn2) >0 )
				{
					val1 = strtol(tkn2, NULL, 10);
				} else val1 = 0;


				if (*(s += strspn(s, " ")) != '\0') {
					size_t tknlen = strcspn(s, " ");
					if ( tknlen < TOKENLENGTH )
					{
						strncpy(tkn3, s, tknlen);
						tkn3[tknlen] = '\0';
						tokens++;
					}
				}

				if ( strlen(tkn3)>0 )
				{
					val2 = strtol(tkn3, NULL, 10);
				} else val2 = 0;

				bool badcmd = false;


				if ( strcmp(tkn1, "shutdown") == 0 )
				{
					uartwrite("Current state of shutdown switches:\r\n");

					uartwritetwoline("BOTS         ", CarState.Shutdown.BOTS?"Closed":"Open");
					uartwritetwoline("Inertia      ", CarState.Shutdown.InertiaSwitch?"Closed":"Open");
					uartwritetwoline("BSPD After   ", CarState.Shutdown.BSPDAfter?"Closed":"Open");
					uartwritetwoline("BSPD Before  ", CarState.Shutdown.BSPDBefore?"Closed":"Open");
					uartwritetwoline("Cockpit      ", CarState.Shutdown.CockpitButton?"Closed":"Open");
					uartwritetwoline("Left         ", CarState.Shutdown.LeftButton?"Closed":"Open");
					uartwritetwoline("Right        ", CarState.Shutdown.RightButton?"Closed":"Open");
					uartwritetwoline("BMS          ", CarState.Shutdown.BMS?"Closed":"Open"); // BMSReason
					uartwritetwoline("IMD          ", CarState.Shutdown.IMD?"Closed":"Open");
					uartwritetwoline("AIR          ", CarState.Shutdown.AIROpen?"Closed":"Open");
				} else

				if ( strcmp(tkn1, "power") == 0 )
				{
					DevicePower device = None;
					bool state = false;

					if ( tokens == 1 ) // we need some sub commands, otherwise show help
					{
						uartwrite("Power command: Help\r\n");
					} else if ( strcmp(tkn2, "status") == 0 || strcmp(tkn2, "state") == 0)
					{
						uartwrite("Power        Exp Act\r\n");
						uartwrite("--------------------\r\n");

						uint8_t listsize = getDevicePowerListSize();

						for ( int i=1;i<=listsize;i++)
						{
							snprintf(str, 80, "%-12s %-4s%-4s\r\n", getDevicePowerNameLong(i), getNodeDeviceExpectedPower(i)?"On":"Off", getNodeDevicePower(i)?"On":"Off");
							uartwrite(str);
						}

					} else
					{

						if ( strcmp(tkn2, "none") == 0)
							device = None;
						else if ( strcmp(tkn2, "buzzer") == 0 )
							device = Buzzer;
						else if ( strcmp(tkn2, "telemetry") == 0 )
							device = Telemetry;
						else if ( strcmp(tkn2, "front1") == 0 )
							device = Front1;
						else if ( strcmp(tkn2, "inverters") == 0 )
							device = Inverters;
						else if ( strcmp(tkn2, "ecu") == 0 )
							device = ECU;
						else if ( strcmp(tkn2, "front2") == 0 )
							device = Front2;
						else if ( strcmp(tkn2, "leftfans") == 0 )
							device = LeftFans;
						else if ( strcmp(tkn2, "rightfans") == 0 )
							device = RightFans;
						else if ( strcmp(tkn2, "leftpump") == 0 )
							device = LeftPump;
						else if ( strcmp(tkn2, "rightpump") == 0 )
							device = RightPump;
						else if ( strcmp(tkn2, "ivt") == 0 )
							device = IVT;
						else if ( strcmp(tkn2, "current") == 0 )
							device = Current;
						else if ( strcmp(tkn2, "tsal") == 0 )
							device = TSAL;
						else if ( strcmp(tkn2, "brake") == 0 )
							device = Brake;
						else if ( strcmp(tkn2, "accu") == 0 )
							device = Accu;
						else if ( strcmp(tkn2, "accufan") == 0 )
							device = AccuFan;

						if ( strcmp(tkn3, "on")  == 0 || strcmp(tkn3, "true")  == 0|| strcmp(tkn3, "enabled") == 0 )
						{
							state = true;
						}
						else if ( strcmp(tkn3, "off") == 0 || strcmp(tkn3, "false") == 0 || strcmp(tkn3, "disabled") == 0 )
						{
							state = false;
						} else
						{
							device = None;
						}

						if ( device != None )
						{
							uartwrite("Manual power request for ");
							uartwrite(getDevicePowerNameLong(device));
							uartwrite(" set ");
							uartwrite(state? "on":"off");
							uartwrite("\r\n");
							setDevicePower( device, state );
						} else
						{
							badcmd = true;
						}
					}


					if ( badcmd )
					{
						uartwrite("Invalid power request given: Help\r\n");
					}
				} else

				if ( strcmp( str, "stat") == 0 )
				{
					// print stats.
					uartwrite("\r\nStatistics output:\r\n");
					vTaskGetRunTimeStats( str ); // fills ringbuffer, need to split into multiple transmissions

					uartwrite(str);

					uartwrite("\r\n");

				} else

				if ( strcmp( str, "list") == 0 )
				{
					// print list.
					uartwrite("\r\nTask List\r\n");
					vTaskList( str );
					uartwrite(str);
					uartwrite("\r\n");
				}
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

