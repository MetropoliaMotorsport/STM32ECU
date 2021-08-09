/*
 * canecu.c
 *
 *  Created on: 30 Dec 2018
 *      Author: Visa
 */

#include "ecumain.h"
#include "dwt_delay.h"
#include "canecu.h"
#include "adcecu.h"
#include "errors.h"
#include "uartecu.h"
#include "debug.h"
#include "output.h"
#include "timerecu.h"
#include "power.h"
#include "queue.h"
#include "watchdog.h"
#include "ecumain.h"
#include "fdcan.h"
#include "semphr.h"
#include "taskpriorities.h"
#include "eeprom.h"
#include "inverter.h"

#ifdef ONECAN
	#define sharedCAN
#endif

FDCAN_HandleTypeDef * hfdcan2p = NULL;

//variables that need to be accessible in ISR's

int cancount;

SemaphoreHandle_t CANBufferUpdating, bus0TXDone, bus1TXDone;

#define CANTXSTACK_SIZE 128*8
#define CANTXTASKNAME  "CANTxTask"
StaticTask_t xCANTXTaskBuffer;
StackType_t xCANTXStack[ CANTXSTACK_SIZE ];

TaskHandle_t CANTxTaskHandle = NULL;


#define CANRXSTACK_SIZE 128*12
#define CANRXTASKNAME  "CANRxTask"
StaticTask_t xCANRXTaskBuffer;
StackType_t xCANRXStack[ CANRXSTACK_SIZE ];

TaskHandle_t CANRxTaskHandle = NULL;

/* The queue is to be created to hold a maximum of 10 uint64_t
variables. */
#define CANTxQUEUE_LENGTH    64
#define CANRxQUEUE_LENGTH    256
#define CANITEMSIZE			sizeof( struct can_msg )

#define CANTxITEMSIZE		CANITEMSIZE
#define CANRxITEMSIZE		CANITEMSIZE

/* The variable used to hold the queue's data structure. */
static StaticQueue_t CANTxStaticQueue, CANRxStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t CANTxQueueStorageArea[ CANTxQUEUE_LENGTH * CANITEMSIZE ];
uint8_t CANRxQueueStorageArea[ CANTxQUEUE_LENGTH * CANITEMSIZE ];

QueueHandle_t CANTxQueue, CANRxQueue;

#if defined( __ICCARM__ )
  #define DMA_BUFFER \
      _Pragma("location=\".dma_buffer\"")
#else
  #define DMA_BUFFER \
      __attribute__((section(".dma_buffer")))
#endif

// ADC conversion buffer, should be aligned in memory for faster DMA?
DMA_BUFFER ALIGN_32BYTES (static uint8_t CANTxBuffer1[1024]);
DMA_BUFFER ALIGN_32BYTES (static uint8_t CANTxBuffer2[1024]);

static uint8_t * CANTxBuffer;

//static uint16_t CANTxBufferPos;
static uint8_t * CurCANTxBuffer;
static uint32_t CANTxLastsend;

bool processCan1Message( FDCAN_RxHeaderTypeDef *RxHeader, uint8_t CANRxData[8]);
bool processCan2Message( FDCAN_RxHeaderTypeDef *RxHeader, uint8_t CANRxData[8]);
void processCanTimeouts( void );

void UART_CANBufferAdd(const can_msg * msg )
{
	xSemaphoreTake(CANBufferUpdating, portMAX_DELAY);

	// r1xxxDbbbbbbbb

	if ( CANTxBuffer - CurCANTxBuffer < 1000 )
	{
		CANTxBuffer += snprintf((char*)CANTxBuffer, 15, "t%1d%03X%1lu", msg->bus, msg->id, msg->dlc);
		memcpy(CANTxBuffer, msg->data, msg->dlc);
		CANTxBuffer += msg->dlc;
		CANTxBuffer[0]= '\n';
		CANTxBuffer += 1;
	}

	xSemaphoreGive(CANBufferUpdating);
//	UART_Transmit(UART2, canstr, len+msg.dlc+1);
}

void UART_CANBufferTransmit( void )
{
	if ( CANTxBuffer - CurCANTxBuffer > 1000 || gettimer() != CANTxLastsend ) // buffer is nearly full or 1ms has ticked over
	{
		CANTxLastsend = gettimer();

		if ( CANTxBuffer - CurCANTxBuffer > 0 )
		{
			UART_Transmit(UART1, CurCANTxBuffer, CANTxBuffer - CurCANTxBuffer );

			if ( CurCANTxBuffer == CANTxBuffer1 )
			{
				CANTxBuffer = CANTxBuffer2;
				CurCANTxBuffer = CANTxBuffer2;
			}
			else
			{
				CANTxBuffer = CANTxBuffer1;
				CurCANTxBuffer = CANTxBuffer1;
			}
		}

	}
}


