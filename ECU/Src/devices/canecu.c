/*
 * ecu.c
 *
 *  Created on: 30 Dec 2018
 *      Author: Visa
 */

#include "ecumain.h"


#ifdef LCD
  #include "vhd44780.h"
#endif


#ifdef ONECAN
	#define sharedCAN
#endif

FDCAN_HandleTypeDef * hfdcan2p = NULL;

//variables that need to be accessible in ISR's

int cancount;


osThreadId_t CANTxTaskHandle;
const osThreadAttr_t CANTxTask_attributes = {
  .name = "CANTxTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 128 * 4
};


osThreadId_t CANRxTaskHandle;
const osThreadAttr_t CANRxTask_attributes = {
  .name = "CANRxTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 128 * 16
};


/* The queue is to be created to hold a maximum of 10 uint64_t
variables. */
#define CANTxQUEUE_LENGTH    32
#define CANRxQUEUE_LENGTH    32
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


bool processCan1Message( FDCAN_RxHeaderTypeDef *RxHeader, uint8_t CANRxData[8]);
bool processCan2Message( FDCAN_RxHeaderTypeDef *RxHeader, uint8_t CANRxData[8]);

void CANTxTask(void *argument)
{

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

	FDCAN_HandleTypeDef * hfdcanp = &hfdcan1;;

	volatile uint32_t * pCANSendError = &Errors.CANSendError1;

	can_msg msg;

	for(;;)
	{
        // UBaseType_t uxQueueMessagesWaiting( QueueHandle_t xQueue );
		if ( xQueueReceive(CANTxQueue,&msg,portMAX_DELAY) )
		{
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

			TxHeader.Identifier = msg.id; // decide on an ECU ID/
			TxHeader.DataLength = msg.dlc << 16; // only two bytes defined in send protocol, check this

			if ( HAL_FDCAN_GetTxFifoFreeLevel(hfdcanp) < 1 )
			{
				// return error, can fifo full.
			//	Error_Handler();
			} else
			if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcanp, &TxHeader, msg.data) != HAL_OK)
			{

				if ( pCANSendError != NULL )
					(*pCANSendError)++;
		//		return 1;
			//	Error_Handler();
			}
		}
	}

	osThreadTerminate(NULL);
}


void CANRxTask(void *argument)
{

	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT( CANRxQueue );

	can_msg msg;

	for(;;)
	{
        // UBaseType_t uxQueueMessagesWaiting( QueueHandle_t xQueue );
		if( xQueueReceive(CANRxQueue,&msg,portMAX_DELAY) )
		{
			FDCAN_RxHeaderTypeDef RxHeader;
			RxHeader.Identifier = msg.id;
			RxHeader.DataLength = msg.dlc;

			if ( msg.bus == bus1)
			{
				if ( !processCan1Message(&RxHeader, msg.data) )
				switch ( msg.id )
				{
					default : // unknown identifier encountered, ignore. Shouldn't be possible to get here due to filters.
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
					default : // unknown identifier encountered, ignore. Shouldn't be possible to get here due to filters.
		#ifdef HPF20
						blinkOutput(LED7, BlinkVeryFast, 1);
		#endif
						break;
				}

			}
		}
	}

	osThreadTerminate(NULL);
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

#ifdef CANFILTERS
  FDCAN_FilterTypeDef	sFilterConfig1;
  HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, DISABLE, DISABLE);
#else
  HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0, DISABLE, DISABLE);
#endif

  HAL_FDCAN_ConfigRxFifoOverwrite(&hfdcan1, FDCAN_RX_FIFO0, FDCAN_RX_FIFO_OVERWRITE);

