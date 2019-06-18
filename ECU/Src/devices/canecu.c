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

//variables that need to be accessible in ISR's

/**
 * @brief set filter states and start CAN module and it's interrupt for CANBUS1
 */
void FDCAN1_start(void)
{
  FDCAN_FilterTypeDef	sFilterConfig1;

  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    // Initialization Error
    Error_Handler();
  }

  if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK) // if can2 not initialised before filters set they are lost
  {
    // Initialization Error
    Error_Handler();
  }

  HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, DISABLE, DISABLE);

  HAL_FDCAN_ConfigRxFifoOverwrite(&hfdcan1, FDCAN_RX_FIFO0, FDCAN_RX_FIFO_OVERWRITE);

  // Configure Rx filter to only accept expected ID's into receive interrupt
  sFilterConfig1.IdType = FDCAN_STANDARD_ID; // standard, not extended FD frame filter.
  sFilterConfig1.FilterType = FDCAN_FILTER_RANGE; // filter all the id's between id1 and id2 in filter definition.
  sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // FDCAN_FILTER_TO_RXFIFO0; // set can1 to receive via fifo0

  sFilterConfig1.FilterIndex = 1;
  sFilterConfig1.FilterID1 = 0x8; // 0xf 0x0 for all
  sFilterConfig1.FilterID2 = 0xB; // 07ff  0x0 for all


  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

 // HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, DISABLE, DISABLE);

  sFilterConfig1.IdType = FDCAN_STANDARD_ID; // standard, not extended FD frame filter.
  sFilterConfig1.FilterType = FDCAN_FILTER_RANGE; // filter all the id's between id1 and id2 in filter definition.
  sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // FDCAN_FILTER_TO_RXFIFO0; // set can1 to receive via fifo0

  sFilterConfig1.FilterIndex++;
  sFilterConfig1.FilterID1 = ECU_CAN_ID;
  sFilterConfig1.FilterID2 = ECU_CAN_ID+1;


  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  sFilterConfig1.FilterIndex++; // filter for PDM/IVT. Has relay statuses and accumulator values.
  sFilterConfig1.FilterID1 = 0x511;
  sFilterConfig1.FilterID2 = 0x528;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  sFilterConfig1.FilterIndex++; // filter for canbus ADC id's
  sFilterConfig1.FilterID1 = 0x600;
  sFilterConfig1.FilterID2 = 0x605;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  sFilterConfig1.FilterIndex++; // filter for fake button presses id's + induce hang.
  sFilterConfig1.FilterID1 = 0x610;
  sFilterConfig1.FilterID2 = 0x614;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  /*
  sFilterConfig1.FilterType = FDCAN_FILTER_MASK;  // configure CANOpen device filters by mask.

  sFilterConfig1.FilterIndex++;
  sFilterConfig1.FilterID1 = InverterL_COBID;
  sFilterConfig1.FilterID2 = 0b00001111111; // 0x1fe - 0x1ff   0x2fe  - 0x2ff    0x77e - 0x77f

  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  } */


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

  // Prepare Tx Headers

  // header for sending time base
  TxHeaderTime.Identifier = 0x100;
  TxHeaderTime.IdType = FDCAN_STANDARD_ID;
  TxHeaderTime.TxFrameType = FDCAN_DATA_FRAME;
  TxHeaderTime.DataLength = FDCAN_DLC_BYTES_3;
  TxHeaderTime.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeaderTime.BitRateSwitch = FDCAN_BRS_OFF;
  TxHeaderTime.FDFormat = FDCAN_CLASSIC_CAN;
  TxHeaderTime.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  TxHeaderTime.MessageMarker = 0;

  // general purpose debug tx header
  TxHeader1.Identifier = 0x101;
  TxHeader1.IdType = FDCAN_STANDARD_ID;
  TxHeader1.TxFrameType = FDCAN_DATA_FRAME;
  TxHeader1.DataLength = FDCAN_DLC_BYTES_8;
  TxHeader1.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeader1.BitRateSwitch = FDCAN_BRS_OFF;
  TxHeader1.FDFormat = FDCAN_CLASSIC_CAN;
  TxHeader1.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  TxHeader1.MessageMarker = 0;

  TxHeader2.Identifier = 0x102;
  TxHeader2.IdType = FDCAN_STANDARD_ID;
  TxHeader2.TxFrameType = FDCAN_DATA_FRAME;
  TxHeader2.DataLength = FDCAN_DLC_BYTES_8;
  TxHeader2.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeader2.BitRateSwitch = FDCAN_BRS_OFF;
  TxHeader2.FDFormat = FDCAN_CLASSIC_CAN;
  TxHeader2.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  TxHeader2.MessageMarker = 0;
}

void FDCAN2_start(void)
{
  FDCAN_FilterTypeDef sFilterConfig2;

#ifdef ONECAN
   hfdcan2 = hfdcan1;
#endif

 /* if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK) // initted already in fdcan1_start
  {
    // Initialization Error
    Error_Handler();
  } */

  HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_REJECT, FDCAN_REJECT, DISABLE, DISABLE);

  // Configure Rx filter for can2
  sFilterConfig2.IdType = FDCAN_STANDARD_ID;
  sFilterConfig2.FilterIndex = 64;
  sFilterConfig2.FilterType = FDCAN_FILTER_RANGE;
  sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // set can2 to receive into fifo1
  sFilterConfig2.FilterID1 = 0x1;
  sFilterConfig2.FilterID2 = 0x1;

