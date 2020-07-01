/*
 * bms.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef BMS_H_
#define BMS_H_

extern CanData BMSVoltage;
extern CanData BMSOpMode;
extern CanData BMSError;

int receiveBMS( void );
void sendBMS( void );

#endif /* BMS_H_ */