#ifdef CANFILTERS
  // Configure Rx filter to only accept expected ID's into receive interrupt
  sFilterConfig1.IdType = FDCAN_STANDARD_ID; // standard, not extended FD frame filter.
  sFilterConfig1.FilterType = FDCAN_FILTER_RANGE; // filter all the id's between id1 and id2 in filter definition.
  sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // FDCAN_FILTER_TO_RXFIFO0; // set can1 to receive via fifo0

  sFilterConfig1.FilterIndex = 1; // BMS CAN 1
  sFilterConfig1.FilterID1 = BMSBASE_ID; // 0xf 0x0 for all
  sFilterConfig1.FilterID2 = BMSBASE_ID+3; // 07ff  0x0 for all


  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

 // HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, DISABLE, DISABLE);

  sFilterConfig1.IdType = FDCAN_STANDARD_ID; // standard, not extended FD frame filter.
  sFilterConfig1.FilterType = FDCAN_FILTER_RANGE; // filter all the id's between id1 and id2 in filter definition.
  sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // FDCAN_FILTER_TO_RXFIFO0; // set can1 to receive via fifo0

  sFilterConfig1.FilterIndex++; // ECU CAN 1
  sFilterConfig1.FilterID1 = ECU_CAN_ID;
  sFilterConfig1.FilterID2 = ECU_CAN_ID+1;


  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  sFilterConfig1.FilterIndex++; // filter for PDM CAN 1
  sFilterConfig1.FilterID1 = PDM_ID;
  sFilterConfig1.FilterID2 = PDM_ID;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }


  #ifdef POWERNODES
  sFilterConfig1.FilterIndex++; // filter for canbus ADC id's
  sFilterConfig1.FilterID1 = NodeErr_ID;
  sFilterConfig1.FilterID2 = NodeAck_ID;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  sFilterConfig1.FilterIndex++; // filter for canbus ADC id's
  sFilterConfig1.FilterID1 = AnalogNode9_ID;
  sFilterConfig1.FilterID2 = PowerNode37_ID;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  #endif

  sFilterConfig1.FilterIndex++; // filter for fake button presses id's + induce hang.
  sFilterConfig1.FilterID1 = AdcSimInput_ID;
  sFilterConfig1.FilterID2 = AdcSimInput_ID+6;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

#ifdef MEMORATOR

  sFilterConfig1.FilterIndex++; // filter for fake button presses id's + induce hang.
  sFilterConfig1.FilterID1 = MEMORATOR_ID;
  sFilterConfig1.FilterID2 = MEMORATOR_ID;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }
#endif


  // Front WheelSpeed CANOpen Filters

   sFilterConfig1.FilterType = FDCAN_FILTER_MASK;  // configure CANOpen device filters by mask.

   sFilterConfig1.FilterIndex++;
   sFilterConfig1.FilterID1 = FLSpeed_COBID;
   sFilterConfig1.FilterID2 = 0b00001111111;

   if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
   {
     // Filter configuration Error
     Error_Handler();
   }

   sFilterConfig1.FilterIndex++;
   sFilterConfig1.FilterID1 = FRSpeed_COBID;
   sFilterConfig1.FilterID2 = 0b00001111111;

   if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
   {
     // Filter configuration Error
     Error_Handler();
   }
#endif

#ifndef ONECAN
  /* Start the FDCAN module */
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
  {
    // Start Error
    Error_Handler();
  }

  // start can receive interrupt for CAN1
  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
  {
    // Notification Error
    Error_Handler();
  }

#endif
}