//  HAL_FDCAN_ConfigInterruptLines

  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  // Inverter CANOpen Filters

  sFilterConfig2.FilterType = FDCAN_FILTER_MASK;  // configure CANOpen device filters by mask.

  sFilterConfig2.FilterIndex++;
  sFilterConfig2.FilterID1 = InverterL_COBID;
  sFilterConfig2.FilterID2 = 0b00001111111; // 0x1fe - 0x1ff   0x2fe  - 0x2ff    0x77e - 0x77f

  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  sFilterConfig2.FilterIndex++;
  sFilterConfig2.FilterID1 = InverterR_COBID;
  sFilterConfig2.FilterID2 = 0b00001111111; // 0x1fe - 0x1ff   0x2fe  - 0x2ff    0x77e - 0x77f

  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  // Front WheelSpeed CANOpen Filters

   sFilterConfig2.FilterType = FDCAN_FILTER_MASK;  // configure CANOpen device filters by mask.

   sFilterConfig2.FilterIndex++;
   sFilterConfig2.FilterID1 = FLSpeed_COBID;
   sFilterConfig2.FilterID2 = 0b00001111111;

   if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK)
   {
     // Filter configuration Error
     Error_Handler();
   }

   sFilterConfig2.FilterIndex++;
   sFilterConfig2.FilterID1 = FRSpeed_COBID;
   sFilterConfig2.FilterID2 = 0b00001111111;

   if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK)
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

#ifndef ONECAN
  // Start the FDCAN module
  if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK)
  {
    //  Start Error
    Error_Handler();
  }

  // start can receive interrupt for second can's messages

  if (HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE , 0) != HAL_OK)
//  if (HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO1_NEW_MESSAGE , 0) != HAL_OK)
  {
    // Notification Error
    Error_Handler();
  }
#endif
}

int ResetCanReceived( void ) // only wait for new data since request.
{
	// inverters.

	ResetCanData(&CanState.InverterLPDO1);
	ResetCanData(&CanState.InverterRPDO1);
	ResetCanData(&CanState.InverterLPDO2);
	ResetCanData(&CanState.InverterRPDO2);
	ResetCanData(&CanState.InverterLPDO3);
	ResetCanData(&CanState.InverterRPDO3);
	ResetCanData(&CanState.InverterLPDO4);
	ResetCanData(&CanState.InverterRPDO4);
	ResetCanData(&CanState.InverterLERR);
	ResetCanData(&CanState.InverterRERR);

	// bms

	ResetCanData(&CanState.BMSVolt);
	ResetCanData(&CanState.BMSError);
	ResetCanData(&CanState.BMSOpMode);

	// pdm

	ResetCanData(&CanState.PDM);

	// ivvt

	ResetCanData(&CanState.IVTI);
	ResetCanData(&CanState.IVTU1);
	ResetCanData(&CanState.IVTU2);
	ResetCanData(&CanState.IVTW);
	ResetCanData(&CanState.IVTWh);
	ResetCanData(&CanState.IVTT);
	ResetCanData(&CanState.IVTAs);

	// front wheel encoders

	ResetCanData(&CanState.FLeftSpeedERR);
	ResetCanData(&CanState.FRightSpeedERR);
	ResetCanData(&CanState.FLeftSpeedPDO1);
	ResetCanData(&CanState.FRightSpeedPDO1);
	ResetCanData(&CanState.FLeftSpeedNMT);
	ResetCanData(&CanState.FRightSpeedNMT);
	return 0;
}

void resetCanTx(volatile uint8_t CANTxData[8])
{
	for(int i = 0;i < 8;i++){
		CANTxData[i]=0;
	}
}


int getNMTstate(volatile struct CanData *data )
{

	if ( data->dlcsize == 1 )
	{

	}
  return 0;
}

void ResetCanData(volatile struct CanData *data )
{
	data->time = 0;
	for ( int i=0; i<data->dlcsize; i++)
	{
		data->data[i] = 0;
	}
	data->newdata = 0;
	data->processed = 0;
}

char CAN1Send( FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData )
{
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
		if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, pTxHeader, pTxData) != HAL_OK)
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

	return returnval;
}


char CAN2Send( FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData )
{
	if ( DeviceState.CAN2 == OFFLINE )
	{
		return 3;
	}

	if ( HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2) < 1 )
	{
		// return error, can fifo full.
		return 2;
	}

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, pTxHeader, pTxData) != HAL_OK)
	{
		Errors.CANSendError2++;
		return 1;
//		Error_Handler();

	}

	return 0;
}

char reTransmitError(uint32_t canid, uint8_t *CANRxData, uint32_t DataLength )
{
#ifndef RETRANSMITBADDATA
	return 0;
#endif
	FDCAN_TxHeaderTypeDef TxHeaderNMT;
	TxHeaderNMT.Identifier = canid;
	TxHeaderNMT.IdType = FDCAN_STANDARD_ID;
	TxHeaderNMT.TxFrameType = FDCAN_DATA_FRAME;

	TxHeaderNMT.DataLength = DataLength<<16;; // only one byte defined, check this
	TxHeaderNMT.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderNMT.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderNMT.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderNMT.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderNMT.MessageMarker = 0;
#ifdef CAN2ERRORSTATUS
	CAN2Send( &TxHeaderNMT, CANRxData );
#endif
	CAN1Send( &TxHeaderNMT, CANRxData ); // return values.
//#endif
	return 0;
}

char reTransmitOnCan1(uint32_t canid, uint8_t *CANRxData, uint32_t DataLength )
{
#ifndef sharedCAN // only retransmit if can1 and can2 are not sharing lines.
	FDCAN_TxHeaderTypeDef TxHeaderNMT;

	TxHeaderNMT.Identifier = canid;
	TxHeaderNMT.IdType = FDCAN_STANDARD_ID;
	TxHeaderNMT.TxFrameType = FDCAN_DATA_FRAME;

	TxHeaderNMT.DataLength = DataLength; // only one byte defined, check this
	TxHeaderNMT.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderNMT.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderNMT.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderNMT.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderNMT.MessageMarker = 0;

	CAN1Send( &TxHeaderNMT, CANRxData ); // return values.
#endif
	return 0;
}