void CANTxTask(void *argument)
{

	 xEventGroupSync( xStartupSync, 0, 1, portMAX_DELAY );

	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT( CANTxQueue );

	FDCAN_TxHeaderTypeDef TxHeader = {
		.Identifier = 0, // decide on an ECU ID/
		.IdType = FDCAN_STANDARD_ID,
		.TxFrameType = FDCAN_DATA_FRAME,
		.DataLength = 0, // only two bytes defined in send protocol, check this
		.ErrorStateIndicator = FDCAN_ESI_ACTIVE,
		.BitRateSwitch = FDCAN_BRS_OFF,
		.FDFormat = FDCAN_CLASSIC_CAN,
		.TxEventFifoControl = FDCAN_NO_TX_EVENTS,
		.MessageMarker = 0
	};

	FDCAN_HandleTypeDef * hfdcanp = &hfdcan1;

	volatile uint32_t * pCANSendError = &Errors.CANSendError1;

	can_msg msg;

	uint8_t watchdogBit = registerWatchdogBit("CANTxTask");

	portTickType cycletick = xTaskGetTickCount();

	portTickType waittick = CYCLETIME;

	TxHeader.Identifier = 0x20; // decide on an ECU ID/
	TxHeader.DataLength = 8 << 16; // only two bytes defined in send protocol, check this
	memset(msg.data, 0, 8);

	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, msg.data);
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, msg.data);

	while( 1 )
	{
		portTickType curtick = xTaskGetTickCount();

		if ( curtick >= cycletick + CYCLETIME)
		{
			cycletick = curtick;
			waittick = CYCLETIME;
			setWatchdogBit(watchdogBit);
		}
		else
		{
			waittick = cycletick-curtick+CYCLETIME;
		}

//		UART_CANBufferTransmit();

		// ensure we can actually send a message -- bad, would leave messages potentially in buffer when no longer relevant.
//		while ( HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) < 2 && HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2) < 2 )
//		{
//			DebugMsg("waiting free can spot.");
//		}

		if ( xQueueReceive(CANTxQueue,&msg,waittick) )
		{
//			UART_CANBufferAdd(&msg);

			if ( msg.bus == bus1)
			{
				hfdcanp = &hfdcan1;
				pCANSendError = &Errors.CANSendError1;
			}
			else if ( msg.bus == bus0)
			{
				hfdcanp = hfdcan2p;
				pCANSendError = &Errors.CANSendError2;
			}

			FDCAN_ProtocolStatusTypeDef CANStatus;

			HAL_FDCAN_GetProtocolStatus(hfdcanp, &CANStatus);

			if ( !CANStatus.ErrorPassive ) // no point in trying to send if error passive.
			{
				TxHeader.Identifier = msg.id; // decide on an ECU ID/
				TxHeader.DataLength = msg.dlc << 16; // only two bytes defined in send protocol, check this


				if ( HAL_FDCAN_GetTxFifoFreeLevel(hfdcanp) == 0 )
				{
					DebugMsg("CAN Tx Buffer full, waiting for buffer empty.");

					bool gotsem = false;

					if ( msg.bus == bus1 )
					{
						gotsem = xSemaphoreTake(bus1TXDone, 10); // a bit of timeout so can't get permanently stuck here.
					}
					else
					{
						gotsem = xSemaphoreTake(bus0TXDone, 10);
					}

					if ( !gotsem )
					{
						DebugMsg("Buffer empty wait failed.");
					}
				}

				if ( HAL_FDCAN_GetTxFifoFreeLevel(hfdcanp) != 0 )
					if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcanp, &TxHeader, msg.data) != HAL_OK)
					{
						DebugPrintf("CAN Tx Send Err code: %d",  hfdcanp->ErrorCode);
						if ( pCANSendError != NULL )
							(*pCANSendError)++;
					}
			} else
			{
				if ( msg.bus == bus1)
				{
					static bool busnotact = false;
					if ( !busnotact )
					{
						DebugPrintf("CAN Tx Bus1 Not actively transmitting.");
						busnotact = true;
					}
				}
				else if ( msg.bus == bus0)
				{
					static bool busnotact = false;
					if ( !busnotact )
					{
						DebugPrintf("CAN Tx Bus0 Not actively transmitting.");
						busnotact = true;
					}
				}

			}
		}
	}

	vTaskDelete(NULL);
}


void CANRxTask(void *argument)
{
	 xEventGroupSync( xStartupSync, 0, 1, portMAX_DELAY );

	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT( CANRxQueue );

	can_msg msg, uartmsg;

	uint8_t watchdogBit = registerWatchdogBit("CANTxTask");

	portTickType cycletick = xTaskGetTickCount();

	portTickType waittick = CYCLETIME;

	bool transmitUARTCan = false;

	uint8_t uartrxstate = 0;
	uint8_t uartin[7] = {0}; // zero out array to ensure it ends in 0, for string termination.

	while( 1 )
	{
		portTickType curtick = xTaskGetTickCount();

		if ( curtick >= cycletick + CYCLETIME)
		{
			// once per cycle CAN stuff. Mostly handle timeouts, check if things received and kick watchdog.
			cycletick = curtick;
			waittick = CYCLETIME;
			setWatchdogBit(watchdogBit);

			processCanTimeouts();
		}
		else
		{
			waittick = cycletick-curtick+CYCLETIME;
		}

		// move uart can to own task? add an init state to wait for a go command for syncing at startup.
		// r1xxxDbbbbbbbb

#if 0
		switch ( uartrxstate )
		{
		case 0 :
			// try and setup receive of header
			if ( UART_Receive( UART1, uartin, 6) )
			{
				uartrxstate = 1;
			} else
				uartrxstate = 99;
			break;

		case 1 :
			// wait for header
			if ( UART_WaitRXDone( UART1, 0 ) )
			{
				if ( uartin[0] == 't' )
				{
					if ( uartin[1] == '1' )
						uartmsg.bus = 1;
					else if ( uartin[1] == '2' )
						uartmsg.bus = 2;
					else
					{
						uartrxstate = 99;
						break;
						// tx error of some sort to reset.
					}

					// convert the last number first, then set to zero so strtoul can be used on the id.
					uartmsg.dlc = strtoul((char *)&uartin[5], NULL, 10);
					uartin[5] = 0;

					uartmsg.id = strtoul((char *)&uartin[2], NULL, 16);

					if ( UART_Receive( UART1, uartmsg.data, uartmsg.dlc) )
					{
						uartrxstate = 2; // we've got header, wait for data.
					} else
						uartrxstate = 99; // error

				} else if ( uartin[0] == 's' )
				{
					// sync message.
					uartrxstate = 0;
				}

			}
			break;
		case 2 : // wait for data
			if ( UART_WaitRXDone( UART1, 0 ) )
			{
				// got packet, throw it out onto the bus?, and into RX queue
				xQueueSend( CANRxQueue, &uartmsg, 0 );
				if ( transmitUARTCan )
					xQueueSend( CANTxQueue, &uartmsg, 0 ); // also send uart received packet out onto real bus.
				uartrxstate = 0;
			}
		case 99 : // error.
		default :
			// cancel any receive, reset state
			uartrxstate = 0;
		}
#endif

		if( xQueueReceive(CANRxQueue,&msg,waittick) )
		{
			FDCAN_RxHeaderTypeDef RxHeader;
			RxHeader.Identifier = msg.id;
			RxHeader.DataLength = msg.dlc;

			if ( msg.id == 0x601 )
			{
				volatile int i = 0;
			}

			if ( msg.bus == bus1)
			{
				if ( !processCan1Message(&RxHeader, msg.data) )
				switch ( msg.id )
				{
					default :
#ifdef HPF20
						blinkOutput(LED7, BlinkVeryFast, 1);
#endif
						break;
				}
			} else
			{
				if ( !processCan2Message(&RxHeader, msg.data) )
				switch ( msg.id )
				{
					default :
#ifdef HPF20
						blinkOutput(LED7, BlinkVeryFast, 1);
#endif
						break;
				}

			}
		}
	}

	vTaskDelete(NULL);
}

/**
 * @brief set filter states and start CAN module and it's interrupt for CANBUS1
 */
void FDCAN1_start(void)
{
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    // Initialization Error
    Error_Handler();
  }
#ifndef ONECAN
   hfdcan2p = &hfdcan2;
  if (HAL_FDCAN_Init(hfdcan2p) != HAL_OK) // if can2 not initialised before filters set they are lost
  {
    // Initialization Error
    Error_Handler();
  }
#endif

  HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0, DISABLE, DISABLE);

  HAL_FDCAN_ConfigRxFifoOverwrite(&hfdcan1, FDCAN_RX_FIFO0, FDCAN_RX_FIFO_OVERWRITE);

