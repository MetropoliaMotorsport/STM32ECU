/*
 * power.h
 *
 *  Created on: Jul 17, 2020
 *      Author: Visa
 */

#ifndef DEVICES_POWER_H_
#define DEVICES_POWER_H_

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



typedef struct Power_msg {
	DevicePower power;
	bool		state;
} Power_msg;

int setHV( bool HV, bool buzzer );
int errorPower( void );
bool CheckShutdown( void );
char * ShutDownOpenStr( void );

void ShutdownCircuitSet( bool state );
int ShutdownCircuitCurrent( void );
int ShutdownCircuitState( void );

int setDevicePower( DevicePower device, bool state );

char * getPNodeWait( void );

int initPower( void );

#endif /* DEVICES_POWER_H_ */
