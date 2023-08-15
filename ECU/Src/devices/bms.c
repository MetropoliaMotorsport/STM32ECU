/*
 * bms.c
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#include "ecumain.h"
#include "errors.h"
#include "canecu.h"
#include "bms.h"
#include "output.h"
#include "power.h"
#include "debug.h"


// bms operation mode, byte 4   normal mode, data logging.
// byte 5, cell with min voltage - mv, use to trigger
// 0x9   byte 6-7 last two.

#ifdef HPF2023
bool processBMSSOC( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
#else
bool processBMSVoltageData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processBMSOpMode( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processBMSError( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
#endif

void BMSTimeout( uint16_t id );

#ifdef HPF2023
CANData  BMSSOC = { &DeviceState.BMS, BMSSOC_ID, 8, processBMSSOC, BMSTimeout, 6000 };
#else
CANData  BMSVoltage = { &DeviceState.BMS, BMSVOLT_ID, 8, processBMSVoltageData, BMSTimeout, 2500 };
CANData  BMSOpMode = { &DeviceState.BMS, BMSBASE_ID, 8, processBMSOpMode, NULL, 0 };
CANData  BMSError = { &DeviceState.BMS, BMSBASE_ID+1, 8, processBMSError, NULL, 0 };
#endif

#ifdef HPF2023
bool processBMSSOC( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool message = false;
	static uint16_t lastcellv = 0;
	static uint32_t count = 0;
	if ( DeviceState.BMSEnabled )
	{
//		uint16_t voltage = CANRxData[2]*256+CANRxData[3];

		if ( true // no current validation on this message.
/*				&& CANRxData[0] == 0
				&& CANRxData[1] == 0
				&& CANRxData[4] == 0
				&& CANRxData[5] == 0
				&& CANRxData[6] == 0
				&& CANRxData[7] == 0 */
	//			&& ( voltage > 480 && voltage < 600 )
				)
		{
			CarState.BMSSOC = CANRxData[0];
			CarState.VoltageBMS = CANRxData[5]*256+CANRxData[4];
			CarState.HighestCellV = CANRxData[2]*256+CANRxData[1];

			Shutdown.BMS = false;

        	if ( CANRxData[3] != 0 ) // In Safestate.
        	{
				setOutputNOW(BMSLED,On); // BMS is in an error state, ensure AMS error led is shown without delay.
        		//Shutdown.BMS = false;
        		Shutdown.BMSReason = CANRxData[1];
                /*
                      0 : str := 'undefined';
                      1 : str := 'overvoltage';
                      2 : str := 'undervoltage';
                      3 : str := 'overtemperature';
                      4 : str := 'undertemperature';
                      5 : str := 'overcurrent';
                      6 : str := 'overpower';
                      7 : str := 'external';
                      8 : str := 'pec_error';
                      9 : str := 'Accumulator Undervoltage';
                      10 : str := 'IVT MOD timeout';
				*/
        	} else
        	{
         		//Shutdown.BMS = true;
         		Shutdown.BMSReason = 0;
        	}
#ifdef BMSDEBUGINFO

        	if ( !message || lastcellv != CarState.HighestCellV  || ( count % 20 ) == 0 )
        	{
        		lastcellv = CarState.HighestCellV;
        		message = true;
        		DebugPrintf("BMS msg SOC %lu high cell mV %lu stackV %lu error state %lu", CarState.BMSSOC, CarState.HighestCellV, CarState.VoltageBMS, CANRxData[3]);
        	}
#endif
        	count++;

			//CarState.LimpRequest = CANRxData[4]; // not yet implemented on new BMS.

			return true;
		} else // bad data.
		{
			return false;
		}
	} else return true;
}

#else
bool processBMSVoltageData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	if ( DeviceState.BMSEnabled )
	{
		uint16_t voltage = CANRxData[2]*256+CANRxData[3];

		if (  DataLength == FDCAN_DLC_BYTES_8
				&& CANRxData[0] == 0
				&& CANRxData[1] == 0
				&& CANRxData[4] < 2 // limp byte
				&& CANRxData[5] == 0
				&& CANRxData[6] == 0xAB
				&& CANRxData[7] == 0xCD
				&& ( voltage > 480 && voltage < 620 ) ) // increased max voltage, so it can be reported better as error if it goes over.
		{
			CarState.VoltageBMS = voltage;
			CarState.LimpRequest = CANRxData[4];
			//if ( CarState.LimpRequest )
			//	blinkOutput(BMSLED,LEDBLINK_TWO,LEDBLINKNONSTOP); // start BMS led blinking to indicate limp mode.
			return true;
		} else // bad data.
		{
			return false;
		}
	} else return true;
}


