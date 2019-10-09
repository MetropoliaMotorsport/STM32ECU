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


uint8_t processBMSVoltage(uint8_t CANRxData[8], uint32_t DataLength )
{
	if ( DeviceState.BMSEnabled )
	{
		static uint8_t receiveerror = 0;
		CanState.BMSVolt.time = gettimer();

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
			receiveerror=0;
			CarState.VoltageBMS = voltage;
			CarState.LimpRequest = CANRxData[4];
			if ( CarState.LimpRequest )
				blinkOutput(BMSLED_Output,LEDBLINK_TWO,LEDBLINKNONSTOP); // start BMS led blinking to indicate limp mode.
			DeviceState.BMS = OPERATIONAL;
			return 1;
		} else // bad data.
		{
			receiveerror++;
			Errors.CANError++;
			Errors.BMSReceive++;
	/*		if ( receiveerror > 10 ), device is still responding, so don't put it offline.
			{
				CarState.VoltageBMS=0;
				DeviceState.BMS = OFFLINE;
				return 0; // returnval = 0;
			} */
#ifdef SENDBADDATAERROR
			CAN_SendStatus(99, BMSReceived,99);
#endif
			reTransmitError(99, CANRxData, DataLength); // put into a retransmit queue?
			return 0;
		}
	} else return 1;
}


uint8_t processBMSOpMode(uint8_t CANRxData[8], uint32_t DataLength )
{
	if ( DeviceState.BMSEnabled )
	{
		static uint8_t receiveerror = 0;
		CanState.BMSOpMode.time = gettimer();

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
			receiveerror=0;
	//		CarState.VoltageBMS = voltage;
	//		DeviceState.BMS = OPERATIONAL;
			return 1;
		} else // bad data.
		{
			receiveerror++;
			Errors.CANError++;
			Errors.BMSReceive++;
	/*		if ( receiveerror > 10 ), device is still responding, so don't put it offline.
			{
				CarState.VoltageBMS=0;
				DeviceState.BMS = OFFLINE;
				return 0; // returnval = 0;
			} */
#ifdef SENDBADDATAERROR
			CAN_SendStatus(99, BMSReceived+40,99);
#endif
			reTransmitError(99, CANRxData, DataLength); // put into a retransmit queue?
			return 0;
		}
	} else return 1;
}

uint8_t processBMSError(uint8_t CANRxData[8], uint32_t DataLength )
{
    if ( DeviceState.BMSEnabled )
    {
        static uint8_t receiveerror = 0;
        CanState.BMSOpMode.time = gettimer();
        
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
            receiveerror=0;
            //        CarState.VoltageBMS = voltage;
            //        DeviceState.BMS = OPERATIONAL;
            return 1;
        } else // bad data.
        {
            receiveerror++;
            Errors.CANError++;
            Errors.BMSReceive++;
            /*        if ( receiveerror > 10 ), device is still responding, so don't put it offline.
             {
             CarState.VoltageBMS=0;
             DeviceState.BMS = OFFLINE;
             return 0; // returnval = 0;
             } */
#ifdef SENDBADDATAERROR
            CAN_SendStatus(99, BMSReceived+41,99);
#endif
            reTransmitError(99, CANRxData, DataLength); // put into a retransmit queue?
            return 0;
        }
    } else return 1;
}


int receiveBMS( void )
{
	uint32_t time=gettimer();
	static uint8_t errorsent = 0;

/*	CarState.VoltageBMS=480;
	DeviceState.BMS = OPERATIONAL;
	return 1; */

	if ( DeviceState.BMSEnabled )
	{

#ifdef NOTIMEOUT
		if ( DeviceState.BMS == OPERATIONAL )
		{
			errorsent = 0;
			return 1;
		} else return 0;
#endif

		if ( time - CanState.BMSVolt.time <= BMSTIMEOUT && DeviceState.BMS == OPERATIONAL )
		{
			errorsent = 0;
			return 1;
		} else
		{
			if ( DeviceState.BMS == OPERATIONAL ) // time - CanState.BMSVolt.time > BMSTIMEOUT )
			{

				if ( errorsent == 0 )
				{
#ifdef errorLED
                    blinkOutput(BMSLED_Output,LEDBLINK_FOUR,255);
#endif
					CAN_SendStatus(200,BMSReceived,(time-CanState.BMSVolt.time)/10);
					errorsent = 1;
					Errors.CANTimeout++;
					Errors.BMSTimeout++;
					CarState.VoltageBMS=0;
					DeviceState.BMS = OFFLINE;
				}

			}
			return 0;
	//		return 1;
		}
	} else // BMS reading disabled, set 'default' values to allow operation regardless.
	{
		DeviceState.BMS = OPERATIONAL;
		CarState.VoltageBMS=540; // set an assumed voltage that allows operation.
		return 1;
	}
return 0;
}


int requestBMS( int nodeid )
{
	return 0; // this is operating cyclically, no extra request needed.
}

