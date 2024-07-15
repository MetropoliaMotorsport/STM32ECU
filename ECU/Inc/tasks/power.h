/*
 * power.h
 *
 *  Created on: Jul 17, 2020
 *      Author: Visa
 */

#ifndef DEVICES_POWER_H_
#define DEVICES_POWER_H_

#include "ecumain.h"

#define TSACTIVEV (300)

typedef enum DevicePowertype {
	None, // ensure 0 is not an actual device.
	Buzzer,
	Inverters,
	Brake,
	SideFans,
	LeftPump,
	RightPump
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
typedef struct Power_Error_msg {
	uint8_t 	nodeid;
	uint32_t	error;
} Power_Error_msg;

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

// shutdown circuit commands
void ShutdownCircuitSet( bool state );
int ShutdownCircuitState( void );
bool CheckBMS( void );
bool CheckTSOff( void );
bool CheckIMD( void );

bool soundBuzzer( void );
void CheckDeviceState();
int initPower( void );

extern TaskHandle_t PowerTaskHandle;

#endif /* DEVICES_POWER_H_ */
