/*
 * debug.h
 *
 *  Created on: 30 Apr 2021
 *      Author: visa
 */

#ifndef DEVICES_DEBUG_H_
#define DEVICES_DEBUG_H_

#include "ecumain.h"

#define MAXDEBUGOUTPUT (80)

// print a message to debug output.
bool DebugMsg( const char * msg);
bool DebugPrintf( const char * format, ... );

int initDebug( void );

#endif /* DEVICES_DEBUG_H_ */