#ifndef ONECAN
  /* Start the FDCAN module */
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
  {
    // Start Error
    Error_Handler();
  }

  // start can receive interrupt for CAN1
  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_FLAG_ERROR_PASSIVE | FDCAN_IT_TIMEOUT_OCCURRED | FDCAN_IT_TX_COMPLETE | FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
  {
    // Notification Error
    Error_Handler();
  }
#endif
}

void FDCAN2_start(void)
{
#ifdef ONECAN
  hfdcan2p = &hfdcan1;
#else
  HAL_FDCAN_ConfigGlobalFilter(hfdcan2p, FDCAN_IT_TIMEOUT_OCCURRED | FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_ACCEPT_IN_RX_FIFO1, DISABLE, DISABLE);
  HAL_FDCAN_ConfigRxFifoOverwrite(hfdcan2p, FDCAN_RX_FIFO1, FDCAN_RX_FIFO_OVERWRITE);
#endif

#ifdef ONECAN
  // Start the FDCAN module
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
  {
    //  Start Error
    Error_Handler();
  }

  // start can receive interrupt for first CAN's messages

  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_FLAG_ERROR_PASSIVE | FDCAN_IT_TIMEOUT_OCCURRED | FDCAN_IT_TX_COMPLETE | FDCAN_IT_RX_FIFO0_NEW_MESSAGE , 0) != HAL_OK)
  {
    // Notification Error
    Error_Handler();
  }

#else
  // Start the FDCAN module
  if (HAL_FDCAN_Start(hfdcan2p) != HAL_OK)
  {
    //  Start Error
    Error_Handler();
  }

  // start can receive interrupt for second can's messages

  if (HAL_FDCAN_ActivateNotification(hfdcan2p, FDCAN_FLAG_ERROR_PASSIVE | FDCAN_IT_TX_COMPLETE | FDCAN_IT_RX_FIFO1_NEW_MESSAGE , 0) != HAL_OK)
  {
    // Notification Error
    Error_Handler();
  }
#endif
}


void resetCanTx(volatile uint8_t CANTxData[8])
{
	for(int i = 0;i < 8;i++){
		CANTxData[i]=0;
	}
}


int getNMTstate(volatile CANData *data )
{

	if ( data->dlcsize == 1 )
	{

	}
  return 0;
}

uint8_t CAN1Send( uint16_t id, uint8_t dlc, uint8_t *pTxData )
{
	can_msg msg;

	msg.id = id;
	msg.dlc = dlc;
	msg.bus = bus1;
	taskENTER_CRITICAL();
	memcpy(msg.data, pTxData, 8);
	taskEXIT_CRITICAL();

	if ( xPortIsInsideInterrupt() )
	{
		if ( !xQueueSendFromISR( CANTxQueue, ( void * ) &msg, NULL ) )
		{
			DebugMsg("failed to add canmsg to bus1 queue!");
		}
	}
	else
	{
		if ( !xQueueSend( CANTxQueue, ( void * ) &msg, ( TickType_t ) 0 ) )
		{
			DebugMsg("failed to add canmsg to bus1 queue!");
		}
	}
	return 0;
}


uint8_t CAN2Send( uint16_t id, uint8_t dlc, uint8_t *pTxData )
{
	can_msg msg;
	msg.id = id;
	msg.dlc = dlc;
	msg.bus = bus0;
	taskENTER_CRITICAL();
	memcpy(msg.data,pTxData, 8);
	taskEXIT_CRITICAL();
	if ( xPortIsInsideInterrupt() )
	{
		if ( !xQueueSendFromISR( CANTxQueue, ( void * ) &msg, NULL ) )
		{
			DebugMsg("failed to add canmsg to bus0 queue!");
		}
	}
	else
	{
		if ( !xQueueSend( CANTxQueue, ( void * ) &msg, ( TickType_t ) 0 ) )
		{
			DebugMsg("failed to add canmsg to bus0 queue!");
		}
	}

	return 0;
}


uint8_t CANSendSDO( enum canbus bus, uint16_t id, uint16_t idx, uint8_t sub, uint32_t data)
{

	uint8_t msg[8];

    msg[0] = 0x22;
    storeLEint16(idx, &msg[1]);
    msg[3] = sub;
    storeLEint32(data, &msg[4]);

#if 1
	static char str[60];
	snprintf(str, 60, "InvSDOsend Id 0x%3X on %s [%2X %2X %2X %2X data %lu] (%lu)", COBSDOS_ID+id, bus==bus0?"bus0":"bus1", msg[0], msg[1], msg[2], msg[3], data, gettimer());
	DebugMsg(str);
#endif
	if ( bus == bus0 )
	{
		CAN2Send( COBSDOS_ID+id, 8, msg );
	} else
	{
		CAN1Send( COBSDOS_ID+id, 8, msg );
	}
	return 0;
}

//canReceiveData


char reTransmitError(uint32_t canid, uint8_t *CANRxData, uint32_t DataLength )
{
#ifndef RETRANSMITBADDATA
	return 0;
#endif

#ifdef CAN2ERRORSTATUS
	CAN2Send( canid, DataLength >> 16, CANRxData );
#endif
	CAN1Send( canid, DataLength >> 16, CANRxData ); // return values.

	return 0;
}

char reTransmitOnCan1(uint32_t canid, uint8_t *CANRxData, uint32_t DataLength )
{
#ifndef sharedCAN // only retransmit if can1 and can2 are not sharing lines.
	CAN1Send( canid, DataLength >> 16, CANRxData ); // return values.
#endif
	return 0;
}

char CAN_NMTSyncRequest( void )
{
	// NMT sync request message.

	// send can id 0x80 to can 0 value 1. Call once per update loop.

	uint8_t CANTxData[1] = { 1 };
	CAN1Send( 0x80, 0, CANTxData ); // return values.
#ifndef sharedCAN
	CAN2Send( 0x80, 0, CANTxData ); // return values.
#endif
	return 1;
	// send to both buses.
}

#ifdef ALTSDO
uint8_t CANSendSDO( uint16_t cod_id, uint16_t  idx,  uint8_t sub, uint32_t data )
{
	uint8_t CANTxData[8] = { 0x2, idx, idx >> 8, sub, data, data >>8, data>>16, data>>24 };

	CAN2Send( cob_id, 8, CANTxData ); // return values.
}
#endif

uint8_t CANSendPDM( uint8_t highvoltage, uint8_t buzz )
{
#ifndef PDMSECONDMESSAGE
	#ifdef FANCONTROL
	uint8_t DataLength = 3; // only two bytes defined in send protocol, check this
	uint8_t CANTxData[3] = { highvoltage, buzz, CarState.FanPowered };
	#else
	uint8_t DataLength = 2; // only two bytes defined in send protocol, check this
	uint8_t CANTxData[2] = { highvoltage, buzz };
	#endif
#else
	uint8_t DataLength = 2;; // only two bytes defined in send protocol, check this
	uint8_t CANTxData[2] = { highvoltage, buzz };
#endif

	return CAN1Send( 0x118, DataLength, CANTxData );
}