char CAN_NMTSyncRequest( void )
{
	// NMT sync request message.

	// send can id 0x80 to can 0 value 1. Call once per update loop.

	FDCAN_TxHeaderTypeDef TxHeaderNMT;

	TxHeaderNMT.Identifier = 0x80;
	TxHeaderNMT.IdType = FDCAN_STANDARD_ID;
	TxHeaderNMT.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderNMT.DataLength = FDCAN_DLC_BYTES_1; // only one byte defined, check this
	TxHeaderNMT.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderNMT.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderNMT.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderNMT.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderNMT.MessageMarker = 0;

	uint8_t CANTxData[1] = { 1 };
	CAN1Send( &TxHeaderNMT, CANTxData ); // return values.
#ifndef sharedCAN
	CAN2Send( &TxHeaderNMT, CANTxData ); // return values.
#endif
	return 1;
	// send to both buses.
}

uint8_t CANSendPDM( uint8_t highvoltage, uint8_t buzz )
{
	FDCAN_TxHeaderTypeDef TxHeaderHV;

	TxHeaderHV.Identifier = 0x118;
	TxHeaderHV.IdType = FDCAN_STANDARD_ID;
	TxHeaderHV.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderHV.DataLength = FDCAN_DLC_BYTES_2; // only two bytes defined in send protocol, check this
	TxHeaderHV.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderHV.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderHV.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderHV.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderHV.MessageMarker = 0;

	uint8_t CANTxData[2] = { highvoltage, buzz };
	return CAN1Send( &TxHeaderHV, CANTxData );
}

char CAN_SendTimeBase( void )
{

	TxHeader1.Identifier = 0x101; // decide on an ECU ID/
	TxHeader1.DataLength = FDCAN_DLC_BYTES_8; // only two bytes defined in send protocol, check this
	TxHeader1.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader1.MessageMarker = 0;

	uint32_t time = gettimer();
	uint8_t CANTxData[8] = {
			CarState.LeftInvState,
			Errors.CANSendError1,
			CarState.RightInvState,
			Errors.CANSendError2,
//			getByte(Errors.CANCount1,0),
//			getByte(Errors.CANCount1,1),
//			getByte(Errors.CANCount2,0),
//			getByte(Errors.CANCount2,1),
			getByte(time,0),
			getByte(time,1),
			getByte(time,2),
			getByte(time,3)
	};
#ifdef CAN2ERRORSTATUS
	CAN2Send( &TxHeader1, CANTxData );
#endif

	return CAN1Send( &TxHeader1, CANTxData );
}

char CAN_SendStatus( char state, char substate, uint32_t errorcode )
{
	TxHeader1.Identifier = ECU_CAN_ID; // decide on an ECU ID/
	TxHeader1.DataLength = FDCAN_DLC_BYTES_8; // only two bytes defined in send protocol, check this
	TxHeader1.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader1.MessageMarker = 0;
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

	uint8_t CANTxData[8] = { stateless, substate, getByte(errorcode, 0), getByte(errorcode, 1), getByte(errorcode, 2), getByte(errorcode, 3),HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1),HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0)};
#ifdef CAN2ERRORSTATUS
	CAN2Send( &TxHeader1, CANTxData );
#endif
	return CAN1Send( &TxHeader1, CANTxData );
}


char CAN_SendErrorStatus( char state, char substate, uint32_t errorcode )
{
	TxHeader1.Identifier = ECU_CAN_ID; // decide on an ECU ID/
	TxHeader1.DataLength = FDCAN_DLC_BYTES_8; // only two bytes defined in send protocol, check this
	TxHeader1.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader1.MessageMarker = 0;
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

	uint8_t CANTxData[8] = { stateless, substate, getByte(errorcode, 0), getByte(errorcode, 1), getByte(errorcode, 2), getByte(errorcode, 3),HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1),HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2)};
#ifdef CAN2ERRORSTATUS
	CAN2Send( &TxHeader1, CANTxData );
#endif
	return CAN1Send( &TxHeader1, CANTxData );
}


char CAN_SendLED( void )
{
	FDCAN_TxHeaderTypeDef TxHeaderHV;

	TxHeaderHV.Identifier = ECU_CAN_ID+2; // decide on an ECU ID/
	TxHeaderHV.IdType = FDCAN_STANDARD_ID;
	TxHeaderHV.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderHV.DataLength = FDCAN_DLC_BYTES_8; // only two bytes defined in send protocol, check this
	TxHeaderHV.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderHV.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderHV.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderHV.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderHV.MessageMarker = 0;
    uint8_t CANTxData[8] = { 10, LEDs[TSLED_Output].state, LEDs[RTDMLED_Output].state, LEDs[TSOFFLED_Output].state,
							LEDs[IMDLED_Output].state, LEDs[BMSLED_Output].state, LEDs[BSPDLED_Output].state, 0};

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
	CAN2Send( &TxHeaderHV, CANTxData );
#endif
	return CAN1Send( &TxHeaderHV, CANTxData );
}


char CAN_SENDINVERTERERRORS( void )
{
	FDCAN_TxHeaderTypeDef TxHeaderHV;

	TxHeaderHV.Identifier = 0xEE; // decide on an ECU ID/
	TxHeaderHV.IdType = FDCAN_STANDARD_ID;
	TxHeaderHV.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderHV.DataLength = FDCAN_DLC_BYTES_8; // only two bytes defined in send protocol, check this
	TxHeaderHV.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderHV.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderHV.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderHV.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderHV.MessageMarker = 0;


    uint8_t CANTxData[8] =
    { CarState.LeftInvState, CarState.RightInvState, CarState.LeftInvStateCheck, CarState.RightInvStateCheck,0,0,0,0 };

#ifdef CAN2ERRORSTATUS
	CAN2Send( &TxHeaderHV, CANTxData );
#endif
	CAN1Send( &TxHeaderHV, CANTxData );

	for( int j=0;j<Errors.InverterError;j++)
	{
		for( int i=0;i<8;i++){
			CANTxData[i] = Errors.InverterErrorHistory[i][j];
		}
#ifdef CAN2ERRORSTATUS
		CAN2Send( &TxHeaderHV, CANTxData );
#endif
		CAN1Send( &TxHeaderHV, CANTxData );
	}

	return 0;
}



