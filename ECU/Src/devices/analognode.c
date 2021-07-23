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


bool processANode1Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode9Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode10Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode11Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode12Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode13Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode14Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode15Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode16Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode17Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode18Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

void ANodeCritTimeout( uint16_t id );

CANData  AnalogNode1 =  { &DeviceState.AnalogNode1, AnalogNode1_ID, 6, processANode1Data, ANodeCritTimeout, NODECRITICALTIMEOUT };
CANData  AnalogNode9 =  { &DeviceState.AnalogNode9, AnalogNode9_ID, 4, processANode9Data, NULL, NODETIMEOUT };
CANData  AnalogNode10 = { &DeviceState.AnalogNode10, AnalogNode10_ID, 6, processANode10Data, NULL, NODETIMEOUT };
CANData  AnalogNode11 = { &DeviceState.AnalogNode11, AnalogNode11_ID, 8, processANode11Data, ANodeCritTimeout, NODECRITICALTIMEOUT };
CANData  AnalogNode12 = { &DeviceState.AnalogNode12, AnalogNode12_ID, 4, processANode12Data, NULL, NODETIMEOUT };
CANData  AnalogNode13 = { &DeviceState.AnalogNode13, AnalogNode13_ID, 4, processANode13Data, NULL, NODETIMEOUT };
CANData  AnalogNode14 = { &DeviceState.AnalogNode14, AnalogNode14_ID, 6, processANode14Data, NULL, NODETIMEOUT };
CANData  AnalogNode15 = { &DeviceState.AnalogNode15, AnalogNode15_ID, 3, processANode15Data, NULL, NODETIMEOUT };
CANData  AnalogNode16 = { &DeviceState.AnalogNode16, AnalogNode16_ID, 3, processANode16Data, NULL, NODETIMEOUT };
CANData  AnalogNode17 = { &DeviceState.AnalogNode17, AnalogNode17_ID, 3, processANode17Data, NULL, NODETIMEOUT };
CANData  AnalogNode18 = { &DeviceState.AnalogNode18, AnalogNode18_ID, 3, processANode18Data, NULL, NODETIMEOUT };


void ANodeCritTimeout( uint16_t id ) // ensure critical ADC values are set to safe defaults if not received.
{
	DebugMsg("ANode timeout.");
	ADCState.Torque_Req_L_Percent=0;
	ADCState.Torque_Req_R_Percent=0;
	ADCState.Regen_Percent=0;
	ADCState.APPSL=0;
	ADCState.APPSR=0;
	ADCState.Regen=0;
    ADCState.BrakeF = APPSBrakeHard;
    ADCState.BrakeR = APPSBrakeHard;
    SetCriticalError();
}

bool processANode1Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 1 First msg.");
		first = true;
	}
	int AccelL = getBEint16(&CANRxData[2]);
	int Regen = getBEint16(&CANRxData[4]);

	if ( DataLength >> 16 == AnalogNode1.dlcsize
		&& ( AccelL < 4096 )
		&& ( Regen < 4096 )
		)
	{
		xTaskNotify( ADCTaskHandle, ( 0x1 << ANode1Bit ), eSetBits);

        ADCState.Torque_Req_L_Percent = getTorqueReqPercL(AccelL*16);
        ADCState.Regen_Percent = getBrakeTravelPerc(Regen*16);
		ADCState.APPSL = AccelL;
		ADCState.Regen = Regen;

		return true;
	} else // bad data.
	{
		xTaskNotify( ADCTaskHandle, ( ( 0x1 << 16 ) << ANode1Bit ), eSetBits); // mark some bad data received.
		return false;
	}
	return true;
}


bool processANode9Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 9 First msg.");
		first = true;
	}
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode9Bit ), eSetBits);

	ADCState.BrakeTemp1 = getBEint16(&CANRxData[0]);
	ADCState.OilTemp1 = CANRxData[2];
	ADCState.WaterTemp1 = CANRxData[3];

	return true;
}


bool processANode10Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 10 First msg.");
		first = true;
	}
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode10Bit ), eSetBits);

	ADCState.Susp1 = getBEint16(&CANRxData[0]);
	ADCState.susp2 = getBEint16(&CANRxData[4]);

	ADCState.OilTemp2 = CANRxData[1];
	ADCState.WaterTemp2 = CANRxData[2];

	return true;
}


bool processANode11Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 11 First msg.");
		first = true;
	}
	ADCState.BrakeTemp2 = getBEint16(&CANRxData[0]);

	uint16_t BrakeF = getBEint16(&CANRxData[2]);
	uint16_t BrakeR = getBEint16(&CANRxData[6]);

	uint16_t AccelR = getBEint16(&CANRxData[4]);

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

        ADCState.BrakeF = BrakeF;
        ADCState.BrakeR = BrakeR;
        ADCState.Torque_Req_R_Percent = getTorqueReqPercR(AccelR);
		ADCState.APPSR = AccelR;

		return true;
	} else // bad data.
	{
		xTaskNotify( ADCTaskHandle, ( ( 0x1 << 16 ) << ANode11Bit ), eSetBits); // mark some bad data received.
		return false;
	}
	return true;
}

// rest of sensors are currently logging data not currently used by ECU ( suspension position sensors, wheel temps, etc. )
// only define so that driver can be informed if they are not online at startup.
bool processANode12Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 12 First msg.");
		first = true;
	}
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode12Bit ), eSetBits);

	ADCState.WaterTemp3 = CANRxData[0];
	ADCState.WaterTemp4 = CANRxData[1];
	ADCState.WaterTemp5 = CANRxData[2];
	ADCState.WaterTemp6 = CANRxData[3];

	return true;
}


