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
#include "timerecu.h"

void ANodeCritTimeout( uint16_t id );

#ifdef HPF2023

bool processANode1Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode10Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode11Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode13Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
CANData  AnalogNode1 =  { &DeviceState.AnalogNode1, AnalogNode1_ID, 6, processANode1Data, ANodeCritTimeout, NODECRITICALTIMEOUT };
CANData  AnalogNode10 = { &DeviceState.AnalogNode10, AnalogNode10_ID, 6, processANode10Data, NULL, NODETIMEOUT };
CANData  AnalogNode11 = { &DeviceState.AnalogNode11, AnalogNode11_ID, 6, processANode11Data, ANodeCritTimeout, NODECRITICALTIMEOUT };
CANData  AnalogNode13 = { &DeviceState.AnalogNode13, AnalogNode13_ID, 4, processANode13Data, NULL, NODETIMEOUT };

#else
bool processANode9Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode12Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode14Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode15Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode16Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode17Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processANode18Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

CANData  AnalogNode9 =  { &DeviceState.AnalogNode9, AnalogNode9_ID, 4, processANode9Data, NULL, NODETIMEOUT };
CANData  AnalogNode12 = { &DeviceState.AnalogNode12, AnalogNode12_ID, 4, processANode12Data, NULL, NODETIMEOUT };
CANData  AnalogNode14 = { &DeviceState.AnalogNode14, AnalogNode14_ID, 6, processANode14Data, NULL, NODETIMEOUT };
CANData  AnalogNode15 = { &DeviceState.AnalogNode15, AnalogNode15_ID, 3, processANode15Data, NULL, NODETIMEOUT };
CANData  AnalogNode16 = { &DeviceState.AnalogNode16, AnalogNode16_ID, 3, processANode16Data, NULL, NODETIMEOUT };
CANData  AnalogNode17 = { &DeviceState.AnalogNode17, AnalogNode17_ID, 3, processANode17Data, NULL, NODETIMEOUT };
CANData  AnalogNode18 = { &DeviceState.AnalogNode18, AnalogNode18_ID, 3, processANode18Data, NULL, NODETIMEOUT };

#endif

void ANodeCritTimeout( uint16_t id ) // ensure critical ADC values are set to safe defaults if not received.
{
	DebugMsg("ANode timeout.");
	xSemaphoreTake(ADCUpdate, portMAX_DELAY);
	ADCStateNew.Torque_Req_L_Percent=0;
	ADCStateNew.Torque_Req_R_Percent=0;
	ADCStateNew.Regen_Percent=0;
	ADCStateNew.APPSL=0;
	ADCStateNew.APPSR=0;
	ADCStateNew.Regen=0;
	ADCStateNew.BrakeF = 0;//APPSBrakeHard;
	ADCStateNew.BrakeR = 0;//APPSBrakeHard;
	xSemaphoreGive(ADCUpdate);
    SetCriticalError( CRITERRANODE );
	DeviceState.timeout = true;
}

uint32_t getOldestANodeCriticalData( void )
{
	uint32_t time = gettimer();
	if ( AnalogNode1.time <  time ) time = AnalogNode1.time;
	if ( AnalogNode11.time <  time ) time = AnalogNode11.time;
	return time;
}

// 00000680         6  A6  2F  03  F9  00  97

// AccelL = 03  F9  --   return (data[0]<<8)+data[1]; 03 * 256 + f9  = 1017    or 63747
// regen = 00  97  --

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

#ifdef ADCDEBUGINFO
	static uint32_t count = 0;
	if ( ( count % 100 ) == 0 )
	{
		DebugPrintf("A1  Acl %lu BrT %lu", AccelL, Regen);
	}
	count++;