void FDCAN2_start(void)
{
#ifdef CANFILTERS
	  FDCAN_FilterTypeDef sFilterConfig2;
#endif

#ifdef ONECAN
  hfdcan2p = &hfdcan1;

  #ifdef CANFILTERS
  sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // set can2 to receive into fifo1
  #endif

#else

  #ifdef CANFILTERS
  sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO1; // set can2 to receive into fifo1
  HAL_FDCAN_ConfigGlobalFilter(hfdcan2p, FDCAN_REJECT, FDCAN_REJECT, DISABLE, DISABLE);
  #else
  HAL_FDCAN_ConfigGlobalFilter(hfdcan2p, FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_ACCEPT_IN_RX_FIFO1, DISABLE, DISABLE);
  #endif
  HAL_FDCAN_ConfigRxFifoOverwrite(hfdcan2p, FDCAN_RX_FIFO1, FDCAN_RX_FIFO_OVERWRITE);
#endif

#ifdef CANFILTERS
  // Configure Rx filter for can2
  sFilterConfig2.IdType = FDCAN_STANDARD_ID;
  sFilterConfig2.FilterIndex = 64;
  sFilterConfig2.FilterType = FDCAN_FILTER_RANGE;

  sFilterConfig2.FilterID1 = 0x1;
  sFilterConfig2.FilterID2 = 0x1;

//  HAL_FDCAN_ConfigInterruptLines

  if (HAL_FDCAN_ConfigFilter(hfdcan2p, &sFilterConfig2) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }


   sFilterConfig2.FilterIndex++; // filter IVT MSG. CAN2
   sFilterConfig2.FilterID1 = IVTMsg_ID;
   sFilterConfig2.FilterID2 = IVTMsg_ID;

   if (HAL_FDCAN_ConfigFilter(hfdcan2p, &sFilterConfig2) != HAL_OK)
   {
     // Filter configuration Error
     Error_Handler();
   }


   sFilterConfig2.FilterIndex++; // IVT CAN2
   sFilterConfig2.FilterID1 = IVTBase_ID;
   sFilterConfig2.FilterID2 = IVTBase_ID+7; // IVTI_ID

   if (HAL_FDCAN_ConfigFilter(hfdcan2p, &sFilterConfig2) != HAL_OK)
   {
     // Filter configuration Error
     Error_Handler();
   }

  // Inverter CANOpen Filters

  sFilterConfig2.FilterType = FDCAN_FILTER_MASK;  // configure CANOpen device filters by mask.

  for ( int i = 0;i<MOTORCOUNT;i++)
  {
	  sFilterConfig2.FilterIndex++;
	  sFilterConfig2.FilterID1 = CarState.Inverters[i].COBID;
	  sFilterConfig2.FilterID2 = 0b00001111111; // 0x1fe - 0x1ff   0x2fe  - 0x2ff    0x77e - 0x77f

	  if (HAL_FDCAN_ConfigFilter(hfdcan2p, &sFilterConfig2) != HAL_OK)
	  {
	    // Filter configuration Error
	    Error_Handler();
	  }
  }

  sFilterConfig2.FilterIndex++; // filter for canbus ADC id's
  sFilterConfig2.FilterID1 = AnalogNode1_ID;
  sFilterConfig2.FilterID2 = AnalogNode1_ID+1;

  if (HAL_FDCAN_ConfigFilter(hfdcan2p, &sFilterConfig2) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }


  /*
  sFilterConfig2.FilterIndex++;
  sFilterConfig2.FilterID1 = 0x77E;
  sFilterConfig2.FilterID2 = 0x77F;

  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  } */
#endif

#ifdef ONECAN
  // Start the FDCAN module
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
  {
    //  Start Error
    Error_Handler();
  }

  // start can receive interrupt for first CAN's messages

  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE , 0) != HAL_OK)
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

  if (HAL_FDCAN_ActivateNotification(hfdcan2p, FDCAN_IT_RX_FIFO1_NEW_MESSAGE , 0) != HAL_OK)
  {
    // Notification Error
    Error_Handler();
  }
#endif
}

/*
int ResetCanReceived( void ) // only wait for new data since request.
{
	// inverters.

	for ( int i=0;i<MOTORCOUNT;i++)
	{
//		ResetCanData(&CanState.InverterPDO1[i]);
		ResetCanData(&CanState.InverterPDO2[i]);
		ResetCanData(&CanState.InverterPDO3[i]);
		ResetCanData(&CanState.InverterPDO4[i]);
		ResetCanData(&CanState.InverterERR[i]);
	}

	// front wheel encoders
#ifndef HPF20
	ResetCanData(&CanState.FLeftSpeedERR);
	ResetCanData(&CanState.FRightSpeedERR);
	ResetCanData(&CanState.FLeftSpeedPDO1);
	ResetCanData(&CanState.FRightSpeedPDO1);
	ResetCanData(&CanState.FLeftSpeedNMT);
	ResetCanData(&CanState.FRightSpeedNMT);
#endif
	return 0;
} */

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

