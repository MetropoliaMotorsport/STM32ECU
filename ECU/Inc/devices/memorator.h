/*
 * memorator.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef MEMORATOR_H_
#define MEMORATOR_H_

extern time_t rtctime;

void processTime(uint8_t CANRxData[8], uint32_t DataLength );
int receivedTime( void );
char * getTimeStr( void );

#endif /* PDM_H_ */

