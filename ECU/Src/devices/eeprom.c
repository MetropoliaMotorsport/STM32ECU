/*
 * eeprom.c
 *
 *  Created on: 20 Feb 2020
 *      Author: Visa
 */

#include "ecumain.h"
#include "eeprom.h"
#include "i2c.h"
#include "i2c-lcd.h"
#include "stm32h7xx_hal.h"

	uint16_t Memory_Address;
	volatile int Remaining_Bytes;

	union { // EEPROMU
		uint8_t buffer[4096];
		struct {
			char version[32]; // block 0  32 bytes
			uint8_t active; // block 1 32 bytes
			uint8_t paddingact[31];
			uint8_t reserved1[32*8]; // blocks 2-9 256 bytes.
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
	} EEPROMdata;

	volatile bool eepromwritinginprogress = false;
	volatile bool eepromreceivedone = false;

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


	eepromdata * getEEPROMBlock(int block )
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
	  /* Turn LED2 on: Transfer in reception process is correct */
//	  toggleOutput(40);
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

		 if ( I2cHandle->Instance == I2C2 ){

			 eepromwritinginprogress = false;

		 }


	  /* Turn LED3 on: Transfer error in reception/transmission process */
		  toggleOutput(LED7_Output);
	}

	int initiliseEEPROM( void ){

		// TODO could be optimised to only read necessary block.

		HAL_Delay(100); // Allow time for EEPROM chip to initialise itself before start trying to access.
		HAL_I2CEx_ConfigAnalogFilter(&hi2c2,I2C_ANALOGFILTER_ENABLE);

		HAL_GPIO_WritePin( EEPROMWC_GPIO_Port, EEPROMWC_Pin, 1); // block writing to eeprom, only reading at init.
		HAL_Delay(1);
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
			return 1;
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
			return result;
		}

		if ( !checkversion((char *)getEEPROMBlock( 0 )) )
		{
			return 1;
		}

		return 0;
		// headers ok, continue.
#endif
	}

	int readEEPROMAddr( uint16_t address, uint16_t size )
	{
		uint32_t startread = gettimer();
		eepromreceivedone = false;
		if(HAL_I2C_Mem_Read_IT(&hi2c2 , (uint16_t)EEPROM_ADDRESS, address, I2C_MEMADD_SIZE_16BIT, (uint8_t*)&EEPROMdata.buffer[address], size)!= HAL_OK)
		{
		/* Reading process Error */
		   return 1;// Error_Handler(); // failed to read data for some reason.
		}

		while ( !eepromreceivedone ) // 4 sec read timeout so will still startup regardless.
		{
			//__WFI();
			HAL_Delay(10);
			if ( gettimer() > startread + 40000 ) // TODO check right way round.
			{
				return 1;
			}
		};

		if ( !eepromreceivedone )
		{
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
		static int errorcount = 0;

		toggleOutputMetal(LED7_Output);
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
						Error_Handler(); //error here is not a hard error, don't hang code.
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
		if ( ! eepromwritinginprogress){

			HAL_GPIO_WritePin( EEPROMWC_GPIO_Port, EEPROMWC_Pin, 0); // enable write pin.

			//setup data to write.

			Remaining_Bytes = sizeof(EEPROMdata);
			Memory_Address = 0;

			// EEPROMdata.buffer

			eepromwritinginprogress = true;


			if ( HAL_TIM_Base_Start_IT(&htim16) != HAL_OK){ // start write timer.
				Error_Handler();
			}
			return 1;

		} else return 0;

	}


	int writeConfigEEPROM( void )
	{
		if ( ! eepromwritinginprogress){

			HAL_GPIO_WritePin( EEPROMWC_GPIO_Port, EEPROMWC_Pin, 0); // enable write pin.

			//setup data to write.

			Remaining_Bytes = sizeof(EEPROMdata.padding1)+ sizeof(eepromdata);
			Memory_Address = EEPROMdata.padding1-EEPROMdata.buffer;

			// EEPROMdata.buffer

			eepromwritinginprogress = true;


			if ( HAL_TIM_Base_Start_IT(&htim16) != HAL_OK){ // start write timer.
				Error_Handler();
			}
			return 1;

		} else return 0;

	}






	int writeEEPROM( int bank )  //write one of the two config banks to EEPROM
	{
		if ( ! eepromwritinginprogress){


			if ( bank > 1 || bank < 0 ) return 0; // invalid bank number given.
			HAL_GPIO_WritePin( EEPROMWC_GPIO_Port, EEPROMWC_Pin, 0); // enable write pin.

			//setup data to write.

		//	if ( bank = 0 )	Memory_Address =

			eepromwritinginprogress = true;

			if ( HAL_TIM_Base_Start_IT(&htim16) != HAL_OK){ // start write timer.
				Error_Handler();
			}
			return 1;

		} else return 0;

	}

	int writeEEPROMCurConf( void )  //write one of the two config banks to EEPROM
	{
		if ( ! eepromwritinginprogress){

			HAL_GPIO_WritePin( EEPROMWC_GPIO_Port, EEPROMWC_Pin, 0); // enable write pin.

			//setup data to write.

		//	if ( bank = 0 )	Memory_Address =

			//setup data to write.

			Remaining_Bytes = 32;
			Memory_Address = &EEPROMdata.block1.MaxTorque-EEPROMdata.buffer;

			eepromwritinginprogress = true;

			if ( HAL_TIM_Base_Start_IT(&htim16) != HAL_OK){ // start write timer.
				Error_Handler();
			}
			return 1;

		} else return 0;

	}



	int writeEEPROMEmergency( void )   // write emergency packet to end of EEPROM.
	{
		return 0;
	}


	bool writeEEPROMDone()
	{
		return !eepromwritinginprogress;

		// handle switchin active block in here.
	}

