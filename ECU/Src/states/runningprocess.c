/*
 * operationreadyness.c
 *
 *  Created on: 13 Apr 2019
 *      Author: Visa
 */

#include "ecumain.h"
#include "operationalprocess.h"
#include "idleprocess.h"
#include "errors.h"
#include "power.h"
#include "timerecu.h"
#include "brake.h"

#include "torquecontrol.h"
#include "adcecu.h"
#include "input.h"
#include "output.h"
#include "lcd.h"
#include "inverter.h"
#include "debug.h"

#include "imu.h"

uint16_t PrintRunning( char *title )
{
	char str[80] = "";

	sprintf(str, "%-6s %8s %3liv", title, getCurTimeStr(), CarState.VoltageBMS); // lcd_geterrors());

	lcd_send_stringline(0,str, 255);

	int Torque = ADCState.Torque_Req_R_Percent/10;
	if ( Torque > 99 ) Torque = 99;

	sprintf(str,"%2linm(%2d%%) APPS:%c%c",
			(int16_t)CarState.Torque_Req, // *1000+15384-1)/15384,
			Torque, //CarState.Torque_Req,
			(CarState.APPSstatus > 0)?'_':'A',
			getBrakeHigh() ?' ':'H');
	lcd_send_stringline(1,str, 255);

	int32_t ActualSpeed = IMUReceived.VelBodyX*0.01*3.6;
	
	//InverterGetSpeed();

	char angdir = ' ';

	if ( ADCState.SteeringAngle < 0) angdir = 'L';
	if ( ADCState.SteeringAngle > 0) angdir = 'R';

	sprintf(str,"Spd:%3likmh Ang:%3d%c", (ActualSpeed), abs(ADCState.SteeringAngle), angdir);
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
#ifdef STMADC
		if ( DeviceState.ADC == OPERATIONAL )
#endif
			sprintf(str, "Bal: Press Brakes");
#ifdef STMADC
		else
			sprintf(str, "Bal: No Data");
#endif
	}

	lcd_send_stringline(0,str, 255);
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
	static uint32_t nextmsg = 0;

	if ( OperationLoops == 0) // reset state on entering/rentering.
	{
		CarState.AllowRegen = getEEPROMBlock(0)->Regen > 0;
		DebugMsg("Entering RTDM State");
/* EV 4.12.1
	* The vehicle must make a characteristic sound, continuously for at least one second and a maximum of three seconds when it enters ready-to-drive mode.
*/
		// send buzzer as entering RTDM
		soundBuzzer();
		lcd_clear();
		readystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.
		resetOutput(RTDMLED,On); // move to ActivateRTDM
		resetOutput(STARTLED,Off);

		allowstop = 0;
		standstill = 0;
        limpcounter = 0;
		InverterAllowTorqueAll(true);

    	if ( CarState.LimpRequest )
    		 CarState.Torque_Req_CurrentMax = LIMPNM;

	}
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, RunningState, readystate );
	}

	uint32_t curtick = gettimer();

	PrintRunning("RTDM:TA");

	invRequestState( OPERATIONAL );

	if ( CheckCriticalError() )
	{
		Errors.ErrorReason = ReceivedCriticalError | ( CheckCriticalError() << 8 );
		Errors.ErrorPlace = 0xE0;
		return OperationalErrorState;
	}

	uint32_t received = OperationalReceive();

	// check data validity, // any critical errors, drop state.

	if  ( !( GetInverterState() == OPERATIONAL
		  || GetInverterState() == PREOPERATIONAL ) )
	{
		Errors.ErrorPlace = 0xE1;
		return OperationalErrorState;
	}

	if ( CheckHVLost() )
	{
		Errors.ErrorReason = HVlostError;
		Errors.ErrorPlace = 0xE2;
		return OperationalErrorState;
	}

	if ( readystate == 0 && GetInverterState() != OPERATIONAL )
	{  // an inverter has changed state from operating after reaching it unexpectedly, fault status of some sort.
		Errors.ErrorPlace = 0xE5;
		return OperationalErrorState;
	}

	bool moving = false;

	if ( InverterGetSpeed() > 100 ) moving = true;

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



	if ( ADCState.Regen_Percent < (REGENMINIMUM * 10) && CheckRTDMActivationRequest() )
	{
		blinkOutput(RTDMLED,BlinkFast,1000);
		lcd_send_stringline( 3, "Disalowing regen", 3);
		DebugMsg("Disalowing regen");
		CarState.AllowRegen = false;
	}


	if ( allowstop && CheckActivationRequest() ) // if requested disable Tractive System, drop state. This should shut off torque control immiedietly.
	{
		return IdleState; // check if need to drop HV in a special order.
	}

