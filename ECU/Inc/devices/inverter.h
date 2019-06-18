/*
 * inverter.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef INVERTER_H_
#define INVERTER_H_

uint8_t processINVError(uint8_t CANRxData[8], uint32_t DataLength, uint8_t Inverter );
uint8_t processINVStatus(uint8_t CANRxData[8], uint32_t DataLength, uint8_t Inverter ); // try to reread if possible?
uint8_t processINVTorque(uint8_t CANRxData[8], uint32_t DataLength, uint8_t Inverter );
uint8_t processINVSpeed(uint8_t CANRxData[8], uint32_t DataLength, uint8_t Inverter ); // try to reread if possible?
uint8_t processINVEmergency(uint8_t CANRxData[8], uint32_t DataLength, uint8_t Inverter ); // try to reread if possible?
uint8_t processINVNMT(uint8_t CANRxData[8], uint32_t DataLength, uint8_t Inverter ); // try to reread if possible?


uint8_t receiveINVNMT( uint8_t Inverter );
uint8_t receiveINVStatus( uint8_t Inverter );
uint8_t receiveINVSpeed( uint8_t Inverter );
uint8_t receiveINVTorque( uint8_t Inverter );


uint8_t requestINV( uint8_t Inverter );
uint8_t invError( uint8_t Inverter );

int8_t GetInverterState( uint16_t Status );
int8_t InverterStateMachine( int8_t Inverter );



#endif /* INVERTER_H_ */