#ifdef PDMSECONDMESSAGE
uint8_t CANSendPDMFAN( void )
{
	uint8_t CANTxData[1] = { CarState.FanPowered };
	return CAN1Send( 0x118, 1, CANTxData );
}
#endif

char CAN_SendTimeBase( void ) // sends how long since power up and primary inverter status.
{
	// TODO check time being sent.
	// TODO get inverter states.

	uint32_t time = gettimer();
	uint8_t CANTxData[8] = {
			//CarState.Inverters[RearLeftInverter].InvState,
			Errors.CANSendError1,
			//CarState.Inverters[RearRightInverter].InvState,
			Errors.CANSendError2,
			//CarState.Inverters[FrontLeftInverter].InvState,
			//CarState.Inverters[FrontRightInverter].InvState,
			getByte(time,2),
			getByte(time,3)
	};
#ifdef CAN2ERRORSTATUS
	CAN2Send( 0x101, 8, CANTxData );
#endif

	return CAN1Send( 0x101, 8, CANTxData );
}

char CAN_SendStatus( char state, char substate, uint32_t errorcode )
{
	uint8_t stateless = 0;
	if ( state == 3 )
	{
	 stateless = state;
	} else stateless = state;

	if ( state == 4 )
	{
	 stateless = state;
	} else stateless = state;

	if ( state == 5 )
	{
	 stateless = state;
	} else stateless = state;

	uint8_t CANTxData[8] = { stateless, substate, getByte(errorcode, 0), getByte(errorcode, 1), getByte(errorcode, 2), getByte(errorcode, 3),HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1), ( ShutdownCircuitState() << 7 )+ HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2)}; // HAL_FDCAN_GetxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0)
#ifdef CAN2ERRORSTATUS
	CAN2Send( ECU_CAN_ID, 8, CANTxData );
#endif
	return CAN1Send( ECU_CAN_ID, 8, CANTxData );
}


char CAN_SendErrorStatus( char state, char substate, uint32_t errorcode )
{

	uint8_t stateless = 0;
	if ( state == 3 )
	{
	 stateless = state;
	} else stateless = state;

	if ( state == 4 )
	{
	 stateless = state;
	} else stateless = state;

	if ( state == 5 )
	{
	 stateless = state;
	} else stateless = state;

	uint8_t CANTxData[8] = { stateless, substate, getByte(errorcode, 0), getByte(errorcode, 1), getByte(errorcode, 2), getByte(errorcode, 3),HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1),HAL_FDCAN_GetTxFifoFreeLevel(hfdcan2p)};
#ifdef CAN2ERRORSTATUS
	CAN2Send( ECU_CAN_ID, 8, CANTxData );
#endif
	return CAN1Send( ECU_CAN_ID, 8, CANTxData );
}


char CAN_SendLED( void )
{
#ifdef __RTOS
    uint8_t CANTxData[8] = { 10, LEDs[TSLED_Output].state, LEDs[RTDMLED_Output].state, LEDs[TSOFFLED_Output].state,
							LEDs[IMDLED_Output].state, LEDs[BMSLED_Output].state, LEDs[BSPDLED_Output].state, ShutdownCircuitState()};

    if ( LEDs[TSLED_Output].blinkingrate )
    	CANTxData[1] = LEDs[TSLED_Output].blinkingrate;
    if ( LEDs[RTDMLED_Output].blinkingrate )
    	CANTxData[2] = LEDs[RTDMLED_Output].blinkingrate;
    if ( LEDs[TSOFFLED_Output].blinkingrate )
    	CANTxData[3] = LEDs[TSOFFLED_Output].blinkingrate;
    if ( LEDs[IMDLED_Output].blinkingrate )
    	CANTxData[4] = LEDs[IMDLED_Output].blinkingrate;
    if ( LEDs[BMSLED_Output].blinkingrate )
    	CANTxData[5] = LEDs[BMSLED_Output].blinkingrate;
    if ( LEDs[BSPDLED_Output].blinkingrate )
    {
    	CANTxData[6] = LEDs[BSPDLED_Output].blinkingrate;
    }
#ifdef CAN2ERRORSTATUS
	CAN2Send( ECU_CAN_ID+2, 8, CANTxData );
#endif
	return CAN1Send( ECU_CAN_ID+2, 8, CANTxData );
#else
	return 0;
#endif
}


char CAN_SENDINVERTERERRORS( void )
{

    uint8_t CANTxData[8] =
    {
      0,0,
	  // TODO get inverter state.
      //CarState.Inverters[RearLeftInverter].InvState,
      //CarState.Inverters[RearRightInverter].InvState,
#ifdef SIEMENS
	  CarState.Inverters[RearLeftInverter].InvStateCheck,
	  CarState.Inverters[RearRightInverter].InvStateCheck,
#else
	  0,0,
#endif
#ifndef HPF20
			0,0,0,0
#else
			0,
			//CarState.Inverters[FrontRightInverter].InvState,
#ifdef SIEMENS
			CarState.Inverters[FrontLeftInverter].InvStateCheck,
			CarState.Inverters[FrontRightInverter].InvStateCheck
#else
			0,0
#endif
#endif
    };

#ifdef CAN2ERRORSTATUS
	CAN2Send( 0xEE, 8, CANTxData );
#endif
	CAN1Send( 0xEE, 8, CANTxData );

	for( int j=0;j<Errors.InverterErrorHistoryPosition;j++)
	{
		for( int i=0;i<8;i++){
			CANTxData[i] = Errors.InverterErrorHistory[j][i];
		}
#ifdef CAN2ERRORSTATUS
		CAN2Send( 0xEE, 8, CANTxData );
		DWT_Delay(100);
#endif
		CAN1Send( 0xEE, 8, CANTxData );
		DWT_Delay(100);
	}

	return 0;
}


char CAN_SendADCValue( uint16_t adcdata, uint8_t index )
{
    uint8_t CANTxData[4] = { 20, index, getByte(adcdata, 0), getByte(adcdata, 1) };
	return CAN1Send(ECU_CAN_ID, 4, CANTxData );
}

