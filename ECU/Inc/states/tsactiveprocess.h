/*
 * operationreadyness.h
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#ifndef TSACTIVEPROCESS_H_
#define TSACTIVEPROCESS_H_

int TSActiveProcess( uint32_t OperationLoops );
int TSActiveRequest( void );
int TSActiveINVRequest( volatile InverterState *Inverter );

#endif /* TSACTIVEPROCESS_H_ */
