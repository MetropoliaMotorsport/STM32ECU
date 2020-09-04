/*
 * operationreadyness.h
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#ifndef RUNNINGPROCESS_H_
#define RUNNINGPROCESS_H_

int RunningProcess( uint32_t OperationLoops, uint32_t targettime );

uint16_t PedalTorqueRequest( void );

#ifdef TORQUEVECTOR
uint16_t TorqueVectorProcess( int torquerequest );
#endif

int ConvertNMToRequest( int NM );

void FanControl( void );

#endif /* RUNNINGPROCESS_H_ */
