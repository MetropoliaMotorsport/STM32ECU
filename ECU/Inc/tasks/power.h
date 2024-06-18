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
	RearFans,
	SideFans,
	LeftPump,
	RightPump,
	TSAL,
	Brake

} DevicePower;

typedef struct {
	bool BOTS;
	bool InertiaSwitch;
	bool BSPDAfter;
	bool BSPDBefore;
	bool CockpitButton;
	bool LeftButton;
	bool RightButton;

	bool BMS;
	uint8_t BMSReason;
	bool IMD;
	bool AIRm;
	bool AIRp;
	bool PRE;
	bool TS_OFF;

} ShutdownState;

extern ShutdownState Shutdown;

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

// log an error in power system.
bool PowerLogError( uint8_t nodeid, uint32_t errorcode);

// shutdown circuit commands
void ShutdownCircuitSet( bool state );
int ShutdownCircuitCurrent( void );
int ShutdownCircuitState( void );
bool CheckBMS( void );
bool CheckTSOff( void );
bool CheckIMD( void );
bool CheckShutdown( void );
void ClearHVLost( void );
bool CheckHVLost( void );


// power node control commands
DevicePower getDevicePowerFromList( uint32_t i );
bool getDevicePower( DevicePower device );
bool setDevicePower( DevicePower device, bool enabled );
bool resetDevicePower( DevicePower device );

// get strings for messages/displays.
char * getDevicePowerNameLong( DevicePower device );
char * getPNodeWait( void );

bool getPowerHVReady( void );
void FanPWMControl( uint8_t leftduty, uint8_t rightduty );

bool soundBuzzer( void );

int initPower( void );

extern TaskHandle_t PowerTaskHandle;

#endif /* DEVICES_POWER_H_ */
