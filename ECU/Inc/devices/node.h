/*
 * powernode.h
 *
 *  Created on: 16 Jun 2020
 *      Author: Visa
 */

#ifndef NODE_H_
#define NODE_H_

#include "ecumain.h"


extern CanData NodeErr; // [BOTS, inertia switch, BSPD.], Telemetry, front power
extern CanData NodeAck;

bool processNodeErrData(uint8_t CANRxData[8], uint32_t DataLength );
bool processNodeAckData(uint8_t CANRxData[8], uint32_t DataLength );

#endif /* NODE_H_ */

