/*
 * powernode.h
 *
 *  Created on: 16 Jun 2020
 *      Author: Visa
 */

#ifndef POWERNODE_H_
#define POWERNODE_H_

#include "ecumain.h"


typedef enum DevicePowertype {
	None, // ensure 0 is not an actual device.
	Buzzer,
	Telemetry,
	Front1,
	Inverters,
	ECU,
	Front2,
	LeftFans,
	RightFans,
	LeftPump,
	RightPump,
	IVT,
	Current,
	TSAL,
	Brake,
	Accu,
	AccuFan

} DevicePower;

extern CANData PowerNodeErr;
extern CANData PowerNodeAck;

extern CANData PowerNode33; // [BOTS, inertia switch, BSPD.], Telemetry, front power
extern CANData PowerNode34;
extern CANData PowerNode35;
extern CANData PowerNode36;
extern CANData PowerNode37;

bool processPNodeErr(uint8_t nodeid, uint32_t errorcode, CANData * datahandle );
bool processPNodeAckData(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );

int receivePowerNodes( void );

int setDevicePower( DevicePower device, bool state );
int sendPowerNodeReq( void );

bool powerErrorOccurred( DevicePower device );

int initPowerNodes( void );

#endif /* POWERNODE_H_ */

