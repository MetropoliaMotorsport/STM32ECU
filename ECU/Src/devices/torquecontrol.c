/*
 * torquecontrol.c
 *
 *  Created on: 01 May 2019
 *      Author: Visa
 */

#include "ecumain.h"
#include "torquecontrol.h"
#include "adcecu.h"
#include "brake.h"
#include "timerecu.h"
#include "inverter.h"
#include "imu.h"

#ifdef MATLAB

#include "SubsystemModelReference.h"   /* Model's header file */
#include "rtwtypes.h"
#endif

#include <stdio.h>
#include <time.h>

int initVectoring( void )
{
#ifdef MATLAB
	  SubsystemModelReference_initialize();
#endif
	return 0;
}

void doVectoring(float Torque_Req, vectoradjust * adj, speedadjust * spd )
{
#ifdef MATLAB
	rtU.bus_Vehicle_velocity = IMUReceived.VelBodyX*0.01;
	rtU.bus_Vehicle_acceleration = IMUReceived.AccelX*0.01;

	rtU.bus_rotation_speed_FL = getInvState(invFL)->Speed * 0.10472; // convert wheel rpm to rad/s
	rtU.bus_rotation_speed_FR = getInvState(invFR)->Speed * 0.10472;
	rtU.bus_rotation_speed_RL = getInvState(invRL)->Speed * 0.10472;
	rtU.bus_rotation_speed_RR = getInvState(invRR)->Speed * 0.10472;

	rtU.bus_Torque_FL = Torque_Req;
	rtU.bus_Torque_FR = Torque_Req;
	rtU.bus_Torque_RL = Torque_Req;
	rtU.bus_Torque_RR = Torque_Req;

	rtU.bus_Vehicle_yaw_rate = IMUReceived.GyroZ*0.001;
	rtU.bus_Vehicle_str_ang = ADCState.SteeringAngle;

	rtU.bus_Pedal_torque_position = ADCState.Torque_Req_R_Percent/10.0;

	rtU.bus_Traction_control_active = getEEPROMBlock(0)->TorqueVectoring;;
	rtU.bus_Velocity_control_active = getEEPROMBlock(0)->TorqueVectoring;;
	rtU.bus_feedback_active = 1;
	rtU.bus_feedforward_active = 1;
	rtU.bus_Torque_vectoring_active = 0;//getEEPROMBlock(0)->TorqueVectoring;

/*
	if ( rtU.velocity < 2.7 ) rtU.velocity = 0;
*/
	// run the matlab code.
	SubsystemModelReference_step();

	CAN_Send4vals( 0x7CE,
			(int16_t)rtU.bus_Vehicle_velocity,
			(int16_t)rtU.bus_Vehicle_acceleration,
			(int16_t)rtU.bus_Vehicle_yaw_rate,
			0);
	CAN_Send4vals( 0x7CF, rtY.TCS_TCS_FL*NMSCALING, rtY.TCS_TCS_FR*NMSCALING, rtY.TCS_TCS_RL*NMSCALING, rtY.TCS_TCS_RR*NMSCALING);
	CAN_Send4vals( 0x7D0, rtY.TCS_RPMmaxFL, rtY.TCS_RPMmaxFR, rtY.TCS_RPMmaxRL, rtY.TCS_RPMmaxRR );


	int maxreq = 0;

	//maxreq=Torque_Req*3; //(1000*(CarState.Torque_Req_CurrentMax*3)/65);

	/*

	// limit max actual possible request per wheel depending on current max power request
	if ( CarState.Torque_Req_CurrentMax > 0 && CarState.Torque_Req_CurrentMax <=20 )
		maxreq=Torque_Req*3;

	if ( CarState.Torque_Req_CurrentMax > 20 && CarState.Torque_Req_CurrentMax <= 40 )
		maxreq=(1000*45/65);

	if ( CarState.Torque_Req_CurrentMax > 40  )
		maxreq=1000;


	adj->RL = Torque_Req+(rtY.tql*1000/65); // take requested adjustment in nm and convert it to to actual request format.
	if ( adj->RL > ( Torque_Req * 3 ) ) adj->RL = Torque_Req*3; // limit adjustment to max 3x original request.

	adj->FL = Torque_Req+(rtY.tql*1000/65); // take requested adjustment in nm and convert it to to actual request format.
	if ( adj->FL > ( Torque_Req * 3 ) ) adj->FL = Torque_Req*3; // limit adjustment to max 3x original request.


	adj->RR = Torque_Req-(rtY.tqr*1000/65);
	if ( adj->RR > ( Torque_Req * 3 ) ) adj->RR = Torque_Req*3;

	adj->FR = Torque_Req-(rtY.tqr*1000/65);
	if ( adj->FR > ( Torque_Req * 3 ) ) adj->FR = Torque_Req*3;

	if ( adj->RL < 0 ) adj->RL = 0;
	if ( adj->RR < 0 ) adj->RR = 0;
	if ( adj->FL < 0 ) adj->FL = 0;
	if ( adj->FR < 0 ) adj->FR = 0;

	if	( adj->RL > maxreq ) adj->RL = maxreq;
	if ( adj->RR > maxreq ) adj->RR = maxreq;
	if	( adj->FL > maxreq ) adj->FL = maxreq;
	if ( adj->FR > maxreq ) adj->FR = maxreq;

*/

	// don't actually use output values yet.
	adj->FL = Torque_Req * NMSCALING;
	adj->FR = Torque_Req * NMSCALING;
	adj->RL = Torque_Req * NMSCALING;
	adj->RR = Torque_Req * NMSCALING;

	uint16_t maxSpeed = getEEPROMBlock(0)->maxRpm;

	spd->FL =maxSpeed;
	spd->FR =maxSpeed;
	spd->RL =maxSpeed;
	spd->RR =maxSpeed;

#else
  #ifdef SIMPLETORQUEVECTOR
	 /*
	  * torque vectoring should activate at >40/<-40 degrees of steering wheel angle.
	  * Full(=10Nm) torque change should be reached linearily at >90/<-90 of steering angle.
	  */
	 // outdated, still for RWD
	int TorqueVectorAddition;

	// ensure torque request altering only happens when a torque request actually exists.

	if ( CarState.TorqueVectoring && Torque_Req > 0 && ADCState.Torque_Req_R_Percent > 0 && ADCState.Torque_Req_L_Percent > 0 && abs(ADCState.SteeringAngle) > 40 )
	{
		TorqueVectorAddition = ConvertNMToRequest(getTorqueVector(ADCState.SteeringAngle))/10; // returns 10x NM request.

		if  ( abs(TorqueVectorAddition) > Torque_Req ){
			if ( TorqueVectorAddition < 0 ) TorqueVectorAddition = 0-Torque_Req;
			else TorqueVectorAddition = Torque_Req;
		}

		int Left = Torque_Req + TorqueVectorAddition;
		int Right = Torque_Req - TorqueVectorAddition;
		// also check wheel speed.

		if ( Left > 1000 ) Left = 1000;
		if ( Right > 1000 ) Right = 1000;
		if ( Left < 0 ) Left = 0;
		if ( Right < 0 ) Right = 0;

		adj->RL = Left;
		adj->RR = Right;
		adj->FL = 0;
		adj->FR = 0;

		return 1; // we modified.
	}
	else
	{
		adj->RL = Left;
		adj->RR = Right;
		adj->FL = 0;
		adj->FR = 0;
		return 0; // we set to zero.
	}


  #else
	// no actual adjustment for now.
	adj->FL = Torque_Req;
	adj->FR = Torque_Req;
	adj->RL = Torque_Req;
	adj->RR = Torque_Req;

	uint16_t maxSpeed = getEEPROMBlock(0)->maxRpm;

	spd->FL =maxSpeed;
	spd->FR =maxSpeed;
	spd->RL =maxSpeed;
	spd->RR =maxSpeed;
  #endif
#endif
}


/*
 * APPS Check, Should ignore regen sensor and only use physical brake.
 *
 */
float PedalTorqueRequest( void ) // returns current Nm request amount.
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
	} else if ( CarState.AllowRegen && ADCState.Regen_Percent > 500 && getEEPROMBlock(0)->Regen )
	{
	    Torque_drivers_request = 0;
	    No_Torque_Until_Pedal_Released = 1;
	    CarState.APPSstatus = 8;
	}
	//   -Normal Driving Conditions
	//   -Implausibility allowed : less or equal to 10 percent
	//   -Brake Pressure allowed : less than 30
	//   -Torque-Brake Violation : Free

#if 0
	else if ( CarState.Current > 1000 || CarState.Power > 1000 ) // if we're drawing more current than allowed, cut torque request till pedal released.
	{
		No_Torque_Until_Pedal_Released = 1;
		Torque_drivers_request = 0;
	    CarState.APPSstatus = 10;
	}
#endif
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
		return (getTorqueReqCurve(ADCState.Torque_Req_R_Percent)*CarState.Torque_Req_CurrentMax)/1000.0; //*NMSCALING)/1000;
	}
	else
	{
		return 0;
	}
}


