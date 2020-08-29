/*
 * analoguenode.h
 *
 *  Created on: 29 Jun 2020
 *      Author: Visa
 */

#ifndef ANALOGNODE_H_
#define ANALOGNODE_H_

#include "ecumain.h"

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

int receiveAnalogNodes( void );

int receiveAnalogNodesCritical( void );

char * getAnalogNodesCriticalStr( void );

char * getAnalogNodesStr( void );

int initAnalogNodes( void );

#endif /* ANALOGNODE_H_ */

