/*
 * shutdown.h
 *
 *  Created on: 15 Jul 2020
 *      Author: Visa
 */

#ifndef SHUTDOWN_H_
#define SHUTDOWN_H_

void ShutdownCircuitSet( bool state );
int ShutdownCircuitCurrent( void );
int ShutdownCircuitState( void );

#endif /* SHUTDOWN_H_ */

