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

void InverterAllowTorque(uint8_t inv, bool allow );
void InverterAllowTorqueAll( bool allow );

typedef struct Inv_msg {
	DeviceStatus state;
	uint8_t inverter;
} Inv_msg;

extern QueueHandle_t InvQueue;

#define ERRORTYPE1RESET  (5500)
#define ERRORTYPE2RESET  (150)

typedef struct { // new structure for inverter related data, so that it can be used as general pointer.
	uint8_t InverterNum;
	bool HighVoltageAllowed;
	bool HighVoltageAvailable;
	DeviceStatus InvStateAct;
	DeviceStatus InvRequested;
	uint16_t InvCommand;

	bool AllowTorque;
	int16_t MaxSpeed;
	int16_t Torque_Req;
	int16_t InvTorque;
	int16_t InvCurrent;
	uint32_t errortime;
	uint8_t errortype;
	uint32_t latchedStatus1;
	uint16_t latchedStatus2;

	int32_t Speed;
	uint16_t COBID;
	bool	MCChannel;
} InverterState_t;  // define external into realmain?

extern DeviceStatus Inverter;

// these need to be defined in the inverter specific c file
uint8_t receiveINVNMT( volatile InverterState_t *Inverter);
uint8_t receiveINVStatus( volatile InverterState_t *Inverter );
uint8_t receiveINVSpeed( volatile InverterState_t *Inverter);
uint8_t receiveINVTorque( volatile InverterState_t *Inverter);

void RearSpeedCalculation( long leftdata, long rightdata );

//uint8_t requestINV( uint8_t Inverter );
uint8_t invError( uint8_t Inverter );

InverterState_t getInvState(uint8_t inv );
bool invertersStateCheck( DeviceStatus state );

bool registerInverterCAN( void );
bool checkStatusCode( uint8_t status );
DeviceStatus InternalInverterState ( uint16_t Status );
char InvSend( volatile InverterState_t *Inverter, int32_t vel, int16_t torque );
void InvResetError( volatile InverterState_t *Inverter );
bool InvStartupCfg( volatile InverterState_t *Inverter );

void InverterSetTorque( vectoradjust *adj, int16_t MaxSpeed );
int InverterGetSpeed( void );
void InverterSetTorqueInd( uint8_t inv, int16_t req, int16_t speed );

DeviceStatus GetInverterState( void );
uint8_t invRequestState( DeviceStatus state );
void resetInv( void );
int initInv( void );
void StopMotors( void );

extern TaskHandle_t InvTaskHandle;

#endif /* INVERTER_H_ */

