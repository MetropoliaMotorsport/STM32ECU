/*
 * errors.h
 *
 *  Created on: 29 Apr 2021
 *      Author: visa
 */

#ifndef STATES_ERRORS_H_
#define STATES_ERRORS_H_

#include "ecumain.h"

int ResetErrors( void );
void LogError( char *message );

int OperationalErrorHandler( uint32_t OperationLoops );

int initERRORState( void );

#endif /* STATES_ERRORS_H_ */
