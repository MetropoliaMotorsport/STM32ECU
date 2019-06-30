/*
 * operationreadyness.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#include "ecumain.h"

//static int OperationLoops = 0;

/*
 * APPS Check
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

	static char No_Torque_Until_Pedal_Released = 0; // persistent local variable
	uint16_t Torque_drivers_request = 0;

	//The absolute value of the difference between the APPS (Accelerator Pedal Position Sensors)

	int difference = abs(ADCState.Torque_Req_L_Percent - ADCState.Torque_Req_R_Percent)/10;

	//The average value of the APPS
	int TorqueRequestPercent = ADCState.Torque_Req_R_Percent / 10; //(ADCState.Torque_Req_L_Percent + ADCState.Torque_Req_R_Percent) /2;

	//   -Implausibility Test Failure : In case of more than 10 percent implausibility between the APPS,torque request is 0
	//   -Implausibility allowed : more than 10 percent
	//   -Brake Pressure allowed : less than 65
	//   -Torque-Brake Violation : Free
	if( difference>10 )
	{
	    Torque_drivers_request = 0;
	    CarState.APPSstatus=1;
	}

	//   -Normal Driving Conditions
	//   -Implausibility allowed : less or equal to 10 percent
	//   -Brake Pressure allowed : less than 65
	//   -Torque-Brake Violation : Free

	else if( difference<=10
			&& ( ADCState.BrakeR < APPSBrakeRelease || ADCState.BrakeF < APPSBrakeRelease ) // 30
			&& No_Torque_Until_Pedal_Released==0 )
	{
		Torque_drivers_request = 1;
		CarState.APPSstatus=0; // 2
	}
	//   -Torque-Brake Violation : Accelerator Pedal and Brake Pedal pressed at the same time
	//   -Accelerator Pedal Travel : More than 25 percent or power > 5kW for more than 500ms

	// time not taken into consideration.

	//   -Brake Pressure allowed : more than 140
	//   -Torque-Brake Violation : Occurred and marked

	else if( difference<=10
			 && ( ADCState.BrakeR > APPSBrakeHard || ADCState.BrakeF > APPSBrakeHard )
			 && ( TorqueRequestPercent>=25 || CarState.Power >= 5000 ) )
	{
		Torque_drivers_request=0;
		No_Torque_Until_Pedal_Released=1;
		CarState.APPSstatus=3;
	}

	//   -After torque-brake violation :  Even if brake pedal is released, and the APPS are more than 5 percent, no torque is allowed
	//   -Accelerator Pedal Travel : More than 5 percent
	//   -Brake Pressure allowed : less than 65
	//   -Torque-Brake Violation : Still exists and won't be freed
	else if( difference<=10
			 && ( ADCState.BrakeR < APPSBrakeRelease && ADCState.BrakeF < APPSBrakeRelease ) // 30
			 && No_Torque_Until_Pedal_Released==1
			 && TorqueRequestPercent >=5 )
	{
		Torque_drivers_request=0;
		CarState.APPSstatus=4;
	}

	//   -After torque-brake violation and the release of the brake pedal, torque is allowed when APPS are less than 5 percent
	//   -Accelerator Pedal Travel : Less than 5 percent
	//   -Brake Pressure allowed : less than 65
	//   -Torque-Brake Violation : Occurred and will be freed

	else if ( difference<=10
			  && ( ADCState.BrakeR < APPSBrakeRelease && ADCState.BrakeF < APPSBrakeRelease ) // 30
			  && No_Torque_Until_Pedal_Released==1
			  && TorqueRequestPercent < 5 )
	{
		No_Torque_Until_Pedal_Released=0;
	    Torque_drivers_request = 1;
	    CarState.APPSstatus=0; // torque ok. // 5
	}
	else if ( difference<=10
			  && ( ADCState.BrakeR > APPSBrakeHard || ADCState.BrakeF > APPSBrakeHard ) // 30
			  && No_Torque_Until_Pedal_Released==0
			  && TorqueRequestPercent < 5 )
	{ // brakes pressed without accelerator, don't allow torque.
	    Torque_drivers_request=0;
	    CarState.APPSstatus=5;
	}
	else 		//  -Any other undefined condition, should never end up here
	{
	    Torque_drivers_request=0;
	    CarState.APPSstatus=99; // 6
	}

	// can send status

	if ( Torque_drivers_request )
	  return ((getTorqueReqCurve(ADCState.Torque_Req_R_Percent)*CarState.Torque_Req_CurrentMax*0.01)*1000/65)/10; // Torque_Req_R_Percent is 1000=100%, so div 10 to give actual value.
	else
	  return 0;
}


int RunningRequest( void )
{
	uint16_t command;

//	ResetCanReceived(); // reset can data before operation to ensure we aren't checking old data from previous cycle.
	CAN_NMTSyncRequest(); // send NMT sync request.

	/* EV 4.12.1
	 * The vehicle must make a characteristic sound, continuously for at least one second and a maximum of three seconds when it enters ready-to-drive mode.
	 */

	sendPDM( 1 ); // buzzer only sounds when value changes from 0 to 1

	if ( GetInverterState( CarState.LeftInvState ) == INVERTERON ) // should be in ready state, so request ON state.
	{
		command = InverterStateMachine( LeftInverter ); // request left inv state machine to On from ready.
		CANSendInverter( command, 0, LeftInverter );
	//	CAN_SendStatus(149,InverterLReceived,0);
	} else
	{
	//	CAN_SendStatus(150,InverterLReceived,0);
	}

	if ( GetInverterState( CarState.RightInvState ) == INVERTERON )
	{
		command = InverterStateMachine( RightInverter );
		CANSendInverter( command, 0, RightInverter );
	//	CAN_SendStatus(149,InverterRReceived,0);
	} else
	{
	//	CAN_SendStatus(150,InverterRReceived,0);
	}
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
		readystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.
		setOutput(RTDMLED_Output,LEDON); // move to ActivateRTDM
		blinkOutput(RTDMLED_Output,LEDON,0);
		allowstop = 0;
		standstill = 0;
        limpcounter = 0;

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


	if  ( !( GetInverterState( CarState.LeftInvState ) == INVERTEROPERATING
		  || GetInverterState( CarState.LeftInvState ) == INVERTERON ) )
	{
		Errors.ErrorPlace = 0xEB;
		return OperationalErrorState;
	}

	if  ( !( GetInverterState( CarState.RightInvState ) == INVERTEROPERATING
		  || GetInverterState( CarState.RightInvState ) == INVERTERON ) )
	{
		Errors.ErrorPlace = 0xEC;
		return OperationalErrorState;
	}

	if ( readystate == 0 &&
		( GetInverterState( CarState.LeftInvState ) != INVERTEROPERATING
		|| GetInverterState( CarState.RightInvState ) != INVERTEROPERATING ) )
	{ // an inverter has changed state from operating after reaching it unexpectedly, fault status of some sort.
		Errors.ErrorPlace = 0xED;
		return OperationalErrorState;
	}
    
    #define RTDMStopTime    3 // 3 seconds.ww

	if ( CarState.SpeedRL < 100  && CarState.SpeedRR < 100 ) // untested
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

		// drop inverter state before switching main state.

		CarState.HighVoltageReady = 0;

		CarState.LeftInvCommand = InverterStateMachine( LeftInverter );

		CANSendInverter( CarState.LeftInvCommand, 0, LeftInverter );

		CarState.RightInvCommand = InverterStateMachine( RightInverter );

		CANSendInverter( CarState.RightInvCommand, 0, RightInverter );

		CANSendInverter( CarState.LeftInvCommand, 0, LeftInverter );
		HAL_Delay(1); // make sure inverters had time to react.
		return IdleState; // check if need to drop HV in a special order.
	}

	if  ( GetInverterState( CarState.LeftInvState ) == INVERTEROPERATING && GetInverterState( CarState.RightInvState ) == INVERTEROPERATING  )
	{
		readystate = 0;
	}

	CarState.Torque_Req_L = 0;  // APPS
        CarState.Torque_Req_R = CarState.Torque_Req_L;

	if ( readystate == 0 ) // only start to request torque when inverters ready.
	{
        // check if limp mode allowed ( don't want for acceleration test ), and if so, if BMS has requested.
        
        if ( CarState.LimpRequest && !CarState.LimpDisable )
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
        if ( CarState.LimpActive && OperationLoops > 200 && CheckRTDMActivationRequest() )
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
        
        // add limp mode cancel potentially
        
        // if allowed, process torque request, else request 0.

		CarState.Torque_Req_L = PedalTorqueRequest();  // APPS
		CarState.Torque_Req_R = CarState.Torque_Req_L;

		if ( CarState.APPSstatus ) setOutput(TSLED_Output,LEDON); else setOutput(TSLED_Output,LEDOFF);

		// Call any additional processing here to alter final request.

//		uint32_t loopstart = gettimer();
//		uint32_t looptimer = 0;

//		send loop time to show point when data processed.

/*		while ( looptimer >= targettime-1 ) // wait till end of 10ms loop.
		{
			looptimer = gettimer() - loopstart;
		}
 */

#ifdef NOTORQUEREQUEST
		CANSendInverter( 0b00001111, 0, LeftInverter );
	    CANSendInverter( 0b00001111, 0 , RightInverter );

#else

		CANSendInverter( 0b00001111, CarState.Torque_Req_L, LeftInverter );
		CANSendInverter( 0b00001111, CarState.Torque_Req_R , RightInverter );
#endif

//		CANSendInverter( InverterStateMachine(LeftInverter), CarState.Torque_Req_L, LeftInverter );
//		CANSendInverter( InverterStateMachine(RightInverter), CarState.Torque_Req_R , RightInverter );
	}

	// if drop status due to error, check if recoverable, or if limp mode possible.

	return RunningState;
}