char CAN_SendADCValue( uint16_t adcdata, uint8_t index )
{
	FDCAN_TxHeaderTypeDef TxHeaderHV;

	TxHeaderHV.Identifier = ECU_CAN_ID; // decide on an ECU ID/
	TxHeaderHV.IdType = FDCAN_STANDARD_ID;
	TxHeaderHV.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderHV.DataLength = FDCAN_DLC_BYTES_4; // only two bytes defined in send protocol, check this
	TxHeaderHV.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderHV.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderHV.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderHV.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderHV.MessageMarker = 0;

    uint8_t CANTxData[4] = { 20, index, getByte(adcdata, 0), getByte(adcdata, 1) };
	return CAN1Send( &TxHeaderHV, CANTxData );
}

char CAN_SendADC( volatile uint32_t *ADC_Data, uint8_t error )
{
	FDCAN_TxHeaderTypeDef TxHeaderHV;

	if ( error == 0 ){
		TxHeaderHV.Identifier = ECU_CAN_ID+0x10; // decide on an ECU ID/
	} else TxHeaderHV.Identifier = ECU_CAN_ID+0x12; // decide on an ECU ID/
	TxHeaderHV.IdType = FDCAN_STANDARD_ID;
	TxHeaderHV.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderHV.DataLength = FDCAN_DLC_BYTES_8; // only two bytes defined in send protocol, check this
	TxHeaderHV.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderHV.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderHV.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderHV.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderHV.MessageMarker = 0;

	uint8_t CANTxData[8] = {
			getByte(ADC_Data[ThrottleLADC], 0), getByte(ADC_Data[ThrottleLADC], 1),
			getByte(ADC_Data[ThrottleRADC], 0), getByte(ADC_Data[ThrottleRADC], 1),
			getByte(ADC_Data[BrakeFADC], 0), getByte(ADC_Data[BrakeFADC], 1),
			getByte(ADC_Data[BrakeRADC], 0), getByte(ADC_Data[BrakeRADC], 1),
	};

	CAN1Send( &TxHeaderHV, CANTxData );

	if ( error == 0 ){
		TxHeaderHV.Identifier = ECU_CAN_ID+0x11; // decide on an ECU ID/
	} else TxHeaderHV.Identifier = ECU_CAN_ID+0x13; // decide on an ECU ID/

//	int16_t steering12bit = ADC_Data[SteeringADC] >> 4;
	uint8_t CANTxData2[8] = {
		//	getByte(ADC_Data[SteeringADC], 0), getByte(ADC_Data[SteeringADC], 1),
			getByte(ADC_Data[SteeringADC], 0), getByte(ADC_Data[SteeringADC], 1),
			getByte(ADC_Data[DrivingModeADC], 0), getByte(ADC_Data[DrivingModeADC], 1),
			getByte(ADC_Data[CoolantTempLADC], 0), getByte(ADC_Data[CoolantTempLADC], 1),
			getByte(ADCState.CoolantTempRRaw, 0), getByte(ADCState.CoolantTempRRaw, 1),
//			getByte(ADC_Data[CoolantTempRADC], 0), getByte(ADC_Data[CoolantTempRADC], 1),
	};

	return CAN1Send( &TxHeaderHV, CANTxData2 );
}

char CAN_SendADCminmax( void )
{
	if ( minmaxADC )
	{
		CAN_SendADCValue(ADC_DataMin[ThrottleLADC],11);
		CAN_SendADCValue(ADC_DataMin[ThrottleRADC],12);
		CAN_SendADCValue(ADC_DataMin[BrakeFADC],13);
		CAN_SendADCValue(ADC_DataMin[BrakeRADC],14);

		CAN_SendADCValue(ADC_DataMin[SteeringADC],15);
		CAN_SendADCValue(ADC_DataMin[DrivingModeADC],16);
		CAN_SendADCValue(ADC_DataMin[CoolantTempLADC],17);
		CAN_SendADCValue(ADC_DataMin[CoolantTempRADC],18);

		CAN_SendADCValue(ADC_DataMax[ThrottleLADC],21);
		CAN_SendADCValue(ADC_DataMax[ThrottleRADC],22);

		CAN_SendADCValue(ADC_DataMax[BrakeFADC],23);
		CAN_SendADCValue(ADC_DataMax[BrakeRADC],24);

		CAN_SendADCValue(ADC_DataMax[SteeringADC],25);
		CAN_SendADCValue(ADC_DataMax[DrivingModeADC],26);
		CAN_SendADCValue(ADC_DataMax[CoolantTempLADC],27);
		CAN_SendADCValue(ADC_DataMax[CoolantTempRADC],28);
	}
	return 0;
}

char CAN_SendIVTTrigger( void )
{
	FDCAN_TxHeaderTypeDef TxHeaderIVT;

	TxHeaderIVT.Identifier = 0x411;
	TxHeaderIVT.IdType = FDCAN_STANDARD_ID;
	TxHeaderIVT.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderIVT.DataLength = FDCAN_DLC_BYTES_8; // only two bytes defined in send protocol, check this
	TxHeaderIVT.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderIVT.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderIVT.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderIVT.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderIVT.MessageMarker = 0;

	uint8_t CANTxData[8] = { 49, 0, 175,0,0,0,0,0 };
	return CAN1Send( &TxHeaderIVT, CANTxData );
}


