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

#define DEVICE_COUNT 9 

#define PNode1Bit  0
#define P
typedef struct devicepowerreqstruct {
	DevicePower device; //
	uint8_t nodeid;
	uint8_t output; // which bit of enable request is this device on
	uint8_t pwm;
	bool expectedstate; // what state are we requesting.
	bool waiting;
	bool actualstate;
	uint8_t dutycycle;
} devicepowerreq;

enum PNoutput {OUT0_1, OUT1_1, OUT2_1, OUT3_1, OUT0_2, OUT1_2, OUT2_2, OUT3_2};

extern CANData PowerNodeErr;

extern CANData PowerNode1; // [BOTS, inertia switch, BSPD.], Telemetry, front power
extern CANData PowerNode2;

extern CANData PowerNode1HeartBeat;
extern CANData PowerNode2HeartBeat;

extern devicepowerreq DevicePowerList[];
bool setNodeDevicePower( DevicePower device, bool state, bool reset );
bool setNodeDevicePWM(DevicePower device, uint8_t dutycycle);

void Set_LV_Devices_On();
void Set_LV_Devices_Off();

int initPowerNodes( void );

#endif /* POWERNODE_H_ */

