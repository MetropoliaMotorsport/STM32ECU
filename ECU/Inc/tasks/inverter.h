/*
 * inverter.h
 *
 *  Created on: 23 Mar 2021
 *      Author: Visa
 */

#ifndef INVERTER_H_
#define INVERTER_H_

#include "ecumain.h"
#include "torquecontrol.h"
#include "lenzeinverter.h"

typedef struct Inv_msg {
	DeviceStatus state;
	uint8_t inverter;
} Inv_msg;

typedef struct InvCfg_msg {
	uint8_t id;
	uint16_t idx;
	uint8_t sub;
	uint32_t data;
} InvCfg_msg;

extern QueueHandle_t InvQueue, InvCfgQueue;

#define ERRORTYPE1RESETTIME  (5500)
#define ERRORTYPE2RESETTIME  (150)

typedef struct { // new structure for inverter related data, so that it can be used as general pointer.
	uint8_t Motor; // index value, for when passed as an individual struct

	bool HighVoltageAvailable;

	DeviceStatus Device;

	uint8_t SetupState;
	uint8_t SetupTries;
	uint32_t SetupLastSeenTime;

	DeviceStatus InvState;
	bool FatalError;
	DeviceStatus InvRequested;
	uint16_t InvCommand;
	uint16_t InvReqCommand;

	bool AllowTorque;
	bool AllowRegen;
	bool AllowReset;

	int16_t MaxSpeed;
	int16_t Torque_Req;
	int16_t InvTorque;
	int16_t InvCurrent;
	uint32_t errortime;
	uint8_t errortype;
	uint32_t latchedStatus1;
	uint16_t latchedStatus2;

	int32_t Speed;
	uint8_t COBID;
	bool MCChannel;

	int16_t AmbTemp;
	int16_t InvVolt;
	int16_t InvTemp;
	int16_t MotorTemp;

} InverterState_t;  // define external into realmain?

extern DeviceStatus Inverter;

// public functions for control of inverter modules.

void InverterAllowTorque(uint8_t inv, bool allow );
void InverterAllowTorqueAll( bool allow );
InverterState_t * getInvState(uint8_t inv );
bool invertersStateCheck( DeviceStatus state );
void InverterSetTorque( vectoradjust *adj, int16_t MaxSpeed );
int InverterGetSpeed( void );
void InverterSetTorqueInd( uint8_t inv, int16_t req, int16_t speed );


bool registerInverterCAN( void );
DeviceStatus InternalInverterState ( uint16_t Status );

DeviceStatus GetInverterState( void );
uint8_t invRequestState( DeviceStatus state );
void resetInv( void );
int initInv( void );

int initNoInv( void );
int getInvOnlineCount( void );

char * getMotorsEnabledStr( void );

// internal functions.
uint32_t getInvExpected(  uint8_t inv );
uint8_t InvSend( volatile InverterState_t *Inverter, bool reset );
void InvResetError( volatile InverterState_t *Inverter );
bool InvStartupCfg( volatile InverterState_t *Inverter );
void InvReset( volatile InverterState_t *Inverter );

bool InvSendSDO( uint16_t id, uint16_t idx, uint8_t sub, uint32_t data);

extern TaskHandle_t InvTaskHandle;

#endif /* INVERTER_H_ */

