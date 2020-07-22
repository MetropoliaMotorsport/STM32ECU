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

bool processTimeData(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );

CANData  Memorator = { &DeviceState.Memorator, MEMORATOR_ID, 3, processTimeData, NULL, 1000 }; // [BOTS, inertia switch, BSPD.], Telemetry, front power


bool processTimeData(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{

	if ( rtctime == 0 ) // only set time once?
	{
		if ( DataLength == FDCAN_DLC_BYTES_6
			&& CANRxData[0] > 19
			&& CANRxData[1] <= 12
			&& CANRxData[2] <= 31
			&& CANRxData[3] <= 24
			&& CANRxData[4] <= 60
			&& CANRxData[5] <= 60
			)
		{
			struct tm receivetime;

			receivetime.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
			receivetime.tm_year = CANRxData[0]+2000-1900; // year is concerted to years from 1900
			receivetime.tm_mon = CANRxData[1]-1; // month starts 0
			receivetime.tm_mday = CANRxData[2];

			receivetime.tm_hour = CANRxData[3];
			receivetime.tm_min = CANRxData[4];
			receivetime.tm_sec = CANRxData[5];
			rtctime  =	mktime ( &receivetime );
			return true;

		} else // bad data.
		{
			return false;
		}
	} return true;
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
