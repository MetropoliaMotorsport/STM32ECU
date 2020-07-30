/*
 * torquecontrol.c
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#include "ecumain.h"

#ifdef MATLAB
#include "A.h"
#include "A_private.h"
#include "A_types.h"
#include "rtwtypes.h"
#endif

#include <stdio.h>
#include <time.h>

int initVectoring( void )
{
#ifdef MATLAB
	A_initialize();
#endif
	return 0;
}

void doVectoring(int16_t Torque_Req, vectoradjust * adj)
{
#ifdef MATLAB

	A_U.Yaw_rate = 0; // IMUData; // IMU
	A_U.Velocity = 0; // IMU
	A_U.Wheel_speed = 0; // Inverter
	A_U.Side_slip = 0; // IMU
//	A_U.Longitudinalacceleration = IMUData.AccelY; // IMU
//	A_U.Lateralacceleration = IMUData.AccelX; // IMU
	A_U.Steering_angle = ADCState.SteeringAngle; // ECU
	A_U.Throttle_request = Torque_Req; // NODE-ADC
	A_U.Regenerationrequest = ADCState.Regen_Percent; // NODE-ADC
	A_U.Torquevectoringmode = 0; // ECU

	A_step();
#endif

	adj->FL = Torque_Req;
	adj->FR = Torque_Req;
	adj->RL = Torque_Req;
	adj->RR = Torque_Req;
}

