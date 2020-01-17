/*
 * interrupts.c
 *
 *  Created on: 10 Jan 2020
 *      Author: Markus Jahn
 */

#include "ecumain.h"

//	0x45,0,8 -> Steering Angle in degrees

void processSTR(uint8_t CANRxData[8], uint32_t DataLength )
{
	static uint8_t receiveerror = 0;
	CanState.STRAngle.time = gettimer();
	//CarState.STRAngle = CANRxData[0]*256+CANRxData[1];

	if ( DataLength == FDCAN_DLC_BYTES_2
		//Why check these?
		//&& CANRxData[0]
		//&& CANRxData[1]

		)
	{
		receiveerror=0;

		CarState.STRAngle = CANRxData[0]*256+CANRxData[1];

		DeviceState.STRAngle = OPERATIONAL;


	} else // bad data.
	{
		receiveerror++;
		Errors.CANError++;

		#ifdef SENDBADDATAERROR
//		CAN_SendStatus(99,PDMReceived,99);
#endif
		reTransmitError(99,CANRxData, DataLength);
	}
}

int receivedSTR( void )
{
	uint32_t time = gettimer();
	if ( CanState.STRAngle.time+STRTIMEOUT >= time )
    {
		return 1;
    }
	else
	{
		return 0;
	}
}


int receiveSTR( void )
{
	uint32_t time=gettimer();
	static uint8_t errorsent;

#ifdef NOTIMEOUT
		if ( DeviceState.STRAngle == OPERATIONAL )
		{
			errorsent = 0;
			return 1;
		} else return 0;
#endif

	if ( time - CanState.STRAngle.time <= STRTIMEOUT )
	{
		errorsent = 0;
		return 1;
	} else
	{
        /* T 11.9.3
         * Safe state is defined depending on the signals as follows:
         • signals only influencing indicators – Indicating a failure of its own function or of the connected system
         */

	//	if ( time - CanState.PDM.time > PDMTIMEOUT )
        if ( DeviceState.STRAngle == OPERATIONAL )
		{


			if ( errorsent == 0 )
			{
				CAN_SendStatus(200,PDMReceived,(time-CanState.STRAngle.time)/10);
				errorsent = 1;
				Errors.CANTimeout++;
				Errors.STRTimeout++;
				DeviceState.STRAngle = OFFLINE;
			}
			return 0;
		}
		return 0; // PDM is SCS, must always time out.
	}
	return 0;
}

int requestSTR( int nodeid )
{
	return 0; // this is operating with cansync, no extra needed.
}
