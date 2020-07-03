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

bool processANode1Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processANode9Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processANode10Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processANode11Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processANode12Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processANode13Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processANode14Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processANode15Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processANode16Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processANode17Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processANode18Data(uint8_t CANRxData[8], uint32_t DataLength );


CanData  AnalogNode1 =  { &DeviceState.AnalogNode1, AnalogNode1_ID, 3, processANode1Data, NULL, NODETIMEOUT };
CanData  AnalogNode9 =  { &DeviceState.AnalogNode9, AnalogNode9_ID, 3, processANode9Data, NULL, NODETIMEOUT };
CanData  AnalogNode10 = { &DeviceState.AnalogNode10, AnalogNode10_ID, 3, processANode10Data, NULL, NODETIMEOUT };
CanData  AnalogNode11=  { &DeviceState.AnalogNode11, AnalogNode11_ID, 3, processANode11Data, NULL, NODETIMEOUT };
CanData  AnalogNode12 = { &DeviceState.AnalogNode12, AnalogNode12_ID, 3, processANode12Data, NULL, NODETIMEOUT };
CanData  AnalogNode13 = { &DeviceState.AnalogNode13, AnalogNode13_ID, 3, processANode13Data, NULL, NODETIMEOUT };
CanData  AnalogNode14 = { &DeviceState.AnalogNode14, AnalogNode14_ID, 3, processANode14Data, NULL, NODETIMEOUT };
CanData  AnalogNode15 = { &DeviceState.AnalogNode15, AnalogNode15_ID, 3, processANode15Data, NULL, NODETIMEOUT };
CanData  AnalogNode16 = { &DeviceState.AnalogNode16, AnalogNode16_ID, 3, processANode16Data, NULL, NODETIMEOUT };
CanData  AnalogNode17 = { &DeviceState.AnalogNode17, AnalogNode17_ID, 3, processANode17Data, NULL, NODETIMEOUT };
CanData  AnalogNode18 = { &DeviceState.AnalogNode18, AnalogNode18_ID, 3, processANode18Data, NULL, NODETIMEOUT };


bool processANode1Data(uint8_t CANRxData[8], uint32_t DataLength )
{
	return true;
}


bool processANode9Data(uint8_t CANRxData[8], uint32_t DataLength )
{
	return true;
}


bool processANode10Data(uint8_t CANRxData[8], uint32_t DataLength )
{
	return true;
}


bool processANode11Data(uint8_t CANRxData[8], uint32_t DataLength )
{
	return true;
}


bool processANode12Data(uint8_t CANRxData[8], uint32_t DataLength )
{
	return true;
}


bool processANode13Data(uint8_t CANRxData[8], uint32_t DataLength )
{
	return true;
}

bool processANode14Data(uint8_t CANRxData[8], uint32_t DataLength )
{
	return true;
}

bool processANode15Data(uint8_t CANRxData[8], uint32_t DataLength )
{
	return true;
}

bool processANode16Data(uint8_t CANRxData[8], uint32_t DataLength )
{
	return true;
}

bool processANode17Data(uint8_t CANRxData[8], uint32_t DataLength )
{
	return true;
}

bool processANode18Data(uint8_t CANRxData[8], uint32_t DataLength )
{
	return true;
}



int receiveAnalogNodes( void ) // any of these missing should just be a warning or note.
{
	uint16_t nodesonline = 0b111111111;

	if ( receivedCANData(&AnalogNode9) ) nodesonline -= 1;
	if ( receivedCANData(&AnalogNode10) ) nodesonline -= 2;
	if ( receivedCANData(&AnalogNode12) ) nodesonline -= 4;
	if ( receivedCANData(&AnalogNode13) ) nodesonline -= 8;
	if ( receivedCANData(&AnalogNode14) ) nodesonline -= 16;
	if ( receivedCANData(&AnalogNode15) ) nodesonline -= 32;
	if ( receivedCANData(&AnalogNode16) ) nodesonline -= 64;
	if ( receivedCANData(&AnalogNode17) ) nodesonline -= 128;
	if ( receivedCANData(&AnalogNode18) ) nodesonline -= 256;

	return nodesonline;
}


int receiveAnalogNodesCritical( void ) // 1 = APPS1 + regen. 11 = APPS2 + brakes  Only lack of these should prevent startup.
{
	uint16_t nodesonline = 0b11;

	if ( receivedCANData(&AnalogNode1) ) nodesonline -= 1;
	if ( receivedCANData(&AnalogNode11) ) nodesonline -= 2;

	return nodesonline;
}