char CAN_SendIVTTurnon( void )
{
	FDCAN_TxHeaderTypeDef TxHeaderIVT;

	TxHeaderIVT.Identifier = 0x411;
	TxHeaderIVT.IdType = FDCAN_STANDARD_ID;
	TxHeaderIVT.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderIVT.DataLength = FDCAN_DLC_BYTES_8; // only two bytes defined in send protocol, check this
	TxHeaderIVT.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderIVT.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderIVT.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderIVT.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderIVT.MessageMarker = 0;

	uint8_t CANTxData[8] = { 52, 1,0,0,0,0,0,0 }; // turn on from pre operation.
	return CAN1Send( &TxHeaderIVT, CANTxData );
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
	FDCAN_TxHeaderTypeDef TxHeaderNMT;

	TxHeaderNMT.Identifier = 0x0;
	TxHeaderNMT.IdType = FDCAN_STANDARD_ID;
	TxHeaderNMT.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderNMT.DataLength = FDCAN_DLC_BYTES_2; // only two bytes defined in send protocol, check this
	TxHeaderNMT.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderNMT.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderNMT.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderNMT.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderNMT.MessageMarker = 0;

#ifndef sharedCAN
	uint8_t CANTxData2[2] = { command, node }; // 0 sends command to all nodes.
	CAN2Send( &TxHeaderNMT, CANTxData2 ); // return values.
#endif
//	DWT_Delay(100); // delay of ~ > 80us needed, or messages entangle and error frame somehow if both can outputs are connected. unknown bug.
	uint8_t CANTxData[2] = { command,node }; // 0 sends command to all nodes.
	CAN1Send( &TxHeaderNMT, CANTxData ); // send command to both buses.
	return 1;
}

char CANSendInverter( uint16_t response, uint16_t request, uint8_t inverter )
{
	FDCAN_TxHeaderTypeDef TxHeaderInverter;


	if(inverter==LeftInverter)
	{
		TxHeaderInverter.Identifier = 0x400 + InverterL_COBID; // 0x47e
	} else
	{
		TxHeaderInverter.Identifier = 0x400 + InverterR_COBID; // 0x47f
	}

	TxHeaderInverter.IdType = FDCAN_STANDARD_ID;
	TxHeaderInverter.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderInverter.DataLength = FDCAN_DLC_BYTES_4; // only two bytes defined in send protocol, check this // four seen in logs
	TxHeaderInverter.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderInverter.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderInverter.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderInverter.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderInverter.MessageMarker = 0;

	uint8_t CANTxData[8];

	resetCanTx(CANTxData);

	storeLEint16(response,&CANTxData[0]);
	storeLEint16(request,&CANTxData[2]);

	return CAN2Send( &TxHeaderInverter, CANTxData );
}


char CAN_SendErrors( void )
{
	FDCAN_TxHeaderTypeDef TxHeaderNMT;

	TxHeaderNMT.Identifier = 102;
	TxHeaderNMT.IdType = FDCAN_STANDARD_ID;
	TxHeaderNMT.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderNMT.DataLength = FDCAN_DLC_BYTES_8; // only two bytes defined in send protocol, check this
	TxHeaderNMT.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderNMT.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderNMT.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderNMT.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderNMT.MessageMarker = 0;

#ifndef sharedCAN
#endif

	uint8_t CANTxData[8] = {0,0,0,0,CarState.LeftInvState,CarState.RightInvState,CarState.LeftInvStateCheck,CarState.RightInvStateCheck}; // 0 sends command to all nodes.
	storeBEint16(Errors.ErrorPlace, &CANTxData[0]);
	storeBEint16(Errors.ErrorReason, &CANTxData[2]);
#ifdef CAN2ERRORSTATUS
	CAN2Send( &TxHeaderNMT, CANTxData );
#endif
	CAN1Send( &TxHeaderNMT, CANTxData ); // send command to both buses.
	return 1;
}


char CANLogDataSlow( void )
{
 return 0;
}

