/*
 * analognode.c
 *
 *  Created on: 29 Jun 2020
 *      Author: Visa
 */

#include "ecumain.h"
#include <stdarg.h>

#include <stdio.h>
#include <time.h>

#define ANALOGNODECOUNT	11

bool processANode1Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processANode9Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processANode10Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processANode11Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processANode12Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processANode13Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processANode14Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processANode15Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processANode16Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processANode17Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processANode18Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );

void ANodeCritTimeout( uint16_t id );

CANData  AnalogNode1 =  { &DeviceState.AnalogNode1, AnalogNode1_ID, 6, processANode1Data, ANodeCritTimeout, NODETIMEOUT };
CANData  AnalogNode9 =  { &DeviceState.AnalogNode9, AnalogNode9_ID, 6, processANode9Data, NULL, NODETIMEOUT };
CANData  AnalogNode10 = { &DeviceState.AnalogNode10, AnalogNode10_ID, 6, processANode10Data, NULL, NODETIMEOUT };
CANData  AnalogNode11=  { &DeviceState.AnalogNode11, AnalogNode11_ID, 6, processANode11Data, ANodeCritTimeout, NODETIMEOUT };
CANData  AnalogNode12 = { &DeviceState.AnalogNode12, AnalogNode12_ID, 6, processANode12Data, NULL, NODETIMEOUT };
CANData  AnalogNode13 = { &DeviceState.AnalogNode13, AnalogNode13_ID, 6, processANode13Data, NULL, NODETIMEOUT };
CANData  AnalogNode14 = { &DeviceState.AnalogNode14, AnalogNode14_ID, 6, processANode14Data, NULL, NODETIMEOUT };
CANData  AnalogNode15 = { &DeviceState.AnalogNode15, AnalogNode15_ID, 6, processANode15Data, NULL, NODETIMEOUT };
CANData  AnalogNode16 = { &DeviceState.AnalogNode16, AnalogNode16_ID, 6, processANode16Data, NULL, NODETIMEOUT };
CANData  AnalogNode17 = { &DeviceState.AnalogNode17, AnalogNode17_ID, 6, processANode17Data, NULL, NODETIMEOUT };
CANData  AnalogNode18 = { &DeviceState.AnalogNode18, AnalogNode18_ID, 6, processANode18Data, NULL, NODETIMEOUT };


void ANodeCritTimeout( uint16_t id ) // ensure critical ADC values are set to safe defaults if not received.
{
	ADCState.Torque_Req_L_Percent=0;
	ADCState.Torque_Req_R_Percent=0;
	ADCState.Regen_Percent=0;
    ADCState.BrakeF = APPSBrakeHard;
    ADCState.BrakeR = APPSBrakeHard;
}

bool processANode1Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
 //   int Val1 = CANRxData[0]*256+CANRxData[1]; // dhab current?
	int AccelL = CANRxData[2]*256+CANRxData[3];
	int Regen = CANRxData[4]*256+CANRxData [5];

	if ( DataLength >> 16 == AnalogNode1.dlcsize
		&& ( AccelL < 4096 )
		&& ( Regen < 4096 )
		)
	{
        ADCState.Torque_Req_L_Percent = getTorqueReqPercL(AccelL*16);
        ADCState.Regen_Percent = getBrakeTravelPerc(Regen*16);

		return true;
	} else // bad data.
	{
		return false;
	}
	return true;
}


bool processANode9Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	return true;
}


bool processANode10Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	return true;
}


bool processANode11Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
 //   int Val1 = CANRxData[0]*256+CANRxData[1]; // dhab current?
	int AccelR = CANRxData[4]*256+CANRxData[5];

	if ( DataLength >> 16 == AnalogNode1.dlcsize
		&&	CANRxData[2] < 240
		&&  CANRxData[3] < 240
		&& ( AccelR < 4096 )
		)
	{
        ADCState.BrakeF = CANRxData[2];
        ADCState.BrakeR = CANRxData[3];
        ADCState.Torque_Req_R_Percent = getTorqueReqPercR(AccelR*16);

		return true;
	} else // bad data.
	{
		return false;
	}
	return true;
}