void ResetCanData(volatile CANData *data )
{
	data->time = 0;
	for ( int i=0; i<data->dlcsize; i++)
	{
	//	data->data[i] = 0;
	}
//	data->newdata = 0;
//	data->processed = 0;
}

char CAN1Send( uint16_t id, uint8_t dlc, uint8_t *pTxData )
{
#ifdef RTOS
	can_msg msg;

	msg.id = id;
	msg.dlc = dlc;
	msg.bus = bus1;
	taskENTER_CRITICAL();
	memcpy(msg.data, pTxData, 8);
	taskEXIT_CRITICAL();

	if ( xPortIsInsideInterrupt() )
		xQueueSendFromISR( CANTxQueue, ( void * ) &msg, NULL );
	else
		xQueueSend( CANTxQueue, ( void * ) &msg, ( TickType_t ) 0 );

#else
 softly	FDCAN_TxHeaderTypeDef TxHeader = {
	.Identifier = id, // decide on an ECU ID/
	.IdType = FDCAN_STANDARD_ID,
	.TxFrameType = FDCAN_DATA_FRAME,
	.DataLength = dlc << 16, // only two bytes defined in send protocol, check this
	.ErrorStateIndicator = FDCAN_ESI_ACTIVE,
	.BitRateSwitch = FDCAN_BRS_OFF,
	.FDFormat = FDCAN_CLASSIC_CAN,
	.TxEventFifoControl = FDCAN_NO_TX_EVENTS,
	.MessageMarker = 0
	};


	if ( DeviceState.CAN1 == OFFLINE )
	{
		return 3;
	}

	// loop till free slot if none, rather than give up as currently set. Let watchdog catch if gets into loop unable to send?
	// perhaps have two send routines, critical and non critical.

	char returnval = 0;
	uint8_t bufferlevel = HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1);
	if (bufferlevel <1 ) // buffer shouldn't fill up under normal use, not sending >30 messages per cycle.
	{
		// return error, can fifo full.
//		CAN_SendStatus( 110, 0, bufferlevel ); // of course this was erroring. need to send specific message, not use a function that calls back to here, recursion.
		return 2;
	}

//	char CAN_SendStatus( char state, char substate, uint32_t errorcode )

//	if ( !HAL_FDCAN_IsRestrictedOperationMode(&hfdcan1) )
	{
		if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, pTxData) != HAL_OK)
		{
				  // Transmission request Error
			//     HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin, 1);
			Errors.CANSendError1++;
			return 1;
			//Error_Handler();

		}
	}// else
	{
//		Errors.CANSendError1++;
	}
#endif
	return 0;
}


char CAN2Send( uint16_t id, uint8_t dlc, uint8_t *pTxData )
{
#ifdef RTOS
	can_msg msg;
	msg.id = id;
	msg.dlc = dlc;
	msg.bus = bus0;
	taskENTER_CRITICAL();
	memcpy(msg.data,pTxData, 8);
	taskEXIT_CRITICAL();
	if ( xPortIsInsideInterrupt() )
		xQueueSendFromISR( CANTxQueue, ( void * ) &msg, NULL );
	else
		xQueueSend( CANTxQueue, ( void * ) &msg, ( TickType_t ) 0 );
#else
	FDCAN_TxHeaderTypeDef TxHeader = {
	.Identifier = id, // decide on an ECU ID/
	.IdType = FDCAN_STANDARD_ID,
	.TxFrameType = FDCAN_DATA_FRAME,
	.DataLength = dlc << 16, // only two bytes defined in send protocol, check this
	.ErrorStateIndicator = FDCAN_ESI_ACTIVE,
	.BitRateSwitch = FDCAN_BRS_OFF,
	.FDFormat = FDCAN_CLASSIC_CAN,
	.TxEventFifoControl = FDCAN_NO_TX_EVENTS,
	.MessageMarker = 0
	};

	if ( DeviceState.CAN2 == OFFLINE )
	{
		return 3;
	}

	if ( HAL_FDCAN_GetTxFifoFreeLevel(hfdcan2p) < 1 )
	{
		// return error, can fifo full.
		return 2;
	}

	if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan2p, &TxHeader, pTxData) != HAL_OK)
	{
		Errors.CANSendError2++;
		return 1;
//		Error_Handler();

	}