char CANLogDataFast( void )
{
	// build data logging blocks
	FDCAN_TxHeaderTypeDef TxHeaderLog;
	uint8_t CANTxData[8];

	TxHeaderLog.IdType = FDCAN_STANDARD_ID;
	TxHeaderLog.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderLog.DataLength = FDCAN_DLC_BYTES_8; // only two bytes defined in send protocol, check this
	TxHeaderLog.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderLog.BitRateSwitch = FDCAN_BRS_OFF; // irrelevant to classic can
	TxHeaderLog.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderLog.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderLog.MessageMarker = 0;
 //   HAL_Delay(1); // for some reason without small delay here the first log ID 0x7C6 only sometimes sent
    // investigate to find out why, perhaps fifo buffer not acting as expect?

	resetCanTx(CANTxData);
	TxHeaderLog.Identifier = 0x7C6;
	storeBEint16(CarState.Torque_Req_L, &CANTxData[0]); 	//torq_req_l can0 0x7C6 0,16be
	storeBEint16(CarState.Torque_Req_R, &CANTxData[2]); 	//torq_req_r can0 0x7C6 16,16be

	storeBEint16(ADCState.BrakeF, &CANTxData[4]); 	//brk_press_f can0 0x7C6 32,16bee
	storeBEint16(ADCState.BrakeR, &CANTxData[6]); 	//brk_press_r can0 0x7C6 48,16be

	CAN1Send( &TxHeaderLog, CANTxData ); // lagging in sending

	resetCanTx(CANTxData);
	TxHeaderLog.Identifier = 0x7C7;
	storeBEint32(CarState.SpeedRL, &CANTxData[0]); //wheel_speed_left_calculated can0 0x7c7 32,32BE

	storeBEint32(CarState.SpeedRR, &CANTxData[4]); //wheel_speed_right_calculated can0 0x7c7 0,32BE
	CAN1Send( &TxHeaderLog, CANTxData );

	resetCanTx(CANTxData);
	TxHeaderLog.Identifier = 0x7C8;
	CANTxData[0] = ADCState.CoolantTempL; //temp_sensor1 can0 0x7c8 0,8
	CANTxData[1] = CarState.Torque_Req_CurrentMax; // CarState.Torque_Req_Max; //torq_req_max can0 0x7c8 8,8
	CANTxData[2] = ADCState.CoolantTempR; 	//temp_sensor_2 can0 0x7c8 16,8
	CANTxData[3] = ADCState.DrivingMode; //future_torq_req_max can0 0x7c8 24,8
	storeBEint16(ADCState.Torque_Req_L_Percent, &CANTxData[4]); //torq_req_l_perc can0 0x7c8 32,16be
	storeBEint16(ADCState.Torque_Req_R_Percent, &CANTxData[6]); //torq_req_r_perc can0 0x7c8 48,16be
	CAN1Send( &TxHeaderLog, CANTxData );

	resetCanTx(CANTxData);
	TxHeaderLog.Identifier = 0x7C9;
	storeBEint16(CarState.LeftInvTorque, &CANTxData[0]); //actual_torque_left_inverter_raw can0 0x7c9 0,16be
	storeBEint16(CarState.RightInvTorque, &CANTxData[2]); //actual_torque_right_inverter_raw can0 0x7c9 16,16be
//	CANTxData[6] = HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1);
//    CANTxData[7] = HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2);

	CAN1Send( &TxHeaderLog, CANTxData );

	resetCanTx(CANTxData);

	// CarState.LeftInvTorque

	TxHeaderLog.Identifier = 0x7CA; // not being sent in current simulink, but is set?
//	CANTxData[6] = HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1);
//    CANTxData[7] = HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2);
	storeBEint32(CarState.brake_balance,&CANTxData[0]); //brake_balance can0 0x7CA 0,32be
	storeBEint32(ADCState.SteeringAngle,&CANTxData[4]);

//	CANTxData[7] = ADCState.SteeringAngle; // 32bit, [3]-[7];
	CAN1Send( &TxHeaderLog, CANTxData );

	resetCanTx(CANTxData);

	TxHeaderLog.Identifier = 0x7CB; // IVT
	/*
	 *                 CurrentR.Caption := IntToStr(msg[1]+256*msg[0]);
                   InvVR.Caption := IntToStr(msg[5]+256*msg[4]);
                   BMSVR.Caption := IntToStr(msg[3]+256*msg[2]);
                   PowerR.Caption := IntToStr(msg[7]+256*msg[6])
	 */

	storeBEint16(CarState.Current, &CANTxData[0]);
	storeBEint16(CarState.VoltageINV, &CANTxData[2]);
	storeBEint16(CarState.VoltageBMS, &CANTxData[4]);
	storeBEint16(CarState.VoltageIVTAccu, &CANTxData[6]);
//	storeBEint16(CarState.Power, &CANTxData[6]);
//	storeBEint16(ADCloops, &CANTxData[6]);
	ADCloops=0;
	CAN1Send( &TxHeaderLog, CANTxData );


	resetCanTx(CANTxData);
	TxHeaderLog.Identifier = 0x7C7;
	storeBEint32(CarState.SpeedRL, &CANTxData[0]); //wheel_speed_left_calculated can0 0x7c7 32,32BE

	storeBEint32(CarState.SpeedRR, &CANTxData[4]); //wheel_speed_right_calculated can0 0x7c7 0,32BE
	CAN1Send( &TxHeaderLog, CANTxData );

	resetCanTx(CANTxData);
	TxHeaderLog.Identifier = 0x7CC;
	storeBEint32(CarState.SpeedFL, &CANTxData[0]); //wheel_speed_left_calculated can0 0x7c7 32,32BE

	storeBEint32(CarState.SpeedFR, &CANTxData[4]); //wheel_speed_right_calculated can0 0x7c7 0,32BE
	CAN1Send( &TxHeaderLog, CANTxData );


	CAN_SendTimeBase();

	return 0;
}

void SetCanData(volatile struct CanData *data, uint8_t *CANRxData, uint32_t DataLength )
{
	int time = gettimer();
	data->count++;
//	if ( data->newdata == 0 ) // only update data if previous data has been looked at.
	{
		data->dlcsize = DataLength>>16;
		data->data[0] = CANRxData[0];
		data->data[1] = CANRxData[1];
		data->data[2] = CANRxData[2];
		data->data[3] = CANRxData[3];
		data->data[4] = CANRxData[4];
		data->data[5] = CANRxData[5];
		data->data[6] = CANRxData[6];
		data->data[7] = CANRxData[7];
		data->time = time;
	//	int copylength = DataLength>>16;
	//	memcpy(data->data, CANRxData, copylength);
	//	data->dlcsize = DataLength>>16;
		data->newdata = 1; // moved to end to ensure data is not read before updated.
	}
}

/* HAL_FDCAN_HighPriorityMessageCallback(FDCAN_HandleTypeDef *hfdcan)
{
// test, put response to emergency events here?
}
*/

/*

(+) HAL_FDCAN_GetProtocolStatus             : Get protocol status
(+) HAL_FDCAN_GetErrorCounters              : Get error counter values

*/

