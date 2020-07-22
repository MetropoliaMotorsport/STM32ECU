/*
 * powernode.c
 *
 *  Created on: 15 Jun 2020
 *      Author: Visa
 */

#include "ecumain.h"
#include <stdarg.h>

#include <stdio.h>
#include <time.h>

#define POWERNODECOUNT		5

#define MAXECUCURRENT 		20
#define MAXFANCURRENT		20
#define MAXPUMPCURRENT		20

typedef struct nodepowerreqstruct {
	uint8_t nodeid; //
	uint8_t output; // enable
	uint8_t state; // request state
	uint8_t error[6];
} nodepowerreq;

typedef struct devicepowerreqstruct {
	DeviceId device; //
	uint8_t nodeid;
	uint8_t output; // enable
	uint8_t state; // request state
} devicepowerreq;

devicepowerreq DevicePowerList[] =
{
		{ Telemetry, 33, 4 },
		{ Front1, 33, 5 },

		{ Inverters, 34, 3 },
		{ ECU, 34, 4 },
		{ Front2, 34, 5 },

		{ LeftFans, 35, 2 },
		{ RightFans, 35, 3 },
		{ LeftPump, 35, 4 },
		{ RightPump, 35, 5 },

		{ IVT, 36, 1 },
		{ Buzzer, 36, 0 },

		{ Current, 37, 4 },
		{ TSAL, 37, 5 },

		{ 0 }
};

nodepowerreq PowerRequests[] =
{		{ 33, 0, 0, {0} },
		{ 34, 0, 0, {0} },
		{ 35, 0, 0, {0} },
		{ 36, 0, 0, {0} },
		{ 37, 0, 0, {0} },
		{ 0 }
};



bool processPNode33Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processPNode34Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processPNode35Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processPNode36Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processPNode37Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );


//bool processPNodeTimeout(uint8_t CANRxData[8], uint32_t DataLength );


bool processPNodeErr(uint8_t nodeid, uint32_t errorcode, CANData * datahandle );
bool processPNodeAckData(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );

CANData  PowerNode33 = { &DeviceState.PowerNode33, PowerNode33_ID, 3, processPNode33Data, NULL, NODETIMEOUT }; // [BOTS, inertia switch, BSPD.], Telemetry, front power
CANData  PowerNode34 = { &DeviceState.PowerNode34, PowerNode34_ID, 4, processPNode34Data, NULL, NODETIMEOUT }; // [shutdown switches.], inverters, ECU, Front,
CANData  PowerNode35 = { &DeviceState.PowerNode35, PowerNode35_ID, 4, processPNode35Data, NULL, NODETIMEOUT }; // Cooling ( fans, pumps )
CANData  PowerNode36 = { &DeviceState.PowerNode36, PowerNode36_ID, 7, processPNode36Data, NULL, NODETIMEOUT }; // BRL, buzz, IVT, ACCUPCB, ACCUFAN, imdfreq, dc_imd?
CANData  PowerNode37 = { &DeviceState.PowerNode37, PowerNode37_ID, 3, processPNode37Data, NULL, NODETIMEOUT }; // [?], Current, TSAL.


int sendPowerNodeErrReset( uint8_t id, uint8_t channel );

