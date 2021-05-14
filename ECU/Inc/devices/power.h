/*
 * power.h
 *
 *  Created on: Jul 17, 2020
 *      Author: Visa
 */

#ifndef DEVICES_POWER_H_
#define DEVICES_POWER_H_

#include "ecumain.h"

typedef enum DevicePowertype {
	None, // ensure 0 is not an actual device.
	Buzzer,
	Telemetry,
	Back1,
	Back2,
	Back3,
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

typedef enum DevicePowerStatetype {
	DirectPowerCmd,
	FanPowerCmd,
	StartupPower,
	IdlePower,
	TSEnabledPower,
	PowerError,
	PowerErrorReset
} DevicePowerState;

typedef struct Power_msg {
	DevicePowerState cmd;
	union {
	DevicePower power;
	uint8_t PWMLeft;
	};
	union {
	bool    enabled;
	uint8_t PWMRight;
	};
} Power_msg;

typedef struct Power_Error_msg {
	uint8_t 	nodeid;
	uint32_t	error;
} Power_Error_msg;

int setRunningPower( bool HV, bool buzzer );
int errorPower( void );
bool CheckShutdown( void );
char * ShutDownOpenStr( void );

bool PowerLogError( uint8_t nodeid, uint32_t errorcode);

void ShutdownCircuitSet( bool state );
int ShutdownCircuitCurrent( void );
int ShutdownCircuitState( void );

bool setDevicePower( DevicePower device, bool enabled );
bool resetDevicePower( DevicePower device );

char * getDevicePowerNameLong( DevicePower device );

char * getPNodeWait( void );

int initPower( void );

void FanPWMControl( uint8_t leftduty, uint8_t rightduty );

extern TaskHandle_t PowerTaskHandle;

#endif /* DEVICES_POWER_H_ */
