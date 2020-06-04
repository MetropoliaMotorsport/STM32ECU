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

static char datatype[20] = "";

static bool ReceiveInProgress = false;
static uint8_t ReceiveType = 0;
static uint32_t TransferSize = 0;
static bool SendInProgress = false;
static uint32_t SendLast = 0;
static uint32_t BufferPos = 0;

static bool eepromwrite = false;
static uint32_t eepromwritestart = 0;

void setDriveMode(void)
{
#ifdef EEPROM
	SetupTorque(0);
#else
	SetupNormalTorque();
#endif

	CarState.LimpDisable = 0;
	CarState.DrivingMode = ADCState.DrivingMode;

	switch ( ADCState.DrivingMode )
	{
		case 1: // 5nm  5 , 5,    0,     5,   5,    10,    15,    20,   25,     30,    64,    65,   0
			CarState.Torque_Req_Max = 5;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 0;
#endif
			break;
		case 2: // 10nm
			CarState.Torque_Req_Max = 25;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 0;
#endif
			break;
		case 3: // 15nm
			CarState.Torque_Req_Max = 25;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 1;
#endif
			break;
		case 4: // 20nm
			CarState.Torque_Req_Max = 35;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 0;
#endif
			break;
		case 5: // 25nm
			CarState.Torque_Req_Max = 35;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 1;
#endif
			break;
		case 6: // 30nm
			CarState.Torque_Req_Max = 65;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 0;
#endif
			#ifdef EEPROM
				SetupTorque(0);
			#else
				SetupLargeLowRangeTorque();
			#endif
			break;
		case 7: // 65nm Track
			CarState.Torque_Req_Max = 65;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 1;
#endif

		#ifdef EEPROM
			SetupTorque(0);
		#else
			SetupLargeLowRangeTorque();
		#endif
			break;
		case 8: // 65nm Accel
			CarState.Torque_Req_Max = 65;
			CarState.LimpDisable = 1;
			#ifdef EEPROM
				SetupTorque(0);
			#else
				SetupLowTravelTorque();
			#endif

			break;

	}

	CarState.Torque_Req_CurrentMax = CarState.Torque_Req_Max;

}



void resetReceive()
{
	BufferPos = 0;
	TransferSize = 0;
	datatype[0] = 0;
	ReceiveInProgress = false;

	ReceiveType = 0;
	for ( int i=0; i<BUFSIZE; i++ ) Buffer[i] = 0;
}

void resetSend()
{
	BufferPos = 0;
	datatype[0] = 0;
	TransferSize = 0;
	SendInProgress = false;
	for ( int i=0; i<BUFSIZE; i++ ) Buffer[i] = 0;
}


void SetDataType( char * str, uint8_t datatype )
{
	switch ( datatype )
	{
		case 0 : // Full EEPROM
			strcpy(str, "FullEEPROM");
			break;
		case 1 : // Full EEPROM
			strcpy(str, "EEBank1");
			break;
		case 2 : // Full EEPROM
			strcpy(str, "EEBank2");
			break;
	}
}

char * GetPedalProfile( uint8_t profile )
{
	switch ( profile )
	{
		case 0 : // Full EEPROM
			return "Lin";
		case 1 : // Full EEPROM
			return "Low";
		case 2 : // Full EEPROM
			return "Acc";
	}
	return "-";
}


