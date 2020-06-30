/*
 * analoguenode.h
 *
 *  Created on: 29 Jun 2020
 *      Author: Visa
 */

#ifndef ANALOGNODE_H_
#define ANALOGNODE_H_

#include "ecumain.h"



extern CanData AnalogNode1;
extern CanData AnalogNode9;
extern CanData AnalogNode10;
extern CanData AnalogNode11;
extern CanData AnalogNode12;
extern CanData AnalogNode13;
extern CanData AnalogNode14;
extern CanData AnalogNode15;
extern CanData AnalogNode16;
extern CanData AnalogNode17;
extern CanData AnalogNode18;

int receiveAnalogNodes( void );

int receiveAnalogNodesCritical( void );

#endif /* ANALOGNODE_H_ */

