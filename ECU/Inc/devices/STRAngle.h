/*
 * STRAngle.h
 *
 *  Created on: Jan 10, 2020
 *      Author: Markus Jahn
 */

#ifndef DEVICES_STRANGLE_H_
#define DEVICES_STRANGLE_H_

void processSTR(uint8_t CANRxData[8], uint32_t DataLength );

int receiveSTR( void );

//int requestSTR( void );

#endif /* DEVICES_STRANGLE_H_ */
