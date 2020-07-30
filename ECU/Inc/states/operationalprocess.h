/*
 * operationalprocess.h
 *
 *  Created on: 14 Mar 2019
 *      Author: drago
 */

#include "ecumain.h"

#ifndef OPERATIONALPROCESS_H_
#define OPERATIONALPROCESS_H_

extern int OperationalState;

typedef void (ResetCommand)( void );

int OperationalProcess( void );
uint16_t CheckErrors( void );
void ResetStateData( void );
int RegisterResetCommand( ResetCommand Handler );

#endif /* OPERATIONALPROCESS_H_ */
