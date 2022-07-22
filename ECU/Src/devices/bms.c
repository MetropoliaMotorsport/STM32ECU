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


// bms operation mode, byte 4   normal mode, data logging.
// byte 5, cell with min voltage - mv, use to trigger
// 0x9   byte 6-7 last two.

bool processBMSVoltageData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processBMSOpMode( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processBMSError( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
void BMSTimeout( uint16_t id );

CANData  BMSVoltage = { &DeviceState.BMS, BMSVOLT_ID, 8, processBMSVoltageData, BMSTimeout, 1000 };
CANData  BMSOpMode = { &DeviceState.BMS, BMSBASE_ID, 8, processBMSOpMode, NULL, 0 };
CANData  BMSError = { &DeviceState.BMS, BMSBASE_ID+1, 8, processBMSError, NULL, 0 };

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
			if ( CarState.LimpRequest )
				blinkOutput(BMSLED,LEDBLINK_TWO,LEDBLINKNONSTOP); // start BMS led blinking to indicate limp mode.
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

        	if ( CANRxData[0] != 0 ) // In Safestate.
        	{
        		Shutdown.BMS = false;
        		Shutdown.BMSReason = CANRxData[1];
				//setOutputNOW(BMSLED, false);
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
				//setOutput(BMSLED, true);
				setOutputNOW(BMSLED, true);
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



void BMSTimeout( uint16_t id )
{
	setOutputNOW(BMSLED, true);
	if ( DeviceState.BMS != OFFLINE )
	{
		CarState.VoltageBMS=0;
		SetCriticalError();
	}
}


int receiveBMS( void )
{
	if ( DeviceState.BMSEnabled )
	{
		return receivedCANData(&BMSVoltage);
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
	RegisterCan2Message(&BMSVoltage);
	RegisterCan2Message(&BMSOpMode);
	RegisterCan2Message(&BMSError);
#endif
	return 0;
}

