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

#define PNode33Bit  0
#define PNode34Bit  1
#define PNode35Bit  2
#define PNode36Bit  3
#define PNode37Bit  4
#define PNodeAllBit ( (0x1 << PNode33Bit) \
					+ (0x1 << PNode34Bit) \
					+ (0x1 << PNode35Bit) \
					+ (0x1 << PNode36Bit) \
					+ (0x1 << PNode37Bit) \
					)

extern CANData PowerNodeErr;
extern CANData PowerNodeAck;

extern CANData PowerNode33; // [BOTS, inertia switch, BSPD.], Telemetry, front power
extern CANData PowerNode34;
extern CANData PowerNode35;
extern CANData PowerNode36;
extern CANData PowerNode37;

bool processPNodeErr(uint8_t nodeid, uint32_t errorcode, CANData * datahandle );
bool processPNodeAckData(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );

void setPowerNodeStr( uint32_t nodesonline );
char * getPNodeStr( void );

uint8_t getDevicePowerListSize( void );

bool sendFanPWM( uint8_t leftduty, uint8_t rightduty );
int sendPowerNodeReq( void );
bool getNodeDevicePower(DevicePower device );
bool getNodeDeviceExpectedPower(DevicePower device );
bool setNodeDevicePower( DevicePower device, bool state, bool reset );
int getPowerDeviceIndex( DevicePower device );

void setAllPowerActualOff( void );

uint32_t powerErrorOccurred( DevicePower device );

int initPowerNodes( void );

#endif /* POWERNODE_H_ */

