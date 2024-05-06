/*
 * torquecontrol.h
 *
 *  Created on: 01 May 2019
 *      Author: Visa
 */

#ifndef TORQUECONTROL_H_
#define TORQUECONTROL_H_

#define TORQUE_VECTORINGBIT			(0)
#define TORQUE_TRACTIONBIT			(1)
#define TORQUE_VELOCITYBIT			(2)
#define TORQUE_FEEDBACKBIT			(3)
#define TORQUE_FEEDFWDBIT			(4)
#define TORQUE_VECTORINGENABLEDBIT	(5)
#define TORQUE_TCSENABLEDBIT		(6)

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
void doVectoring(float Torque_Req, vectoradjust * adj, speedadjust * spd, int16_t pedalreq );

int getTorqueReqCurve(int16_t pedalreq);
float PedalTorqueRequest( int16_t *used_pedal_percent );

#endif /* TORQUECONTROL_H_ */

