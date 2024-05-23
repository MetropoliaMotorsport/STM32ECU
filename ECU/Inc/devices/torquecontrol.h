/*
 * torquecontrol.h
 *
 *  Created on: 01 May 2019
 *      Author: Visa
 */

#ifndef TORQUECONTROL_H_
#define TORQUECONTROL_H_

#include "eeprom.h"

#define TORQUE_VECTORINGBIT			(0)
#define TORQUE_TRACTIONBIT			(1)
#define TORQUE_VELOCITYBIT			(2)
#define TORQUE_FEEDBACKBIT			(3)
#define TORQUE_FEEDFWDBIT			(4)
#define TORQUE_VECTORINGENABLEDBIT	(5)
#define TORQUE_TCSENABLEDBIT		(6)


struct Table {
	uint16_t *Input;
	int16_t *Output;
	uint16_t Elements;
};

typedef struct InterpolationTables { // pointers to array data for linear interpolation values, use elements field to know how large arrays are.
		struct Table Steering;
		struct Table SteeringAngle;

		struct Table BrakeF;
		struct Table BrakeR;

		struct Table Regen;

		struct Table AccelL;
		struct Table AccelR;

		struct Table TorqueCurve;

		struct Table Coolant;

		struct Table ModeSelector;
#ifdef TORQUEVECTOR
		struct Table TorqueVector;
#endif
} InterpolationTables_t;

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
int getBrakeTravelPerc( int16_t pedalreq );
int getTorqueReqPercL( int16_t pedalreq );
int getTorqueReqPercR( int16_t pedalreq );



float PedalTorqueRequest( int16_t *used_pedal_percent );

void SetupTorque( uint8_t pedal );
bool SetupInterpolationTables( eepromdata* eepromdatahandle );



#endif /* TORQUECONTROL_H_ */

