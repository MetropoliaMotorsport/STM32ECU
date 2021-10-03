/*
 * torquecontrol.h
 *
 *  Created on: 01 May 2019
 *      Author: Visa
 */

#ifndef TORQUECONTROL_H_
#define TORQUECONTROL_H_

typedef struct vectoradjusttype {
	float FL;
	float FR;
	float RL;
	float RR;
} vectoradjust;

typedef struct speedadjusttype {
	uint16_t FL;
	uint16_t FR;
	uint16_t RL;
	uint16_t RR;
} speedadjust;

int initVectoring( void );
void doVectoring(float Torque_Req, vectoradjust * adj, speedadjust * spd );

float PedalTorqueRequest( void );

#endif /* TORQUECONTROL_H_ */

