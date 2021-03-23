/*
 * bms.h
 *
 *  Created on: 01 May 2019
 *      Author: Visa
 */

#ifndef BMS_H_
#define BMS_H_

extern CANData BMSVoltage;
extern CANData BMSOpMode;
extern CANData BMSError;

int receiveBMS( void );
void sendBMS( void );

int initBMS( void );

#endif /* BMS_H_ */

