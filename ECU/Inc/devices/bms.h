/*
 * bms.h
 *
 *  Created on: 01 May 2019
 *      Author: Visa
 */

#ifndef BMS_H_
#define BMS_H_


extern CANData BMSSOC;

void sendBMS( void );

int receiveBMS( void );

void resetBMS( void );

int initBMS( void );

#endif /* BMS_H_ */