char CAN_SendADC( volatile uint32_t *ADC_Data, uint8_t error )
{
	uint16_t id = ECU_CAN_ID+0x10;

	if ( error != 0 ){
		id = ECU_CAN_ID+0x12;
	}

	uint8_t CANTxData[8] = {
			getByte(ADC_Data[ThrottleLADC], 0), getByte(ADC_Data[ThrottleLADC], 1),
			getByte(ADC_Data[ThrottleRADC], 0), getByte(ADC_Data[ThrottleRADC], 1),
			getByte(ADC_Data[BrakeFADC], 0), getByte(ADC_Data[BrakeFADC], 1),
			getByte(ADC_Data[BrakeRADC], 0), getByte(ADC_Data[BrakeRADC], 1),
	};

	CAN1Send( id, 8, CANTxData );

	if ( error == 0 ){
		id = ECU_CAN_ID+0x11;
	} else id = ECU_CAN_ID+0x13;

//	int16_t steering12bit = ADC_Data[SteeringADC] >> 4;
#ifdef HPF19
	uint8_t CANTxData2[8] = {
		//	getByte(ADC_Data[SteeringADC], 0), getByte(ADC_Data[SteeringADC], 1),
			getByte(ADC_Data[SteeringADC], 0), getByte(ADC_Data[SteeringADC], 1),
			getByte(ADC_Data[DrivingModeADC], 0), getByte(ADC_Data[DrivingModeADC], 1),
			getByte(ADC_Data[CoolantTempLADC], 0), getByte(ADC_Data[CoolantTempLADC], 1),
			getByte(ADCState.CoolantTempRRaw, 0), getByte(ADCState.CoolantTempRRaw, 1),
//			getByte(ADC_Data[CoolantTempRADC], 0), getByte(ADC_Data[CoolantTempRADC], 1),
	};

	return CAN1Send( id, 8, CANTxData2 );

#endif

#ifdef HPF20
	uint8_t CANTxData2[8] = { // TODO Update for HPF20
			0, 0,
			0, 0,
			0, 0,
			0, 0,
	};
	return CAN1Send( id, 8, CANTxData2 );
#endif
}

char CAN_SendADCminmax( void )
{
#ifdef STMADC
	if ( minmaxADC )
	{
		CAN_SendADCValue(ADC_DataMin[ThrottleLADC],11);
		CAN_SendADCValue(ADC_DataMin[ThrottleRADC],12);
		CAN_SendADCValue(ADC_DataMin[BrakeFADC],13);
		CAN_SendADCValue(ADC_DataMin[BrakeRADC],14);
#ifdef HPF19
		CAN_SendADCValue(ADC_DataMin[SteeringADC],15);
		CAN_SendADCValue(ADC_DataMin[DrivingModeADC],16);
		CAN_SendADCValue(ADC_DataMin[CoolantTempLADC],17);
		CAN_SendADCValue(ADC_DataMin[CoolantTempRADC],18);
#endif
		CAN_SendADCValue(ADC_DataMax[ThrottleLADC],21);
		CAN_SendADCValue(ADC_DataMax[ThrottleRADC],22);

		CAN_SendADCValue(ADC_DataMax[BrakeFADC],23);
		CAN_SendADCValue(ADC_DataMax[BrakeRADC],24);

#ifdef HPF19
		CAN_SendADCValue(ADC_DataMax[SteeringADC],25);
		CAN_SendADCValue(ADC_DataMax[DrivingModeADC],26);
		CAN_SendADCValue(ADC_DataMax[CoolantTempLADC],27);
		CAN_SendADCValue(ADC_DataMax[CoolantTempRADC],28);
#endif
	}
#endif
	return 0;
}

// send nmt command to all nodes.

/*
Start						cs = 1 (01hex) 		Node ID
Stop  						cs = 2 (02hex) 		Node ID
Enter Pre-Operational		cs = 128 (80hex)	Node ID
Reset Node					cs = 129 (81hex)	Node ID
Reset Communication			cs = 130 (82hex)	Node ID

*/

char CAN_NMT( uint8_t command, uint8_t node )
{
#ifndef sharedCAN
	uint8_t CANTxData2[2] = { command, node }; // 0 sends command to all nodes.
	CAN2Send( 0,2, CANTxData2 ); // return values.
#endif
//	DWT_Delay(100); // delay of ~ > 80us needed, or messages entangle and error frame somehow if both can outputs are connected. unknown bug.
	uint8_t CANTxData[2] = { command,node }; // 0 sends command to all nodes.
	CAN1Send( 0,2, CANTxData ); // send command to both buses.
	return 1;
}

char CAN_ConfigRequest( uint8_t command, uint8_t success )
{
	uint8_t CANTxData[3] = { 0x21, command, success }; // 0 sends command to all nodes.
	CAN1Send( 0x20, 3, CANTxData ); // send command to both buses.
	return 1;
}


char CAN_SendErrors( void )
{
#ifndef sharedCAN
#endif

	uint8_t CANTxData[8] = {
			0,0,
			// TODO get inverer state.
			//CarState.Inverters[RearLeftInverter].InvState,
			//CarState.Inverters[RearRightInverter].InvState,
	#ifdef SIEMENS
			CarState.Inverters[RearLeftInverter].InvStateCheck,
			CarState.Inverters[RearRightInverter].InvStateCheck,
	#else
			0,0,
	#endif
#ifndef HPF20
			0,0,0,0
#else
			0,
			//CarState.Inverters[FrontRightInverter].InvState,
	#ifdef SIEMENS
			CarState.Inverters[FrontLeftInverter].InvStateCheck,
			CarState.Inverters[FrontRightInverter].InvStateCheck
	#else
			0,0
	#endif
#endif
    };
//  old message
//	uint8_t CANTxData[8] = {0,0,0,0,CarState.RearLeftInvState,CarState.RearRightInvState,CarState.RearLeftInvStateCheck,CarState.RearRightInvStateCheck}; // 0 sends command to all nodes.
	storeBEint16(Errors.ErrorPlace, &CANTxData[0]);
	storeBEint16(Errors.ErrorReason, &CANTxData[2]);
#ifdef CAN2ERRORSTATUS
	CAN2Send( 0x66, 8, CANTxData );
#endif
	CAN1Send( 0x66, 8, CANTxData ); // send command to both buses.
	return 1;
}


char CANLogDataSlow( void )
{
 return 0;
}