#endif
	if (
		AccelL < 4096
		 && Regen < 4096
	)
	{
		xSemaphoreTake(ADCUpdate, portMAX_DELAY);
#ifndef APPSFIXL

		int percentage = ((100000/(1831-423))*(AccelL-423))/100;
		if ( percentage > 100 )
			percentage = 0;
		if ( percentage < 0 )
			percentage = 0;
		ADCStateNew.Torque_Req_L_Percent = percentage;  //getTorqueReqPercL(AccelL);
		ADCStateNew.APPSL = AccelL;
#endif
#ifdef APPSFIXR
		ADCStateNew.Torque_Req_R_Percent = getTorqueReqPercL(AccelL);
		ADCStateNew.APPSR = AccelL;
#endif
		ADCStateNew.Regen_Percent = getBrakeTravelPerc(Regen);
		ADCStateNew.Regen = Regen;
		xSemaphoreGive(ADCUpdate);
		xTaskNotify( ADCTaskHandle, ( 0x1 << ANode1Bit ), eSetBits);
		return true;
	} else // bad data.
	{
		xTaskNotify( ADCTaskHandle, ( ( 0x1 << 16 ) << ANode1Bit ), eSetBits); // mark some bad data received.
		return false;
	}
	return true;
}

#ifndef HPF2023
bool processANode9Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 9 First msg.");
		first = true;
	}
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode9Bit ), eSetBits);

	ADCStateSensors.BrakeTemp1 = getBEint16(&CANRxData[0]);
	ADCStateSensors.OilTemp1 = CANRxData[2];
	ADCStateSensors.WaterTemp1 = CANRxData[3];

	return true;
}
#endif

bool processANode10Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 10 First msg.");
		first = true;
	}
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode10Bit ), eSetBits);

	ADCStateSensors.Susp1 = getBEint16(&CANRxData[0]);
	ADCStateSensors.susp2 = getBEint16(&CANRxData[4]);

	ADCStateSensors.OilTemp2 = CANRxData[1];
	ADCStateSensors.WaterTemp2 = CANRxData[2];

	return true;
}


//  1    00000694         8  FC  D6  FE  10  FE  11  00  5B   87958.622804 R

// brakef = FE  10
// braker = 00  5B
// accelr =  FE  11

//

bool processANode11Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 11 First msg.");
		first = true;
	}
#ifdef HPF2023
	uint16_t AccelR = getBEint16(&CANRxData[0]);
	int16_t BrakeF = (int16_t)getBEint16(&CANRxData[2]);
	int16_t BrakeR = (int16_t)getBEint16(&CANRxData[4]);
#else
	ADCStateSensors.BrakeTemp2 = getBEint16(&CANRxData[0]);

	uint16_t BrakeF = getBEint16(&CANRxData[2]);
	uint16_t BrakeR = getBEint16(&CANRxData[6]);

	uint16_t AccelR = getBEint16(&CANRxData[4]);
#endif

#ifdef ADCDEBUGINFO
	static uint32_t count = 0;
	if ( ( count % 100 ) == 0 )
	{
		DebugPrintf("A11 BrF %d BrR %d AcR %d", BrakeF, BrakeR, AccelR);
	}
	count++;
#endif

	if (
	 ( AccelR < 4096 ) // make sure not pegged fully down.
		)
	{
		xSemaphoreTake(ADCUpdate, portMAX_DELAY);
		if ( BrakeF > 0)
			ADCStateNew.BrakeF = BrakeF;
		else
			ADCStateNew.BrakeF = 0;

		if ( BrakeR > 0 )
        	ADCStateNew.BrakeR = BrakeR;
		else
			ADCStateNew.BrakeR = 0;
#ifndef APPSFIXR
        ADCStateNew.Torque_Req_R_Percent = getTorqueReqPercR(AccelR);
		ADCStateNew.APPSR = AccelR;
#endif
#ifdef APPSFIXL
		ADCStateNew.Torque_Req_L_Percent = getTorqueReqPercL(AccelR);
		ADCStateNew.APPSL = AccelR;
#endif

		xSemaphoreGive(ADCUpdate);
		xTaskNotify( ADCTaskHandle, ( 0x1 << ANode11Bit ), eSetBits);
		return true;
	} else // bad data.
	{
		xTaskNotify( ADCTaskHandle, ( ( 0x1 << 16 ) << ANode11Bit ), eSetBits); // mark some bad data received.
		return false;
	}
	return true;
}

