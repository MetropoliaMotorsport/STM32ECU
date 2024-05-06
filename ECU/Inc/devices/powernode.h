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



#define PNodeAllBit ( (0x1 << PNode1Bit) \
					+ (0x1 << PNode2Bit) \
					) // + (0x1 << PNode36Bit) // 36 is currently not powered, don't check for.

#define PNODECRITICALBITS	( (0x1 << PNode1Bit) )  // brake and buzzer are critical



#define POWERNODE_FAN_BIT  (PNode2Bit)

extern CANData PowerNodeErr;
extern CANData PowerNodeAck;

extern CANData PowerNode1; // [BOTS, inertia switch, BSPD.], Telemetry, front power
extern CANData PowerNode2;


bool processPNodeErr(const uint8_t nodeid, const uint32_t errorcode, const CANData * datahandle );
bool processPNodeAckData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

uint32_t getOldestPNodeData( void );

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
char * PNodeGetErrStr( uint32_t error );
bool PNodeGetErrType( uint32_t error );

int initPowerNodes( void );

#endif /* POWERNODE_H_ */

