/*
 * inverter.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef INVERTER_H_
#define INVERTER_H_

#define InverterRL_COBID			(0x7E) // 126 // swap
#define InverterRR_COBID			(0x7F) // 127 // swap

#ifdef HPF20
#define InverterFL_COBID			(0x7C) // 124 // swap
#define InverterFR_COBID			(0x7D) // 125 // swap
#endif

extern CANData InverterCANErr[];;
extern CANData InverterCANNMT[];
extern CANData InverterCANPDO1[];
extern CANData InverterCANPDO2[];
extern CANData InverterCANPDO3[];
extern CANData InverterCANPDO4[];

uint8_t receiveINVNMT( volatile InverterState *Inverter);
uint8_t receiveINVStatus( volatile InverterState *Inverter );
uint8_t receiveINVSpeed( volatile InverterState *Inverter);
uint8_t receiveINVTorque( volatile InverterState *Inverter);

uint8_t requestINV( uint8_t Inverter );
uint8_t invError( uint8_t Inverter );

int8_t GetInverterState( uint16_t Status );
bool invertersStateCheck( int state );
int8_t InverterStateMachine( volatile InverterState *Inverter);

void resetInv( void );
int initInv( void );

#endif /* INVERTER_H_ */