bool processPNode33Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{

	if ( DataLength >> 16 == PowerNode33.dlcsize
		&& CANRxData[0] <= 0b00011101 // max possible value. check for zeros in unused fields?
		&& CANRxData[1] < 255
		&& ( CANRxData[2] >= 0 && CANRxData[2] < 255 )
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

bool processPNode34Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{

	if ( DataLength >> 16 == PowerNode34.dlcsize
		&& CANRxData[0] <= 0b00011100 // max possible value. check for zeros in unused fields?
		&& CANRxData[1] < 255
		&& ( CANRxData[2] >= 0 && CANRxData[2] <= 255 )
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


bool processPNode35Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle ) // Cooling
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

bool processPNode36Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle ) // Rear
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

bool processPNode37Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
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


bool processPNodeAckData(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{

//	CANRxData[0] ==     // nodeid
	for ( int i=0;PowerRequests[i].nodeid != 0;i++)
	{
		if ( PowerRequests[i].nodeid == CANRxData[0] && CANRxData[1] == 1 ) // check for power request change.
		{
			PowerRequests[i].output ^= CANRxData[2];
			PowerRequests[i].state ^= CANRxData[3];

			//TODO if output = 0 and state != 0 then something is on that we didn't expect? update local expectation or clear.

//			PowerRequests[i].output = 0; // we've had ack that request on this node was processed, clear it.
//			PowerRequests[i].state = 0;
		}
	}
	return true;
}



// ERROR list from powernode main.h


#define WARN_UNDERVOLT_U5				193
#define WARN_OVERVOLT_U5				194
#define WARN_UNDERTEMP_U5				195
#define WARN_OVERTEMP_U5				196
#define WARN_UNDERCURR_U5I0				197
#define WARN_OVERCURR_U5I0				198
#define WARN_UNDERCURR_U5I1				199
#define WARN_OVERCURR_U5I1				200
#define ERROR_OVERCURR_TRIP_U5_0		201 //PO0
#define ERROR_OVERCURR_TRIP_U5_1		202 //PO1
#define WARN_UNDERVOLT_U6				203
#define WARN_OVERVOLT_U6				204
#define WARN_UNDERTEMP_U6				205
#define WARN_OVERTEMP_U6				206
#define WARN_UNDERCURR_U6I0				207
#define WARN_OVERCURR_U6I0				208
#define WARN_UNDERCURR_U6I1				209
#define WARN_OVERCURR_U6I1				210
#define ERROR_OVERCURR_TRIP_U6_0		211 //PO2
#define ERROR_OVERCURR_TRIP_U6_1		212 //PO3
#define WARN_UNDERVOLT_U7				213
#define WARN_OVERVOLT_U7				214
#define WARN_UNDERTEMP_U7				215
#define WARN_OVERTEMP_U7				216
#define WARN_UNDERCURR_U7I0				217
#define WARN_OVERCURR_U7I0				218
#define WARN_UNDERCURR_U7I1				219
#define WARN_OVERCURR_U7I1				220
#define ERROR_OVERCURR_TRIP_U7_0		221 //PO4
#define ERROR_OVERCURR_TRIP_U7_1		222 //PO5


//
// command 12 is used to clear an error from switch shutoff
// until the error is cleared the node will regularly send a warning
// about which channel has been shutoff, byte 2 should be set to the
// number of which channel to clear (0 to 5)

// for example: [1][12][3] will clear an error on channel 3 from node 1



#define U5I0_SWITCH_OFF				129 //PO0
#define U5I1_SWITCH_OFF				130 //PO1
#define U6I0_SWITCH_OFF				131 //PO2
#define U6I1_SWITCH_OFF				132 //PO3
#define U7I0_SWITCH_OFF				133 //PO4
#define U7I1_SWITCH_OFF				134 //PO5



bool processPNodeErr(uint8_t nodeid, uint32_t errorcode, CANData * datahandle )
{
	// acknowledge error received.
	uint8_t channel = 255;
	// set an error state to be acted upon elsewhere.

	switch ( errorcode )
	{
		case U5I0_SWITCH_OFF : channel = 0; break; //PO0
		case U5I1_SWITCH_OFF : channel = 1; break; //PO1
		case U6I0_SWITCH_OFF : channel = 2; break; //PO2
		case U6I1_SWITCH_OFF : channel = 3; break; //PO3
		case U7I0_SWITCH_OFF : channel = 4; break; //PO4
		case U7I1_SWITCH_OFF : channel = 5; break; //PO5
	}

	if ( channel != 255 )
	{
		sendPowerNodeErrReset( nodeid,  channel );
		for ( int i=0;PowerRequests[i].nodeid != 0;i++)
		{ // find the node record where error occurred.
			if ( PowerRequests[i].nodeid == nodeid )
			{
				if ( PowerRequests[i].error[channel] < 255 ) PowerRequests[i].error[channel]++;
				// keep a count of how many times error occurred.
			}
		}

		return true; // handled error.
	}

	return false;
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


int setinitialdevicepower( void )
{
	return 0;
}

int average(int count, ...)
{
	return 0;
}


int setDevicePower( DeviceId device, bool state )
{
	int powerreqset = 1;

	for ( int i=0;DevicePowerList[i].device != 0;i++)
	{
		if ( DevicePowerList[i].device == device )
		{ // found the device in list, try to set request.

			for ( int j=0;PowerRequests[j].nodeid != 0;j++)
			{
				if ( PowerRequests[j].nodeid ==  DevicePowerList[i].nodeid )
				{
					// check existing request.
					bool enabled = PowerRequests[j].output & (0x1 <<  DevicePowerList[i].output);
					if ( !enabled || // no request not yet made
						( enabled && (PowerRequests[j].state & (0x1 << DevicePowerList[i].output) ) != state ) // request different to previously not processed request
					)
					{
						PowerRequests[j].output |= (0x1 << DevicePowerList[i].output); // set enable output
						if ( state == true )
						  PowerRequests[j].state |=(0x1 << DevicePowerList[i].output); // set bit
						else
						  PowerRequests[j].state &= ~(0x1 << DevicePowerList[i].output); // unset bit
						powerreqset = 0;
					} else powerreqset = 1; // request already set
				}
				if ( powerreqset == 0 ) break;
			}
			if ( powerreqset == 0 ) break;
		}
	}
	return powerreqset; // return if device was found and request set.
}

int sendPowerNodeErrReset( uint8_t id, uint8_t channel )
{
	uint8_t candata[8] = { id, 12, };
	CAN1Send(NodeCmd_ID, 3, candata );
	return 0;
}

bool powerErrorOccurred( DeviceId device )
{

	for ( int i=0;DevicePowerList[i].device != 0;i++)
	{
		if ( DevicePowerList[i].device == device )
		{ // found the device in list, try to set request.

			for ( int j=0;PowerRequests[j].nodeid != 0;j++)
			{
				if ( PowerRequests[j].nodeid ==  DevicePowerList[i].nodeid )
				{

					if ( PowerRequests[j].error[DevicePowerList[i].output] )
						return true;
					else
						return false;
				}
			}
		}
	}

	return false; // TODO implement checking received errors.
}


int sendPowerNodeReq( void )
{
	uint8_t candata[8] = { 0, 1, 0, 0 };
	bool senderror = false;

	for ( int i=0;i<POWERNODECOUNT;i++)
	{

		if ( PowerRequests[i].output != 0 )
		{
			candata[0] = PowerRequests[i].nodeid;
			candata[2] = PowerRequests[i].output;
			candata[3] = PowerRequests[i].state;
			CAN1Send(NodeCmd_ID, 8, candata );
			// don't reset request until we've seen an ack -> use ack handler to clear request.
		}

	};
	return senderror;
}
