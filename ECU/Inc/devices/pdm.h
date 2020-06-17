/*
 * pdm.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef PDM_H_
#define PDM_H_

void processPDM(uint8_t CANRxData[8], uint32_t DataLength );
void processPDMVolts(uint8_t CANRxData[8], uint32_t DataLength );
void PDMTimeout( void );
int receivePDM( void );
int errorPDM( void );
int sendPDM( int buzzer );
void initPDM( void );

#endif /* PDM_H_ */

