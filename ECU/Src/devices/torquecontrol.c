/*
 * torquecontrol.c
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#include "ecumain.h"

#ifdef MATLAB
#include "Controller_design.h"
//#include "Controller_design_private.h"
//#include "Controller_design_types.h"
#include "rtwtypes.h"
#endif

#include <stdio.h>
#include <time.h>

int initVectoring( void )
{
#ifdef MATLAB
	  Controller_design_initialize();
#endif
	return 0;
}

void doVectoring(int16_t Torque_Req, vectoradjust * adj)
{
#ifdef MATLAB
	 rtU.Steering = ADCState.SteeringAngle;
	 rtU.Modeselection = CarState.TorqueVectoringMode;
	 rtU.Yawrate =  IMUReceived.GyroZ*0.001;
	 rtU.velocity =  IMUReceived.VelBodyX*0.01;

	 Controller_design_step();
	 int maxreq = 0;

	 if ( CarState.Torque_Req_CurrentMax < 55 )
		 maxreq=(1000*(CarState.Torque_Req_CurrentMax+10)/65);
	 else
	     maxreq=(1000*CarState.Torque_Req_CurrentMax/65);

	 adj->RL = Torque_Req+rtY.tql*1000/65;
	 adj->RR = Torque_Req-rtY.tqr*1000/65;

	 if ( adj->RL < 0 ) adj->RL = 0;
	 if ( adj->RR < 0 ) adj->RR = 0;

	 if ( adj->RL > Torque_Req * 2 ) adj->RL = Torque_Req*2;
	 if ( adj->RR > Torque_Req * 2 ) adj->RR = Torque_Req*2;

	 if	( adj->RL > maxreq ) adj->RL = maxreq;
	 if ( adj->RR > maxreq ) adj->RR = maxreq;

#endif
}

