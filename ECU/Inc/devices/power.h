/*
 * power.h
 *
 *  Created on: Jul 17, 2020
 *      Author: visa
 */

#ifndef DEVICES_POWER_H_
#define DEVICES_POWER_H_

int setHV( bool buzzer );
int errorPower( void );
bool CheckShutdown( void );
char * ShutDownOpenStr( void );

int initPower( void );

#endif /* DEVICES_POWER_H_ */
