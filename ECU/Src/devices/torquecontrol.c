/*
 * torquecontrol.c
 *
 *  Created on: 01 May 2019
 *      Author: Visa
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

	 if ( rtU.velocity < 2.7 ) rtU.velocity = 0;

	 // run the matlab code.
	 Controller_design_step();

	 CAN_SendTorq2( /*rtY.val1*/ IMUReceived.GyroZ, rtY.val2, rtY.val3, IMUReceived.VelBodyX);

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

#else
  #ifdef SIMPLETORQUEVECTOR
	 /*
	  * torque vectoring should activate at >40/<-40 degrees of steering wheel angle.
	  * Full(=10Nm) torque change should be reached linearily at >90/<-90 of steering angle.
	  */
		int TorqueVectorAddition;

		// ensure torque request altering only happens when a torque request actually exists.

		if ( CarState.TorqueVectoring && torquerequest > 0 && ADCState.Torque_Req_R_Percent > 0 && ADCState.Torque_Req_L_Percent > 0 && abs(ADCState.SteeringAngle) > 40 )
		{
			TorqueVectorAddition = ConvertNMToRequest(getTorqueVector(ADCState.SteeringAngle))/10; // returns 10x NM request.

			if  ( abs(TorqueVectorAddition) > torquerequest ){
				if ( TorqueVectorAddition < 0 ) TorqueVectorAddition = 0-torquerequest;
				else TorqueVectorAddition = torquerequest;
			}

			int Left = torquerequest + TorqueVectorAddition;
			int Right = torquerequest - TorqueVectorAddition;
			// also check wheel speed.

			if ( Left > 1000 ) Left = 1000;
			if ( Right > 1000 ) Right = 1000;
			if ( Left < 0 ) Left = 0;
			if ( Right < 0 ) Right = 0;

			CarState.Torque_Req_L = Right;  // wheels wrong way round? , swapped for now, was + left before.
			CarState.Torque_Req_R = Left; // check and fix properly if inverters configured wrong way round.
			return 1; // we modified.
		}
		else
		{
			CarState.Torque_Req_L = torquerequest;  // wheels wrong way round? , swapped for now, was + left before.
			CarState.Torque_Req_R = torquerequest;
			return 0; // we set to zero.
		}


  #else
	 // no actual adjustment.
	 adj->FL = Torque_Req;
	 adj->FR = Torque_Req;
	 adj->RL = Torque_Req;
	 adj->RR = Torque_Req;
  #endif
#endif
}



/*
 * APPS Check, Should ignore regen sensor and only use physical brake.
 *
 */
