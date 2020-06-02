/*
 * configuration.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#include "main.h"
#include "ecumain.h"

#define BUFSIZE (4096)

static uint8_t Buffer[BUFSIZE];

static bool ReceiveInProgress = false;
static uint32_t TransferSize = 0;
static bool SendInProgress = false;
static uint32_t SendLast = 0;
static uint32_t BufferPos = 0;


// move full data receive handling to can interrupt, would work faster?

uint8_t processConfig(uint8_t CANRxData[8], uint32_t DataLength, volatile InverterState *Inverter ) // try to reread if possible?
{
	return 0;
}


void resetReceive()
{
	BufferPos = 0;
	TransferSize = 0;
	ReceiveInProgress = false;
	for ( int i=0; i<BUFSIZE; i++ ) Buffer[i] = 0;
}

void resetSend()
{
	BufferPos = 0;
	TransferSize = 0;
	SendInProgress = false;
	for ( int i=0; i<BUFSIZE; i++ ) Buffer[i] = 0;
}

// checks if device initial values appear OK.
int CheckConfigurationRequest( void )
{
	FDCAN_TxHeaderTypeDef TxHeaderData;

	TxHeaderData.Identifier = 0x21;
	TxHeaderData.IdType = FDCAN_STANDARD_ID;
	TxHeaderData.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderData.DataLength = FDCAN_DLC_BYTES_8; // only one byte defined, check this
	TxHeaderData.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderData.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderData.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderData.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderData.MessageMarker = 0;

//	static int configstart = 0;
	int returnvalue = 0;

	static uint8_t initialconfig = 0;

	static uint8_t testingactive = 0;

	if ( !initialconfig )
	{
		SetupADCInterpolationTables(); // setup default ADC lookup tables.
		initialconfig = 1; // call interpolation table setup once only.
		CarState.Torque_Req_Max = 5;
		CarState.Torque_Req_CurrentMax = 5;
	}

	if ( ReceiveInProgress && gettimer() > CanState.ECUConfig.time + 10000 )
	{ // don't get stuck in receiving data for more than 1 second if data flow stopped.
		ReceiveInProgress = false;
		lcd_send_stringpos(3,0,"Receive Timeout.   ");
		// TODO send timeout error
	}

	// check for config change messages. - broken?

	// data receive block [ block sequence[2], data size[1] ]


	char str[20] = "";


	if ( SendInProgress && gettimer() > SendLast + 10000 )
	{
		resetSend();
		lcd_send_stringpos(3,0,"Send timeout    ");

	}


	if ( CanState.ECUConfig.newdata )
	{
		CanState.ECUConfig.newdata = 0;

		if ( SendInProgress && CanState.ECUConfig.data[0] == 30 ) 			// send processing.
		{
			// ack received.

			 if ( CanState.ECUConfig.data[1] == 1 ) // ack
			 {
				 if ( BufferPos < TransferSize )
				 {
					 uint8_t SendSize = 4;
					 if ( BufferPos+SendSize > TransferSize ) SendSize = TransferSize-BufferPos;

					 uint8_t CANTxData[8] = { 9, BufferPos>>8,BufferPos, SendSize, 0, 0, 0, 0};

					 for ( int i=0;i<SendSize;i++)
					 {
						 CANTxData[4+i] = Buffer[BufferPos+i];
					 }

					 sprintf(str,"Send     %.4i  ", BufferPos );

					 lcd_send_stringpos(3,0,str);

					 CAN1Send(&TxHeaderData, CANTxData);
					 BufferPos += SendSize;
					 SendLast = gettimer();

				 } else
				 {
				    BufferPos = TransferSize;
					uint8_t CANTxData[8] = { 9, BufferPos>>8,BufferPos, 0, 0, 0, 0, 0};

					CAN1Send(&TxHeaderData, CANTxData);
					SendInProgress = false;

 					lcd_send_stringpos(3,0,"Send Done.    ");
				 }

			 } else if ( CanState.ECUConfig.data[1] == 99 )
			 { // error.
				 SendInProgress = false;
				 lcd_send_stringpos(3,0,"Send Error Rec  ");
				 resetSend();
			 }

		} else
		if ( ReceiveInProgress )
		{ // in middle of receiving a data block, ignore anything else.
			int receivepos = (CanState.ECUConfig.data[1]*256+CanState.ECUConfig.data[2]);
			// check receive buffer address matches sending position
			if ( receivepos != BufferPos )
			{
				// unexpected data sequence, reset receive status;

				resetReceive();
				lcd_send_stringpos(3,0,"Receive OutSeq.   ");
				CAN_SendStatus(ReceivingData,ReceiveErr,0);

				// TODO receive error

			} else // position good, continue.
			{

				if (BufferPos+CanState.ECUConfig.data[3]<=BUFSIZE)
				{

					sprintf(str,"Receive     %.4i  ", receivepos );

					lcd_send_stringpos(3,0,str);

					memcpy(&Buffer[BufferPos],(uint8_t*)&CanState.ECUConfig.data[4],CanState.ECUConfig.data[3]);

					if (CanState.ECUConfig.data[3] <  4) // data received ok, but wasn't full block. end of data.
					{
						ReceiveInProgress = false;
						BufferPos+=CanState.ECUConfig.data[3];
						lcd_send_stringpos(3,0,"Receive Done.   ");
						// call a callback to process the fully received data?

					} else
					{
						BufferPos+=4; // wait for next block.
						returnvalue =  ReceivingData;
					}
					CAN_SendStatus(ReceivingData,ReceiveAck,0);

				} else
				{
					// TODO tried to receive too much data! error.
					resetReceive();
					lcd_send_stringpos(3,0,"Receive Error.    ");
					CAN_SendStatus(ReceivingData, ReceiveErr,0);
				}
			}

			// if ( data size < 5 then end receiving )
		} else
		if ( CanState.ECUConfig.data[0] != 0)
		{

	//		returnvalue = ReceivingConfig;
			switch ( CanState.ECUConfig.data[0] )
			{
				case 1 : // send ADC
					CAN_SendADC(ADC_Data,0);
					break;
				case 2 :
					CAN_SendADCminmax();
					break;
				case 3 :
					// toggle HV force loop.
					if ( CarState.TestHV )
					{
						CarState.TestHV = 0;
						testingactive = 0;
						CAN_ConfigRequest(3, 0 );
					}
					else
					{
						CarState.TestHV = 1;
						testingactive = 1;
						CAN_ConfigRequest( 3, 1 );
					}

					break;

				case 8 : // start receiving data packet. bytes 2 & 3 define how much data being sent.


					TransferSize = CanState.ECUConfig.data[1]*256+CanState.ECUConfig.data[2];

					if ( TransferSize <= BUFSIZE )
					{
						resetReceive();
						ReceiveInProgress = true;
						returnvalue = ReceivingData;

						sprintf(str,"Receivesize %.4i ", TransferSize);

						CAN_SendStatus(ReceivingData,ReceiveAck,0);

						lcd_send_stringpos(3,0,str);
					} else
					{
						// TODO error, invalid receive size.
					}

					break;// receive config data.


				case 9 : // receive data
					lcd_send_stringpos(3,0,"Unexpected receive. ");
					CAN_SendStatus(ReceivingData,ReceiveErr,0);
					break;

				case 10 : // send data

						if ( CanState.ECUConfig.data[1] == 1 ) // 1 == EEPROM
						{
							resetSend();

							memcpy(Buffer, getEEPROMBuffer(), 4096);
							TransferSize = 4096;

						//	for ( int i=0;i<129;i++) Buffer[i] = i;
						//	TransferSize = 129;

							SendInProgress = true; // initiate transfer.
							SendLast = gettimer();
//							CAN_SendStatus(ReceivingData,(uint8_t)TransferSize>>8,(uint8_t)TransferSize);

						    BufferPos = 0;
							uint8_t CANTxData[8] = { 8, TransferSize>>8,TransferSize, 0, 0, 0, 0, 0};
						//	CANTxData[1] = TransferSize>>8;
						//	CANTxData[2] = TransferSize;

							CAN1Send(&TxHeaderData, CANTxData);

							lcd_send_stringpos(3,0,"Send Start     "); // gets here ok.
						}

					break;

				case 30 :
				/*	lcd_send_stringpos(3,0,"Unexpected Ack. ");
					CAN_SendStatus(ReceivingData,ReceiveErr,0); */
					break;


				default : // unknown request.
					break;

			}
		}

		// config data packet received, process.
	}

	// if can config request testing mode, send acknowledgement, then return 10;

	if ( testingactive ) returnvalue = TestingState;

	return returnvalue; // return a requested driving state, or that in middle of config?
}