#endif

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
	CAN1Send( 0x80, 1, CANTxData ); // return values.
#ifndef sharedCAN
	CAN2Send( 0x80, 1, CANTxData ); // return values.
#endif
	return 1;
	// send to both buses.
}

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

	uint32_t time = gettimer();
	uint8_t CANTxData[8] = {
			CarState.Inverters[RearLeftInverter].InvState,
			Errors.CANSendError1,
			CarState.Inverters[RearRightInverter].InvState,
			Errors.CANSendError2,
			CarState.Inverters[FrontLeftInverter].InvState,
			CarState.Inverters[FrontRightInverter].InvState,
	//		getByte(time,0),
	//		getByte(time,1),
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

	uint8_t CANTxData[8] = { stateless, substate, getByte(errorcode, 0), getByte(errorcode, 1), getByte(errorcode, 2), getByte(errorcode, 3),HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1),HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2)}; // HAL_FDCAN_GetxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0)
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
#ifndef RTOS
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
    { CarState.Inverters[RearLeftInverter].InvState,CarState.Inverters[RearRightInverter].InvState, CarState.Inverters[RearLeftInverter].InvStateCheck, CarState.Inverters[RearRightInverter].InvStateCheck,
#ifndef HPF20
			0,0,0,0
#else
			CarState.Inverters[FrontRightInverter].InvState, CarState.Inverters[FrontLeftInverter].InvStateCheck, CarState.Inverters[FrontRightInverter].InvStateCheck
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
	return 0;
}

char CAN_SendIVTTrigger( void )
{
	uint8_t CANTxData[8] = { 49, 0, 175,0,0,0,0,0 };
	return CAN1Send( IVTCmd_ID, 8, CANTxData );
}