/**
 * interrupt rx callback for canbus messages
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
// moved everything to one fifo.  consider using both for prioritisation
{
	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t CANRxData[8];
	if(hfdcan->Instance == FDCAN1){
		toggleOutput(LED3_Output);
		Errors.CANCount1++;
	} else if(hfdcan->Instance == FDCAN2) {
		toggleOutput(LED2_Output);
		 Errors.CANCount2++;
	}

	if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
    // Retreive Rx messages from RX FIFO0
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, CANRxData) != HAL_OK)
		{
			// Reception Error
			Error_Handler();
	//		CAN_SendErrorStatus(103,103,103);
		}


		uint8_t bufferlevel = HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1,FDCAN_RX_FIFO0);
		if (bufferlevel > 25 ) // buffer shouldn't fill up under normal use, not sending >30 messages per cycle.
		{
			// return error, can fifo full.
//			CAN_SendErrorStatus( 111, 0, bufferlevel );
			bufferlevel++;
		}

		// process incoming packet
		switch ( RxHeader.Identifier )
        {
			case 0x9 :  // BMS OpMode
  //              processBMSOpMode(CANRxData, RxHeader.DataLength);
                break;

			case 0xA :  // BMS Error
  //              processBMSError(CANRxData, RxHeader.DataLength);
				break;

			case 0xB :  // BMS can0 id 0xA - BMS total voltage.
	//			offset 0 length 32: power
    //			offset 32 length 16 big endian: BatAmps
	//			offset 48 length 16: BatVoltage
				processBMSVoltage(CANRxData, RxHeader.DataLength);
				break;

			case 0x20 : // messages to ECU specifically.
				SetCanData((struct CanData *)&CanState.ECU, CANRxData, RxHeader.DataLength );
				break;

			case 0x21 : // messages to ECU specifically.
				SetCanData((struct CanData *)&CanState.ECUConfig, CANRxData, RxHeader.DataLength );
				break;
				//  0x500-0x505 ? PDM, what. ?


// IVT

			case 0x511 : // IVT Control Message
				//SetCanData((struct CanData *)&CanState.IVTMsg, CANRxData, RxHeader.DataLength );
#ifdef retransmitIVT
//				reTransmitOnCan1(0x511,CANRxData,RxHeader.DataLength);
#endif
				break;

			case IVTI_ID : // IVT Current 0x521,24,24BE * 0.001 -> Accu_Voltage // not in current logs. -- current, not voltage.
		        // Accu_Current.data.longint = CANRxData[3]*16777216+CANRxData[4]*65536+CANRxData[5]*256+CANRxData[6];
				processIVT(CANRxData, RxHeader.DataLength, IVTI_ID );
				break;

			case IVTU1_ID : // IVT Voltage1 0x522
				processIVT(CANRxData, RxHeader.DataLength, IVTU1_ID );
				break;

			case IVTU2_ID : // IVT Can0 0x523,24,24BE * 0.001 -> Accu_Current -- voltage, not current
				// Accu_Voltage.data.longint = CANRxData[3]*16777216+CANRxData[4]*65536+CANRxData[5]*256+CANRxData[6];
				processIVT(CANRxData, RxHeader.DataLength, IVTU2_ID );
				break;

			case IVTU3_ID : // IVT Voltage3 0x524
				processIVT(CANRxData, RxHeader.DataLength, IVTU3_ID );
				break;
			case IVTT_ID : // IVT Temp 0x525
				processIVT(CANRxData, RxHeader.DataLength, IVTT_ID );
/*#ifdef retransmitIVT
				reTransmitOnCan1(0x525,CANRxData,RxHeader.DataLength);
#endif */
				break;
			case IVTW_ID : // IVT Wattage 0x526
				processIVT(CANRxData, RxHeader.DataLength, IVTW_ID );
				break;
			case IVTAs_ID : // IVT As? 0x527
				processIVT(CANRxData, RxHeader.DataLength, IVTAs_ID );
/*#ifdef retransmitIVT
				reTransmitOnCan1(0x527,CANRxData,RxHeader.DataLength);
#endif */
				break;
			case IVTWh_ID : // IVT WattHours 0x528
				processIVT(CANRxData, RxHeader.DataLength, IVTWh_ID );
				break;


// PDM

			case 0x520 : // PDM can0
				//	0x520,0,8 -> BMS_relay_status
				//	0x520,8,8 -> IMD_relay_status
				//	0x520,16,8 -> BSPD_relay_status
				processPDM(CANRxData, RxHeader.DataLength );
//				SetCanData((struct CanData *)&CanState.PDM, CANRxData, RxHeader.DataLength );
				break;


				// formulaSIM commands.

			case 0x600 : // debug ID to send arbitraty 'ADC' values for testing.
				if( CANRxData[0] == 1 && CANRxData[1]== 99 ) // if received value in ID is not 0 assume true and switch to fakeADC over CAN.
				{
			//		stopADC(); //  disable ADC DMA interrupt to stop processing ADC input.
					// crashing if breakpoint ADC interrupt after this, just check variable in interrupt handler for now.
					usecanADC = 1; // set global state to use canbus ADC for feeding values.
					CANADC.SteeringAngle = 0; // set ADC_Data for steering
					CANADC.Torque_Req_L_Percent = 0; // set ADC_data for Left Throttle
					CANADC.Torque_Req_R_Percent = 0; // set ADC_data for Right Throttle
					CANADC.BrakeF = 0; // set ADC_data for Front Brake
					CANADC.BrakeR = 0; // set ADC_data for Rear Brake
					CANADC.DrivingMode = 5; // set ADC_Data for driving mode
					CANADC.CoolantTempL = 20; // set ADC_data for First Coolant Temp
					CANADC.CoolantTempR = 20; // set ADC_data for Second Coolant Temp
				} else // value of 0 received, switch back to real ADC.
				{
					usecanADC = 0;
				}
				break;
			case 0x601 : // debug ID for steering data.
				CANADC.SteeringAngle = CANRxData[0]; // set ADC_Data for steering
				CANADC.Torque_Req_L_Percent = CANRxData[1]; // set ADC_data for Left Throttle
				CANADC.Torque_Req_R_Percent = CANRxData[2]; // set ADC_data for Right Throttle
				CANADC.BrakeF = CANRxData[3]; // set ADC_data for Front Brake
				CANADC.BrakeR = CANRxData[4]; // set ADC_data for Rear Brake
				CANADC.DrivingMode = CANRxData[5]; // set ADC_Data for driving mode
				CANADC.CoolantTempL = CANRxData[6]; // set ADC_data for First Coolant Temp
				CANADC.CoolantTempR = CANRxData[7]; // set ADC_data for Second Coolant Temp
				break;

			case 0x605 : // debug ID for temperature data.
				if( usecanADC )
				{

				}
				break;
			case 0x610 : // debug id for CAN TS input
				if(CANRxData[0]){
					Input[TS_Switch].pressed = 1; // TS_Switch
					Input[TS_Switch].lastpressed = gettimer();
				}
				break;
			case 0x611 : // debug id for CAN RTDM input
				if(CANRxData[0]){
					Input[2].pressed = 1; // RTDM_Switch
					Input[2].lastpressed = gettimer();
				}
				break;
			case 0x612 : // debug id for CAN Stop Motors button
				if(CANRxData[0]){
					Input[6].pressed = 1;  // StartStop_Switch
					Input[6].lastpressed = gettimer();
				}
				break;
