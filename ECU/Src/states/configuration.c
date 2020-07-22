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
static uint8_t eepromwritetype = 0;
static uint32_t eepromwritestart = 0;


static uint8_t ECUConfigdata[8] = {0};
static bool	   ECUConfignewdata = false;
static uint32_t ECUConfigDataTime = 0;

bool GetConfigCmd(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );

CANData ECUConfig = { NULL, 21, 8, GetConfigCmd, NULL, 0 };

static bool configReset = false;

bool checkConfigReset( void )
{
	if ( configReset )
	{
/*
		CanState.ECU.newdata = 0; // we've seen message in error state loop.
		if ( ( CanState.ECU.data[0] == 0x99 ) && ( CanState.ECU.data[1] == 0x99 ))
		{
			return true;
		}
*/
		configReset = false;
		return true;
	} else return false;
}

void ConfigReset( void )
{
	configReset = false;
}


bool GetConfigCmd(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	if ( ECUConfignewdata )
	{
// TODO received data before processing old, send error?
	} else
	{
		ECUConfigDataTime= gettimer();
		memcpy(ECUConfigdata, CANRxData, 8);
		ECUConfignewdata = true; // moved to end to ensure data is not read before updated.
	}
	return true;
}


void setDriveMode(void)
{
#ifndef HPF19

//	EEPROMdata

	if ( DeviceState.EEPROM == ENABLED )
	{
		CarState.PedalProfile = getEEPROMBlock(0)->PedalProfile;
		SetupTorque(CarState.PedalProfile);
		CarState.LimpDisable = !getEEPROMBlock(0)->LimpMode;
		CarState.Torque_Req_Max = getEEPROMBlock(0)->MaxTorque;
	} else
	{
		CarState.PedalProfile = 0;
		SetupTorque(CarState.PedalProfile);
		CarState.LimpDisable = 0;
		CarState.Torque_Req_Max = 5;
	}


#else
	SetupNormalTorque();
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

#endif

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

char * GetPedalProfile( uint8_t profile , bool shortform )
{
	switch ( profile )
	{
		case 0 : // Full EEPROM
			if ( shortform )
				return "Lin";
			else return "Linear";
		case 1 : // Full EEPROM
			if ( shortform )
				return "Low";
			else return "Low Range";
		case 2 : // Full EEPROM
			if ( shortform )
				return "Acc";
			else return "Acceleration";
	}
	return NULL;
}


void doMenuIntEdit( char * display, char * menuitem, bool selected,	bool * editing,
		volatile uint8_t * value, const uint8_t * validvalues )
{
	char str[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(str, "%c%s:", (selected) ? '>' :' ', menuitem);
	memcpy(display, str, len);

	if ( selected  )
	{
		if ( CheckButtonPressed(Config_Input) )
		{
			*editing = !*editing;
			GetLeftRightPressed(); // clear out any buffered presses when weren't editing.
		}


		if ( *editing )
		{
			display[LCDCOLUMNS-1-5] = '<';
			display[LCDCOLUMNS-1] = '>';
			int change = GetLeftRightPressed();
			int position = 0;

			// find current value position. will default to first item if an invalid value was given.
			for ( int i=1;validvalues[i]!=0;i++)
			{
				if ( *value ==  validvalues[i] ) position = i;
			}

			// work out new value change attempted.
			if ( change < 0 && position > 0)
			{
				*value = validvalues[position-1];
			} else if ( change > 0 && validvalues[position+1] != 0)
			{
				*value = validvalues[position+1];
			}
	//		if ( change + *value >= min && change + *value <= max ) *value+=change;
		}
	}

	// print value
	len = sprintf(str, "%3d", *value);
	memcpy(&display[LCDCOLUMNS-1-3], str, len);
}



void doMenuPedalEdit( char * display, char * menuitem, bool selected, bool * editing,
		volatile uint8_t * value )
{
	char str[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(str, "%c%s", (selected) ? '>' :' ', menuitem);
	memcpy(display, str, len);

	if ( selected  )
	{
		if ( CheckButtonPressed(Config_Input) )
		{
			*editing = !*editing;
			GetLeftRightPressed(); // clear out any buffered presses when weren't editing.
		}


		if ( *editing )
		{
			display[LCDCOLUMNS-1-10] = '<';
			display[LCDCOLUMNS-1] = '>';
			int change = GetLeftRightPressed();
			int max = 0;

			for ( max=0; GetPedalProfile(max, false)!=NULL;max++);

			if ( change + *value >= 0 && change + *value <= max-1 ) *value+=change;
		}
	}

	// print value
	len = sprintf(str, "%9s", GetPedalProfile(*value, false));
	if ( len > 9 ) len = 9;
	memcpy(&display[LCDCOLUMNS-1-9], str, len);
}



void doMenuBoolEdit( char * display, char * menuitem, bool selected, bool * editing,
		volatile bool * value )
{
	char str[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(str, "%c%s", (selected) ? '>' :' ', menuitem);
	memcpy(display, str, len);

	if ( selected  )
	{
		if ( CheckButtonPressed(Config_Input) )
		{
			*editing = !*editing;
			GetLeftRightPressed(); // clear out any buffered presses when weren't editing.
		}


		if ( *editing )
		{
			display[LCDCOLUMNS-1-6] = '<';
			display[LCDCOLUMNS-1] = '>';
			int change = GetLeftRightPressed();
			if ( change != 0 )
				*value= !*value;
		}
	}

	// print value
	len = sprintf(str, "%5s", (*value)? "True" : "False");
	memcpy(&display[LCDCOLUMNS-1-5], str, len);
}


bool DoMenu()
{
	static bool inmenu = false;
	static int8_t selection = 0;
	static int8_t top = 0;
	static bool inedit = false;

	static int menusize = 4;

	static char MenuLines[6][21] = { 0 };

	const uint8_t torquevals[] = {0,5,10,25,65,0}; // zero terminate so function can find end.

	if ( inmenu )
	{
		if ( selection == 0 && CheckButtonPressed(Config_Input) )
		{
			inmenu = false;
			inedit = false;
			if (writeEEPROMDone() ) // try to commit config changes. How to deal with fail?
			{
				writeEEPROMCurConf();
			}
			return false;
		}


		if ( !inedit )
		{
			selection+=GetUpDownPressed(); // allow position adjustment if not editing item.

			if ( selection <  0) selection=0;
			if ( selection > menusize-1) selection=menusize-1;

			if ( top + 2 < selection ) top +=1;
			if ( selection < top ) top -=1;
		}

		strcpy(MenuLines[0], "Config Menu:");

		sprintf(MenuLines[1], "%cBack & Save", (selection==0) ? '>' :' ');
		doMenuIntEdit( MenuLines[2], "Max Nm", (selection==1), &inedit, &getEEPROMBlock(0)->MaxTorque, torquevals );
		doMenuPedalEdit( MenuLines[3], "Accel", (selection==2), &inedit, &getEEPROMBlock(0)->PedalProfile );
		doMenuBoolEdit( MenuLines[4], "LimpDisable", (selection==3), &inedit, &getEEPROMBlock(0)->LimpMode);

//		sprintf(MenuLines[3], "%cAccel: %s", (selection==2) ? '>' :' ', GetPedalProfile(CarState.PedalProfile, false));

		lcd_send_stringline( 0, MenuLines[0], 5 );

		for ( int i=0;i<3;i++)
		{
			 lcd_send_stringline( i+1, MenuLines[i+top+1], 5 );
		}
		return true;
	}

	if ( !inmenu )
	{
		if ( CheckButtonPressed(Config_Input) )
		{
			inmenu = true;
			GetUpDownPressed(); // clear any queued actions.
			return true;
		}
	}


	return false;

}


// checks if device initial values appear OK.
int CheckConfigurationRequest( void )
{

	char str[40] = "";

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

	if ( ReceiveInProgress && gettimer() > ECUConfigDataTime + 10000 )
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
			eepromwritetype=0;
		} else if ( gettimer() > eepromwritestart + 100000 )
		{
			lcd_send_stringline(3,"EEPROM Write timeout", 1);
			eepromwrite=false;
			eepromwritetype=0;
		} else
			lcd_send_stringline(3,"EEPROM Write", 1);
	}


	if ( ! ( SendInProgress || ReceiveInProgress ) )
	{
		DoMenu();
	}

	sprintf(str,"Conf: %dnm %s %c", CarState.Torque_Req_Max, GetPedalProfile(CarState.PedalProfile, true), (!CarState.LimpDisable)?'T':'F' );
	lcd_send_stringline(3,str, 255);

	// check for config change messages. - broken?

	// data receive block [ block sequence[2], data size[1] ]

	if ( ECUConfignewdata )
	{
		ECUConfignewdata = false;

		if ( SendInProgress && ECUConfigdata[0] == 30 ) 			// send processing.
		{
			// ack received.

			 if ( ECUConfigdata[1] == 1 ) // ack
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

					 CAN1Send(0x21, 8, CANTxData);
					 BufferPos += SendSize;
					 SendLast = gettimer();

				 } else
				 {
				    BufferPos = TransferSize;
					uint8_t CANTxData[8] = { 9, BufferPos>>8,BufferPos, 0, 0, 0, 0, 0};

					CAN1Send(0x21, 8, CANTxData);
					SendInProgress = false;

 					lcd_send_stringline(3,"Send Done", 1);
				 }

			 } else if ( ECUConfigdata[1] == 99 )
			 { // error.
				 SendInProgress = false;
				 lcd_send_stringline(3,"Send Error Rec", 1);
				 resetSend();
			 }

		} else
		if ( ReceiveInProgress )
		{ // in middle of receiving a data block, ignore anything else.
			int receivepos = (ECUConfigdata[1]*256+ECUConfigdata[2]);
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

				if (BufferPos+ECUConfigdata[3]<=TransferSize)
				{

					sprintf(str,"Get: %s %.4d", datatype, receivepos );

					strpad(str, 20);

					lcd_send_stringline(3,str, 1);

					memcpy(&Buffer[BufferPos],(uint8_t*)&ECUConfigdata[4],ECUConfigdata[3]);

					if (ECUConfigdata[3] <  4) // data received ok, but wasn't full block. end of data.
					{

						ReceiveInProgress = false;



						BufferPos+=ECUConfigdata[3];

						if ( checkversion((char *)Buffer) ) // received data has valid header.
						{

							switch ( ReceiveType )
							{
								case 0 : // Full EEPROM
									eepromwritetype=0;
									memcpy(getEEPROMBuffer(), Buffer,  TransferSize);
									break;

								case 1 : // Block 1
									eepromwritetype=1;
									TransferSize = sizeof(eepromdata);
									// copy block to both memory areas.
									memcpy(getEEPROMBlock(1), Buffer, TransferSize);
									memcpy(getEEPROMBlock(2), Buffer, TransferSize);
									break;
//								case 2 : // Block 2
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
		if ( ECUConfigdata[0] != 0)
		{

	//		returnvalue = ReceivingConfig;
			switch ( ECUConfigdata[0] )
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


					TransferSize = ECUConfigdata[1]*256+ECUConfigdata[2];

					if ( TransferSize <= BUFSIZE )
					{
						resetReceive();
						ReceiveType = ECUConfigdata[3];
						SetDataType( datatype, ReceiveType );
						TransferSize = ECUConfigdata[1]*256+ECUConfigdata[2];
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

					SetDataType(datatype, ECUConfigdata[1] );

					switch ( ECUConfigdata[1] )
					{
						case 0 : // Full EEPROM
							TransferSize = 4096;
							memcpy(Buffer, getEEPROMBuffer(), TransferSize);
							break;

						case 1 : // EEPROM Block
							TransferSize = sizeof(eepromdata);
							memcpy(Buffer, getEEPROMBlock(1), TransferSize);
							break;
						case 2 :
							TransferSize = sizeof(eepromdata);
							memcpy(Buffer, getEEPROMBlock(2), TransferSize);
							break;
					}


					SendInProgress = true; // initiate transfer.
					SendLast = gettimer();

				    BufferPos = 0;

					if ( TransferSize > 0 ){
						uint8_t CANTxData[8] = { 8, TransferSize>>8,TransferSize, ECUConfigdata[1], 0, 0, 0, 0};
						CAN1Send(0x21, 8, CANTxData);

						char str[20] = "Send: ";

						strncpy(str, datatype, 20);
						strpad(str, 20);
						lcd_send_stringline(3,str, 1);
					} else lcd_send_stringline(3,"Bad EEPROM Send Req", 1);

					break;

				case 11 : // test eeprom writing.
					eepromwrite=true;

					eepromwritestart = gettimer();

					switch ( eepromwritetype )
					{
						case 0 :
							lcd_send_stringline(3,"Full EEPROM Write", 1);
							writeFullEEPROM();
							break;
						case 1 :
							lcd_send_stringline(3,"Config EEPROM Write", 1);
							writeConfigEEPROM();
							break;
					}

					if ( eepromwritetype == 2 )
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

