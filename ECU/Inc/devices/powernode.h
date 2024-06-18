/*
 * powernode.h
 *
 *  Created on: 16 Jun 2020
 *      Author: Visa
 */

#ifndef POWERNODE_H_
#define POWERNODE_H_

#include "ecumain.h"
#include "power.h"

#define PNode1Bit  0
#define PNode2Bit  1

enum PNoutput {OUT0_1, OUT1_1, OUT2_1, OUT3_1, OUT0_2, OUT1_2, OUT2_2, OUT3_2};

extern CANData PowerNodeErr;

extern CANData PowerNode1; // [BOTS, inertia switch, BSPD.], Telemetry, front power
extern CANData PowerNode2;

uint32_t getOldestPNodeData( void );

bool getNodeDevicePower(DevicePower device );
bool setNodeDevicePower( DevicePower device, bool state, bool reset );
int getPowerDeviceIndex( DevicePower device );

int initPowerNodes( void );

#endif /* POWERNODE_H_ */

