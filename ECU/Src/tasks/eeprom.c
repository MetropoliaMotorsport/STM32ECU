/*
 * eeprom.c
 *
 *  Created on: 20 Feb 2020
 *      Author: Visa
 */

#include "ecumain.h"
#include "eeprom.h"
#include "errors.h"
#include "output.h"
#include "i2c.h"
#include "i2c-lcd.h"
#include "timerecu.h"
#include "lcd.h"
//#include "stm32h7xx_hal.h"
#include "tim.h"
#include "taskpriorities.h"
#include "debug.h"

uint16_t Memory_Address;
volatile int Remaining_Bytes;

typedef union { // EEPROMU
	uint8_t buffer[4096];
	struct {
		char version[32]; // block 0  32 bytes
		uint8_t active; // block 1 32 bytes
		uint8_t paddingact[31];
		union {
			uint8_t reserved1[32*8]; // blocks 2-9 256 bytes.
			runtimedata_t runtimedata;
		};
		union {
			uint8_t padding1[32*50]; // force the following structure to be aligned to start of a 50 block area.
			eepromdata block1; // block 10-59
		};
		union {
			uint8_t padding2[32*50];
			eepromdata block2; // block 60-109
		};

		uint8_t reserved2[32*14]; // block 110-123  448 bytes
		uint8_t errorlogs[32*4]; // block 124-127  128 bytes
	};
} EEPROMdataType;

DMA_BUFFER EEPROMdataType EEPROMdata;

runtimedata_t * runtimedata_p = &EEPROMdata.runtimedata;

volatile bool eepromwritinginprogress = false;
volatile bool eepromreceivedone = false;
static uint32_t EEPROMConfigDataTime = 0;
static uint8_t EEPROMConfigdata[8] = {0};
static bool	   EEPROMConfignewdata = false;

int EEPROMReceive( void );
int EEPROMSend( void );
void DoEEPROMTimeouts( void );
void resetReceive( void );
void resetSend( void );
void SetDataType( char * str, uint8_t datatype );

#define BUFSIZE (4096)

// Stick buffer in DMA compatible memory.
DMA_BUFFER ALIGN_32BYTES (static uint8_t Buffer[BUFSIZE]);

static char datatype[20] = "";


#define EEPROMSTACK_SIZE 128*2
#define EEPROMTASKNAME  "EEPROMTask"
StaticTask_t xEEPROMTaskBuffer;
StackType_t xEEPROMStack[ EEPROMSTACK_SIZE ];

TaskHandle_t EEPROMTaskHandle;

#define EEPROMQUEUE_LENGTH    20
#define EEPROMITEMSIZE		sizeof( EEPROM_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t EEPROMStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t EEPROMQueueStorageArea[ EEPROMQUEUE_LENGTH * EEPROMITEMSIZE ];

QueueHandle_t EEPROMQueue;

static bool ReceiveInProgress = false;
static uint8_t ReceiveType = 0;
static uint32_t TransferSize = 0;
static bool SendInProgress = false;
static uint32_t SendLast = 0;
static uint32_t BufferPos = 0;

static bool eepromwrite = false;
static uint8_t eepromwritetype = 0;
static uint32_t eepromwritestart = 0;

static time_t lastruntimesaved = 0;

xTimerHandle timerHndlRunningData;

