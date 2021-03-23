/*
 * inverter.h
 *
 *  Created on: 23 Mar 2021
 *      Author: Visa
 */

#ifndef INVERTER_H_
#define INVERTER_H_

typedef struct Inv_msg {
	DeviceStatus state;
} Inv_msg;

extern QueueHandle_t InvQueue;

#ifndef RTOS
DeviceStatus GetInverterState( uint16_t Status );
int8_t InverterStateMachine( volatile InverterState *Inverter);
#endif

// these need to be defined in the inverter specific c file
uint8_t receiveINVNMT( volatile InverterState *Inverter);
uint8_t receiveINVStatus( volatile InverterState *Inverter );
uint8_t receiveINVSpeed( volatile InverterState *Inverter);
uint8_t receiveINVTorque( volatile InverterState *Inverter);

uint8_t requestINV( uint8_t Inverter );
uint8_t invError( uint8_t Inverter );

bool invertersStateCheck( DeviceStatus state );

uint8_t invRequestState( DeviceStatus state );
void resetInv( void );
int initInv( void );
void StopMotors( void );

#endif /* INVERTER_H_ */

