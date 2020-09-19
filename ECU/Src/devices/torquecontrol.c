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

	 // run the matlab code.
	 Controller_design_step();

	 CAN_SendTorq2( rtY.val1, rtY.val2, rtY.val3 );

	 int maxreq = 0;

	 //maxreq=Torque_Req*3; //(1000*(CarState.Torque_Req_CurrentMax*3)/65);

	 // limit max actual possible request per wheel depending on current max power request
	 if ( CarState.Torque_Req_CurrentMax > 0 && CarState.Torque_Req_CurrentMax <=20 )
	     maxreq=Torque_Req*3;

	 if ( CarState.Torque_Req_CurrentMax > 20 && CarState.Torque_Req_CurrentMax <= 40 )
	     maxreq=(1000*45/65);

	 if ( CarState.Torque_Req_CurrentMax > 40  )
	     maxreq=1000;

	 adj->RL = Torque_Req+(rtY.tql*1000/65); // take requested adjustment in nm and convert it to to actual request format.
	 if ( adj->RL > ( Torque_Req * 3 ) ) adj->RL = Torque_Req*3; // limit adjustment to max 3x original request.

	 adj->RR = Torque_Req-(rtY.tqr*1000/65);
	 if ( adj->RR > ( Torque_Req * 3 ) ) adj->RR = Torque_Req*3;

	 if ( adj->RL < 0 ) adj->RL = 0;
	 if ( adj->RR < 0 ) adj->RR = 0;

	 if	( adj->RL > maxreq ) adj->RL = maxreq;
	 if ( adj->RR > maxreq ) adj->RR = maxreq;

#endif
}

