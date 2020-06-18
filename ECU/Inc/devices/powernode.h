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

/*
#define DeviceBuzzer 	1
#define DeviceTelemetry	2
#define DeviceFront1	3


#define DeviceInverters	4
#define DeviceECU		5
#define DeviceFront2	6

#define DeviceLeftFans	7
#define DeviceRightFans	8
#define DeviceLeftPump 	9
#define DeviceRightPump 10

#define DeviceIVT		11
#define DeviceBuzzer	12

#define DeviceCurrentMeasurement	13
#define DeviceTSAL		14
*/

extern CanData PowerNodeErr;
extern CanData PowerNodeAck;

extern CanData PowerNode33; // [BOTS, inertia switch, BSPD.], Telemetry, front power
extern CanData PowerNode34;
extern CanData PowerNode35;
extern CanData PowerNode36;
extern CanData PowerNode37;

int receivePowerNodes( void );

int setDevicePower( DeviceId device, bool state );
int sendPowerNodeReq( void );

#endif /* POWERNODE_H_ */