bool processBMSOpMode( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	if ( DeviceState.BMSEnabled )
	{
//		uint16_t voltage = CANRxData[2]*256+CANRxData[3];

		if ( true // no current validation on this message.
/*				&& CANRxData[0] == 0
				&& CANRxData[1] == 0
				&& CANRxData[4] == 0
				&& CANRxData[5] == 0
				&& CANRxData[6] == 0
				&& CANRxData[7] == 0 */
	//			&& ( voltage > 480 && voltage < 600 )
				)
		{
			return true;
		} else // bad data.
		{
			return false;
		}
	} else return true;
}

bool processBMSError( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
    if ( DeviceState.BMSEnabled )
    {
  //      uint16_t voltage = CANRxData[2]*256+CANRxData[3];

        if ( true // no current validation on this message.
        /*                && CANRxData[0] == 0
         && CANRxData[1] == 0
         && CANRxData[4] == 0
         && CANRxData[5] == 0
         && CANRxData[6] == 0
         && CANRxData[7] == 0 */
            //            && ( voltage > 480 && voltage < 600 )
            )
        {
    		CarState.LowestCellV = CANRxData[6]*256+CANRxData[7];
    		//CarState.HighestCellV =

        	if ( CANRxData[0] != 0 ) // In Safestate.
        	{
				setOutputNOW(BMSLED,On);
        		Shutdown.BMS = false;
        		Shutdown.BMSReason = CANRxData[1];
                /*
                      0 : str := 'undefined';
                      1 : str := 'overvoltage';
                      2 : str := 'undervoltage';
                      3 : str := 'overtemperature';
                      4 : str := 'undertemperature';
                      5 : str := 'overcurrent';
                      6 : str := 'overpower';
                      7 : str := 'external';
                      8 : str := 'pec_error';
                      9 : str := 'Accumulator Undervoltage';
                      10 : str := 'IVT MOD timeout';
				*/
        	} else
        	{
         		Shutdown.BMS = true;
         		Shutdown.BMSReason = 0;
        	}

            return true;
        } else // bad data.
        {
            return false;
        }
    } else return true;
}
#endif

void BMSTimeout( uint16_t id )
{
	setOutputNOW(BMSLED,On);
	Shutdown.BMS = true;
	DebugMsg("BMS Timeout");
	CAN_SendErrorStatus(199,0,0);
	if ( DeviceState.BMS != OFFLINE )
	{
		CarState.VoltageBMS=0;
		SetCriticalError(CRITERRBMSTIMEOUT);
	}
}


int receiveBMS( void )
{
	if ( DeviceState.BMSEnabled )
	{
#ifdef HPF2023
		return receivedCANData(&BMSSOC);
#else
		return receivedCANData(&BMSVoltage);
#endif
	}
	else // BMS reading disabled, set 'default' values to allow operation regardless.
	{
		DeviceState.BMS = OPERATIONAL;
		CarState.VoltageBMS=540; // set an assumed voltage that allows operation.
		return 1;
	}
}


int requestBMS( int nodeid )
{
	return 0; // this is operating cyclically, no extra request needed.
}

void resetBMS( void )
{
#ifdef BMSEnable
	DeviceState.BMSEnabled = ENABLED;
#else
	DeviceState.BMSEnabled = DISABLED;
#endif

	DeviceState.BMS = OFFLINE;
	CarState.VoltageBMS=0;
}

int initBMS( void )
{
	RegisterResetCommand(resetBMS);

	resetBMS();
#ifdef HPF19
	RegisterCan1Message(&BMSVoltage);
	RegisterCan1Message(&BMSOpMode);
	RegisterCan1Message(&BMSError);
#else
	#ifdef HPF2023
	RegisterCan2Message(&BMSSOC);
	#else
	RegisterCan2Message(&BMSVoltage);
	RegisterCan2Message(&BMSOpMode);
	RegisterCan2Message(&BMSError);
	#endif
#endif
	return 0;
}

