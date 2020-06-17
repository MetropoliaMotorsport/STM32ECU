/*
 * memorator.c
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#include "ecumain.h"

#include <stdio.h>
#include <time.h>

time_t rtctime;


void processTime(uint8_t CANRxData[8], uint32_t DataLength )
{

	if ( rtctime == 0 ) // only set time once?
	{
		static uint8_t receiveerror = 0;
		CanState.Memorator.time = gettimer();

		if ( DataLength == FDCAN_DLC_BYTES_6
			&& CANRxData[0] > 19
			&& CANRxData[1] <= 12
			&& CANRxData[2] <= 31
			&& CANRxData[3] <= 24
			&& CANRxData[4] <= 60
			&& CANRxData[5] <= 60
			)
		{
			receiveerror=0;

			struct tm receivetime;

			receivetime.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
			receivetime.tm_year = CANRxData[0]+2000-1900; // year is concerted to years from 1900
			receivetime.tm_mon = CANRxData[1]-1; // month starts 0
			receivetime.tm_mday = CANRxData[2];

			receivetime.tm_hour = CANRxData[3];
			receivetime.tm_min = CANRxData[4];
			receivetime.tm_sec = CANRxData[5];
			rtctime  =	mktime ( &receivetime );

			DeviceState.Memorator = OPERATIONAL; // received data without error bit set, so we can assume operational state
		} else // bad data.
		{
			receiveerror++;
			Errors.CANError++;
			Errors.MemoratorReceive++;
	/*		if ( receiveerror > 10 ), device is still responding, so don't put it offline.
			{
				CarState.VoltageBMS=0;
				DeviceState.BMS = OFFLINE;
				return 0; // returnval = 0;
			} */
	#ifdef SENDBADDATAERROR
			CAN_SendStatus(99,MEMORATORReceived,99);
	#endif
			reTransmitError(99,CANRxData, DataLength);
		}
	}
}

// write a generic handler?
int receiveTime( void )
{
	uint32_t time=gettimer();
	static uint8_t errorsent;

#ifdef NOTIMEOUT
		if ( DeviceState.Memorator == OPERATIONAL )
		{
			errorsent = 0;
			return 1;
		} else return 0;
#endif

	if ( time - CanState.Memorator.time <= MEMORATORTIMEOUT && DeviceState.Memorator == OPERATIONAL )
	{
		errorsent = 0;
		return 1;
	} else
	{
        if ( DeviceState.Memorator == OPERATIONAL )
		{

			if ( errorsent == 0 )
			{
				CAN_SendStatus(200,MEMORATORReceived,(time-CanState.Memorator.time)/10);
				errorsent = 1;
				Errors.CANTimeout++;
				Errors.MemoratorTimeout++;
				DeviceState.Memorator = OFFLINE;
			}
			return 0;
		}
		return 0; // PDM is SCS, must always time out.
	}
	return 0;
}

char * getTimeStr( void ){
	static char timestr[9] = "00:00:00";
	static time_t lasttime = 0;
	if ( rtctime != 0 )
	{
		if (lasttime != rtctime )
		{
			struct tm curtime;
		    curtime = *localtime(&rtctime);
		    strftime(timestr, sizeof(timestr), "%H:%M:%S", &curtime);
		}
		return timestr;
	} else return "";
}

int initTime( void )
{
	rtctime = 0;
	return 0;
}
