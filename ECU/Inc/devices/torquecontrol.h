/*
 * torquecontrol.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef TORQUECONTROL_H_
#define TORQUECONTROL_H_

typedef struct vectoradjusttype {
	int16_t FL;
	int16_t FR;
	int16_t RL;
	int16_t RR;
} vectoradjust;

int initVectoring( void );
void doVectoring(int16_t Torque_Req, vectoradjust * adj);

#endif /* TORQUECONTROL_H_ */

