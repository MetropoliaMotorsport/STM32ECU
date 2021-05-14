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

#define ANode1Bit  0
#define ANode9Bit  1
#define ANode10Bit 2
#define ANode11Bit 3
#define ANode12Bit 4
#define ANode13Bit 5
#define ANode14Bit 6
#define ANode15Bit 7
#define ANode16Bit 8
#define ANode17Bit 9
#define ANode18Bit 10

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

bool processANodeErr(uint8_t nodeid, uint32_t errorcode, CANData * datahandle );
bool processANodeAckData(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );

int receiveAnalogNodes( void );

int receiveAnalogNodesCritical( void );

char * getAnalogNodesCriticalStr( void );

char * getAnalogNodesStr( void );

int initAnalogNodes( void );

#endif /* ANALOGNODE_H_ */

