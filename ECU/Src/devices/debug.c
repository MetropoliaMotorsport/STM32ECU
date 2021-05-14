/*
 * debug.c
 *
 *  Created on: 30 Apr 2021
 *      Author: visa
 */

#include "debug.h"
#include "ecumain.h"

#include "usart.h"
#include "power.h"
#include "powernode.h"

// freeRTOS
#include "semphr.h"

#include <stdio.h>
#include <stdarg.h>

typedef struct Debug_msg {
	char str[MAXDEBUGOUTPUT];
	//uint32_t msgval;
} Debug_msg;


#define DEBUGSTACK_SIZE 128*6
#define DEBUGTASKNAME  "DebugTask"
#define DEBUGTASKPRIORITY 1
StaticTask_t xDEBUGTaskBuffer;
StackType_t xDEBUGStack[ DEBUGSTACK_SIZE ];

TaskHandle_t DebugTaskHandle = NULL;


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


// add message to uart message queue.
bool DebugMsg( char * msg)
{
	struct Debug_msg debugmsg;
	strncpy( debugmsg.str, msg, MAXDEBUGOUTPUT );
	return xQueueSendToBack(DebugQueue,&debugmsg,0); // send it to error state handler queue for display to user.
}

bool redraw;

// also handle printing debug messages here.
char uartWait( char *ch )
{
	if(HAL_UART_Receive_IT(&huart2, (uint8_t *)ch, 1) != HAL_OK) {
		return 0;
	}

	struct Debug_msg msg;

	redraw = false;

	while ( 1 )
	{
		if( xSemaphoreTake(DebugUARTRxDone, 0) == pdTRUE) {
			return 1;
		}

		if ( xQueueReceive(DebugQueue,&msg,10) )
		{
			uartwrite(msg.str);
			uartwrite("\r\n");
			redraw = true;
		}
	}
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

	redraw = false;

	uartwrite(DEBUGPROMPT);

	char ch = 0;

	bool esc = false;

	while (1) {
		// just to be on safe side then.
		int read = uartWait(&ch);

		if ( redraw )
		{
			// we received debug output during keyboard wait, resend prompt and current imput to help.
			uartwrite("x1b[k");
			uartwrite(DEBUGPROMPT);
			uartwrite(str);
		}

		// didn't receive a char, skip.
		if ( read == 0 )
		{
			continue; // nothing to do this loop, return to start.
		}

		inputpos += read;

		bool endline = false;

		// 27 = esc -- second keycode coming.
		// 27/91/67 = left
		// 91/68 = right
		// ch 8 == backspace.
		if ( esc )
		{
			if ( ch == 79 ) // f key
				continue;
				// ch == 80 = F1
				// ch == 81 = f2 ...
			if ( ch == 91)
				continue;
			if ( ch == 68 ) // left
			{}
			if ( ch == 67 ) // right
			{}
			esc = false;
			continue;
		} else
		if ( ch == 27 )
		{
			esc = true;
			continue;
		} else
		if ( ch == 8 )
		{
			if ( charcount > 0 )
			{
				str[charcount] = 0;
				--charcount;
				uartwritech(ch);
				uartwritech(' ');
				uartwritech(ch);
			}
		} else
		if ( !( ch == '\n' || ch == '\r') )
		{
			if ( ch >= 32) // only process printable charecters.
			{
				str[charcount] = ch;
				str[charcount+1] = 0;
				uartwritech(ch);
				++charcount;
			}
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
				char tkn1[TOKENLENGTH] = "";
				char tkn2[TOKENLENGTH] = "";
				char tkn3[TOKENLENGTH] = "";

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
					} else
					{
						strncpy(tkn1, "too long", tknlen);
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

				if ( strcmp(tkn1, "fanpwm") == 0 )
				{
					if ( tokens != 3 )
					{
						uartwrite("Please give left and right pwm duty.\r\n");
					} else
					{
						if ( val1 > 255 || val2 > 255 || val1 < 0 || val2 < 0 )
						{
							uartwrite("invalid PWM duty cycles given");
						} else
						{
							snprintf(str, 80, "Requesting fan PWMs Left: %4d Right: %4d\r\n", val1, val2);
							uartwrite(str);

							FanPWMControl( val1, val2 );
						}
					}
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
						uartwrite("Power        Exp Act Err\r\n");
						uartwrite("------------------------\r\n");

						uint8_t listsize = getDevicePowerListSize();

						for ( int i=1;i<=listsize;i++)
						{
							snprintf(str, 80, "%-12s %-4s%-4s%-4d\r\n",
										getDevicePowerNameLong(i),
										getNodeDeviceExpectedPower(i)?"On":"Off",
										getNodeDevicePower(i)?"On":"Off",
										powerErrorOccurred(i)
									//    powerErrorOccurred(i)?"Yes":"No"
							);
							uartwrite(str);
						}
					} else if ( strcmp(tkn2, "all") == 0 )
					{
						if ( strcmp(tkn3, "reset") == 0 )
						{
							uartwrite("Power error reset for all\r\n");
							for ( int i=1; i <= AccuFan; i++ )
								resetDevicePower( i );
						} else
						{
							if ( strcmp(tkn3, "on")  == 0 || strcmp(tkn3, "true")  == 0|| strcmp(tkn3, "enabled") == 0 )
							{
								state = true;
							}
							else if ( strcmp(tkn3, "off") == 0 || strcmp(tkn3, "false") == 0 || strcmp(tkn3, "disabled") == 0 )
							{
								state = false;
							} else
							{
								badcmd = true;
							}


							if ( !badcmd )
							{
								uartwrite("Manual power request for all power set ");
								uartwrite(state? "on":"off");
								uartwrite("\r\n");

								for ( int i=1; i <= AccuFan; i++ )
									setDevicePower( i, state );

							} else
							{
								badcmd = true;
							}
						}
					}
					else
					{
						if ( strcmp(tkn2, "none") == 0)
							device = None;
						else if ( strcmp(tkn2, "buzzer") == 0 )
							device = Buzzer;
						else if ( strcmp(tkn2, "back1") == 0 )
							device = Back1;
						else if ( strcmp(tkn2, "back2") == 0 )
							device = Back2;
						else if ( strcmp(tkn2, "back3") == 0 )
							device = Back3;
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

						if ( strcmp(tkn3, "reset")  == 0 )
						{
							uartwrite("Power error reset for ");
							uartwrite(getDevicePowerNameLong(device));
							uartwrite("\r\n");
							resetDevicePower(device);
						} else
						{
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
				} else

				{
					uartwrite("Unknown command: ");
					uartwrite(tkn1);
					uartwrite("\r\n");
				}

			}

			charcount = 0;
			str[0] = 0;
			// print prompt to request further input.
			uartwrite(DEBUGPROMPT);

		}
	}

	vTaskDelete(NULL);
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

	DebugTaskHandle = xTaskCreateStatic(
						  DebugTask,
						  DEBUGTASKNAME,
						  DEBUGSTACK_SIZE,
						  ( void * ) 1,
						  DEBUGTASKPRIORITY,
						  xDEBUGStack,
						  &xDEBUGTaskBuffer );

	return 0;
}