bool processANode13Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 13 First msg.");
		first = true;
	}
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode13Bit ), eSetBits);

	ADCState.Susp3 = getBEint16(&CANRxData[0]);
	ADCState.susp4 = getBEint16(&CANRxData[2]);

	return true;
}

bool processANode14Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 14 First msg.");
		first = true;
	}
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode14Bit ), eSetBits);

	ADCState.BrakeTemp3 = getBEint16(&CANRxData[0]);
	ADCState.BrakeTemp4 = getBEint16(&CANRxData[2]);
	ADCState.OilTemp3 = CANRxData[3];
	ADCState.OilTemp4 = CANRxData[4];

	return true;
}

bool processANode15Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 15 First msg.");
		first = true;
	}
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode15Bit ), eSetBits);

	ADCState.TireTemp1 = CANRxData[0];
	ADCState.TireTemp2 = CANRxData[1];
	ADCState.TireTemp3 = CANRxData[2];

	return true;
}

bool processANode16Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 16 First msg.");
		first = true;
	}
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode16Bit ), eSetBits);

	ADCState.TireTemp4 = CANRxData[0];
	ADCState.TireTemp5 = CANRxData[1];
	ADCState.TireTemp6 = CANRxData[2];

	return true;
}

bool processANode17Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 17 First msg.");
		first = true;
	}
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode17Bit ), eSetBits);

	ADCState.TireTemp7 = CANRxData[0];
	ADCState.TireTemp8 = CANRxData[1];
	ADCState.TireTemp9 = CANRxData[2];

	return true;
}

bool processANode18Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 18 First msg.");
		first = true;
	}
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode18Bit ), eSetBits);

	ADCState.TireTemp10 = CANRxData[0];
	ADCState.TireTemp11 = CANRxData[1];
	ADCState.TireTemp12 = CANRxData[2];

	return true;
}


char ANodeCritStr[10] = "";
char ANodeStr[15] = "";

void setAnalogNodesStr( const uint32_t nodesonline ) // any of these missing should just be a warning or note.
{
	ANodeStr[0] = 0;

	uint8_t pos = 0;


	if ( ANodeAllBit & ( 0x1 << ANode1Bit ) )
	if ( !(nodesonline & ( 0x1 << ANode1Bit )) )
	{
		ANodeStr[pos] = 'I';
		ANodeStr[pos+1] = '\0';
		pos++;
	}

	if ( ANodeAllBit & ( 0x1 << ANode9Bit ) )
	if ( !(nodesonline & ( 0x1 << ANode9Bit )) )
	{
		ANodeStr[pos] = '9';
		ANodeStr[pos+1] = '\0';
		pos++;
	}

	if ( ANodeAllBit & ( 0x1 << ANode10Bit ) )
	if ( !(nodesonline & ( 0x1 << ANode10Bit )) )
	{
		ANodeStr[pos] = '0';
		ANodeStr[pos+1] = '\0';
		pos++;
	}

	if ( ANodeAllBit & ( 0x1 << ANode11Bit ) )
	if ( !(nodesonline & ( 0x1 << ANode11Bit )) )
	{
		ANodeStr[pos] = '1';
		ANodeStr[pos+1] = '\0';
		pos++;
	}

	if ( ANodeAllBit & ( 0x1 << ANode12Bit ) )
	if ( !(nodesonline & ( 0x1 << ANode12Bit )) )
	{
		ANodeStr[pos] = '2';
		ANodeStr[pos+1] = '\0';
		pos++;
	}

	if ( ANodeAllBit & ( 0x1 << ANode13Bit ) )
	if ( !(nodesonline & ( 0x1 << ANode13Bit )) )
	{
		ANodeStr[pos] = '3';
		ANodeStr[pos+1] = '\0';
		pos++;
	}

	if ( ANodeAllBit & ( 0x1 << ANode14Bit ) )
	if ( !(nodesonline & ( 0x1 << ANode14Bit )) )
	{
		ANodeStr[pos] = '4';
		ANodeStr[pos+1] = '\0';
		pos++;
	}

	if ( ANodeAllBit & ( 0x1 << ANode15Bit ) )
	if ( !(nodesonline & ( 0x1 << ANode15Bit )) )
	{
		ANodeStr[pos] = '5';
		ANodeStr[pos+1] = '\0';
		pos++;
	}

	if ( ANodeAllBit & ( 0x1 << ANode16Bit ) )
	if ( !(nodesonline & ( 0x1 << ANode16Bit )) )
	{
		ANodeStr[pos] = '6';
		ANodeStr[pos+1] = '\0';
		pos++;
	}

	if ( ANodeAllBit & ( 0x1 << ANode17Bit ) )
	if ( !(nodesonline & ( 0x1 << ANode17Bit )) )
	{
		ANodeStr[pos] = '7';
		ANodeStr[pos+1] = '\0';
		pos++;
	}

	if ( ANodeAllBit & ( 0x1 << ANode18Bit ) )
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


void setAnalogNodesCriticalStr( const uint32_t nodesonline ) // 1 = APPS1 + regen. 11 = APPS2 + brakes  Only lack of these should prevent startup.
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


bool processANodeErr( const uint8_t nodeid, const uint32_t errorcode, const CANData * datahandle )
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

