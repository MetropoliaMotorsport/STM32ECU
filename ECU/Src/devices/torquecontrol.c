/*
 * torquecontrol.c
 *
 *  Created on: 01 May 2019
 *      Author: Visa
 */

#include "ecumain.h"
#include "torquecontrol.h"
#include "node_device.h"
#include "brake.h"
#include "timerecu.h"
#include "inverter.h"
#include "imu.h"
#include "node_device.h"
#ifdef MATLAB

#include "../matlab/Regeneration_ert_rtw/Regeneration.h"
#include "../matlab/TorqueVectoring_ert_rtw/TorqueVectoring.h"
#include "../matlab/TractionControl_ert_rtw/TractionControl.h"
#endif

#include <stdio.h>
#include <time.h>
#include "eeprom.h"

volatile InterpolationTables_t InterpolationTable;

int initVectoring(void) {
#ifdef MATLAB
	TractionControl_initialize();
	TorqueVectoring_initialize();
	Regeneration_initialize();
#endif
	return 0;
}

void doVectoring(float Torque_Req, vectoradjust *adj, speedadjust *spd,
		int16_t pedalreq) {

#ifdef TORQUEBALANCE
	uint8_t torquebalInt = getEEPROMBlock(0)->TorqueBal;
	uint8_t torquebal = 0;

	if ( Torque_Req > 0 )
	{
		if ( torquebalInt == 50 || torquebalInt == 0 )
		{
			adj->FL = Torque_Req;
			adj->FR = Torque_Req;
			adj->RL = Torque_Req;
			adj->RR = Torque_Req;
		} else if ( torquebalInt > 50 )
		{
			torquebal = torquebalInt / ( 100 - torquebalInt );
			adj->FL = Torque_Req / torquebal;
			adj->FR = Torque_Req / torquebal;
			adj->RL = Torque_Req;
			adj->RR = Torque_Req;
		} else
		{
			torquebal = torquebalInt / ( 100 - torquebalInt );
			adj->FL = Torque_Req;
			adj->FR = Torque_Req;
			adj->RL = Torque_Req / torquebal;
			adj->RR = Torque_Req / torquebal;
		}
	} else
	{
		adj->FL = Torque_Req;
		adj->FR = adj->FL;
		adj->RL = adj->FL;
		adj->RR = adj->FL;
	}
#else
	adj->FL = Torque_Req;
	adj->FR = adj->FL;
	adj->RL = adj->FL;
	adj->RR = adj->FL;
#endif

	uint16_t maxSpeed;
	//if ( Torque_Req > 0 )
	maxSpeed = getEEPROMBlock(0)->maxRpm;
	/*
	 else
	 maxSpeed = 20; // allow a slight rotation to spot bad setup.
	 */
	spd->FL = maxSpeed;
	spd->FR = maxSpeed;
	spd->RL = maxSpeed;
	spd->RR = maxSpeed;

#ifdef MATLAB
	// general config, not from dynamic state

	uint8_t torqueoptions = getEEPROMBlock(0)->TorqueVectoring;

	TractionControl_U.TC_enabled =
			torqueoptions & (1 << TORQUE_TCSENABLEDBIT) ? 1 : 0;
	TorqueVectoring_U.TorqueVectoringEnabled =
			torqueoptions & (1 << TORQUE_VECTORINGENABLEDBIT) ? 1 : 0;
	TorqueVectoring_U.FeedbackEnabled =
			torqueoptions & (1 << TORQUE_FEEDBACKBIT) ? 1 : 0;
	TorqueVectoring_U.FeedForwardEnabled =
			torqueoptions & (1 << TORQUE_FEEDFWDBIT) ? 1 : 0;
	TorqueVectoring_U.TorquePedal = pedalreq;

	Regeneration_U.select_operating_mode = getEEPROMBlock(0)->Regen == 2;
	Regeneration_U.regen_optimizer_on = 1;

	TractionControl_U.Desiredwheelslip = 0; /* '<Root>/UpperSlipThreshold' */
	TractionControl_U.LowerSlipThreshold = 0; /* '<Root>/LowerSlipThreshold' */
	TractionControl_U.Proportionalgain = 30;
	TractionControl_U.Integralgain = 6;
	TractionControl_U.Derivativegain = 2;

	Regeneration_U.max_regen_torque = -getEEPROMBlock(0)->regenMax;
	; // should be specified as negative value
	Regeneration_U.U_cell_max_possible_mV = 4160;

	// This is right velocity to be used in torque vectoring -> velocity in IMU x direction//speed;
	TractionControl_U.VehicleSpeed = TorqueVectoring_U.VehicleSpeed =
			Regeneration_U.speed = IMUReceived.VelBodyX * 0.01;
	int16_t VELUSED = TractionControl_U.VehicleSpeed;

	TractionControl_U.BrakePressure =
			BrakeRear.data > BrakeFront.data ?
					BrakeRear.data : BrakeFront.data;
	TractionControl_U.WheelRotVelocityFL = getInvState(invFL)->Speed * 0.10472; // convert wheel rpm to rad/s
	TractionControl_U.WheelRotVelocityFR = getInvState(invFR)->Speed * 0.10472;
	TractionControl_U.WheelRotVelocityRL = getInvState(invRL)->Speed * 0.10472;
	TractionControl_U.WheelRotVelocityRR = getInvState(invRR)->Speed * 0.10472;

	TractionControl_step();

	TorqueVectoring_U.VehicleYawRate = IMUReceived.GyroZ * 0.001;
	TorqueVectoring_U.StrAngleDeg = SteeringAngle.data; // SteeringAngle;

	TorqueVectoring_step();

	Regeneration_U.static_P_min_lim = -44; // regeneration power that we can regen always with from -100 - 0 kW should be negative
	Regeneration_U.Torque_pedal = APPS2.data / 10.0;
	Regeneration_U.brake_pedal_position = BPPS.data / 10.0;
	Regeneration_U.pedal_rege_thresh_endurance_max = 10; // allow regen if throttle less than 10%
	Regeneration_U.IVT_WhCalculated = CarState.Wh;

	Regeneration_U.U_cell_max_mV = CarState.HighestCellV; // TODO get max cellv from bms message

	Regeneration_U.Slip_FL = TractionControl_Y.slipFL;
	Regeneration_U.Slip_FR = TractionControl_Y.slipFR;
	Regeneration_U.Slip_RL = TractionControl_Y.slipRL;
	Regeneration_U.Slip_RR = TractionControl_Y.slipRR;

	Regeneration_step();

#if 0
	/* External outputs (root outports fed by signals with default storage) */
	typedef struct {
	  real_T MotorRegenPowerLimNegkW;      /* '<Root>/MotorRegenPowerLimNegkW' */
	  real_T SOC;                          /* '<Root>/SOC' */
	} Regeneration_ExtY;
#endif

	CarState.SOC = Regeneration_Y.SOC;

	adj->FL += Regeneration_Y.regenFL - TractionControl_Y.TC_FL
			- TorqueVectoring_Y.TVFL;
	adj->FR += Regeneration_Y.regenFR - TractionControl_Y.TC_FR
			+ TorqueVectoring_Y.TVFR;
	adj->RL += Regeneration_Y.regenRL - TractionControl_Y.TC_RL
			- TorqueVectoring_Y.TVRL;
	adj->RR += Regeneration_Y.regenRR - TractionControl_Y.TC_RR
			+ TorqueVectoring_Y.TVRR;

	CAN_Send4vals(0x7CE, VELUSED, 0, IMUReceived.GyroZ, 0);

	CAN_Send4vals(0x7CF, (int16_t) TractionControl_Y.TC_FL * NMSCALING,
			(int16_t) TractionControl_Y.TC_FR * NMSCALING,
			(int16_t) TractionControl_Y.TC_RL * NMSCALING,
			(int16_t) TractionControl_Y.TC_RR * NMSCALING);
	CAN_Send4vals(0x7D0, (int16_t) Regeneration_Y.regenFL * NMSCALING,
			(int16_t) Regeneration_Y.regenFR * NMSCALING,
			(int16_t) Regeneration_Y.regenRL * NMSCALING,
			(int16_t) Regeneration_Y.regenRR * NMSCALING);
	CAN_Send4vals(0x7D1, (int16_t) TorqueVectoring_Y.TVFL * NMSCALING,
			(int16_t) TorqueVectoring_Y.TVFR * NMSCALING,
			(int16_t) TorqueVectoring_Y.TVRL * NMSCALING,
			(int16_t) TorqueVectoring_Y.TVRR * NMSCALING);
#endif

	float avg = 0;
	avg += adj->FL;
	avg += adj->FR;
	avg += adj->RL;
	avg += adj->RR;
	avg = avg / 4;

	CarState.Torque_Req = avg;
}

