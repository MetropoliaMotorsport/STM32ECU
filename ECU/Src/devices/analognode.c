/*
 * analognode.c
 *
 *  Created on: 29 Jun 2020
 *      Author: Visa
 */

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "ecumain.h"

#include "freertos.h"
#include "task.h"
#include "queue.h"

#include "operationalprocess.h"
#include "adcecu.h"
#include "debug.h"
#include "errors.h"

#include "analognode.h"

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

CANData  AnalogNode1 =  { &DeviceState.AnalogNode1, AnalogNode1_ID, 6, processANode1Data, ANodeCritTimeout, NODECRITICALTIMEOUT };
CANData  AnalogNode9 =  { &DeviceState.AnalogNode9, AnalogNode9_ID, 4, processANode9Data, NULL, NODETIMEOUT };
CANData  AnalogNode10 = { &DeviceState.AnalogNode10, AnalogNode10_ID, 6, processANode10Data, NULL, NODETIMEOUT };
CANData  AnalogNode11=  { &DeviceState.AnalogNode11, AnalogNode11_ID, 8, processANode11Data, ANodeCritTimeout, NODECRITICALTIMEOUT };
CANData  AnalogNode12 = { &DeviceState.AnalogNode12, AnalogNode12_ID, 4, processANode12Data, NULL, NODETIMEOUT };
CANData  AnalogNode13 = { &DeviceState.AnalogNode13, AnalogNode13_ID, 4, processANode13Data, NULL, NODETIMEOUT };
CANData  AnalogNode14 = { &DeviceState.AnalogNode14, AnalogNode14_ID, 6, processANode14Data, NULL, NODETIMEOUT };
CANData  AnalogNode15 = { &DeviceState.AnalogNode15, AnalogNode15_ID, 3, processANode15Data, NULL, NODETIMEOUT };
CANData  AnalogNode16 = { &DeviceState.AnalogNode16, AnalogNode16_ID, 3, processANode16Data, NULL, NODETIMEOUT };
CANData  AnalogNode17 = { &DeviceState.AnalogNode17, AnalogNode17_ID, 3, processANode17Data, NULL, NODETIMEOUT };
CANData  AnalogNode18 = { &DeviceState.AnalogNode18, AnalogNode18_ID, 3, processANode18Data, NULL, NODETIMEOUT };


void ANodeCritTimeout( uint16_t id ) // ensure critical ADC values are set to safe defaults if not received.
{
	ADCState.Torque_Req_L_Percent=0;
	ADCState.Torque_Req_R_Percent=0;
	ADCState.Regen_Percent=0;
    ADCState.BrakeF = APPSBrakeHard;
    ADCState.BrakeR = APPSBrakeHard;
    SetCriticalError();
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
		xTaskNotify( ADCTaskHandle, ( 0x1 << ANode1Bit ), eSetBits);

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
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode9Bit ), eSetBits);

	return true;
}


bool processANode10Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode10Bit ), eSetBits);

	return true;
}


bool processANode11Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
 //   int Val1 = CANRxData[0]*256+CANRxData[1]; // dhab current?

	uint16_t BrakeTemp1 = CANRxData[0]+CANRxData[1]*256;

	uint16_t BrakeF =  CANRxData[2]+CANRxData[3]*256;
	uint16_t BrakeR =  CANRxData[4]+CANRxData[5]*256;

	uint16_t AccelR = CANRxData[6]+CANRxData[7]*256;

	// HPF 20 raw value R ~3300 for 0%

    // 19000 for 100%

	uint32_t dlc =  DataLength >> 16;

	if ( dlc == AnalogNode11.dlcsize
	//	&&	CANRxData[2] < 240 // TODO UNCOMMENT WHEN BRAKE SENSORS PLUGGED IN?
	//	&&  CANRxData[3] < 240
		&& ( AccelR < 65000 ) // make sure not pegged fully down.
		)
	{
		xTaskNotify( ADCTaskHandle, ( 0x1 << ANode11Bit ), eSetBits);

        ADCState.BrakeF = BrakeF;//CANRxData[2];
        ADCState.BrakeR = BrakeR;//CANRxData[3];
        ADCState.Torque_Req_R_Percent = getTorqueReqPercR(AccelR);

		return true;
	} else // bad data.
	{
		return false;
	}
	return true;
}

bool processANode12Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode12Bit ), eSetBits);

	return true;
}


