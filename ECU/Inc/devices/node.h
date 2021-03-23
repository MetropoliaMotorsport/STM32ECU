/*
 * node.h
 *
 *  Created on: 16 Jun 2020
 *      Author: Visa
 */

#ifndef NODE_H_
#define NODE_H_

#include "ecumain.h"


extern CANData NodeErr; // [BOTS, inertia switch, BSPD.], Telemetry, front power
extern CANData NodeAck;

int initNodes( void );

#endif /* NODE_H_ */

