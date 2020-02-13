/*
 * inverter.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef INVERTER_H_
#define INVERTER_H_

uint8_t processINVError(uint8_t CANRxData[8], uint32_t DataLength, volatile InverterState *Inverter);
uint8_t processINVStatus(uint8_t CANRxData[8], uint32_t DataLength, volatile InverterState *Inverter); // try to reread if possible?
uint8_t processINVTorque(uint8_t CANRxData[8], uint32_t DataLength, volatile InverterState *Inverter);
uint8_t processINVSpeed(uint8_t CANRxData[8], uint32_t DataLength, volatile InverterState *Inverter); // try to reread if possible?
uint8_t processINVEmergency(uint8_t CANRxData[8], uint32_t DataLength, volatile InverterState *Inverter); // try to reread if possible?
uint8_t processINVNMT(uint8_t CANRxData[8], uint32_t DataLength, volatile InverterState *Inverter); // try to reread if possible?

uint8_t receiveINVNMT( volatile InverterState *Inverter);
uint8_t receiveINVStatus( volatile InverterState *Inverter );
uint8_t receiveINVSpeed( volatile InverterState *Inverter);
uint8_t receiveINVTorque( volatile InverterState *Inverter);


uint8_t requestINV( uint8_t Inverter );
uint8_t invError( uint8_t Inverter );

int8_t GetInverterState( uint16_t Status );
bool invertersStateCheck( int state );
int8_t InverterStateMachine( volatile InverterState *Inverter);



#endif /* INVERTER_H_ */

