/*
 * powernode.h
 *
 *  Created on: 16 Jun 2020
 *      Author: Visa
 */

#ifndef POWERNODE_H_
#define POWERNODE_H_

#include "ecumain.h"

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
char * getPNodeStr( void );

uint8_t getDevicePowerListSize( void );

int sendPowerNodeReq( void );
bool getNodeDevicePower(DevicePower device );
bool getNodeDeviceExpectedPower(DevicePower device );
int setNodeDevicePower( DevicePower device, bool state );

bool powerErrorOccurred( DevicePower device );

int initPowerNodes( void );

#endif /* POWERNODE_H_ */

