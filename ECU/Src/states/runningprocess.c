/*
 * operationreadyness.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#include "ecumain.h"


uint16_t PrintRunning( char *title )
{
	char str[80] = "";

	sprintf(str, "%-6s %8s %3liv", title, getCurTimeStr(), lcd_geterrors());//, CarState.VoltageBMS);

	lcd_send_stringline(0,str, 255);

	int Torque = ADCState.Torque_Req_R_Percent/10;
	if ( Torque > 99 ) Torque = 99;

	sprintf(str,"%2linm(%2d%%) APPS:%c",
//	sprintf(str,"%2linm(%2d%%,%2liac) APPS:%c",
			(CarState.Torque_Req*1000+15384-1)/15384,
			Torque, //CarState.Torque_Req,
			(CarState.APPSstatus > 0)?'_':'A');
	lcd_send_stringline(1,str, 255);

	int speed = 0xFFFF;

	// find slowest wheel to define speed.
	for ( int i = 0; i<MOTORCOUNT;i++)
	{
		if (CarState.Speed[i] < speed )
			speed = CarState.Speed[i];
	}

	CarState.ActualSpeed = speed;

	char angdir = ' ';

	if ( ADCState.SteeringAngle < 0) angdir = 'L';
	if ( ADCState.SteeringAngle > 0) angdir = 'R';

	sprintf(str,"Spd:%3likmh Ang:%3d%c", (CarState.ActualSpeed), abs(ADCState.SteeringAngle), angdir);
	lcd_send_stringline(2,str, 255);
	return 0;
}

uint16_t PrintBrakeBalance( void )
{
	char str[255];

	if ( CarState.brake_balance < 255 )
	{
		sprintf(str,"Bal: %.3dF %.3dR (%.2d%%)", ADCState.BrakeF, ADCState.BrakeR, CarState.brake_balance );
	} else
	{
		if ( DeviceState.ADC == OPERATIONAL )
			sprintf(str, "Bal: Press Brakes");
		else
			sprintf(str, "Bal: No Data");
	}

	lcd_send_stringline(0,str, 255);
	return 0;
}


/*
 * APPS Check, Should ignore regen sensor and only use physical brake.
 *
 */
uint16_t PedalTorqueRequest( void ) // returns Nm request amount.
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

	//The average value of the APPS
	int TorqueRequestPercent = getTorqueReqCurve(ADCState.Torque_Req_R_Percent) / 10;

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
			&& ( ADCState.BrakeR < APPSBrakeRelease || ADCState.BrakeF < APPSBrakeRelease ) // 30
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
			 && ( ADCState.BrakeR >= APPSBrakeHard || ADCState.BrakeF >= APPSBrakeHard )
			 && ( TorqueRequestPercent>=25 || CarState.Power >= 5000 ) && APPSTriggerTime == 0 )
	{
		APPSTriggerTime = gettimer();
		Torque_drivers_request = 1; // still sllow torque till timer elapsed.
		CarState.APPSstatus=3;
	}

	else if( difference<=10
			 && ( ADCState.BrakeR >= APPSBrakeHard || ADCState.BrakeF >= APPSBrakeHard ) // Hopefully fixed likely bug with 300ms
			 && ( TorqueRequestPercent>=25 || CarState.Power >= 5000 ) && gettimer()-APPSTriggerTime < APPSBRAKETIME ) // 300ms brake allowance
	{
		Torque_drivers_request = 1;
		CarState.APPSstatus = 3;
	}
#endif

	else if( difference<=10
			 && ( ADCState.BrakeR >= APPSBrakeHard || ADCState.BrakeF >= APPSBrakeHard )
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
			 && ( ADCState.BrakeR < APPSBrakeRelease && ADCState.BrakeF < APPSBrakeRelease ) // 30
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
			  && ( ADCState.BrakeR < APPSBrakeRelease && ADCState.BrakeF < APPSBrakeRelease ) // 30
#ifdef REGEN

#endif
			  && No_Torque_Until_Pedal_Released==1
			  && TorqueRequestPercent < 5 )
	{
		No_Torque_Until_Pedal_Released=0;
	    Torque_drivers_request = 1;
	    APPSTriggerTime = 0;
	    CarState.APPSstatus=0; // torque ok. // 5
	}
	else if ( difference<=10
			  && ( ADCState.BrakeR >= APPSBrakeHard || ADCState.BrakeF >= APPSBrakeHard ) // 30
			  && No_Torque_Until_Pedal_Released==0
			  && TorqueRequestPercent < 5 )
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

#ifdef SIMPLETORQUEVECTOR
uint16_t TorqueVectorProcess( int torquerequest )
{
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
}
#endif


