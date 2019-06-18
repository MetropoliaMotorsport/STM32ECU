/*
 * sicksensor.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef SICKENCODER_H_
#define SICKENCODER_H_

int sickState( uint8_t canid );
uint8_t processSickNMT(uint8_t CANRxData[8], uint32_t DataLength, uint16_t encoder );
uint8_t processSickEncoder(uint8_t CANRxData[8], uint32_t DataLength, uint16_t encoder );
int receiveSick( uint8_t canid );
int requestSick( int nodeid );

#endif /* SICKENCODER_H_ */

