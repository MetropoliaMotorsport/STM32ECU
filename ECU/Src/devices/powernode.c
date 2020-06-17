/*
 * powernode.c
 *
 *  Created on: 15 Jun 2020
 *      Author: Visa
 */

#include "ecumain.h"

#include <stdio.h>
#include <time.h>

time_t rtctime;

#define MAXECUCURRENT 20

bool processPNode33Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processPNode34Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processPNode35Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processPNode36Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processPNode37Data(uint8_t CANRxData[8], uint32_t DataLength );

bool processPNodeErrData(uint8_t CANRxData[8], uint32_t DataLength );
bool processPNodeAckData(uint8_t CANRxData[8], uint32_t DataLength );

CanData  PowerNode33 = { &DeviceState.PowerNode33, PowerNode33_ID, 3, processPNode33Data, NULL, 1000 }; // [BOTS, inertia switch, BSPD.], Telemetry, front power
CanData  PowerNode34 = { &DeviceState.PowerNode34, PowerNode34_ID, 4, processPNode34Data, NULL, 1000 }; // [shutdown switches.], inverters, ECU, Front,
CanData  PowerNode35 = { &DeviceState.PowerNode35, PowerNode35_ID, 4, processPNode35Data, NULL, 1000 }; // Cooling ( fans, pumps )
CanData  PowerNode36 = { &DeviceState.PowerNode36, PowerNode36_ID, 7, processPNode36Data, NULL, 1000 }; // BRL, buzz, IVT, ACCUPCB, ACCUFAN, imdfreq, dc_imd?
CanData  PowerNode37 = { &DeviceState.PowerNode37, PowerNode37_ID, 3, processPNode37Data, NULL, 1000 }; // [?], Current, TSAL.

CanData  PowerNodeErr = { NULL, PowerNodeErr_ID, 8, processPNodeErrData, NULL, 0 };
CanData  PowerNodeAck = { NULL, PowerNodeAck_ID, 3, processPNodeAckData, NULL, 0 };


#define MAXPNODEERRORS		40

struct PowerNodeError
{
	uint8_t nodeid;
	uint32_t error;
} PowerNodeErrors[MAXPNODEERRORS];
uint8_t PowerNodeErrorCount = 0;

volatile bool PowerNode33Ack = false;
volatile bool PowerNode34Ack = false;
volatile bool PowerNode35Ack = false;
volatile bool PowerNode36Ack = false;
volatile bool PowerNode37Ack = false;

uint32_t Get_Error(uint8_t errorpage, uint8_t errorbit )
{
//	canErrors[(error/32)]  |= (1<<(error%32));
//	canErrorToTransmit |= (1<<(error/32));
	return errorpage*32+errorbit;
}


bool ProcessPNodeErrorData(uint8_t data[8], uint32_t DataLength)
{
	if ( DataLength >> 16 == PowerNodeErr.dlcsize )
	{
		uint32_t errorcode=(data[2]*16777216+data[3]*65536+data[4]*256+data[5]);

		for ( int i = 0;i<32;i++){
			if ( errorcode & (1<<(i) ) )
			{
				if ( PowerNodeErrorCount < MAXPNODEERRORS )
				{
					PowerNodeErrors[PowerNodeErrorCount].nodeid = data[0];
					PowerNodeErrors[PowerNodeErrorCount].error = Get_Error(data[1], i );
					PowerNodeErrorCount++;
				}
			}

		}
	}
	return true;
}

bool processPNode33Data(uint8_t CANRxData[8], uint32_t DataLength )
{

	if ( DataLength >> 16 == PowerNode33.dlcsize
		&& CANRxData[0] <= 0b00011101 // max possible value. check for zeros in unused fields?
		&& CANRxData[1] < 100
		&& ( CANRxData[2] >= 0 && CANRxData[2] <= 100 )
		)
	{
//		CarState.BOTS = (CANRxData[0] & (0x1 << 4) );
//		CarState.InertiaSwitch = (CANRxData[0] & (0x1 << 0) );
//		CarState.BSPDAfter = (CANRxData[0] & (0x1 << 2) );
//		CarState.BSPDBefore = (CANRxData[0] & (0x1 << 3) );
//		CarState.I_Telementry =  CANRxData[1];
//		CarState.I_Front1 =  CANRxData[2];
		return true;

	} else // bad data.
	{
		return false;
	}
}