uint16_t PedalTorqueRequest( void ) // returns current Nm request amount.
{
	//T 11.8.8:  If an implausibility occurs between the values of the APPSs and persists for more than 100 ms

	//[EV ONLY] The power to the motor(s) must be immediately shut down completely.
	//It is not necessary to completely deactivate the tractive system, the motor controller(s)
	// shutting down the power to the motor(s) is sufficient.

	// current code immediately orders no torque from motors if check fails.

	//100 ms, allow ~8 10ms loops before triggering?

	static uint32_t APPSTriggerTime = 0;

	static char No_Torque_Until_Pedal_Released = 0; // persistent local variable
	uint16_t Torque_drivers_request = 0;

	//The absolute value of the difference between the APPS (Accelerator Pedal Position Sensors)

	int difference = abs(ADCState.Torque_Req_L_Percent - ADCState.Torque_Req_R_Percent)/10;

	//The average value of the APPS // signals pedal travel equivalent to ≥25 % desired motor torque
	int TorqueRequestPercent = getTorqueReqCurve(ADCState.Torque_Req_R_Percent) / 10;

	// The commanded motor torque must remain at 0 N m until the APPS signals less than 5 % pedal travel
	// and 0 N m desired motor torque, regardless of whether the brakes are still actuated or not.

	int TorqueRequestTravelPercent = ADCState.Torque_Req_R_Percent / 10;

	//   -Implausibility Test Failure : In case of more than 10 percent implausibility between the APPS,torque request is 0
	//   -Implausibility allowed : more than 10 percent
	//   -Torque-Brake Violation : Free
	if( difference>10 )
	{
	    Torque_drivers_request = 0;
	    CarState.APPSstatus = 1;
	}

	//   -Normal Driving Conditions
	//   -Implausibility allowed : less or equal to 10 percent
	//   -Brake Pressure allowed : less than 30
	//   -Torque-Brake Violation : Free

	else if( difference<=10
			&& getBrakeLow() // 30
			&& No_Torque_Until_Pedal_Released == 0 )
	{
		Torque_drivers_request = 1;
		APPSTriggerTime = 0;
		CarState.APPSstatus = 0; // 2
	}
	//   -Torque-Brake Violation : Accelerator Pedal and Brake Pedal pressed at the same time
	//   -Accelerator Pedal Travel : More than 25 percent or power > 5kW for more than 500ms
	//   -Brake Pressure allowed : more than 30
	//   -Torque-Brake Violation : Occurred and marked
#ifdef APPSALLOWBRAKE
	else if( difference<=10
			 && getBrakeHigh()
			 && ( TorqueRequestPercent>=25 || CarState.Power >= 5000 ) && APPSTriggerTime == 0 )
	{
		APPSTriggerTime = gettimer();
		Torque_drivers_request = 1; // still sllow torque till timer elapsed.
		CarState.APPSstatus=3;
	}

	else if( difference<=10
			 && getBrakeHigh() // Hopefully fixed likely bug with 300ms
			 && ( TorqueRequestPercent>=25 || CarState.Power >= 5000 ) && gettimer()-APPSTriggerTime < APPSBRAKETIME ) // 300ms brake allowance
	{
		Torque_drivers_request = 1;
		CarState.APPSstatus = 3;
	}
#endif

	else if( difference<=10
			 && getBrakeHigh()
			 && ( TorqueRequestPercent>=25 || CarState.Power >= 5000 )
#ifdef APPSALLOW450MSBRAKE
			 && gettimer()-APPSTriggerTime >= APPSBRAKETIME
#endif
			)
	{
		Torque_drivers_request = 0; // near max allowed time of torque with braking, trigger no torque finally.
		No_Torque_Until_Pedal_Released = 1;
		CarState.APPSstatus = 4;
	}

	//   -After torque-brake violation :  Even if brake pedal is released, and the APPS are more than 5 percent, no torque is allowed
	//   -Accelerator Pedal Travel : More than 5 percent
	//   -Brake Pressure allowed : less than 30
	//   -Torque-Brake Violation : Still exists and won't be freed
	else if( difference<=10
			 && ( getBrakeLow()  ) // 30
			 && No_Torque_Until_Pedal_Released==1
			 && TorqueRequestPercent >=5 )
	{
		Torque_drivers_request = 0;
		CarState.APPSstatus = 5;
	}

	//   -After torque-brake violation and the release of the brake pedal, torque is allowed when APPS are less than 5 percent
	//   -Accelerator Pedal Travel : Less than 5 percent
	//   -Brake Pressure allowed : less than 30
	//   -Torque-Brake Violation : Occurred and will be freed

	else if ( difference<=10
			  && getBrakeLow() // 30
#ifdef REGEN

#endif
			  && No_Torque_Until_Pedal_Released==1
			  && TorqueRequestTravelPercent < 5 )
	{
		No_Torque_Until_Pedal_Released=0;
	    Torque_drivers_request = 1;
	    APPSTriggerTime = 0;
	    CarState.APPSstatus=0; // torque ok. // 5
	}
	else if ( difference<=10
			  && getBrakeHigh() // 30
			  && No_Torque_Until_Pedal_Released==0
			  && TorqueRequestTravelPercent < 5 )
	{ // brakes pressed without accelerator, don't allow torque.
	    Torque_drivers_request = 0;
	    CarState.APPSstatus = 6;
	}
	else 		//  -Any other undefined condition, should never end up here
	{
	    Torque_drivers_request = 0;
	    CarState.APPSstatus = 99; // 6
	}

	// calculate actual torque request
	if ( Torque_drivers_request != 0)
	{

		return getTorqueReqCurve(ADCState.Torque_Req_R_Percent)*CarState.Torque_Req_CurrentMax/65; //  *0.01/10*1000 unnecessary calculations, works out to 1, gets rid of floating point

		// return torquerequest;// Torque_Req_R_Percent is 1000=100%, so div 10 to give actual value.
	}
	  //	  return getTorqueReqCurve(ADCState.Torque_Req_R_Percent)*CarState.Torque_Req_CurrentMax*0.01)*1000/65)/10; // Torque_Req_R_Percent is 1000=100%, so div 10 to give actual value.
	else
	{
//	  CarState.Torque_Req_L = 0;  // wheels wrong way round? , swapped for now, was + left before.
//	  CarState.Torque_Req_R = 0;
	  return 0;
	}
}


