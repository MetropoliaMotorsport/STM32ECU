/*
 * brake.h
 *
 *  Created on: 11 May 2021
 *      Author: Visa
 */

#ifndef BRAKE_H_
#define BRAKE_H_

bool getBrakeLight( void );
bool getBrakeLow( void );
uint8_t getBrakeHigh( void );
uint8_t getBrakeRTDM( void );

uint8_t getBrake( void );

int initBrake( void );

#endif /* BRAKE_H_ */

