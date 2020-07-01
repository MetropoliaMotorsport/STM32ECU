/*
 * pdm.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef PDM_H_
#define PDM_H_

extern CanData PDMCanData;

int receivePDM( void );
int errorPDM( void );
int sendPDM( int buzzer );
void initPDM( void );

#endif /* PDM_H_ */