bool processANode13Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode13Bit ), eSetBits);

	return true;
}

bool processANode14Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode14Bit ), eSetBits);

	return true;
}

bool processANode15Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode15Bit ), eSetBits);

	return true;
}

bool processANode16Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode16Bit ), eSetBits);

	return true;
}

bool processANode17Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode17Bit ), eSetBits);

	return true;
}

bool processANode18Data(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode18Bit ), eSetBits);

	return true;
}


char ANodeCritStr[10] = "";
char ANodeStr[15] = "";

void setAnalogNodesStr( uint32_t nodesonline ) // any of these missing should just be a warning or note.
{
	ANodeStr[0] = 0;

	uint8_t pos = 0;

	if ( !(nodesonline & ( 0x1 << ANode9Bit )) )
	{
		ANodeStr[pos] = '9';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( !(nodesonline & ( 0x1 << ANode10Bit )) )
	{
		ANodeStr[pos] = '0';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( !(nodesonline & ( 0x1 << ANode12Bit )) )
	{
		ANodeStr[pos] = '2';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( !(nodesonline & ( 0x1 << ANode13Bit )) )
	{
		ANodeStr[pos] = '3';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( !(nodesonline & ( 0x1 << ANode14Bit )) )
	{
		ANodeStr[pos] = '4';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( !(nodesonline & ( 0x1 << ANode15Bit )) )
	{
		ANodeStr[pos] = '5';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( !(nodesonline & ( 0x1 << ANode16Bit )) )
	{
		ANodeStr[pos] = '6';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( !(nodesonline & ( 0x1 << ANode17Bit )) )
	{
		ANodeStr[pos] = '7';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
	if ( !(nodesonline & ( 0x1 << ANode18Bit )) )
	{
		ANodeStr[pos] = '8';
		ANodeStr[pos+1] = '\0';
		pos++;
	}
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


void setAnalogNodesCriticalStr( uint32_t nodesonline ) // 1 = APPS1 + regen. 11 = APPS2 + brakes  Only lack of these should prevent startup.
{
	ANodeCritStr[0] = 0;

	uint8_t pos = 0;

	if ( !(nodesonline & ( 0x1 << ANode1Bit )) )
	{
		ANodeCritStr[pos] = '1';
		ANodeCritStr[pos+1] = '\0';
		pos++;
	}
	if ( !(nodesonline & ( 0x1 << ANode11Bit )) )
	{
		ANodeCritStr[pos] = 'A';
		ANodeCritStr[pos+1] = '\0';
		pos++;
	}
}


//warning and error codes
#define ERR_CAN_FIFO_FULL			1
#define ERR_SEND_FAILED				2
#define ERR_RECIEVED_INVALID_ID		3
#define ERR_RECIEVE_FAILED			4
#define ERR_INVALID_COMMAND			5
#define ERR_COMMAND_SHORT			6

#define ERR_WRONG_BYTES				33
#define ERR_INCORRECT_TF			34
#define ERR_INCORRECT_TF_VOLTAGE	35
#define ERR_INCORRECT_TF_NTC		36
#define ERR_INCORRECT_TF_I_TRANS	37

#define WARN_OVERCURR				49
#define ERR_OVERCURR_SHUTOFF		50

#define ERR_INVALID_CONFIG_ID		65


bool processANodeErr(uint8_t nodeid, uint32_t errorcode, CANData * datahandle )
{
	// acknowledge error received.

	char str[41] = "ANode ";

	snprintf(str, 41, "ANode %d ", nodeid);

	switch ( errorcode ) // switch off errors, need to be ACK'ed to stop sending error code.
	{
		case ERR_WRONG_BYTES : strncat(str, "Wrong Bytes", 40); break;
		case ERR_INCORRECT_TF : strncat(str, "Incorrect TF", 40); break;
		case ERR_INCORRECT_TF_VOLTAGE :  strncat(str, "Incorrect TFV", 40); break;
		case ERR_INCORRECT_TF_NTC :  strncat(str, "Incorrect TF NTC", 40); break;
		case ERR_INCORRECT_TF_I_TRANS :  strncat(str, "Incorrect TF_I", 40); break;

		case WARN_OVERCURR :  strncat(str,"warn OverCurr", 40);  break;
		case ERR_OVERCURR_SHUTOFF :  strncat(str,"Err OverCurrOff", 40); break;
	}

	DebugMsg(str);

	return true;
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