#ifdef debug
//REMOVE FROM LIVE CODE.
			case 0x613 : // debug id to induce a hang state, for testing watchdog.
				while ( 1 ){
					// do nothing.
				}
				break;
#endif


			case  0x0 :  // id 0x0,0,8 -> nmt_status // master should not be receiving, only sending.
				break;
// speed sensor CAN ID's

			case 0x80 + FLSpeed_COBID :
//				CanState.FLeftSpeedERR
				break;

			case 0x80 + FRSpeed_COBID :
//				CanState.FRightSpeedERR
				break;

			case 0x180 + FLSpeed_COBID :
				processSickEncoder( CANRxData, RxHeader.DataLength, FLSpeed_COBID );
				break;

			case 0x180 + FRSpeed_COBID :
				processSickEncoder( CANRxData,  RxHeader.DataLength, FRSpeed_COBID );
				break;

			case 0x700 + FLSpeed_COBID : // Front left speed NMT monitoring id
			    processSickNMT(CANRxData, RxHeader.DataLength, FLSpeed_COBID );
				break;

			case 0x700 + FRSpeed_COBID : // Front Right speed NMT monitoring id
				processSickNMT(CANRxData, RxHeader.DataLength, FRSpeed_COBID );
				break;

// Inverter CAN ID's
			case 0x80 + InverterL_COBID :
			    processINVError( CANRxData, RxHeader.DataLength, LeftInverter );
				break;

			case 0x80 + InverterR_COBID :
                processINVError( CANRxData, RxHeader.DataLength, RightInverter );
		//		SetCanData((struct CanData *)&CanState.InverterRERR, CANRxData, RxHeader.DataLength );
				break;

			case 0x180 + InverterL_COBID : // 0x1FE,0,16LE -> Status_Right_Inverter
			  	processINVStatus(CANRxData, RxHeader.DataLength, LeftInverter );
				break;

			case 0x180 + InverterR_COBID :  // 0x1FF,0,16LE -> Status_Left_Inverter;
			    processINVStatus(CANRxData, RxHeader.DataLength, RightInverter );
				break;

			case 0x280 + InverterL_COBID :  // 0x2FE,16,32LE -> Speed_Right_Inverter
			    processINVSpeed(CANRxData, RxHeader.DataLength, LeftInverter );
				break;

			// 0x3fe/f and 0x4fe/f also sent by inverters, ignored in elektrobit.

			case 0x380 + InverterL_COBID :
                processINVTorque(CANRxData, RxHeader.DataLength, LeftInverter );
				break;
                
			case 0x480 + InverterL_COBID :  // not used
				break;

			case 0x280 + InverterR_COBID :  // 0x2FF,16,32LE -> Speed_Left_Inverter
				processINVSpeed(CANRxData, RxHeader.DataLength, RightInverter );
				break;

			case 0x380 + InverterR_COBID :
			    processINVTorque(CANRxData, RxHeader.DataLength, RightInverter );
				break;
                
			case 0x480 + InverterR_COBID : // not used
				break;

			case 0x700 + InverterL_COBID : // 0x77E,8,16LE -> // inverter NMT monitoring id
			    processINVNMT(CANRxData, RxHeader.DataLength, LeftInverter );
				/* Actual_Torque_Right_Inverter_Raw.data.longint = CANRxData[2]*256+CANRxData[1];? looks like wrong ID */
				break;

			default : // unknown identifier encountered, ignore. Shouldn't be possible to get here due to filters.
				break;
		}

		RxHeader.Identifier = 0; // workaround: rx header does not seem to get reset properly?
								 // receiving e.g. 15 after 1314 seems to result in 1315

		if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
		{
      // Notification Error
			Error_Handler();
		}
	}
}

/**
 * interrupt rx callback for canbus, send error messages here?
 *
 *
 */
void __HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t CANRxData[8];
	setOutput(LED2_Output,LEDON); // turn on internal LED to indicate can receive activity.

	if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET) // if there is a message waiting process it
	{
		//timercount = __HAL_TIM_GetCounter(&htim3);
		/* Retrieve Rx message from RX FIFO1 */
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &RxHeader, CANRxData) != HAL_OK)
		{
			/* Reception Error */
			Error_Handler();
		}

		// check rest of header data? Can2 is inverter information
		switch ( RxHeader.Identifier ){ // identify which data packet we are processing.






			default : // any other received packets, shouldn't be any due to filters.
				break;
		}

	/*	if ((RxHeader2.Identifier == 0x2) && (RxHeader2.IdType == FDCAN_STANDARD_ID) && (RxHeader2.DataLength == FDCAN_DLC_BYTES_1))
		{
		//	ubKeyNumber = CANRxData[0];
		} */

		RxHeader.Identifier = 0; // workaround: rx header does not seem to get reset properly?
								 // receiving e.g. 15 after 1314 seems to result in 1315

		// send notification that can message has been read
		if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) != HAL_OK)
		{
			/* Notification Error */
			Error_Handler();
		}
	}
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *canp) {

// Re-enable receive interrupts

// (The error handling code in HAL_CAN_IRQHandler() disables this for some reason)
 setOutput(LED3_Output,LEDON);
 setOutput(LED2_Output,LEDON);
 setOutput(LED1_Output,LEDON);
 setOutputNOW(IMDLED_Output,LEDON);
 while ( 1 ){

 }
//__HAL_CAN_ENABLE_IT(canp, FDCAN_I FDCAN_IT_FMP1);
// (or only re-enable the one you are using)
}
