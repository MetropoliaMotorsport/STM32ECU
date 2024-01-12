/*
 * memorator.c
 *
 *  Created on: 01 May 2019
 *      Author: Visa
 */

#include "ecumain.h"
#include "memorator.h"
#include "timerecu.h"

#include <stdio.h>
#include <time.h>

bool processTimeData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

CANData  Memorator = { &DeviceState.Memorator, MEMORATOR_ID, 3, processTimeData, NULL, 1000 }; // [BOTS, inertia switch, BSPD.], Telemetry, front power


bool processTimeData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{

	if ( ! isRTCSet() ) // only set time once?
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
			setRTC( mktime( &receivetime ) );
			return true;

		} else // bad data.
		{
			return false;
		}
	} return true;
}

void resetMemorator( void )
{
//	rtctime = 0;
//	setRTC( 0 );
}

int initMemorator( void )
{
	RegisterResetCommand(resetMemorator);

	resetMemorator();

	RegisterCan1Message(&Memorator);
	return 0;
}
