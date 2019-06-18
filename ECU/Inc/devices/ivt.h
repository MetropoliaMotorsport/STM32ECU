/*
 * ivt.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef IVT_H_
#define IVT_H_

uint8_t processIVT(uint8_t CANRxData[8], uint32_t DataLength, uint16_t field );

int receiveIVT( void );

int requestIVT( void );

#endif /* IVT_H_ */