#ifndef HPF2023
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
	ADCStateSensors.WaterTemp3 = CANRxData[0];
	ADCStateSensors.WaterTemp4 = CANRxData[1];
	ADCStateSensors.WaterTemp5 = CANRxData[2];
	ADCStateSensors.WaterTemp6 = CANRxData[3];
	return true;
}
#endif

bool processANode13Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 13 First msg.");
		first = true;
	}
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode13Bit ), eSetBits);

	ADCStateSensors.Susp3 = getBEint16(&CANRxData[0]);
	ADCStateSensors.susp4 = getBEint16(&CANRxData[2]);

	return true;
}

#ifndef HPF2023
bool processANode14Data(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("Analog node 14 First msg.");
		first = true;
	}
	xTaskNotify( ADCTaskHandle, ( 0x1 << ANode14Bit ), eSetBits);

	ADCStateSensors.BrakeTemp3 = getBEint16(&CANRxData[0]);
	ADCStateSensors.BrakeTemp4 = getBEint16(&CANRxData[2]);
	ADCStateSensors.OilTemp3 = CANRxData[3];
	ADCStateSensors.OilTemp4 = CANRxData[4];

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

	ADCStateSensors.TireTemp1 = CANRxData[0];
	ADCStateSensors.TireTemp2 = CANRxData[1];
	ADCStateSensors.TireTemp3 = CANRxData[2];

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

	ADCStateSensors.TireTemp4 = CANRxData[0];
	ADCStateSensors.TireTemp5 = CANRxData[1];
	ADCStateSensors.TireTemp6 = CANRxData[2];

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

	ADCStateSensors.TireTemp7 = CANRxData[0];
	ADCStateSensors.TireTemp8 = CANRxData[1];
	ADCStateSensors.TireTemp9 = CANRxData[2];

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

	ADCStateSensors.TireTemp10 = CANRxData[0];
	ADCStateSensors.TireTemp11 = CANRxData[1];
	ADCStateSensors.TireTemp12 = CANRxData[2];

	return true;
}
#endif

char ANodeCritStr[10] = "";
char ANodeStr[15] = "";

void setAnalogNodesStr( const uint32_t nodesonline ) // any of these missing should just be a warning or note.
{
	ANodeStr[0] = 0;

	uint8_t pos = 0;


	if ( ANodeAllBit & ( 0x1 << ANode1Bit ) )
	if ( !(nodesonline & ( 0x1 << ANode1Bit )) )
	{
		ANodeStr[pos] = 'i';
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
	if ( ANodeCritStr[0] == 0) return "";
	return ANodeCritStr;
}

char * getAnalogNodesStr( void )
{
	if ( ANodeStr[0] == 0) return "";
	return ANodeStr;
}


void setAnalogNodesCriticalStr( const uint32_t nodesonline ) // 1 = APPS1 + regen. 11 = APPS2 + brakes  Only lack of these should prevent startup.
{
	ANodeCritStr[0] = 0;

	uint8_t pos = 0;

	if ( !(nodesonline & ( 0x1 << ANode1Bit )) )
	{
		ANodeCritStr[pos] = 'I';
		ANodeCritStr[pos+1] = '\0';
		pos++;
	}
	if ( !(nodesonline & ( 0x1 << ANode11Bit )) )
	{
		ANodeCritStr[pos] = '1';
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
	assert_param(ADCUpdate);

	RegisterResetCommand(resetAnalogNodes);

	resetAnalogNodes();

	RegisterCan2Message(&AnalogNode1);
#ifndef HPF2023
	RegisterCan1Message(&AnalogNode9);
#endif
	RegisterCan1Message(&AnalogNode10);
	RegisterCan1Message(&AnalogNode11);

#ifndef HPF2023
	RegisterCan1Message(&AnalogNode12);
#endif
	RegisterCan1Message(&AnalogNode13);
#ifndef HPF2023
	RegisterCan1Message(&AnalogNode14);
	RegisterCan1Message(&AnalogNode15);
	RegisterCan1Message(&AnalogNode16);
	RegisterCan1Message(&AnalogNode17);
	RegisterCan1Message(&AnalogNode18);
#endif

	return 0;
}