/*
 * APPS Check, Should ignore regen sensor and only use physical brake.
 *
 */


int16_t linearInterpolate(uint16_t Input[], int16_t Output[], uint16_t count, uint16_t RawInput)
{
	//TODO implement linear interpolation
   return RawInput;
}

int getTorqueReqCurve(int16_t pedalreq) {
	

	return pedalreq;
}

int getBrakeTravelPerc(int16_t pedalreq) {
	return pedalreq;
}

int getTorqueReqPercL(int16_t pedalreq) {
	return pedalreq;
}

int getTorqueReqPercR(int16_t pedalreq) {
	return pedalreq;
}


float PedalTorqueRequest(int16_t *used_pedal_percent) // returns current Nm request amount.
{

	uint16_t APPS1_raw = APPS1.data;
	uint16_t APPS2_raw = APPS2.data;
	uint16_t BPPS_raw = BPPS.data;


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


	int difference = abs(
			APPS1.data - APPS2.data) / 10;

#ifdef TORQUE_LEFT_PRIMARY
	int torqueperc = APPS1.data;
#else
	int torqueperc = APPS2.data;
#endif


	//The average value of the APPS // signals pedal travel equivalent to ≥25 % desired motor torque
	int TorqueRequestPercent = getTorqueReqCurve(torqueperc) / 10; //TODO Implement curve
	*used_pedal_percent = TorqueRequestPercent;

	// The commanded motor torque must remain at 0 N m until the APPS signals less than 5 % pedal travel
	// and 0 N m desired motor torque, regardless of whether the brakes are still actuated or not.

	int TorqueRequestTravelPercent = torqueperc / 10;

	//   -Implausibility Test Failure : In case of more than 10 percent implausibility between the APPS,torque request is 0
	//   -Implausibility allowed : more than 10 percent
	//   -Torque-Brake Violation : Free
	if (difference > TORQUE_DIFFERENCE) {
		Torque_drivers_request = 0;
		CarState.APPSstatus = 1;
	} else if (CarState.AllowRegen

			&& BPPS.data > (REGENMINIMUM * 10)

			&& BPPS_raw > (REGENMINIMUM * 10)

			&& getEEPROMBlock(0)->Regen) {
		Torque_drivers_request = 0;
		No_Torque_Until_Pedal_Released = 1;
		CarState.APPSstatus = 8;
	}
	//   -Normal Driving Conditions
	//   -Implausibility allowed : less or equal to 10 percent
	//   -Brake Pressure allowed : less than 30
	//   -Torque-Brake Violation : Free

	else if (difference <= TORQUE_DIFFERENCE && getBrakeLow() // 30
			&& No_Torque_Until_Pedal_Released == 0) {
		Torque_drivers_request = 1;
		APPSTriggerTime = 0;
		CarState.APPSstatus = 0; // 2
	}
	//   -Torque-Brake Violation : Accelerator Pedal and Brake Pedal pressed at the same time
	//   -Accelerator Pedal Travel : More than 25 percent or power > 5kW for more than 500ms
	//   -Brake Pressure allowed : more than 30
	//   -Torque-Brake Violation : Occurred and marked
#ifdef APPSALLOWBRAKE
	else if (difference <= TORQUE_DIFFERENCE && getBrakeHigh()
			&& (TorqueRequestPercent >= 25 || CarState.Power >= 5000)
			&& APPSTriggerTime == 0) {
		APPSTriggerTime = gettimer();
		Torque_drivers_request = 1; // still sllow torque till timer elapsed.
		CarState.APPSstatus = 3;
	}

	else if (difference <= TORQUE_DIFFERENCE
			&& getBrakeHigh() // Hopefully fixed likely bug with 300ms
			&& ( TorqueRequestPercent>=25 || CarState.Power >= 5000 ) && gettimer()-APPSTriggerTime < APPSBRAKETIME)// 300ms brake allowance
			{
		Torque_drivers_request = 1;
		CarState.APPSstatus = 3;
	}
#endif

	else if (difference <= TORQUE_DIFFERENCE && getBrakeHigh()
			&& (TorqueRequestPercent >= 25 || CarState.Power >= 5000)
#ifdef APPSALLOW450MSBRAKE
			 && gettimer()-APPSTriggerTime >= APPSBRAKETIME
#endif
			) {
		Torque_drivers_request = 0; // near max allowed time of torque with braking, trigger no torque finally.
		No_Torque_Until_Pedal_Released = 1;
		CarState.APPSstatus = 4;
	}

	//   -After torque-brake violation :  Even if brake pedal is released, and the APPS are more than 5 percent, no torque is allowed
	//   -Accelerator Pedal Travel : More than 5 percent
	//   -Brake Pressure allowed : less than 30
	//   -Torque-Brake Violation : Still exists and won't be freed
	else if (difference <= TORQUE_DIFFERENCE
			&& (getBrakeLow()) // 30
			&& No_Torque_Until_Pedal_Released == 1
			&& TorqueRequestPercent >= 5) {
		Torque_drivers_request = 0;
		CarState.APPSstatus = 5;
	}

	//   -After torque-brake violation and the release of the brake pedal, torque is allowed when APPS are less than 5 percent
	//   -Accelerator Pedal Travel : Less than 5 percent
	//   -Brake Pressure allowed : less than 30
	//   -Torque-Brake Violation : Occurred and will be freed

	else if (difference <= TORQUE_DIFFERENCE
			&& getBrakeLow() // 30
			&& No_Torque_Until_Pedal_Released == 1
			&& TorqueRequestTravelPercent < 5) {
		No_Torque_Until_Pedal_Released = 0;
		Torque_drivers_request = 1;
		APPSTriggerTime = 0;
		CarState.APPSstatus = 0; // torque ok. // 5
	} else if (difference <= TORQUE_DIFFERENCE
			&& getBrakeHigh() // 30
			&& No_Torque_Until_Pedal_Released == 0
			&& TorqueRequestTravelPercent < 5) { // brakes pressed without accelerator, don't allow torque.
		Torque_drivers_request = 0;
		CarState.APPSstatus = 6;
	} else 		//  -Any other undefined condition, should never end up here
	{
		Torque_drivers_request = 0;
		CarState.APPSstatus = 99; // 6
	}

	// calculate actual torque request
	if (Torque_drivers_request != 0) {
		return (getTorqueReqCurve(torqueperc) * CarState.Torque_Req_CurrentMax)
				/ 1000.0; //*NMSCALING)/1000;
	} else {
		return 0;
	}
}

