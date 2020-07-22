/*
 * inverter.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef INVERTER_H_
#define INVERTER_H_

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



#endif /* INVERTER_H_ */