bool processANode12Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	return true;
}


bool processANode13Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	return true;
}

bool processANode14Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	return true;
}

bool processANode15Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	return true;
}

bool processANode16Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	return true;
}

bool processANode17Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	return true;
}

bool processANode18Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	return true;
}


char ANodeCritStr[10] = "";
char ANodeStr[15] = "";

int receiveAnalogNodes( void ) // any of these missing should just be a warning or note.
{
	uint16_t nodesonline = 0b111111111;

	ANodeStr[0] = 0;

	uint8_t pos = 0;

	if ( receivedCANData(&AnalogNode9) ) nodesonline -= 1;
	else
	{
		ANodeStr[pos] = '9';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( receivedCANData(&AnalogNode10) ) nodesonline -= 2;
	else
	{
		ANodeStr[pos] = '0';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( receivedCANData(&AnalogNode12) ) nodesonline -= 4;
	else
	{
		ANodeStr[pos] = '2';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( receivedCANData(&AnalogNode13) ) nodesonline -= 8;
	else
	{
		ANodeStr[pos] = '3';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( receivedCANData(&AnalogNode14) ) nodesonline -= 16;
	else
	{
		ANodeStr[pos] = '4';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( receivedCANData(&AnalogNode15) ) nodesonline -= 32;
	else
	{
		ANodeStr[pos] = '5';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( receivedCANData(&AnalogNode16) ) nodesonline -= 64;
	else
	{
		ANodeStr[pos] = '6';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( receivedCANData(&AnalogNode17) ) nodesonline -= 128;
	else
	{
		ANodeStr[pos] = '7';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( receivedCANData(&AnalogNode18) ) nodesonline -= 256;
	else
	{
		ANodeStr[pos] = '8';
		ANodeStr[pos+1] = '\0';
		pos++;
	}

	return nodesonline;
}


char * getAnalogNodesCriticalStr( void )
{
	if ( ANodeCritStr[0] == 0) return NULL;
	return ANodeCritStr;
}

char * getAnalogNodesStr( void )
{
	if ( ANodeStr[0] == 0) return NULL;
	return ANodeStr;
}


int receiveAnalogNodesCritical( void ) // 1 = APPS1 + regen. 11 = APPS2 + brakes  Only lack of these should prevent startup.
{
	uint16_t nodesonline = 0b11;

	ANodeCritStr[0] = 0;

	uint8_t pos = 0;

	if ( receivedCANData(&AnalogNode1) ) nodesonline -= 1;
	else
	{
		ANodeCritStr[pos] = '1';
		ANodeCritStr[pos+1] = '\0';
		pos++;
	}
	if ( receivedCANData(&AnalogNode11) ) nodesonline -= 2;
	{
		ANodeCritStr[pos] = 'A';
		ANodeCritStr[pos+1] = '\0';
		pos++;
	}

	return nodesonline;
}


void resetAnalogNodes( void )
{
    ADCState.BrakeF = 0;
    ADCState.BrakeR = 0;
    ADCState.Torque_Req_R_Percent = 0;
    ADCState.Torque_Req_L_Percent = 0;
    ADCState.Regen_Percent = 0;

	CarState.brake_balance = 0;

}

int initAnalogNodes( void )
{

	RegisterResetCommand(resetAnalogNodes);

	resetAnalogNodes();

	RegisterCan2Message(&AnalogNode1);
	RegisterCan1Message(&AnalogNode9);
	RegisterCan1Message(&AnalogNode10);
	RegisterCan1Message(&AnalogNode11);
	RegisterCan1Message(&AnalogNode12);
	RegisterCan1Message(&AnalogNode13);
	RegisterCan1Message(&AnalogNode14);
	RegisterCan1Message(&AnalogNode15);
	RegisterCan1Message(&AnalogNode16);
	RegisterCan1Message(&AnalogNode17);
	RegisterCan1Message(&AnalogNode18);

	return 0;
}

