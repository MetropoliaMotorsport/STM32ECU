/*
 * analoguenode.h
 *
 *  Created on: 29 Jun 2020
 *      Author: Visa
 */

#ifndef ANALOGNODE_H_
#define ANALOGNODE_H_

#include "ecumain.h"
#include "canecu.h"


struct analognode
{
	uint16_t sensor1;
	uint16_t sensor2;
	uint16_t sensor3;
	uint16_t sensor4;
	uint16_t sensor5;
	uint16_t sensor6;
	uint16_t sensor7;
	uint16_t sensor8;
	uint16_t sensor9;
	uint16_t sensor10;
	uint16_t sensor11;
	uint16_t sensor12;
	uint16_t sensor13;
	uint16_t sensor14;
	uint16_t sensor15;
	uint16_t sensor16;
};



#define ANode1Bit  (0)
#define ANode2Bit  (1)


#define ANodeBADDataShift (16)


#define AnodeCriticalBit ( (0x1 << ANode1Bit ) + (0x1 << ANode11Bit) )


extern CANData AnalogNode1;
extern CANData AnalogNode2;

uint32_t getOldestANodeCriticalData( void );

bool processANodeErr( const uint8_t nodeid, const uint32_t errorcode, const CANData * datahandle );
bool processANodeAckData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

void setAnalogNodesStr( const uint32_t nodesonline );

void setAnalogNodesCriticalStr( const uint32_t nodesonline );

char * getAnalogNodesCriticalStr( void );

char * getAnalogNodesStr( void );

int initAnalogNodes( void );

#endif /* ANALOGNODE_H_ */