char CANLogDataFast( void )
{
	// build data logging blocks
	uint8_t CANTxData[8];


 //   HAL_Delay(1); // for some reason without small delay here the first log ID 0x7C6 only sometimes sent
    // investigate to find out why, perhaps fifo buffer not acting as expect?

	resetCanTx(CANTxData);
	// TODO get torque request
	//storeBEint16(CarState.Inverters[RearLeftInverter].Torque_Req, &CANTxData[0]); 	//torq_req_l can0 0x7C6 0,16be
	//storeBEint16(CarState.Inverters[RearRightInverter].Torque_Req, &CANTxData[2]); 	//torq_req_r can0 0x7C6 16,16be

	storeBEint16(ADCState.BrakeF, &CANTxData[4]); 	//brk_press_f can0 0x7C6 32,16bee
	storeBEint16(ADCState.BrakeR, &CANTxData[6]); 	//brk_press_r can0 0x7C6 48,16be

	CAN1Send( 0x7C6, 8, CANTxData ); // lagging in sending

	resetCanTx(CANTxData);

	// TODO get speed.
	//storeBEint32(CarState.Inverters[RearLeftInverter].Speed, &CANTxData[0]); //wheel_speed_left_calculated can0 0x7c7 32,32BE

	//storeBEint32(CarState.Inverters[RearRightInverter].Speed, &CANTxData[4]); //wheel_speed_right_calculated can0 0x7c7 0,32BE
	CAN1Send( 0x7C7, 8, CANTxData );

	resetCanTx(CANTxData);

	CANTxData[0] = ADCState.CoolantTempL; //temp_sensor1 can0 0x7c8 0,8
	CANTxData[1] = CarState.Torque_Req_CurrentMax; // CarState.Torque_Req_Max; //torq_req_max can0 0x7c8 8,8
	CANTxData[2] = ADCState.CoolantTempR; 	//temp_sensor_2 can0 0x7c8 16,8
	CANTxData[3] = ADCState.DrivingMode; //future_torq_req_max can0 0x7c8 24,8
	storeBEint16(ADCState.Torque_Req_L_Percent/10, &CANTxData[4]); //torq_req_l_perc can0 0x7c8 32,16be
	storeBEint16(ADCState.Torque_Req_R_Percent/10, &CANTxData[6]); //torq_req_r_perc can0 0x7c8 48,16be
	CAN1Send( 0x7C8, 8, CANTxData );

	resetCanTx(CANTxData);

	// TODO get torque

	//storeBEint16(CarState.Inverters[RearLeftInverter].InvTorque, &CANTxData[0]); //actual_torque_left_inverter_raw can0 0x7c9 0,16be
	//storeBEint16(CarState.Inverters[RearRightInverter].InvTorque, &CANTxData[2]); //actual_torque_right_inverter_raw can0 0x7c9 16,16be

	CAN1Send( 0x7C9, 8, CANTxData );

	resetCanTx(CANTxData);

  // not being sent in current simulink, but is set?
	storeBEint32(CarState.brake_balance,&CANTxData[0]); //brake_balance can0 0x7CA 0,32be
	storeBEint32(ADCState.SteeringAngle,&CANTxData[4]);

	CAN1Send( 0x7CA, 8, CANTxData );

	resetCanTx(CANTxData);

 // IVT

	storeBEint16(CarState.Current, &CANTxData[0]);
#ifdef IVTEnable
	storeBEint16(CarState.VoltageINV, &CANTxData[2]);
	storeBEint16(CarState.VoltageIVTAccu, &CANTxData[6]);
#endif
	storeBEint16(CarState.VoltageBMS, &CANTxData[4]);

//	storeBEint16(CarState.Power, &CANTxData[6]);
//	storeBEint16(ADCloops, &CANTxData[6]);
#ifdef STMADC
	ADCloops=0;
#endif
	CAN1Send( 0x7CB, 8, CANTxData );

	resetCanTx(CANTxData);
	CANTxData[0] = (int8_t)CarState.Current;
	for ( int i=0;i<MOTORCOUNT;i++)
		CANTxData[1+i] = (int8_t)getInvState(i)->InvCurrent;

	CAN1Send( 0x7CC, 8, CANTxData );

	resetCanTx(CANTxData);
	for ( int i=0;i<MOTORCOUNT;i++)
	{
		CANTxData[i] = getInvState(i)->InvTemp;
		CANTxData[i+4] = getInvState(i)->MotorTemp;
	}

	storeBEint16(CarState.LowestCellV, &CANTxData[5]);

	CAN1Send( 0x7CD, 8, CANTxData );


	CAN_SendTimeBase();

	return 0;
}


// process an incoming CAN data packet.
void processCANData(CANData * datahandle, uint8_t * CANRxData, uint32_t DataLength )
{
	// stamp when we received it

	if ( datahandle->getData == NULL )
	{ // No handler, just update timestamp and move along.
		datahandle->time = gettimer();
		return;
	}

	bool baddlc = false;
	bool baddata = false;

	if ( datahandle->dlcsize != DataLength >> 16)
	{
		baddlc = true;
	}
	else
	{
		if ( datahandle->getData(CANRxData, DataLength, datahandle))
		{
			datahandle->receiveerr = 0;
			*datahandle->devicestate = OPERATIONAL;
		} else baddata = true;
	}

	datahandle->time = gettimer();

	if ( baddata || baddlc ) // bad data.
	{
		Errors.CANError++;
		datahandle->receiveerr++;
#ifdef SENDBADDATAERROR
		CAN_SendStatus(ReceiveErr,0,datahandle->id);
#endif
#ifdef RETRANSMITBADDATAERROR
		reTransmitError(99,CANRxData, DataLength);
#endif
	}

}


int receivedCANData( CANData * datahandle )
{
	if ( datahandle->devicestate == NULL )
	{
		return -1; // no device state associated.
	}
	uint32_t time=gettimer();

#ifdef NOTIMEOUT
		if ( datahandle->devicestate == OPERATIONAL )
		{
			datahandle->errorsent = false;
			return 1;
		} else return 0;
#endif

	if ( datahandle->timeout > 0 )
	{
		if ( time - datahandle->time <= datahandle->timeout && *datahandle->devicestate == OPERATIONAL )
		{
			datahandle->errorsent = false;
			return 1;
		} else
		{
			if ( *datahandle->devicestate != OFFLINE )
			{
				if ( datahandle->doTimeout != NULL ) datahandle->doTimeout(datahandle->id);

				if ( !datahandle->errorsent )
				{
					CAN_SendStatus(ReceiveTimeout,0,(time-datahandle->time)/10); // TODO assign a better ID
					datahandle->errorsent = true;
					Errors.CANTimeout++;
					*datahandle->devicestate = OFFLINE;
				}
				return 0;
			}
			return 0;
		}

	} else return 1; // set to never time out.
}


CANData * CanBUS1Messages[2048]; // every possible id, so that can do a direct ID lookup.
uint32_t CANBUS1MessageCount;

CANData * CanBUS2Messages[2048];
uint32_t CANBUS2MessageCount;

#define MAXTIMEOUTLIST	128

CANData * CanTimeoutList[MAXTIMEOUTLIST];
uint32_t CanTimeoutListCount;

bool RegisterCanTimeout( CANData * CanMessage)
{
	if ( CanTimeoutListCount < MAXTIMEOUTLIST) // if there's a timeout, register timeout handler.
	{
		if ( CanMessage->timeout > 0 )
		{
			CanTimeoutList[CanTimeoutListCount] = CanMessage;
			CanTimeoutListCount++;
		}
		return true;
	} else
		return false;
}

