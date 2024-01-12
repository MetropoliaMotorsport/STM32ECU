/*
 * preoperation.h
 *
 *  Created on: 14 Mar 2019
 *      Author: Visa
 */

#ifndef PREOPERATION_H_
#define PREOPERATION_H_

int PreOperationState( uint32_t OperationLoops );

void setTestMotors( bool state );
bool getTestMotors( void );

#endif /* PREOPERATION_H_ */
