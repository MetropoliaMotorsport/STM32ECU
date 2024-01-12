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

#define ANode1Bit  (0)
#define ANode9Bit  (1)
#define ANode10Bit (2)
#define ANode11Bit (3)
#define ANode12Bit (4)
#define ANode13Bit (5)
#define ANode14Bit (6)
#define ANode15Bit (7)
#define ANode16Bit (8)
#define ANode17Bit (9)
#define ANode18Bit (10)

#define ANodeBADDataShift (16)

#ifdef ALLANODES
#define ANodeAllBit ( (0x1 << ANode1Bit ) \
					+ (0x1 << ANode9Bit)  \
					+ (0x1 << ANode10Bit) \
					+ (0x1 << ANode11Bit) \
					+ (0x1 << ANode12Bit) \
					+ (0x1 << ANode13Bit) \
					+ (0x1 << ANode14Bit) \
					+ (0x1 << ANode15Bit) \
					+ (0x1 << ANode16Bit) \
					+ (0x1 << ANode17Bit) \
					+ (0x1 << ANode18Bit) \
					)
#else
#define ANodeAllBit ( (0x1 << ANode1Bit ) \
					+ (0x1 << ANode11Bit) \
					)
#endif

#define AnodeCriticalBit ( (0x1 << ANode1Bit ) + (0x1 << ANode11Bit) )


extern CANData AnalogNode1;
extern CANData AnalogNode9;
extern CANData AnalogNode10;
extern CANData AnalogNode11;
extern CANData AnalogNode12;
extern CANData AnalogNode13;
extern CANData AnalogNode14;
extern CANData AnalogNode15;
extern CANData AnalogNode16;
extern CANData AnalogNode17;
extern CANData AnalogNode18;

uint32_t getOldestANodeCriticalData( void );

bool processANodeErr( const uint8_t nodeid, const uint32_t errorcode, const CANData * datahandle );
bool processANodeAckData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

void setAnalogNodesStr( const uint32_t nodesonline );

void setAnalogNodesCriticalStr( const uint32_t nodesonline );

char * getAnalogNodesCriticalStr( void );

char * getAnalogNodesStr( void );

int initAnalogNodes( void );

#endif /* ANALOGNODE_H_ */