int RegisterCan1Message(CANData * CanMessage)
{
	char str[80];
	if ( CanMessage != NULL && CanMessage->id != 0)
	{
		if ( CanBUS1Messages[CanMessage->id] != NULL)
		{
			snprintf(str, 80, "Tried to add a duplicate CAN id %3X on bus1!", CanMessage->id);
			DebugMsg(str);
		}
		else
		{
			if (! RegisterCanTimeout( CanMessage) )
				return 1;

			CanBUS1Messages[CanMessage->id] = CanMessage;
			CANBUS1MessageCount++;
		}
		return 0;
	} else return 1;
}

int RegisterCan2Message(CANData * CanMessage)
{
	char str[80];
#ifdef sharedCAN
	return RegisterCan1Message(CanMessage);
#else
	if ( CanMessage != NULL && CanMessage->id != 0)
	{
		if ( CanBUS2Messages[CanMessage->id] != NULL)
		{
			snprintf(str, 80, "Tried to add a duplicate CAN id %3X on bus0!", CanMessage->id);
			DebugMsg(str);
		}
		else
		{
			if (! RegisterCanTimeout( CanMessage) )
				return 1;

			CanBUS2Messages[CanMessage->id] = CanMessage;
			CANBUS2MessageCount++;
		}
		return 0;
	} else return 1;
#endif
}

bool processCan1Message( FDCAN_RxHeaderTypeDef *RxHeader, uint8_t CANRxData[8])
{
	if ( CanBUS1Messages[RxHeader->Identifier] != NULL)
	{
		processCANData(CanBUS1Messages[RxHeader->Identifier], CANRxData, RxHeader->DataLength );
		return true;
	} return false; // ID not registered in handler.
}

bool processCan2Message( FDCAN_RxHeaderTypeDef *RxHeader, uint8_t CANRxData[8])
{
	if ( CanBUS2Messages[RxHeader->Identifier] != NULL)
	{
		processCANData(CanBUS2Messages[RxHeader->Identifier], CANRxData, RxHeader->DataLength );
		return true;
	} return false; // ID not registered in handler.
}

void processCanTimeouts( void )
{
	for ( int i = 0; i < CanTimeoutListCount; i++ )
	{
		receivedCANData(CanTimeoutList[i]);
	}
//	receiveAnalogNodes();
//	receivePowerNodes();
//	receiveIMU();
//	receiveIVT();
//	receiveBMS();
#ifdef PDM
//	receivePDM();
#endif
}




/**
 * interrupt rx callback for canbus messages
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	FDCAN_RxHeaderTypeDef RxHeader;

	if(hfdcan->Instance == FDCAN1){
		blinkOutput(LED3, BlinkVeryFast, Once);
		Errors.CANCount1++;
	} else if(hfdcan->Instance == FDCAN2) {
		blinkOutput(LED2, BlinkVeryFast, Once);
		 Errors.CANCount2++;
	}

	if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
		/* We have not woken a task at the start of the ISR. */
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		can_msg msg;

		uint8_t CANRxData[8];

		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, CANRxData) != HAL_OK)
		{
			// Reception Error
			Error_Handler();
		}

		msg.id = RxHeader.Identifier;
		msg.bus = bus1;
		msg.dlc = RxHeader.DataLength;
		memcpy(msg.data, CANRxData, 8);

		xQueueSendFromISR( CANRxQueue, &msg, &xHigherPriorityTaskWoken );

	    if( xHigherPriorityTaskWoken )
	    {
	        /* Actual macro used here is port specific. */
	        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	    }

		volatile uint8_t bufferlevel = HAL_FDCAN_GetRxFifoFillLevel(hfdcan,FDCAN_RX_FIFO0);
		if (bufferlevel > 25 ) // buffer shouldn't fill up under normal use, not sending >30 messages per cycle.
		{
			// return error, can fifo full.
//			CAN_SendErrorStatus( 111, 0, bufferlevel );
			bufferlevel++;
		}

		// process incoming packet

		RxHeader.Identifier = 0; // workaround: rx header does not seem to get reset properly?
								 // receiving e.g. 15 after 1314 seems to result in 1315

		if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
		{
			Error_Handler();
		}
	}
}

/**
 * interrupt rx callback for canbus, send error messages here?
 *
 *
 */
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t CANRxData[8];

	if(hfdcan->Instance == FDCAN1){
		blinkOutput(LED3, BlinkVeryFast, Once);
		Errors.CANCount1++;
	} else if(hfdcan->Instance == FDCAN2) {
		blinkOutput(LED2, BlinkVeryFast, Once);
		Errors.CANCount2++;
	}

	if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET) // if there is a message waiting process it
	{
		BaseType_t xHigherPriorityTaskWoken;

		/* We have not woken a task at the start of the ISR. */
		xHigherPriorityTaskWoken = pdFALSE;

		can_msg msg;

		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &RxHeader, CANRxData) != HAL_OK)
		{
			// Reception Error
			Error_Handler();
		}

		msg.id = RxHeader.Identifier;
		msg.bus = bus0;
		msg.dlc = RxHeader.DataLength;
		memcpy(msg.data, CANRxData, 8);

		xQueueSendFromISR( CANRxQueue, &msg, &xHigherPriorityTaskWoken );

	    if( xHigherPriorityTaskWoken )
	    {
	        /* Actual macro used here is port specific. */
	        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	    }

		volatile uint8_t bufferlevel = HAL_FDCAN_GetRxFifoFillLevel(hfdcan,FDCAN_RX_FIFO1);
		if (bufferlevel > 25 ) // buffer shouldn't fill up under normal use, not sending >30 messages per cycle.
		{
			// return error, can fifo full.
//			CAN_SendErrorStatus( 111, 0, bufferlevel );
			bufferlevel++;
		}

		// process incoming packet

		RxHeader.Identifier = 0; // workaround: rx header does not seem to get reset properly?
								 // receiving e.g. 15 after 1314 seems to result in 1315

		if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) != HAL_OK)
		{
			Error_Handler();
		}
	}
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *canp) {

// Re-enable receive interrupts

// (The error handling code in HAL_CAN_IRQHandler() disables this for some reason)
 setOutput(LED3,On);
 setOutput(LED2,On);
 setOutput(LED1,On);
 setOutputNOW(IMDLED,On);
 while ( 1 ){

 }
//__HAL_CAN_ENABLE_IT(canp, FDCAN_I FDCAN_IT_FMP1);
// (or only re-enable the one you are using)
}


