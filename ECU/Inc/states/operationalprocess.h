/*
 * operationalprocess.h
 *
 *  Created on: 14 Mar 2019
 *      Author: Visa
 */

#include "ecumain.h"

#ifndef OPERATIONALPROCESS_H_
#define OPERATIONALPROCESS_H_

extern int OperationalState;

int OperationalProcess( void );
uint16_t CheckErrors( void );
void ResetStateData( void );

#endif /* OPERATIONALPROCESS_H_ */
