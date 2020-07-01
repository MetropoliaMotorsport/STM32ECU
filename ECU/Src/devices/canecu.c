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
#ifndef ONECAN
  if (HAL_FDCAN_Init(hfdcan2p) != HAL_OK) // if can2 not initialised before filters set they are lost
  {
    // Initialization Error
    Error_Handler();
  }
#endif

  HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, DISABLE, DISABLE);

  HAL_FDCAN_ConfigRxFifoOverwrite(&hfdcan1, FDCAN_RX_FIFO0, FDCAN_RX_FIFO_OVERWRITE);

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
/*
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

  TxHeader2.Identifier = 0x102;
  TxHeader2.IdType = FDCAN_STANDARD_ID;
  TxHeader2.TxFrameType = FDCAN_DATA_FRAME;
  TxHeader2.DataLength = FDCAN_DLC_BYTES_8;
  TxHeader2.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeader2.BitRateSwitch = FDCAN_BRS_OFF;
  TxHeader2.FDFormat = FDCAN_CLASSIC_CAN;
  TxHeader2.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  TxHeader2.MessageMarker = 0; */
}

void FDCAN2_start(void)
{
  FDCAN_FilterTypeDef sFilterConfig2;

#ifdef ONECAN
   hfdcan2p = &hfdcan1;

   sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // set can2 to receive into fifo1

#else
   hfdcan2p = &hfdcan2;

 /* if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK) // initted already in fdcan1_start
  {
    // Initialization Error
    Error_Handler();
  } */

  HAL_FDCAN_ConfigGlobalFilter(hfdcan2p, FDCAN_REJECT, FDCAN_REJECT, DISABLE, DISABLE);
  HAL_FDCAN_ConfigRxFifoOverwrite(hfdcan2p, FDCAN_RX_FIFO1, FDCAN_RX_FIFO_OVERWRITE);
  sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO1; // set can2 to receive into fifo1
#endif
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

  for ( int i = 0;i<INVERTERCOUNT;i++)
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
  if (HAL_FDCAN_Start(hfdcan2) != HAL_OK)
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

int ResetCanReceived( void ) // only wait for new data since request.
{
	// inverters.

	for ( int i=0;i<INVERTERCOUNT;i++)
	{
		ResetCanData(&CanState.InverterPDO1[i]);
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
}

void resetCanTx(volatile uint8_t CANTxData[8])
{
	for(int i = 0;i < 8;i++){
		CANTxData[i]=0;
	}
}


int getNMTstate(volatile CanData *data )
{

	if ( data->dlcsize == 1 )
	{

	}
  return 0;
}

void ResetCanData(volatile CanData *data )
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

	return returnval;
}


char CAN2Send( uint16_t id, uint8_t dlc, uint8_t *pTxData )
{
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
/*	if ( state == 99 )
	{
		volatile int i = 1;
	}
*/

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
	CAN2Send( ECU_CAN_ID+2, 8, CANTxData );
#endif
	return CAN1Send( ECU_CAN_ID+2, 8, CANTxData );
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

void SetCanData(volatile CanData *data, uint8_t *CANRxData, uint32_t DataLength )
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


void processCANData(CanData * datahandle, uint8_t CANRxData[8], uint32_t DataLength )
{
	datahandle->time = gettimer();

	if ( datahandle->devicestate == NULL )
	{ // No handler.
		return;
	}

	if ( datahandle->getData(CANRxData, DataLength))
	{
		datahandle->receiveerr = 0;
		*datahandle->devicestate = OPERATIONAL;
	} else // bad data.
	{
		Errors.CANError++;
		datahandle->receiveerr++;
#ifdef SENDBADDATAERROR
		CAN_SendStatus(ReceiveErr,0,datahandle->id);
#endif
		reTransmitError(99,CANRxData, DataLength);
	}
}


// write a generic handler?
int receivedCANData( CanData * datahandle )
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



/* HAL_FDCAN_HighPriorityMessageCallback(FDCAN_HandleTypeDef *hfdcan)
{
// test, put response to emergency events here?
}
*/

/*

(+) HAL_FDCAN_GetProtocolStatus             : Get protocol status
(+) HAL_FDCAN_GetErrorCounters              : Get error counter values

*/

bool processInvMessage( FDCAN_RxHeaderTypeDef *RxHeader, uint8_t CANRxData[8])
{
	bool processed = false;

	for ( int i=0;i<INVERTERCOUNT;i++) // FIX, checks not working.
	{
		 if ( RxHeader->Identifier - 0x80 - CarState.Inverters[i].COBID == 0 )
		 {
				processINVError( CANRxData, RxHeader->DataLength, &CarState.Inverters[i]);
				processed = true;
		 }

		 if (RxHeader->Identifier - 0x180 - CarState.Inverters[i].COBID == 0)  // 0x1FE,16,32LE -> Status_Right_Inverter
		 {
				processINVStatus(CANRxData, RxHeader->DataLength, &CarState.Inverters[i]);
				processed = true;
		 }


		 if (RxHeader->Identifier - 0x280 - CarState.Inverters[i].COBID == 0)  // 0x2FE,16,32LE -> Speed_Right_Inverter
		 {
				processINVSpeed(CANRxData, RxHeader->DataLength, &CarState.Inverters[i]);
				processed = true;
		 }

		 if ( RxHeader->Identifier - 0x380 - CarState.Inverters[i].COBID == 0)  // 0x3fe/f and 0x4fe/f also sent by inverters, ignored in elektrobit.
		 {
				processINVTorque(CANRxData, RxHeader->DataLength, &CarState.Inverters[i]);
				processed = true;
		 }

		 if ( RxHeader->Identifier - 0x480 - CarState.Inverters[i].COBID == 0)  // not used
		 {
			  processed = true;
		 }

		 if ( RxHeader->Identifier - 0x700 - CarState.Inverters[i].COBID == 0)  // 0x77E,8,16LE -> // inverter NMT monitoring id // layout of inverters for HPF20 - one handles each side of car.
		 {
				processINVNMT(CANRxData, RxHeader->DataLength, &CarState.Inverters[i]);
				processed = true;
		 }
	}

	return processed;

}



bool Fifo1Process(uint32_t id, uint8_t CANRxData[8], uint32_t DataLength)
{
	bool processed = true;

	switch ( id ){ // identify which data packet we are processing.

#ifdef ANALOGNODES
	case AnalogNode1_ID :
		processCANData(&AnalogNode1, CANRxData, DataLength );
		break;
#endif

// IVT
#if IVT_BUS == CANB0
			case 0x511 : // IVT Control Message
				//SetCanData((struct CanData *)&CanState.IVTMsg, CANRxData, RxHeader.DataLength );
				break;

			case IVTI_ID : // IVT Current 0x521,24,24BE * 0.001 -> Accu_Voltage // not in current logs. -- current, not voltage.
		        // Accu_Current.data.longint = CANRxData[3]*16777216+CANRxData[4]*65536+CANRxData[5]*256+CANRxData[6];
				processIVT(CANRxData, DataLength, IVTI_ID );
				break;

			case IVTU1_ID : // IVT Voltage1 0x522
				processIVT(CANRxData, DataLength, IVTU1_ID );
				break;

			case IVTU2_ID : // IVT Can0 0x523,24,24BE * 0.001 -> Accu_Current -- voltage, not current
				// Accu_Voltage.data.longint = CANRxData[3]*16777216+CANRxData[4]*65536+CANRxData[5]*256+CANRxData[6];
				processIVT(CANRxData, DataLength, IVTU2_ID );
				break;

			case IVTU3_ID : // IVT Voltage3 0x524
				processIVT(CANRxData, DataLength, IVTU3_ID );
				break;
			case IVTT_ID : // IVT Temp 0x525
				processIVT(CANRxData, DataLength, IVTT_ID );
				break;
			case IVTW_ID : // IVT Wattage 0x526
				processIVT(CANRxData, DataLength, IVTW_ID );
				break;
			case IVTAs_ID : // IVT As? 0x527
				processIVT(CANRxData, DataLength, IVTAs_ID );
				break;
			case IVTWh_ID : // IVT WattHours 0x528
				processIVT(CANRxData, DataLength, IVTWh_ID );
				break;
#endif
	default : // any other received packets, shouldn't be any due to filters.
		processed = false;
		break;
	}
	return processed;
}

/**
 * interrupt rx callback for canbus messages
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
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

		uint8_t bufferlevel = HAL_FDCAN_GetRxFifoFillLevel(hfdcan,FDCAN_RX_FIFO0);
		if (bufferlevel > 25 ) // buffer shouldn't fill up under normal use, not sending >30 messages per cycle.
		{
			// return error, can fifo full.
//			CAN_SendErrorStatus( 111, 0, bufferlevel );
			bufferlevel++;
		}

		// process incoming packet

	//
#ifdef ONECAN
		if ( !Fifo1Process(RxHeader.Identifier, CANRxData, RxHeader.DataLength) )
#endif

#if INV1_BUS == CANB1
		if ( !processInvMessage( &RxHeader, CANRxData ) )
#endif
		switch ( RxHeader.Identifier )
        {
			case BMSBASE_ID+1 :  // BMS OpMode
				processCANData(&BMSOpMode, CANRxData, RxHeader.DataLength );
                break;

			case BMSBASE_ID+2 :  // BMS Error
				processCANData(&BMSError, CANRxData, RxHeader.DataLength );
				break;

			case BMSVOLT_ID :  // BMS can0 id 0xA - BMS total voltage.
	//			offset 0 length 32: power
    //			offset 32 length 16 big endian: BatAmps
	//			offset 48 length 16: BatVoltage
				processCANData(&BMSVoltage, CANRxData, RxHeader.DataLength );
				break;

			case 0x20 : // messages to ECU specifically.
				SetCanData((CanData *)&CanState.ECU, CANRxData, RxHeader.DataLength );
				break;

			case 0x21 : // config messages to ECU.
				GetConfigCmd(CANRxData, RxHeader.DataLength );
//				SetCanData((struct CanData *)&CanState.ECUConfig, CANRxData, RxHeader.DataLength );
				break;
				//  0x500-0x505 ? PDM, what. ?

			case MEMORATOR_ID :
				processCANData(&Memorator, CANRxData, RxHeader.DataLength );
				break;

// IVT

#if IVT_BUS == CANB1

			case IVTMsg_ID : // IVT Control Message
				//SetCanData((struct CanData *)&CanState.IVTMsg, CANRxData, RxHeader.DataLength );
				break;

			case IVTI_ID : // IVT Current 0x521,24,24BE * 0.001 -> Accu_Voltage // not in current logs. -- current, not voltage.
		        // Accu_Current.data.longint = CANRxData[3]*16777216+CANRxData[4]*65536+CANRxData[5]*256+CANRxData[6];
//				processIVTIData( CANRxData, RxHeader.DataLength );
				processIVT(CANRxData, RxHeader.DataLength, IVTI_ID );
				break;

			case IVTU1_ID : // IVT Voltage1 0x522
//				processIVTU1Data( CANRxData, RxHeader.DataLength );
				processIVT(CANRxData, RxHeader.DataLength, IVTU1_ID );
				break;

			case IVTU2_ID : // IVT Can0 0x523,24,24BE * 0.001 -> Accu_Current -- voltage, not current
				// Accu_Voltage.data.longint = CANRxData[3]*16777216+CANRxData[4]*65536+CANRxData[5]*256+CANRxData[6];
//				processIVTU2Data( CANRxData, RxHeader.DataLength );
				processIVT(CANRxData, RxHeader.DataLength, IVTU2_ID );
				break;

			case IVTU3_ID : // IVT Voltage3 0x524
//				processIVTU3Data( CANRxData, RxHeader.DataLength );
				processIVT(CANRxData, RxHeader.DataLength, IVTU3_ID );
				break;
			case IVTT_ID : // IVT Temp 0x525
//				processIVTTData( CANRxData, RxHeader.DataLength );
				processIVT(CANRxData, RxHeader.DataLength, IVTT_ID );
				break;
			case IVTW_ID : // IVT Wattage 0x526
//				processIVTWData( CANRxData, RxHeader.DataLength );
				processIVT(CANRxData, RxHeader.DataLength, IVTW_ID );
				break;
			case IVTAs_ID : // IVT As? 0x527
//				processIVTAsData( CANRxData, RxHeader.DataLength );
				processIVT(CANRxData, RxHeader.DataLength, IVTAs_ID );
				break;
			case IVTWh_ID : // IVT WattHours 0x528
//				processIVTWhData( CANRxData, RxHeader.DataLength );
				processIVT(CANRxData, RxHeader.DataLength, IVTWh_ID );
				break;
#endif

#ifdef POWERNODES
			case NodeErr_ID :
				processCANData(&NodeErr, CANRxData, RxHeader.DataLength );
				break;

			case NodeAck_ID :
				processCANData(&NodeAck, CANRxData, RxHeader.DataLength );
				break;

			case PowerNode33_ID :
				processCANData(&PowerNode33, CANRxData, RxHeader.DataLength );
				break;
			case PowerNode34_ID :
				processCANData(&PowerNode34, CANRxData, RxHeader.DataLength );
				break;
			case PowerNode35_ID :
				processCANData(&PowerNode35, CANRxData, RxHeader.DataLength );
				break;
			case PowerNode36_ID :
				processCANData(&PowerNode36, CANRxData, RxHeader.DataLength );
				break;
			case PowerNode37_ID :
				processCANData(&PowerNode37, CANRxData, RxHeader.DataLength );
				break;
#endif

#ifdef ANALOGNODES
			case AnalogNode9_ID :
				processCANData(&AnalogNode9, CANRxData, RxHeader.DataLength );
				break;
			case AnalogNode10_ID :
				processCANData(&AnalogNode10, CANRxData, RxHeader.DataLength );
				break;
			case AnalogNode11_ID :
				processCANData(&AnalogNode11, CANRxData, RxHeader.DataLength );
				break;
			case AnalogNode12_ID :
				processCANData(&AnalogNode12, CANRxData, RxHeader.DataLength );
				break;
			case AnalogNode13_ID :
				processCANData(&AnalogNode13, CANRxData, RxHeader.DataLength );
				break;
			case AnalogNode14_ID :
				processCANData(&AnalogNode14, CANRxData, RxHeader.DataLength );
				break;
			case AnalogNode15_ID :
				processCANData(&AnalogNode15, CANRxData, RxHeader.DataLength );
				break;
			case AnalogNode16_ID :
				processCANData(&AnalogNode16, CANRxData, RxHeader.DataLength );
				break;
			case AnalogNode17_ID :
				processCANData(&AnalogNode16, CANRxData, RxHeader.DataLength );
				break;
			case AnalogNode18_ID :
				processCANData(&AnalogNode16, CANRxData, RxHeader.DataLength );
				break;
#endif

// PDM

			case PDM_ID : // PDM can0
				//	0x520,0,8 -> BMS_relay_status
				//	0x520,8,8 -> IMD_relay_status
				//	0x520,16,8 -> BSPD_relay_status
				//  0x529,24,8 AIR Voltage
				//  0x529,32,8 LV Voltage.
				processCANData(&PDMCanData, CANRxData, RxHeader.DataLength );
				break;


				// PDMvolts

			case 0x529 : // PDM can0

				break;


				// formulaSIM commands.

			case AdcSimInput_ID : // debug ID to send arbitraty 'ADC' values for testing.
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

			case AdcSimInput_ID+1 : // debug ID for steering data.
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
			case AdcSimInput_ID+2 : // debug id for CAN TS input
				if(CANRxData[0]){
					Input[TS_Input].pressed = 1; // TS_Switch
					Input[TS_Input].lastpressed = gettimer();
				}
				break;
			case AdcSimInput_ID+3 : // debug id for CAN RTDM input
				if(CANRxData[0]){
					Input[RTDM_Input].pressed = 1; // RTDM_Switch
					Input[RTDM_Input].lastpressed = gettimer();
				}
				break;
			case AdcSimInput_ID+4 : // debug id for CAN Stop Motors button
				if(CANRxData[0]){
					Input[StartStop_Input].pressed = 1;  // StartStop_Switch
					Input[StartStop_Input].lastpressed = gettimer();
				}
				break;
#ifdef debug
//REMOVE FROM LIVE CODE.
			case AdcSimInput_ID+5 : // debug id to induce a hang state, for testing watchdog.
				while ( 1 ){
					// do nothing.
				}
				break;
#endif
			case AdcSimInput_ID+6 : // controller input.
				switch ( CANRxData[0])
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
				break;

			case  0x0 :  // id 0x0,0,8 -> nmt_status // master should not be receiving, only sending.
				break;
// speed sensor CAN ID's
#ifdef FRONTSPEED
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

#endif
// Inverter CAN ID's

			default : // unknown identifier encountered, ignore. Shouldn't be possible to get here due to filters.
				toggleOutput(LED7_Output);
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
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
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

	if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET) // if there is a message waiting process it
	{
		//timercount = __HAL_TIM_GetCounter(&htim3);
		/* Retrieve Rx message from RX FIFO1 */
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &RxHeader, CANRxData) != HAL_OK)
		{
			/* Reception Error */
			Error_Handler();
		}

		uint8_t bufferlevel = HAL_FDCAN_GetRxFifoFillLevel(hfdcan,FDCAN_RX_FIFO1);
		if (bufferlevel > 25 ) // buffer shouldn't fill up under normal use, not sending >30 messages per cycle.
		{
			// return error, can fifo full.
//			CAN_SendErrorStatus( 111, 0, bufferlevel );
			bufferlevel++;
		}

		// check rest of header data? Can2 is inverter information
#if INV1_BUS == CANB0
		if ( !processInvMessage( &RxHeader, CANRxData ) )
#endif
		Fifo1Process(RxHeader.Identifier, CANRxData, RxHeader.DataLength);

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