void HAL_FDCAN_TimeoutOccurredCallback(FDCAN_HandleTypeDef *hfdcan)
{
	DebugMsg("CanTX Timeout");

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(hfdcan->Instance == FDCAN1){
		xSemaphoreGiveFromISR(bus1TXDone, &xHigherPriorityTaskWoken);
	} else if(hfdcan->Instance == FDCAN2) {
		xSemaphoreGiveFromISR(bus0TXDone, &xHigherPriorityTaskWoken);
	}

	if ( xHigherPriorityTaskWoken )
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

void HAL_FDCAN_TxFifoEmptyCallback(FDCAN_HandleTypeDef *hfdcan)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(hfdcan->Instance == FDCAN1){
		xSemaphoreGiveFromISR(bus1TXDone, &xHigherPriorityTaskWoken);
	} else if(hfdcan->Instance == FDCAN2) {
		xSemaphoreGiveFromISR(bus0TXDone, &xHigherPriorityTaskWoken);
	}

	if ( xHigherPriorityTaskWoken )
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// not sure where properly defined

#define FDCAN_ELEMENT_MASK_EFC   ((uint32_t)0x00800000U) /* Event FIFO Control          */

void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
	if(hfdcan->Instance == FDCAN1){
		if ( DeviceState.CAN1 == OPERATIONAL )
		{
			if ( ErrorStatusITs != FDCAN_ELEMENT_MASK_EFC )
			{
				DebugPrintf("Can ErrorStatus bus1 %4x", ErrorStatusITs);
			}
		}

	} else if(hfdcan->Instance == FDCAN2) {
		if ( DeviceState.CAN0 == OPERATIONAL )
		{
			if ( ErrorStatusITs != FDCAN_ELEMENT_MASK_EFC )
				DebugPrintf("Can ErrorStatus bus2 %4x", ErrorStatusITs);
		}
	}
}


int CheckCanError( void )
{
	int result = 0;

	FDCAN_ProtocolStatusTypeDef CAN1Status, CAN2Status;

	HAL_FDCAN_GetProtocolStatus(&hfdcan1, &CAN1Status);
	HAL_FDCAN_GetProtocolStatus(&hfdcan2, &CAN2Status);

	static uint8_t offcan1 = 0;
#ifndef ONECAN
	static uint8_t offcan2 = 0;
#endif
#ifdef RECOVERCAN
	static uint32_t offcan1time = 0;
#ifndef ONECAN
	static uint32_t offcan2time = 0;
#endif
#endif

	if ( CAN1Status.BusOff) // detect passive error instead and try to stay off bus till clears?
	{
	//	Errors.ErrorPlace = 0xAA;
		  blinkOutput(TSOFFLED, LEDBLINK_FOUR, 1);
		  HAL_FDCAN_Stop(&hfdcan1);
		  CAN_SendStatus(255,0,0);

		  if ( offcan1 == 0 )
		  {
#ifdef RECOVERCAN
			  offcan1time = gettimer();
#endif
			  offcan1 = 1;
			  DeviceState.CAN1 = OFFLINE;
//					  offcan1count++; // increment occurances of coming off bus. if reach threshhold, go to error state.
		  }
		  Errors.ErrorPlace = 0xF1;
		  LogError("CANBUS1 Down");
//		  result +=1;
	}

#ifdef RECOVERCAN
	if ( CAN1Status.BusOff && offcan1time+1000 >  gettimer() )
	{

		if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) // add a 5 cycle counter before try again? check in can1 send to disable whilst bus not active.
		{
		// Start Error
			 LogError("CANBUS1 Offline");
			 Errors.ErrorPlace = 0xF2;
			 result +=2;
		} else
		{
			offcan1 = 0;
			Errors.ErrorPlace = 0xF1;
			LogError("CANBUS1 Up");
			DeviceState.CAN1 = OPERATIONAL;
			CAN_SendStatus(254,0,0);
		}
	}
#endif

#ifndef ONECAN
	if ( CAN2Status.BusOff) // detect passive error instead and try to stay off bus till clears?
	{
	//	Errors.ErrorPlace = 0xAA;
		blinkOutput(BMSLED, LEDBLINK_FOUR, 1);
		HAL_FDCAN_Stop(&hfdcan2);
		CAN_SendStatus(255,0,0);
		DeviceState.CAN0 = OFFLINE;

		if ( offcan2 == 0 )
		{
#ifdef RECOVERCAN
		  offcan2time = gettimer();
#endif
		  offcan2 = 1;
		}
		Errors.ErrorPlace = 0xF3;
		LogError("CANBUS0 Down");
		//		  result +=4;
	}

#ifdef RECOVERCAN
	if ( CAN2Status.BusOff && offcan2time+5000 >  gettimer() )
	{
		if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK) // add a 5 cycle counter before try again? check in can1 send to disable whilst bus not active.
		{
		// Start Error
			Errors.ErrorPlace = 0xF4;
			LogError("CANBUS0 Offline");
			result +=8;
		} else
		{
			offcan2 = 0;
			LogError("CANBUS0 Up");
			DeviceState.CAN0 = OPERATIONAL;
			CAN_SendStatus(254,0,0);
		}
	}
#endif
#endif

	return result;

}

void resetCAN( void )
{
#ifdef LOGGINGON
	DeviceState.LoggingEnabled = ENABLED;
#else
	DeviceState.LoggingEnabled = DISABLED;
#endif

	DeviceState.CAN1 = OPERATIONAL;
	DeviceState.CAN0 = OPERATIONAL;

}

int initCAN( void )
{
	CANTxBuffer = CANTxBuffer1;
	CurCANTxBuffer = CANTxBuffer1;

	CanTimeoutListCount = 0;
	RegisterResetCommand(resetCAN);
	resetCAN();

	CANBufferUpdating = xSemaphoreCreateMutex();
	bus0TXDone = xSemaphoreCreateMutex();
	bus1TXDone = xSemaphoreCreateMutex();

	MX_FDCAN1_Init();
#ifndef ONECAN
	MX_FDCAN2_Init();
#endif

	FDCAN1_start(); // sets up can ID filters and starts can bus 1
	FDCAN2_start(); // starts up can bus 2

	CANTxQueue = xQueueCreateStatic( CANTxQUEUE_LENGTH,
								CANTxITEMSIZE,
								CANTxQueueStorageArea,
								&CANTxStaticQueue );

	vQueueAddToRegistry(CANTxQueue, "CANTxQueue" );

	CANRxQueue = xQueueCreateStatic( CANRxQUEUE_LENGTH,
								CANRxITEMSIZE,
								CANRxQueueStorageArea,
								&CANRxStaticQueue );

	vQueueAddToRegistry(CANRxQueue, "CANRxQueue" );

	CANTxTaskHandle = xTaskCreateStatic(
						  CANTxTask,
						  CANTXTASKNAME,
						  CANTXSTACK_SIZE,
						  ( void * ) 1,
						  CANTXTASKPRIORITY,
						  xCANTXStack,
						  &xCANTXTaskBuffer );

	CANRxTaskHandle = xTaskCreateStatic(
						  CANRxTask,
						  CANRXTASKNAME,
						  CANRXSTACK_SIZE,
						  ( void * ) 1,
						  CANRXTASKPRIORITY,
						  xCANRXStack,
						  &xCANRXTaskBuffer );

	CAN_SendStatus(0,0,1); // earliest possible place to send can message signifying wakeup.

	return 0;
}

