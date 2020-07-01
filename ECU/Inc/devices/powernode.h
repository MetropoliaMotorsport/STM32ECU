/*
 * powernode.h
 *
 *  Created on: 16 Jun 2020
 *      Author: Visa
 */

#ifndef POWERNODE_H_
#define POWERNODE_H_

#include "ecumain.h"


typedef enum DeviceIDtype {
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
	TSAL

} DeviceId;

extern CanData PowerNodeErr;
extern CanData PowerNodeAck;

extern CanData PowerNode33; // [BOTS, inertia switch, BSPD.], Telemetry, front power
extern CanData PowerNode34;
extern CanData PowerNode35;
extern CanData PowerNode36;
extern CanData PowerNode37;

bool processPNodeErr(uint8_t nodeid, uint32_t errorcode );
bool processPNodeAckData(uint8_t CANRxData[8], uint32_t DataLength );

int receivePowerNodes( void );

int setDevicePower( DeviceId device, bool state );
int sendPowerNodeReq( void );

bool powerErrorOccurred( DeviceId device );

#endif /* POWERNODE_H_ */