bool SetupInterpolationTables(eepromdata* eepromdatahandle) {
 // calibrated input range for steering, from left lock to center to right lock.
    // check if this can be simplified?

	if ( checkversion(data->VersionString) )
	{

		int i = 0;
		BrakeRInput[2] = data->BrakeRPresInput[0];
		BrakeRInput[3] = data->BrakeRPresInput[1];

		BrakeROutput[2] = data->ADCBrakeRPresOutput[0];
		BrakeROutput[3] = data->ADCBrakeRPresOutput[1];

		BrakeFInput[2] = data->ADCBrakeFPresInput[0];
		BrakeFInput[3] = data->ADCBrakeFPresInput[1];

		BrakeFOutput[2] = data->ADCBrakeFPresOutput[0];
		BrakeFOutput[3] = data->ADCBrakeFPresOutput[1];


		i = 0;

		int TravMin = data->TorqueReqLInput[0];
		int TravMax = data->TorqueReqLInput[1];
		if ( TravMax == 0 ) // not calibrated, force 0 output
		{
			TravMin = 64000;
			TravMax = 64000;
		}
		int TravMinOffset = 10;
		int TravMaxOffset = 98;

		if ( data->TorqueReqLInput[3] != 0 )
		{
			TravMinOffset = data->TorqueReqLInput[2];
			TravMaxOffset = data->TorqueReqLInput[3];
		}

		TorqueReqLInput[2] = (TravMax-TravMin)/100*TravMinOffset+TravMin;
		TorqueReqLInput[3] = (TravMax-TravMin)/100*TravMaxOffset+TravMin;

#ifdef TORQUEERRORCHECK
		uint32_t absolutemax = TravMax*1.1;
		if ( absolutemax > 0xFFFF )
			absolutemax + 64000;

		TorqueReqLInput[4] = absolutemax;
		TorqueReqLInput[5] = TorqueReqLInput[4]+1;
#endif

		TravMin = data->TorqueReqRInput[0];
		TravMax = data->TorqueReqRInput[1];
		if ( TravMax == 0 )
		{
			TravMin = 64000;
			TravMax = 64000;
		}
		TravMinOffset = 10;
		TravMaxOffset = 98;

		if ( data->TorqueReqRInput[3] != 0 )
		{
			TravMinOffset = data->TorqueReqRInput[2];
			TravMaxOffset = data->TorqueReqRInput[3];
		}

		TorqueReqRInput[2] = (TravMax-TravMin)/100*TravMinOffset+TravMin;
		TorqueReqRInput[3] = (TravMax-TravMin)/100*TravMaxOffset+TravMin;

#ifdef TORQUEERRORCHECK
		absolutemax = TravMax*1.1;
		if ( absolutemax > 0xFFFF )
			absolutemax + 64000;


		TorqueReqRInput[4] = absolutemax;
		TorqueReqRInput[5] = TorqueReqRInput[4]+1;
#endif

		TravMin = data->BrakeTravelInput[0];
		TravMax = data->BrakeTravelInput[1];
		if ( TravMax == 0 )
		{
			TravMin = 64000;
			TravMax = 64000;
		}
		TravMinOffset = 10;
		TravMaxOffset = 98;

		if ( data->BrakeTravelInput[3] != 0 )
		{
			TravMinOffset = data->BrakeTravelInput[2];
			TravMaxOffset = data->BrakeTravelInput[3];
		}

	// regen
		BrakeTravelInput[2] = (TravMax-TravMin)/100*TravMinOffset+TravMin;
		BrakeTravelInput[3] = (TravMax-TravMin)/100*TravMaxOffset+TravMin;

#ifdef TORQUEERRORCHECK
		absolutemax = TravMax*1.1;
		if ( absolutemax > 0xFFFF )
			absolutemax + 64000;

		BrakeTravelInput[4] = absolutemax;
		BrakeTravelInput[5] = BrakeTravelInput[4]+1;
#endif

		if (data->pedalcurves[i].PedalCurveInput[1] != 0)
		{
			TorqueCurveCount = 0;

			i = 0;

			for (;data->pedalcurves[i].PedalCurveInput[1]!=0;i++) // first number could be 0, but second will be non zero.
			{
				TorqueCurveCount++;

				int j=0;
				do {
					TorqueInputs[i][j]=data->pedalcurves[i].PedalCurveInput[j];
					TorqueOutputs[i][j]=data->pedalcurves[i].PedalCurveOutput[j];
					j++;

				} while ( data->pedalcurves[i].PedalCurveInput[j] != 0);
	//			if ( j < 3 ) j = 0;
				TorqueCurveSize[i] = j;
			}
		}

		InterpolationTables.BrakeR.Input = BrakeRInput;
		InterpolationTables.BrakeR.Output = BrakeROutput;

		InterpolationTables.BrakeR.Elements = BrakeRSize;

		InterpolationTables.BrakeF.Input = BrakeFInput;
		InterpolationTables.BrakeF.Output = BrakeFOutput;

		InterpolationTables.BrakeF.Elements = BrakeFSize;

		InterpolationTables.Regen.Input = BrakeTravelInput;
		InterpolationTables.Regen.Output = BrakeTravelOutput;

		InterpolationTables.Regen.Elements = BrakeTravelSize;

		InterpolationTables.AccelL.Input = TorqueReqLInput;
		InterpolationTables.AccelL.Output = TorqueReqLOutput;

		InterpolationTables.AccelL.Elements = TorqueReqLSize;

		InterpolationTables.AccelR.Input = TorqueReqRInput;
		InterpolationTables.AccelR.Output = TorqueReqROutput;

		InterpolationTables.AccelR.Elements = TorqueReqRSize;

		InterpolationTables.TorqueCurve.Input = TorqueInputs[0];
		InterpolationTables.TorqueCurve.Output = TorqueOutputs[0];

		InterpolationTables.TorqueCurve.Elements = TorqueCurveSize[0];

		InterpolationTables.Coolant.Input = CoolantInput;
		InterpolationTables.Coolant.Output = CoolantOutput;

		InterpolationTables.Coolant.Elements = CoolantSize;

		InterpolationTables.ModeSelector.Input = DrivingModeInput;
		InterpolationTables.ModeSelector.Output = DrivingModeOutput;

		InterpolationTables.ModeSelector.Elements = DriveModeSize;

    	return true;
	} else return false;
}

void SetupTorque(uint8_t pedal) {
	//TODO implement
	InterpolationTable.TorqueCurve.Input = TorqueInputs[pedal];
	InterpolationTable.TorqueCurve.Output = TorqueOutputs[pedal];
	InterpolationTable.TorqueCurve.Elements = TorqueCurveSize[pedal];
}