char CAN_SendIVTTurnon( void )
{
	uint8_t CANTxData[8] = { 52, 1,0,0,0,0,0,0 }; // turn on from pre operation.
	return CAN1Send( IVTCmd_ID, 8, CANTxData );
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

char CANSendInverter( uint16_t response, uint16_t request, uint8_t inverter )
{

#ifdef HPF19
left  // 0x47e
right // 0x47f
#endif
	uint8_t CANTxData[8];

	resetCanTx(CANTxData);

	storeLEint16(response,&CANTxData[0]);
	storeLEint16(request,&CANTxData[2]);

	return CAN2Send( 0x400 + CarState.Inverters[inverter].COBID, 4, CANTxData );
}


char CAN_SendErrors( void )
{
#ifndef sharedCAN
#endif

	uint8_t CANTxData[8] = { CarState.Inverters[RearLeftInverter].InvState,CarState.Inverters[RearRightInverter].InvState, CarState.Inverters[RearLeftInverter].InvStateCheck, CarState.Inverters[RearRightInverter].InvStateCheck,
#ifndef HPF20
			0,0,0,0
#else
			CarState.Inverters[FrontRightInverter].InvState, CarState.Inverters[FrontLeftInverter].InvStateCheck, CarState.Inverters[FrontRightInverter].InvStateCheck
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
	storeBEint16(CarState.Inverters[RearLeftInverter].Torque_Req, &CANTxData[0]); 	//torq_req_l can0 0x7C6 0,16be
	storeBEint16(CarState.Inverters[RearRightInverter].Torque_Req, &CANTxData[2]); 	//torq_req_r can0 0x7C6 16,16be

	storeBEint16(ADCState.BrakeF, &CANTxData[4]); 	//brk_press_f can0 0x7C6 32,16bee
	storeBEint16(ADCState.BrakeR, &CANTxData[6]); 	//brk_press_r can0 0x7C6 48,16be

	CAN1Send( 0x7C6, 8, CANTxData ); // lagging in sending

	resetCanTx(CANTxData);

	storeBEint32(CarState.Inverters[RearLeftInverter].Speed, &CANTxData[0]); //wheel_speed_left_calculated can0 0x7c7 32,32BE

	storeBEint32(CarState.Inverters[RearRightInverter].Speed, &CANTxData[4]); //wheel_speed_right_calculated can0 0x7c7 0,32BE
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

	storeBEint16(CarState.Inverters[RearLeftInverter].InvTorque, &CANTxData[0]); //actual_torque_left_inverter_raw can0 0x7c9 0,16be
	storeBEint16(CarState.Inverters[RearRightInverter].InvTorque, &CANTxData[2]); //actual_torque_right_inverter_raw can0 0x7c9 16,16be

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
	ADCloops=0;
	CAN1Send( 0x7CB, 8, CANTxData );


	resetCanTx(CANTxData);

	storeBEint32(CarState.Inverters[RearLeftInverter].Speed, &CANTxData[0]); //wheel_speed_left_calculated can0 0x7c7 32,32BE

	storeBEint32(CarState.Inverters[RearRightInverter].Speed, &CANTxData[4]); //wheel_speed_right_calculated can0 0x7c7 0,32BE
	CAN1Send( 0x7C7, 8, CANTxData );

	resetCanTx(CANTxData);

#ifndef HPF20
	storeBEint32(CarState.SpeedFL, &CANTxData[0]); //wheel_speed_left_calculated can0 0x7c7 32,32BE

	storeBEint32(CarState.SpeedFR, &CANTxData[4]); //wheel_speed_right_calculated can0 0x7c7 0,32BE
#else
	storeBEint32(CarState.Inverters[FrontLeftInverter].Speed, &CANTxData[0]); //wheel_speed_left_calculated can0 0x7c7 32,32BE

	storeBEint32(CarState.Inverters[FrontRightInverter].Speed, &CANTxData[4]); //wheel_speed_right_calculated can0 0x7c7 0,32BE
#endif
	CAN1Send( 0x7CC, 8, CANTxData );

	CAN_SendTimeBase();

	return 0;
}

void SetCanData(volatile CANData *data, uint8_t *CANRxData, uint32_t DataLength )
{
	int time = gettimer();
//	data->count++;
//	if ( data->newdata == 0 ) // only update data if previous data has been looked at.
	{
#ifdef OLDCAN
		data->dlcsize = DataLength>>16;
		data->data[0] = CANRxData[0];
		data->data[1] = CANRxData[1];
		data->data[2] = CANRxData[2];
		data->data[3] = CANRxData[3];
		data->data[4] = CANRxData[4];
		data->data[5] = CANRxData[5];
		data->data[6] = CANRxData[6];
		data->data[7] = CANRxData[7];
#endif
		data->time = time;
	//	int copylength = DataLength>>16;
	//	memcpy(data->data, CANRxData, copylength);
	//	data->dlcsize = DataLength>>16;
//		data->newdata = 1; // moved to end to ensure data is not read before updated.
	}
}


void processCANData(CANData * datahandle, uint8_t * CANRxData, uint32_t DataLength )
{
	datahandle->time = gettimer();

	if ( datahandle->getData == NULL )
	{ // No handler.
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


// write a generic handler?
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
			if ( *datahandle->devicestate == OPERATIONAL )
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

int RegisterCan1Message(CANData * CanMessage)
{
	if ( CanMessage != NULL && CanMessage->id != 0)
	{
		CanBUS1Messages[CanMessage->id] = CanMessage;
		CANBUS1MessageCount++;
		return 0;
	} else return 1;
}

int RegisterCan2Message(CANData * CanMessage)
{
#ifdef sharedCAN
	return RegisterCan1Message(CanMessage);
#else
	if ( CanMessage != NULL && CanMessage->id != 0)
	{
		CanBUS2Messages[CanMessage->id] = CanMessage;
		CANBUS2MessageCount++;
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

// TODO Investigate new potential memory corruption. Workaround attemped using circular buffer.


#ifndef RTOS
struct {
		FDCAN_RxHeaderTypeDef RxHeader;
		uint8_t CANRxData[8];
} CanRX[8];

volatile uint8_t CanRXPos = 0;
#endif

/**
 * interrupt rx callback for canbus messages
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	FDCAN_RxHeaderTypeDef RxHeader;

	if(hfdcan->Instance == FDCAN1){
#ifdef RTOS
		blinkOutput(LED3, BlinkVeryFast, Once);
#else
		toggleOutput(LED3);
#endif
		Errors.CANCount1++;
	} else if(hfdcan->Instance == FDCAN2) {
#ifdef RTOS
		blinkOutput(LED2, BlinkVeryFast, Once);
#else
		toggleOutput(LED2);
#endif
		 Errors.CANCount2++;
	}

	if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
#ifndef RTOS
		uint8_t *CANRxData;

		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &CanRX[curpos].RxHeader, &CANRxData) != HAL_OK)
		{
			// Reception Error
			Error_Handler();
		}

		CanRXPos++;
		if ( CanRXPos > 7 ) CanRXPos = 0;

		RxHeader = CanRX[curpos].RxHeader;
		CANRxData = CanRX[curpos].CANRxData;

		if ( !processCan1Message(&RxHeader, CANRxData) )
		switch ( RxHeader.Identifier )
		{
			default : // unknown identifier encountered, ignore. Shouldn't be possible to get here due to filters.
#ifdef HPF20
				toggleOutput(LED7);
#endif
				break;
		}

#else
		BaseType_t xHigherPriorityTaskWoken;

		/* We have not woken a task at the start of the ISR. */
		xHigherPriorityTaskWoken = pdFALSE;

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
#endif

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
#ifdef RTOS
		blinkOutput(LED3, BlinkVeryFast, Once);
#else
		toggleOutput(LED3);
#endif
		Errors.CANCount1++;
	} else if(hfdcan->Instance == FDCAN2) {
#ifdef RTOS
		blinkOutput(LED2, BlinkVeryFast, Once);
#else
		toggleOutput(LED2);
#endif
		 Errors.CANCount2++;
	}

	if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET) // if there is a message waiting process it
	{
#ifndef RTOS
		/* Retrieve Rx message from RX FIFO1 */
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &RxHeader, CANRxData) != HAL_OK)
		{
			/* Reception Error */
			Error_Handler();
		}

		processCan2Message(&RxHeader, CANRxData);
#else
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
#endif

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

void resetCAN( void )
{
#ifdef LOGGINGON
	DeviceState.LoggingEnabled = ENABLED;
#else
	DeviceState.LoggingEnabled = DISABLED;
#endif
}

int initCAN( void )
{
	RegisterResetCommand(resetCAN);
	resetCAN();

	MX_FDCAN1_Init();
#ifndef ONECAN
	MX_FDCAN2_Init();
#endif

	FDCAN1_start(); // sets up can ID filters and starts can bus 1
	FDCAN2_start(); // starts up can bus 2

#ifdef RTOS
	CANTxQueue = xQueueCreateStatic( CANTxQUEUE_LENGTH,
								CANTxITEMSIZE,
								CANTxQueueStorageArea,
								&CANTxStaticQueue );

	vQueueAddToRegistry(OutputQueue, "CANTxQueue" );

	CANRxQueue = xQueueCreateStatic( CANRxQUEUE_LENGTH,
								CANRxITEMSIZE,
								CANRxQueueStorageArea,
								&CANRxStaticQueue );

	vQueueAddToRegistry(OutputQueue, "CANRxQueue" );

	CANTxTaskHandle = osThreadNew(CANTxTask, NULL, &CANTxTask_attributes);
	CANRxTaskHandle = osThreadNew(CANRxTask, NULL, &CANRxTask_attributes);

	CAN_SendStatus(0,0,1); // earliest possible place to send can message signifying wakeup.
#endif


	return 0;
}