void EEPROMTask(void *argument)
{

	ReceiveInProgress = false;
	ReceiveType = 0;
	TransferSize = 0;
	SendInProgress = false;
	SendLast = 0;
	BufferPos = 0;

	eepromwrite = false;
	eepromwritetype = 0;
	eepromwritestart = 0;

	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT( EEPROMQueue );

	EEPROM_msg msg;

	lastruntimesaved = EEPROMdata.runtimedata.time;

	while( 1 )
	{
		if ( ! eepromwritinginprogress) // no writing in progress, process next queue item to write.
			// Read only needs to happen at startup.
		{
			if ( EEPROMdata.runtimedata.time != lastruntimesaved )
			{
				xTimerStart( timerHndlRunningData, 0 );
				//writeEEPROMRunningData();
				lastruntimesaved = EEPROMdata.runtimedata.time;
			}

			if ( xQueueReceive(EEPROMQueue,&msg,0) )
			{
				eepromwritinginprogress = true;
				HAL_GPIO_WritePin( EEPROMWC_GPIO_Port, EEPROMWC_Pin, 0); // enable write pin.

				switch ( msg.cmd )
				{
				case EEPROMCurConf:
						Remaining_Bytes = 32;
						// EEPROMdata.buffer = memory address of start of block, calculate relative address of write
						Memory_Address = &getEEPROMBlock(0)->ConfigStart - EEPROMdata.buffer;
					break;

				case EEPROMRunningData:

					Remaining_Bytes = 32;
					Memory_Address = (void*)&EEPROMdata.runtimedata - (void*)EEPROMdata.buffer;
	// add data dump save here.
					break;

				case writeEEPROM0:
					Remaining_Bytes = sizeof(eepromdata);
					Memory_Address =  &getEEPROMBlock(1)->BlockStart - EEPROMdata.buffer;
					break;
				case writeEEPROM1:
					Remaining_Bytes = sizeof(eepromdata);
					Memory_Address =  &getEEPROMBlock(2)->BlockStart - EEPROMdata.buffer;
					break;
				case writeEEPROMC:
					Remaining_Bytes = sizeof(eepromdata);
					Memory_Address =  &getEEPROMBlock(0)->BlockStart - EEPROMdata.buffer;
					break;
				case FullConfigEEPROM:
					Remaining_Bytes = sizeof(eepromdata); // sizeof(EEPROMdata.padding1)
						Memory_Address =  &getEEPROMBlock(0)->BlockStart - EEPROMdata.buffer;
				break;
				case FullEEPROM:
					Remaining_Bytes = sizeof(EEPROMdata);
					Memory_Address = 0;
					break;
				case zeroEEPROM:
					memset(EEPROMdata.buffer, 0, sizeof(EEPROMdata));
					Remaining_Bytes = sizeof(EEPROMdata);
					Memory_Address = 0;
					break;
				default:
					break;
				}

				if ( HAL_TIM_Base_Start_IT(&htim16) != HAL_OK){ // start write timer.
					Error_Handler();
				}
			}
		}

		DoEEPROMTimeouts(); // right now, eeprom functionality is mostly defined by can receive handler

		vTaskDelay( CYCLETIME );
	}

	vTaskDelete(NULL);
}

bool GetEEPROMCmd( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	EEPROMConfigDataTime= gettimer();
	memcpy(EEPROMConfigdata, CANRxData, 8);
	EEPROMConfignewdata = true; // moved to end to ensure data is not read before updated.
	DoEEPROM();
	return true;
}

bool EEPROMBusy( void )
{
	return ( SendInProgress || ReceiveInProgress || eepromwritinginprogress );
}


void DoEEPROMTimeouts( void )
{
	if ( ReceiveInProgress && gettimer() > EEPROMConfigDataTime + MS1000 )
	{ // don't get stuck in receiving data for more than 1 second if data flow stopped.
		ReceiveInProgress = false;
		lcd_send_stringline(3,"Receive Timeout", 1);
		// TODO send timeout error
	}

	if ( SendInProgress && gettimer() > SendLast + MS1000 )
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
		} else if ( gettimer() > eepromwritestart + MS1000 )
		{
			lcd_send_stringline(3,"EEPROM Write timeout", 1);
			eepromwrite=false;
			eepromwritetype=0;
		} else
			lcd_send_stringline(3,"EEPROM Write", 1);
	}
}