void FanControl( void )
{
	if(ADCState.Torque_Req_R_Percent > TORQUEFANLATCHPERCENTAGE*10) // if APPS position pver 50% trigger fan latched on.
	{
		CarState.FanPowered = 1;
	}
}

int RunningRequest( void )
{
//	ResetCanReceived(); // reset can data before operation to ensure we aren't checking old data from previous cycle.
	CAN_NMTSyncRequest(); // send NMT sync request.

	/* EV 4.12.1
	 * The vehicle must make a characteristic sound, continuously for at least one second and a maximum of three seconds when it enters ready-to-drive mode.
	 */

	setHV( true, true ); // buzzer only sounds when value changes from 0 to 1

#ifdef POWERNODES
	setDevicePower(Buzzer, 1);
	// FANS.
#endif

	invRequestState( OPERATIONAL );

	// else inverter not in expected state.
	return 0;
}


int RunningProcess( uint32_t OperationLoops, uint32_t targettime )
{
	// EV4.11.3 RTDM Check
	// Closing the shutdown circuit by any part defined in EV 6.1.2 must not (re-)activate the TS.
	// Additional action must be required.

	static uint16_t readystate;
	static uint32_t standstill;
	static uint8_t allowstop;
    static uint32_t limpcounter;

	if ( OperationLoops == 0) // reset state on entering/rentering.
	{
		lcd_clear();
		readystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.
		setOutput(RTDMLED,On); // move to ActivateRTDM
		blinkOutput(RTDMLED,On,0);
		allowstop = 0;
		standstill = 0;
        limpcounter = 0;

        CarState.AllowTorque = true;

    	if ( CarState.LimpRequest )
    		 CarState.Torque_Req_CurrentMax = LIMPNM;

		// send buzzer as entering RTDM
	}
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, RunningState, readystate );
	}

	PrintRunning("RTDM:TA");

	RunningRequest();

	uint8_t OperationalError = OperationalReceiveLoop();

	switch ( OperationalError )
	{
		case 0 : break;
//		case 1 : ReceiveNonCriticalError = 1; break;
		case OperationalErrorState :
//			Errors.ErrorPlace = 0xEA;
			Errors.ErrorReason = OperationalError;
			return OperationalErrorState;
	}

	// check data validity, // any critical errors, drop state.


	if  ( !( DeviceState.Inverter == OPERATIONAL
		  || DeviceState.Inverter == PREOPERATIONAL ) )
	{
		Errors.ErrorPlace = 0xE0;
		return OperationalErrorState;
	}

	if ( readystate == 0 && DeviceState.Inverter != OPERATIONAL )
	{  // an inverter has changed state from operating after reaching it unexpectedly, fault status of some sort.
		Errors.ErrorPlace = 0xE5;
		return OperationalErrorState;
	}

	bool moving = false;

	for ( int i=0;i<MOTORCOUNT;i++){
		if ( CarState.Inverters[i].Speed > 100 ) moving = true;
	}

	if ( !moving ) // untested
	{
		standstill++;
		if ( standstill > RTDMStopTime*100 )
		{
			allowstop = 1;
		}
	} else
	{
		standstill = 0;
		allowstop = 0;
	}

	// check for unexpected can registration messages-> error state.

	if ( CheckErrors() )
	{
			Errors.ErrorPlace = 0xEE;
			return OperationalErrorState; // something has triggered an error, drop to error state to deal with it.
	}

	if ( allowstop && CheckActivationRequest() ) // if requested disable Tractive System, drop state. This should shut off torque control immiedietly.
	{
		// send inverter request for ONLINE but not running?

		// TODO drop inverter state before switching main state.

		return IdleState; // check if need to drop HV in a special order.
	}

	for ( int i=0;i<MOTORCOUNT;i++)
	{
		CarState.Inverters[i].Torque_Req = 0; // APPS
	}

	if ( invertersStateCheck(OPERATIONAL) ) // returns true if all inverters match state == 0 ) // only start to request torque when inverters ready, which is signified by state of 0
	{
        // check if limp mode allowed ( don't want for acceleration test ), and if so, if BMS has requested.

        if ( ( CarState.LimpRequest && !CarState.LimpDisable ) || ADCState.CoolantTempR > COOLANTLIMPTEMP )
        {
            CarState.LimpActive = 1;
        }

        if ( CarState.LimpActive && CarState.Torque_Req_CurrentMax > LIMPNM )
        {
            limpcounter++;
            if ( ( limpcounter % 10 ) == 0 ) // every 100ms decrease nm request
            	CarState.Torque_Req_CurrentMax--;
        }

#ifdef ALLOWLIMPCANCEL
        // don't allow immiediete exit from limp mode if it was already triggered
        if ( CarState.LimpActive && OperationLoops > 200 && CheckRTDMActivationRequest() && ADCState.CoolantTempR < COOLANTLIMPTEMP)
        {
            CarState.LimpDisable = 1;
            CarState.LimpActive = 0;
        }

        if ( CarState.LimpActive && !CarState.LimpRequest && ADCState.CoolantTempR < COOLANTLIMPEXITTEMP  )
        {
            CarState.LimpDisable = 1;
            CarState.LimpActive = 0;
        }

        if ( CarState.LimpDisable && CarState.Torque_Req_CurrentMax < CarState.Torque_Req_Max )
        {
            limpcounter++;
            if ( ( limpcounter % 10 ) == 0 ) // every 100ms increase nm request back to original setting.
            	CarState.Torque_Req_CurrentMax++;

            if (CarState.Torque_Req_CurrentMax == CarState.Torque_Req_Max)
                CarState.LimpActive = 0;
        }
#endif

        // if allowed, process torque request, else request 0.

        CarState.Torque_Req = PedalTorqueRequest();  // calculate request from APPS

        if ( CarState.Torque_Req == 0 )
        {
        		// TODO check for regen request if torque request zero.  send negative torque?
        }

    	for ( int i=0;i<MOTORCOUNT;i++)  // set all wheels to same torque request
    	{
    		CarState.Inverters[i].Torque_Req = CarState.Torque_Req;
    	} // if any inverter is not ready, readystate will not be 0.

#ifdef SIMPLETORQUEVECTOR

// first check if we've requested toggling state and toggle it accelerator pedal not pressed.
		if ( CheckTSActivationRequest() && ADCState.Torque_Req_R_Percent < 5 && ADCState.Torque_Req_L_Percent < 5 ) // toggle torque vectoring with TS start button.
		{
			if ( CarState.TorqueVectoring ) CarState.TorqueVectoring = 0; else CarState.TorqueVectoring = 1;
		}

		if ( CarState.TorqueVectoring ) // handle indicator of active state.
		{
			blinkOutput(TSOFFLED_Output,LEDBLINK_ONE,LEDBLINKNONSTOP); // blinking green led = vectoring on.
		} else
		{
	//		setOutput(TSOFFLED_Output,LEDON); // steady LED = not enabled
			blinkOutput(TSOFFLED_Output,LEDON,LEDBLINKNONSTOP);
		}

        TorqueVectorProcess( Torque_Req );
#endif
		if ( CarState.APPSstatus ) setOutput(TSLED,On); else setOutput(TSLED,Off);

		// Call any additional processing here to alter final request.

#ifdef CONTROLTEST // this is very unlikely working in any way.
		static uint16_t LeftAdjust = 0;
		static uint16_t RightAdjust = 0;

		if ( CarState.DrivingMode == 8
			&& DeviceState.FrontSpeedSensors == ENABLED
			&& DeviceState.FLSpeed == OPERATIONAL
			&& DeviceState.FRSpeed == OPERATIONAL )
		{
				int16_t AverageFrontWheelSpeed = ( CarState.SpeedFL + CarState.SpeedFR ) / 2;
				if ( AverageFrontWheelSpeed > MINSPEEDFORCONTROL )
				{
					if ( CarState.SpeedRL > AverageFrontWheelSpeed * 1.1 ) // more than 10% slip
					{
						if ( LeftAdjust < 50 ) LeftAdjust++;
					} else
					{
						if ( LeftAdjust > 0 ) LeftAdjust--;
					}

					if ( CarState.SpeedRL > AverageFrontWheelSpeed * 1.1 ) // more than 10% slip
					{
						if ( RightAdjust < 50 ) RightAdjust++;
					} else
					{
						if ( RightAdjust > 0 ) RightAdjust--;
					}
				} else // control not active reset
				{
					if ( LeftAdjust > 0 ) LeftAdjust--;
					if ( RightAdjust > 0 ) RightAdjust--;
				}
		}

		CarState.Torque_Req_L = CarState.Torque_Req_L * ( 100 - LeftAdjust ) / 100;
		CarState.Torque_Req_R = CarState.Torque_Req_R * ( 100 - RightAdjust ) / 100;

#endif

//		uint32_t loopstart = gettimer();
//		uint32_t looptimer = 0;

//		send loop time to show point when data processed.

/*		while ( looptimer >= targettime-1 ) // wait till end of 10ms loop.
		{
			looptimer = gettimer() - loopstart;
		}
 */

#ifdef FANCONTROL
		FanControl();
#endif

	    for ( int i=0;i<MOTORCOUNT;i++)
	    {
#ifdef NOTORQUEREQUEST
	    	CarState.Inverters[i].Torque_Req = 0;
#else
	    	CarState.Inverters[i].Torque_Req = CarState.Torque_Req;
#endif
	    }

	}

	// if drop status due to error, check if recoverable, or if limp mode possible.

	return RunningState;
}
