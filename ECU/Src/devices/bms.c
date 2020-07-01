/*
 * bms.c
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#include "ecumain.h"

// bms operation mode, byte 4   normal mode, data logging.
// byte 5, cell with min voltage - mv, use to trigger
// 0x9   byte 6-7 last two.

bool processBMSVoltageData(uint8_t CANRxData[8], uint32_t DataLength );
void BMSTimeout( uint16_t id );

CanData  BMSVoltage = { &DeviceState.BMS, BMSVOLT_ID, 3, processBMSVoltageData, BMSTimeout, 1000 };
CanData  BMSOpMode = { &DeviceState.BMS, BMSVOLT_ID, 3, NULL, NULL, 1000 };
CanData  BMSError = { &DeviceState.BMS, BMSVOLT_ID, 3, NULL, NULL, 1000 };



bool processBMSVoltageData(uint8_t CANRxData[8], uint32_t DataLength )
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
				&& ( voltage > 480 && voltage < 600 ) )
		{
			CarState.VoltageBMS = voltage;
			CarState.LimpRequest = CANRxData[4];
			if ( CarState.LimpRequest )
				blinkOutput(BMSLED_Output,LEDBLINK_TWO,LEDBLINKNONSTOP); // start BMS led blinking to indicate limp mode.
			return true;
		} else // bad data.
		{
			return false;
		}
	} else return true;
}


bool processBMSOpMode(uint8_t CANRxData[8], uint32_t DataLength )
{
	if ( DeviceState.BMSEnabled )
	{
//		uint16_t voltage = CANRxData[2]*256+CANRxData[3];

		if (  DataLength == FDCAN_DLC_BYTES_8
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

bool processBMSError(uint8_t CANRxData[8], uint32_t DataLength )
{
    if ( DeviceState.BMSEnabled )
    {
  //      uint16_t voltage = CANRxData[2]*256+CANRxData[3];
        
        if (  DataLength == FDCAN_DLC_BYTES_8
        /*                && CANRxData[0] == 0
         && CANRxData[1] == 0
         && CANRxData[4] == 0
         && CANRxData[5] == 0
         && CANRxData[6] == 0
         && CANRxData[7] == 0 */
            //            && ( voltage > 480 && voltage < 600 )
            )
        {
            return true;
        } else // bad data.
        {

            return false;
        }
    } else return true;
}



void BMSTimeout( uint16_t id )
{
	CarState.VoltageBMS=0;
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