int DoEEPROM( void )
{
	int returnval = 0;

	char str[40] = "";

	DoEEPROMTimeouts();

	if ( EEPROMConfignewdata )
	{
		EEPROMConfignewdata = false;

		if ( SendInProgress && EEPROMConfigdata[0] == 30 ) 			// send processing.
		{
			// ack received.

			 if ( EEPROMConfigdata[1] == 1 ) // ack
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

			 } else if ( EEPROMConfigdata[1] == 99 )
			 { // error.
				 SendInProgress = false;
				 lcd_send_stringline(3,"Send Error Rec", 1);
				 resetSend();
			 }

		} else
		if ( ReceiveInProgress )
		{ // in middle of receiving a data block, ignore anything else.
			int receivepos = (EEPROMConfigdata[1]*256+EEPROMConfigdata[2]);
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

				if (BufferPos+EEPROMConfigdata[3]<=TransferSize)
				{

					sprintf(str,"Get: %s %.4d", datatype, receivepos );

					strpad(str, 20, true);

					lcd_send_stringline(3,str, 1);
					memcpy(&Buffer[BufferPos],(uint8_t*)&EEPROMConfigdata[4],EEPROMConfigdata[3]);
					if (EEPROMConfigdata[3] <  4) // data received ok, but wasn't full block. end of data.
					{

						ReceiveInProgress = false;

						BufferPos+=EEPROMConfigdata[3];

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

							returnval = InitialConfig; // initialconfig = true; // rerun adc config etc for new data in memory.
							lcd_send_stringline(3,"Get Done", 1);
						} else
						{
							lcd_send_stringline(3,"Get Bad Header", 1);
						}

						// don't commit to eeprom unless get write request.

						// TODO verify eeprom, move to eeprom.c
					//	memcpy(getEEPROMBuffer(), Buffer, 4096); // copy received data into local eeprom buffer before write.

						// what to do with received data depends on what data was. Flag complete.

						// call a callback to process the fully received data?

					} else
					{
						BufferPos+=4; // wait for next block.
						returnval = ReceivingData;
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
			if ( EEPROMConfigdata[0] != 0)
			{
		//		returnvalue = ReceivingConfig;
				switch ( EEPROMConfigdata[0] )
				{
					case 8 : // start receiving data packet. bytes 2 & 3 define how much data being sent.
						returnval = EEPROMReceive();
						break;// receive config data.

					case 9 : // receive data
						lcd_send_stringline(3,"Unexpected Data", 1);
						CAN_SendStatus(ReceivingData,ReceiveErr,0);
						break;

					case 10 : // send data
	#ifdef __RTOS
						EEPROM_msg msg;

						msg.cmd = send;
	#else
						EEPROMSend();
	#endif
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
								writeFullConfigEEPROM();
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
	}
return returnval;
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

void resetReceive( void )
{
	BufferPos = 0;
	TransferSize = 0;
	datatype[0] = 0;
	ReceiveInProgress = false;

	ReceiveType = 0;
	for ( int i=0; i<BUFSIZE; i++ ) Buffer[i] = 0;
}

void resetSend( void )
{
	BufferPos = 0;
	datatype[0] = 0;
	TransferSize = 0;
	SendInProgress = false;
	for ( int i=0; i<BUFSIZE; i++ ) Buffer[i] = 0;
}

// initialise data receive session.
int EEPROMReceive( void )
{
	int returnvalue = 0;
	char str[40] = "";

	TransferSize = EEPROMConfigdata[1]*256+EEPROMConfigdata[2];

	if ( TransferSize <= BUFSIZE )
	{
		resetReceive();
		ReceiveType = EEPROMConfigdata[3];
		SetDataType( datatype, ReceiveType );
		TransferSize = EEPROMConfigdata[1]*256+EEPROMConfigdata[2];
		ReceiveInProgress = true;
		returnvalue = ReceivingData;

		snprintf(str,40,"DataGet: %s %.4lu", datatype, TransferSize);

		strpad(str, 20, true);

		CAN_SendStatus(ReceivingData,ReceiveAck,0);

		lcd_send_stringline(3,str, 1);
	} else
	{
		LogError("EEPROM: Inv Rcv Size");
	}
	return returnvalue;
}


// initialise data send session.
int EEPROMSend( void )
{
	resetSend();

	SetDataType(datatype, EEPROMConfigdata[1] );

	switch ( EEPROMConfigdata[1] )
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
		uint8_t CANTxData[8] = { 8, TransferSize>>8, TransferSize, EEPROMConfigdata[1], 0, 0, 0, 0};
		CAN1Send(0x21, 8, CANTxData); // inform client that about to send data with data send packet.

		char str[40];

		sprintf(str,"Send: %s %.4lu", datatype, TransferSize);

		strpad(str, 20, true);

		lcd_send_stringline(3,str, 1);
	} else lcd_send_stringline(3,"Bad EEPROM Send Req", 1);
	return 0;
}


bool checkversion(char * data)
{
	if ( strcmp( (char *) data, EEPROMVERSIONSTR ) == 0 )
		return true;
	else
		return false;
}

uint8_t * getEEPROMBuffer()
{
	return EEPROMdata.buffer;
}


eepromdata * getEEPROMBlock( int block )
{

	/*		if ( activeblock==1 )
				return EEPROMdata.block1
	*/

	if ( block == 0 )
	{
		if ( EEPROMdata.active==1 )
			return &EEPROMdata.block1;
		else
			return &EEPROMdata.block2; // no valid data.
	} else
	if ( block == 1 )
			return &EEPROMdata.block1; // no valid data.
	else
	if ( block == 2 )
			return &EEPROMdata.block2; // no valid data.
	return NULL;
}

// I2C Mem transfers only used for EEPROM currently so no need to check handle yet.

/**
  * @brief  Tx Transfer completed callback.
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report end of DMA Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
  /* Turn LED1 on: Transfer in transmission process is correct */
//	toggleOutput(44);
//		sendnext = true;
	{
//			HAL_GPIO_WritePin( EEPROMWC_GPIO_Port, EEPROMWC_Pin, 1); // lock eeprom again to prevent false writes.
//			senti2c = true;
	}

	// if all sent i2csendinprogress
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	// MemRX type HAL library call only done on eeprom, so if here, set it done.
	eepromreceivedone = true;
}

/**
  * @brief  I2C error callbacks.
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
 void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
	 if ( I2cHandle->Instance == I2C3 ){
	//	 volatile I2C_HandleTypeDef temp = *I2cHandle;
		 LCD_I2CError();
	 }  // HAL_I2C_ERROR_AF

/*	 if ( I2cHandle->Instance == I2C4 ){
	//	 volatile I2C_HandleTypeDef temp = *I2cHandle;
		 LCD_I2CError();
	 }  // HAL_I2C_ERROR_AF */

	 if ( I2cHandle->Instance == I2C2 ){

		 eepromwritinginprogress = false;

	 }

	 blinkOutput(LED5, BlinkVeryFast, 1);
}


int startupReadEEPROM( void )
{
	int retval = 0;
	// block writing to eeprom, only reading at init to prevent potential corruption.
	HAL_GPIO_WritePin( EEPROMWC_GPIO_Port, EEPROMWC_Pin, 1);

	vTaskDelay(100); // Allow some time for EEPROM chip to initialise itself before start trying to access.
	HAL_I2CEx_ConfigAnalogFilter(&hi2c2,I2C_ANALOGFILTER_ENABLE);

//#define READFULLEEPROM
#ifdef READFULLEEPROM

	eepromreceivedone = false;
	// start reading eeprom into ram, done t bootup so don't ne

	int result = readEEPROMAddr( 0, sizeof(EEPROMdata)+1);
	if ( result != HAL_OK )
	{
		return result;
	}

	if ( checkversion(EEPROMdata.buffer) )
	{
		return 0;
	} else return 1;
#else

	// read version header.

	int result = readEEPROMAddr( 0, 32 );
	if ( result != HAL_OK )
	{
		return result;
	}

	if ( !checkversion((char *)EEPROMdata.buffer) )
	{
		DebugPrintf("EEprom version bad, resetting data\n\r");
		resetEEPROM();

		UARTwrite("Eeprom reset.\r\n");
		retval = 1;
	}

	// only read active block in.
	Memory_Address = &EEPROMdata.active-EEPROMdata.buffer;


	result = readEEPROMAddr( &EEPROMdata.active-EEPROMdata.buffer, 1 );
	if ( result != HAL_OK )
	{
		return result;
	}

	uint16_t offset = ( uint8_t * ) getEEPROMBlock( 0 )- EEPROMdata.buffer;

	result = readEEPROMAddr( offset, sizeof(eepromdata) );
	if ( result != HAL_OK )
	{
		DebugPrintf("EEprom read fail");
		return result;
	}

	if ( checkversion((char *)getEEPROMBlock( 0 )) )
	{
		DebugPrintf("EEprom active block %d OK", EEPROMdata.active);
		return retval;
	}
	// right now, active block is never switched in practice.
	DebugPrintf("EEprom active config data  not found, resetting and using 1\n\r");
	resetEEPROM();
	return retval;
	// headers ok, continue.
#endif
}


int readEEPROMAddr( uint16_t address, uint16_t size )
{
	uint32_t startread = gettimer();
	eepromreceivedone = false;
	if(HAL_I2C_Mem_Read_IT(&hi2c2 , (uint16_t)EEPROM_ADDRESS, address, I2C_MEMADD_SIZE_16BIT, (uint8_t*)&EEPROMdata.buffer[address], size)!= HAL_OK)
	{
		DebugPrintf("EEPROM read failed to start");
	/* Reading process Error */
	   return 1; // Error_Handler(); // failed to read data for some reason.
	}

	while ( !eepromreceivedone ) // 4 sec read timeout so will still startup regardless.
	{
		//__WFI();
		HAL_Delay(10);
		if ( gettimer() > startread + MS1000*4 ) // TODO check right way round.
		{
			return 1;
		}
	};

	if ( !eepromreceivedone )
	{
		DebugPrintf("EEPROM read failed to finish");
		return 2;
	}
	return 0;
}


int readEEPROM( void ){
  eepromreceivedone = false;
  if(HAL_I2C_Mem_Read_IT(&hi2c2 , (uint16_t)EEPROM_ADDRESS, 0, I2C_MEMADD_SIZE_16BIT, (uint8_t*)EEPROMdata.buffer, sizeof(EEPROMdata)+1)!= HAL_OK)
  {
	/* Reading process Error */
	   Error_Handler();
  }
  return 0;
}


//	static bool eepromerror = false;

#define EEPROMMAXERROR (5)

void commitEEPROM( void ) // progress EEPROM writing by sending next block over i2c, call from writing loop ( interrupt )
{
	HAL_TIM_Base_Stop_IT(&htim16);

	static int errorcount = 0;

	toggleOutputMetal(LED7);
	if ( eepromwritinginprogress )
	{
		if ( Remaining_Bytes == 0 )
		{
			if ( hi2c2.State == HAL_I2C_STATE_READY )
			{
				// done with writing, lock write pin again.
				HAL_GPIO_WritePin( EEPROMWC_GPIO_Port, EEPROMWC_Pin, 1);
				eepromwritinginprogress = false;
			}
		} else
		{
			if ( hi2c2.State == HAL_I2C_STATE_READY && Remaining_Bytes > 0 ){ // i2c not busy

				if(HAL_I2C_Mem_Write_IT(&hi2c2 , (uint16_t)EEPROM_ADDRESS, Memory_Address, I2C_MEMADD_SIZE_16BIT, (uint8_t*)(EEPROMdata.buffer + Memory_Address), EEPROM_PAGESIZE)!= HAL_OK)
				{
					Error_Handler(); //TODO error here is not a hard error, don't hang code.
				}
				errorcount = 0;
				Remaining_Bytes -= EEPROM_PAGESIZE;
				if ( Remaining_Bytes <0 ) Remaining_Bytes = 0;
				Memory_Address += EEPROM_PAGESIZE;
			}  else  { errorcount++; }

// count > 0
			if( eepromwritinginprogress && errorcount <= EEPROMMAXERROR ) { // if not done sending and not in error state, start timer to next send event.
				if ( HAL_TIM_Base_Start_IT(&htim16) != HAL_OK){
					Error_Handler(); // timer should always be able to start.
				}
			} else if ( errorcount > EEPROMMAXERROR )
			{
				Errors.eepromerror++;// true; // failed to send complete message
				eepromwritinginprogress = false;
			}
		}
	}
}


int writeFullEEPROM( void )
{
	EEPROM_msg msg = {FullEEPROM};
	return xQueueSend(EEPROMQueue,&msg,0);
}


int writeFullConfigEEPROM( void )
{
	EEPROM_msg msg = {FullConfigEEPROM};
	return xQueueSend(EEPROMQueue,&msg,0);
}


int writeEEPROM( int bank )  //write one of the two config banks to EEPROM
{
	if ( bank > 1 || bank < 0 ) return 0; // invalid bank number given.

	EEPROM_msg msg = {bank?writeEEPROM1:writeEEPROM0};
	return xQueueSend(EEPROMQueue,&msg,0);
}


int writeEEPROMCurConf( void )  //write one of the two config banks to EEPROM
{
	EEPROM_msg msg = {EEPROMCurConf};
	return xQueueSend(EEPROMQueue,&msg,0);
}

int writeEEPROMRunningData( void )   // write emergency packet to end of EEPROM.
{
	EEPROM_msg msg = {EEPROMRunningData};
	return xQueueSend(EEPROMQueue,&msg,0);
	return 0;
}


int writeEEPROMEmergency( void )   // write emergency packet to end of EEPROM.
{
	return 0;
}


bool writeEEPROMDone( void )
{
	return !eepromwritinginprogress;
}


bool stopEEPROM( void )
{
	return false;
}

void clearRunningData( void )
{
	EEPROMdata.runtimedata.time = 0;
	EEPROMdata.runtimedata.maxIVTI = 0;
	EEPROMdata.runtimedata.maxMotorI[0] = 0;
	EEPROMdata.runtimedata.maxMotorI[1] = 0;
	EEPROMdata.runtimedata.maxMotorI[2] = 0;
	EEPROMdata.runtimedata.maxMotorI[3] = 0;

	writeEEPROMRunningData();
}

static void saveRunningData(xTimerHandle pxTimer) {
	writeEEPROMRunningData();
}

bool resetEEPROM( void )
{
	memset(EEPROMdata.buffer, 0, sizeof(EEPROMdata));

	snprintf(EEPROMdata.version, "%s", EEPROMVERSIONSTR);

	eepromdata * data = &EEPROMdata.block1;
	EEPROMdata.active = 1;

	data->EnabledMotors=0b1111;
	snprintf(data->VersionString, "%s", EEPROMVERSIONSTR);	
	data->pedalcurves[0].PedalCurveInput[0] = 50;
	data->pedalcurves[0].PedalCurveInput[1] = 950;
	data->pedalcurves[0].PedalCurveInput[2] = 0;
	data->pedalcurves[0].PedalCurveOutput[0] = 0;
	data->pedalcurves[0].PedalCurveOutput[1] = 1000;
	data->pedalcurves[0].PedalCurveOutput[2] = 0;

	data->pedalcurves[1].PedalCurveInput[0] = 50;
	data->pedalcurves[1].PedalCurveInput[1] = 500;
	data->pedalcurves[1].PedalCurveInput[2] = 0;
	data->pedalcurves[1].PedalCurveOutput[0] = 0;
	data->pedalcurves[1].PedalCurveOutput[1] = 1000;
	data->pedalcurves[1].PedalCurveOutput[2] = 0;

	data->pedalcurves[2].PedalCurveInput[0] = 50;
	data->pedalcurves[2].PedalCurveInput[1] = 600;
	data->pedalcurves[2].PedalCurveInput[2] = 950;
	data->pedalcurves[2].PedalCurveInput[3] = 0;
	data->pedalcurves[2].PedalCurveOutput[0] = 0;
	data->pedalcurves[2].PedalCurveOutput[1] = 400;
	data->pedalcurves[2].PedalCurveOutput[2] = 1000;
	data->pedalcurves[2].PedalCurveOutput[3] = 0;
	Remaining_Bytes = sizeof(EEPROMdata);
	Memory_Address = 0;
	eepromwritinginprogress = true;
	HAL_GPIO_WritePin( EEPROMWC_GPIO_Port, EEPROMWC_Pin, 0); // enable write pin.
	if ( HAL_TIM_Base_Start_IT(&htim16) != HAL_OK) // start write timer.
		Error_Handler();


// wait for write
	vTaskDelay(20);
	while ( EEPROMBusy() )
	{
		vTaskDelay(20);
	}
	DebugPrintf("EEPROM Reset");
}

bool clearEEPROM( void )
{
	EEPROM_msg msg = {zeroEEPROM};
	return xQueueSend(EEPROMQueue,&msg,0);
}

bool initEEPROM( void )
{
	lcd_send_stringscroll("Load EEPRom");

	bool EEPROMInitok = true;

	MX_I2C2_Init();
	int eepromstatus = startupReadEEPROM();

	switch ( eepromstatus )
	{
	case 0 :
		lcd_send_stringscroll("EEPRom Read");
		DeviceState.EEPROM = ENABLED;
		break;
	case 1 :
		lcd_send_stringscroll("EEPRom Reset");
		DeviceState.EEPROM = ENABLED;
		EEPROMInitok = false;
		break;
	default :
		lcd_send_stringscroll("EEPRom Read Fail");
		DeviceState.EEPROM = DISABLED;
		HAL_Delay(3000); // ensure message can be seen.
		EEPROMInitok = false;
	};

	EEPROMQueue = xQueueCreateStatic( EEPROMQUEUE_LENGTH,
							  EEPROMITEMSIZE,
							  EEPROMQueueStorageArea,
							  &EEPROMStaticQueue );

	vQueueAddToRegistry(EEPROMQueue, "EEPROMQueue" );

	timerHndlRunningData = xTimerCreate(
	      "runningdata", /* name */
	      pdMS_TO_TICKS(200), /* period/time */
	      pdFALSE, /* auto reload */
	      (void*)0, /* timer ID */
		  saveRunningData); /* callback */

	EEPROMTaskHandle = xTaskCreateStatic(
						  EEPROMTask,
						  EEPROMTASKNAME,
						  EEPROMSTACK_SIZE,
						  ( void * ) 1,
						  EEPROMTASKPRIORITY,
						  xEEPROMStack,
						  &xEEPROMTaskBuffer );

	return EEPROMInitok;
}