// checks if device initial values appear OK.
int CheckConfigurationRequest( void )
{

	char str[40] = "";

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

	static bool initialconfig = true;

	static uint8_t testingactive = 0;

	if ( initialconfig )
	{
		if ( !SetupADCInterpolationTables(getEEPROMBlock(0)) )
		{
				// bad config.
		}
		// TODO move to better place. setup default ADC lookup tables.
		initialconfig = false; // call interpolation table setup once only.
		CarState.Torque_Req_Max = 5;
		CarState.Torque_Req_CurrentMax = 5;
	}

	if ( ReceiveInProgress && gettimer() > CanState.ECUConfig.time + 10000 )
	{ // don't get stuck in receiving data for more than 1 second if data flow stopped.
		ReceiveInProgress = false;
		lcd_send_stringline(3,"Receive Timeout", 1);
		// TODO send timeout error
	}

	if ( SendInProgress && gettimer() > SendLast + 10000 )
	{
		resetSend();
		lcd_send_stringline(3,"Send timeout", 1);
	}


	if ( eepromwrite )
	{

		if ( writeEEPROMDone() )
		{
			lcd_send_stringline(3,"EEPROM Write done", 1);
			eepromwrite=false;
		} else if ( gettimer() > eepromwritestart + 100000 )
		{
			lcd_send_stringline(3,"EEPROM Write timeout", 1);
			eepromwrite=false;
		}
	}

	sprintf(str,"Conf: %dnm, %s", CarState.Torque_Req_Max, GetPedalProfile(CarState.PedalProfile) );
	lcd_send_stringline(3,str, 255);

	// check for config change messages. - broken?

	// data receive block [ block sequence[2], data size[1] ]

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

					 sprintf(str,"Send: %s %.4lu ", datatype, BufferPos );

					 lcd_send_stringline(3,str, 1);

					 CAN1Send(&TxHeaderData, CANTxData);
					 BufferPos += SendSize;
					 SendLast = gettimer();

				 } else
				 {
				    BufferPos = TransferSize;
					uint8_t CANTxData[8] = { 9, BufferPos>>8,BufferPos, 0, 0, 0, 0, 0};

					CAN1Send(&TxHeaderData, CANTxData);
					SendInProgress = false;

 					lcd_send_stringline(3,"Send Done", 1);
				 }

			 } else if ( CanState.ECUConfig.data[1] == 99 )
			 { // error.
				 SendInProgress = false;
				 lcd_send_stringline(3,"Send Error Rec", 1);
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
				lcd_send_stringline(3,"Get OutSeq", 1);
				CAN_SendStatus(ReceivingData,ReceiveErr,0);

				// TODO receive error

			} else // position good, continue.
			{

				if (BufferPos+CanState.ECUConfig.data[3]<=TransferSize)
				{

					sprintf(str,"Get: %s %.4d", datatype, receivepos );

					strpad(str, 20);

					lcd_send_stringline(3,str, 1);

					memcpy(&Buffer[BufferPos],(uint8_t*)&CanState.ECUConfig.data[4],CanState.ECUConfig.data[3]);

					if (CanState.ECUConfig.data[3] <  4) // data received ok, but wasn't full block. end of data.
					{
						ReceiveInProgress = false;
						BufferPos+=CanState.ECUConfig.data[3];

						if ( checkversion((char *)Buffer) ) // received data has valid header.
						{

							switch ( ReceiveType )
							{
								case 0 : // Full EEPROM
									memcpy(getEEPROMBuffer(), Buffer,  TransferSize);
									break;

								case 1 : // Block 1
									TransferSize = sizeof(eepromdata);
									memcpy(getEEPROMBlock(1), Buffer, TransferSize);
									break;
								case 2 : // Block 2
									TransferSize = sizeof(eepromdata);
									memcpy(getEEPROMBlock(2), Buffer, TransferSize);
									break;
							}

							initialconfig = true; // rerun adc config etc for new data in memory.
							lcd_send_stringline(3,"Get Done", 1);
						} else
						{
							lcd_send_stringline(3,"Get Bad Header", 1);

						}

						// don't commit to eeprom unless get write request.

						// TODO verify eeprom, move to eeprom.c
					//	memcpy(getEEPROMBuffer(), Buffer, 4096); // copy received data into local eeprom buffer before write.

						// what to do with received data depends on what data was. Flag complete.

//						lcd_send_stringpos(3,0,"Writing To EEPROM.  ");

//						writeFullEEPROM();

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
					lcd_send_stringline(3,"Receive Error", 1);
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
						ReceiveType = CanState.ECUConfig.data[3];
						SetDataType( datatype, ReceiveType );
						TransferSize = CanState.ECUConfig.data[1]*256+CanState.ECUConfig.data[2];
						ReceiveInProgress = true;
						returnvalue = ReceivingData;

						sprintf(str,"DataGet: %s %.4lu", datatype, TransferSize);

						strpad(str, 20);

						CAN_SendStatus(ReceivingData,ReceiveAck,0); // TODO move ack to receive loop with timer to see message?

						lcd_send_stringline(3,str, 1);
					} else
					{
						// TODO error, invalid receive size.
					}

					break;// receive config data.


				case 9 : // receive data
					lcd_send_stringline(3,"Unexpected Data", 1);
					CAN_SendStatus(ReceivingData,ReceiveErr,0);
					break;

				case 10 : // send data
					resetSend();

					SetDataType(datatype, CanState.ECUConfig.data[1] );

					switch ( CanState.ECUConfig.data[1] )
					{
						case 0 : // Full EEPROM
							TransferSize = 4096;
							memcpy(Buffer, getEEPROMBuffer(), TransferSize);
							break;

						case 1 : // Full EEPROM
							TransferSize = sizeof(eepromdata);
							memcpy(Buffer, getEEPROMBlock(1), TransferSize);
							break;
						case 2 : // Full EEPROM
							TransferSize = sizeof(eepromdata);
							memcpy(Buffer, getEEPROMBlock(2), TransferSize);
							break;
					}


					SendInProgress = true; // initiate transfer.
					SendLast = gettimer();

				    BufferPos = 0;

					if ( TransferSize > 0 ){
						uint8_t CANTxData[8] = { 8, TransferSize>>8,TransferSize, CanState.ECUConfig.data[1], 0, 0, 0, 0};
						CAN1Send(&TxHeaderData, CANTxData);

						char str[20] = "Send: ";

						strncpy(str, datatype, 20);
						strpad(str, 20);
						lcd_send_stringline(3,str, 1);
					} else lcd_send_stringline(3,"Bad EEPROM Send Req", 1);

					break;

				case 11 : // test eeprom writing.
					eepromwrite=true;

					eepromwritestart = gettimer();

					lcd_send_stringline(3,"Full EEPROM Write", 1);

					writeFullEEPROM();

					break;

				case 30 :
				/*	lcd_send_stringpos(3,0,"Unexpected Ack. ");
					CAN_SendStatus(ReceivingData,ReceiveErr,0); */
					break;

				default : // unknown request.
					break;

			}
		} else
		{
// deal with local data.
		}
		// not transferring data.

		// config data packet received, process.
	}

	// if can config request testing mode, send acknowledgement, then return 10;

	if ( testingactive ) returnvalue = TestingState;

	return returnvalue; // return a requested driving state, or that in middle of config?
}

