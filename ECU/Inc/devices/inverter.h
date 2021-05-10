/*
 * inverter.h
 *
 *  Created on: 23 Mar 2021
 *      Author: Visa
 */

#ifndef INVERTER_H_
#define INVERTER_H_

#include "torquecontrol.h"

void InverterAllowTorque(bool allow);

typedef struct Inv_msg {
	DeviceStatus state;
} Inv_msg;

extern QueueHandle_t InvQueue;

typedef struct { // new structure for inverter related data, so that it can be used as general pointer.
	uint8_t InverterNum;
	bool HighVoltageAllowed;
	bool HighVoltageAvailable;
	uint16_t InvState;
	uint16_t InvBadStatus;
#ifdef SIEMENS
	uint16_t InvStateCheck;
	uint16_t InvStateCheck3;
#endif
	uint16_t InvCommand;

	int16_t Torque_Req;
	int16_t InvTorque;
	int16_t InvCurrent;

	int32_t Speed;
	uint16_t COBID;
	bool	MCChannel;
} InverterState;  // define external into realmain?

struct invState {
	bool AllowTorque;
	int16_t maxSpeed;
	InverterState Inverter[MOTORCOUNT];
};

// these need to be defined in the inverter specific c file
uint8_t receiveINVNMT( volatile InverterState *Inverter);
uint8_t receiveINVStatus( volatile InverterState *Inverter );
uint8_t receiveINVSpeed( volatile InverterState *Inverter);
uint8_t receiveINVTorque( volatile InverterState *Inverter);

uint8_t requestINV( uint8_t Inverter );
uint8_t invError( uint8_t Inverter );

bool invertersStateCheck( DeviceStatus state );

bool registerInverterCAN( void );
bool checkStatusCode( uint8_t status );
DeviceStatus InternalInverterState ( uint16_t Status );
char InvSend( volatile InverterState *Inverter, int32_t vel, int16_t torque );
void InvResetError( volatile InverterState *Inverter );
bool InvStartupCfg( volatile InverterState *Inverter );


void InverterSetTorque( vectoradjust *adj, int16_t MaxSpeed );
int InverterGetSpeed( void );

DeviceStatus GetInverterState( void );
uint8_t invRequestState( DeviceStatus state );
void resetInv( void );
int initInv( void );
void StopMotors( void );

#endif /* INVERTER_H_ */