bool processPNode34Data(uint8_t CANRxData[8], uint32_t DataLength )
{

	if ( DataLength >> 16 == PowerNode34.dlcsize
		&& CANRxData[0] <= 0b00011100 // max possible value. check for zeros in unused fields?
		&& CANRxData[1] < 255
		&& ( CANRxData[2] >= 0 && CANRxData[2] <= MAXECUCURRENT )
		&& CANRxData[3] < 255
		)
	{
		CarState.ShutdownSwitchesClosed = CANRxData[0];
//		CarState.I_Inverters =  CANRxData[1];
//		CarState.I_ECU =  CANRxData[2];
//		CarState.I_Front2 =  CANRxData[3];
		return true;

	} else // bad data.
	{
		return false;
	}
}

#define MAXFANCURRENT		20
#define MAXPUMPCURRENT		20

bool processPNode35Data(uint8_t CANRxData[8], uint32_t DataLength ) // Cooling
{

	if ( DataLength >> 16 == PowerNode35.dlcsize
		&& ( CANRxData[0] >= 0 && CANRxData[0] <= MAXFANCURRENT )
		&& ( CANRxData[1] >= 0 && CANRxData[1] <= MAXFANCURRENT )
		&& ( CANRxData[2] >= 0 && CANRxData[2] <= MAXPUMPCURRENT )
		&& ( CANRxData[3] >= 0 && CANRxData[3] <= MAXPUMPCURRENT )
		)
	{
//		CarState.I_LeftFans = CANRxData[0];
//		CarState.I_RightFans =  CANRxData[1];
//		CarState.I_LeftPump =  CANRxData[2];
//		CarState.I_RightPump =  CANRxData[3];
		return true;

	} else // bad data.
	{
		return false;
	}
}

bool processPNode36Data(uint8_t CANRxData[8], uint32_t DataLength ) // Rear
{

	if ( DataLength >> 16 == PowerNode36.dlcsize
		&& ( CANRxData[0] >= 0 && CANRxData[0] <= 10 )
		&& ( CANRxData[1] >= 0 && CANRxData[1] <= 10 )
		&& ( CANRxData[2] >= 0 && CANRxData[2] <= 10 )
		&& ( CANRxData[3] >= 0 && CANRxData[3] <= 10 )
		&& ( CANRxData[4] >= 0 && CANRxData[4] <= 10 )
		&& ( CANRxData[5] >= 0 && CANRxData[5] <= 255 )
		&& ( CANRxData[6] >= 0 && CANRxData[6] <= 100 )
		)
	{
		CarState.I_BrakeLight = CANRxData[0];
		CarState.I_Buzzers = CANRxData[1];
		CarState.I_IVT = CANRxData[2];
		CarState.I_AccuPCBs = CANRxData[3];
		CarState.I_AccuFans = CANRxData[4];
		CarState.Freq_IMD = CANRxData[5];
		CarState.DC_IMD = CANRxData[6];

		return true;
	} else // bad data.
	{
		return false;
	}
}

bool processPNode37Data(uint8_t CANRxData[8], uint32_t DataLength )
{

	if ( DataLength >> 16 == PowerNode37.dlcsize
		&& CANRxData[0] <= 0b00011101 // max possible value. check for zeros in unused fields?
		&& CANRxData[1] < 100
		&& ( CANRxData[2] >= 0 && CANRxData[2] <= 100 )
		)
	{
//		CarState.??? = (CANRxData[0] & (0x1 << 4) );
//		CarState.I_currentMeasurement =  CANRxData[1];
//		CarState.I_TSAL =  CANRxData[2];
		return true;
	} else // bad data.
	{
		return false;
	}
}

bool processPNodeErrData(uint8_t CANRxData[8], uint32_t DataLength )
{
	return true;
}
bool processPNodeAckData(uint8_t CANRxData[8], uint32_t DataLength )
{
	return true;
}

int receivePowerNodes( void )
{
	uint8_t nodesonline = 0b00011111;

	if ( receivedCANData(&PowerNode33) ) nodesonline -= 1;
	if ( receivedCANData(&PowerNode34) ) nodesonline -= 2;
	if ( receivedCANData(&PowerNode35) ) nodesonline -= 4;
	if ( receivedCANData(&PowerNode36) ) nodesonline -= 8;
	if ( receivedCANData(&PowerNode37) ) nodesonline -= 16;
	return nodesonline;
}

#define DeviceIVT 1

int setdevicepower( uint32_t device, bool state )
{
	switch ( device )
	{
		case DeviceIVT :
			if ( receivePowerNodes() & (0x1 << 3) ? "6" : "")
			{

			}

		break;

		default:
			 break;

	}

	return 0;
}