//	if ( invertersStateCheck(OPERATIONAL) ) // returns true if all inverters match state
	{
        // check if limp mode allowed ( don't want for acceleration test ), and if so, if BMS has requested.

        if ( ( CarState.LimpRequest && !CarState.LimpDisable ) || ADCState.CoolantTempR > COOLANTLIMPTEMP )
        {
            CarState.LimpActive = 1;
			CarState.LimpNM = LIMPNM;
        }

		if ( !CarState.LimpActive && ADCState.CoolantTempR > COOLANTLIMPTEMPHALF )
		{
			CarState.LimpActive = 2;
			CarState.LimpNM = CarState.Torque_Req_Max / 2;

			if ( CarState.LimpNM < LIMPNM )
				CarState.LimpNM = LIMPNM;
		}

        if ( CarState.LimpActive && CarState.Torque_Req_CurrentMax > CarState.LimpNM  )
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

        //CarState.Torque_Req = PedalTorqueRequest();  // calculate request from APPS

		int16_t pedalreq;
		float torque_req = PedalTorqueRequest(&pedalreq);

#ifdef TORQUEVECTOR

// first check if we've requested toggling state and toggle it if accelerator pedal not pressed.
		if ( CheckTSActivationRequest() && ADCState.Torque_Req_R_Percent < 5 && ADCState.Torque_Req_L_Percent < 5 ) // toggle torque vectoring with TS start button.
		{
			if ( CarState.TorqueVectoring ) CarState.TorqueVectoring = 0; else CarState.TorqueVectoring = 1;
		}

		if ( CarState.TorqueVectoring ) // handle indicator of active state.
		{
			//blinkOutput(TSOFFLED_Output,LEDBLINK_ONE,LEDBLINKNONSTOP); // blinking green led = vectoring on.
		} else
		{
	//		setOutput(TSOFFLED_Output,LEDON); // steady LED = not enabled
			//blinkOutput(TSOFFLED_Output,LEDON,LEDBLINKNONSTOP);
		}
#endif

		vectoradjust adj = { 0 };
		speedadjust spd = { 0 };
#ifdef REQUIRETS
		if ( CheckTSOff() || Shutdown.AIRp )
		{
			CarState.Torque_Req = 0;
		}
#endif

        if ( CarState.AllowRegen && CarState.LimpActive == 0 ) // only check for regen if alowed and not in limp.
        {
    		if ( ADCState.Regen_Percent > (REGENMINIMUM*10) && getEEPROMBlock(0)->Regen>0 ) // no torque request, but we do have a regen request, return that.
    		{
				//if ( getEEPROMBlock(0)->Regen == 2 )
				//	doRegen(ADCState.Regen_Percent, ADCState.SteeringAngle, &adj);
				//else
				{
					torque_req = - ( ( ( getEEPROMBlock(0)->regenMax * ADCState.Regen_Percent ) ) / 1000 ) ;
				}
				if ( torque_req < 0)
					CarState.RegenLight = true;
    		} else
				CarState.RegenLight = false;
        } else
			CarState.RegenLight = false;

		if ( curtick > nextmsg )
		{
			nextmsg = curtick + 1000;
			DebugPrintf("Current req %f pedals %lu %lu %lu brakes %lu %lu", torque_req, ADCState.Torque_Req_R_Percent, ADCState.Torque_Req_L_Percent, ADCState.Regen_Percent, ADCState.BrakeF, ADCState.BrakeR );
		}

		// doVectoring sets the final torque request value for state.
		doVectoring( torque_req, &adj, &spd, pedalreq ); // process vectoring after getting positive or negative request.

		if ( CarState.APPSstatus )
			setOutput(TSLED,On);
		else
			setOutput(TSLED,Off);

		int8_t inttorque = torque_req;
		int8_t adjtorque = adj.RL;

		uint8_t CANTxData[8] = {
				ADCState.Torque_Req_L_Percent / 10, ADCState.Regen_Percent / 10,
				CarState.LimpActive << 1 | CarState.AllowRegen, getEEPROMBlock(0)->Regen,
				ADCState.BrakeF, ADCState.BrakeR,
				inttorque, adjtorque
		};

		CAN1Send( 0x7D2, 8, CANTxData );

		InverterSetTorque(&adj, &spd);
	}

	// if drop status due to error, check if recoverable, or if limp mode possible.

	return RunningState;
}
