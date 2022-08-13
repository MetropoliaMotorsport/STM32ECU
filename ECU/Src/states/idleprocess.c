/*
 * operationreadyness.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */


#include "ecumain.h"
#include "idleprocess.h"
#include "runningprocess.h"
#include "operationalprocess.h"
#include "configuration.h"
#include "errors.h"
#include "input.h"
#include "inverter.h"
#include "timerecu.h"
#include "lcd.h"
#include "output.h"
#include "power.h"
#include "debug.h"
#include "adcecu.h"

uint32_t OperationalReceive( void )
{
	uint32_t returnvalue = 0;
	if (returnvalue == 0xFF)
	{ returnvalue =
#ifdef HPF19
			(0x1 << FLeftSpeedReceived) + // initialise return value to all devices in error state ( bit set )
				  (0x1 << FRightSpeedReceived) +
#endif
				  (0x1 << BMSReceived)+
				  (0x1 << IVTReceived)+
#ifndef POWERNODES
				  (0x1 << PDMReceived)+
#endif
				  (0x1 << InverterReceived)+ // TODO inverter receive
				  (0x1 << PedalADCReceived);
#ifdef HPF20
#endif
	}


	// check all inverters are present.
	int invcount = 0;
	for ( int i=0;i<MOTORCOUNT; i++)
	{
		if ( getInvState(i)->Device == OFFLINE )
		{
			invcount++;
		}
	}

	if ( invcount == MOTORCOUNT )
		returnvalue &= ~(0x1 << (InverterReceived));

#ifdef HPF19
	if (DeviceState.FrontSpeedSensors == ENABLED )
	{
		if ( receiveSick(FLSpeed_COBID) ) returnvalue &= ~(0x1 << FLeftSpeedReceived);
		if ( receiveSick(FRSpeed_COBID) ) returnvalue &= ~(0x1 << FRightSpeedReceived);
	} else
	{
		returnvalue &= ~(0x1 << FLeftSpeedReceived);
		returnvalue &= ~(0x1 << FRightSpeedReceived);
	}
#endif

#ifndef POWERNODES
	if ( receivePDM() ) returnvalue &= ~(0x1 << PDMReceived);
#endif
	if ( DeviceState.BMS == OPERATIONAL )
		returnvalue &= ~(0x1 << BMSReceived);

	if ( DeviceState.IVT == OPERATIONAL )
		returnvalue &= ~(0x1 << IVTReceived);

		// need new function to check for ADC input, so that more workable with a CAN node.
    if ( DeviceState.ADCSanity == 0 ) returnvalue &= ~(0x1 << PedalADCReceived); // change this to just indicate ADC received in some form.

    return returnvalue;
}


int IdleProcess( uint32_t OperationLoops ) // idle, inverters on.
{
	static uint16_t readystate;
	static uint8_t TSRequested;

	static uint32_t HVEnableTimer;

	// request ready states from devices.

	if ( OperationLoops == 0) // reset state on entering/rentering.
	{
		readystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.
		DebugMsg("Entering Idle State");
		ShutdownCircuitSet( false );
							 //12345678901234567890
		invRequestState( BOOTUP ); // request to go into ready for HV

	    resetOutput(STARTLED, Off);
	    resetOutput(RTDMLED, Off);

		lcd_clear();
		InverterAllowTorqueAll(false);

		HVEnableTimer = 0;
		TSRequested = 0;
	}
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, IdleState, readystate );
	}


	if ( CarState.allowtsactivation )
		PrintRunning("TS:Off");
	else
		PrintRunning("TS:BAD");

	uint32_t received = OperationalReceive();

	// check what not received here, only error for inverters

	if ( received != 0 ) // not all expected data received in window.
	{
		DebugMsg("Errorplace 0x9A not received data");
		CAN_SendErrorStatus(1, OperationalState, received);
		Errors.ErrorPlace = 0x9A;
		Errors.OperationalReceiveError = received;
		Errors.State = OperationalState;
		return OperationalErrorState;
	}

	if ( CheckCriticalError() )
	{
		DebugMsg("Errorplace 0xCA Critical error.");
		Errors.ErrorPlace = 0xCA;
		Errors.ErrorReason = ReceivedCriticalError;
		return OperationalErrorState;
	}

	// at this state, everything is ready to be powered up.

	int invcount = 0;
	for ( int i=0;i<MOTORCOUNT; i++)
	{
		if ( getInvState(i)->Device != OFFLINE )
		{
			invcount++;
		}
	}

	if ( invcount == MOTORCOUNT // invertersStateCheck(STOPPED) // returns true if all inverters match state
	  && CarState.VoltageBMS > MINHV
#ifdef IVTEnable
	#ifndef NOTSAL
	  && CarState.VoltageINV > -5 // 18
	#endif
#endif
#ifdef SHUTDOWNSWITCHCHECK
//	  && CheckShutdown() // only allow TS enabling if shutdown switches are all closed, as it would otherwise fail
#endif
	  ) // minimum accumulator voltage to allow TS, set a little above BMS limit, so we can
	{
		static bool first = false;

		if ( !first )
		{
			first = true;
			DebugMsg("Ready to enable TS");
		}

		readystate = 0;
		if ( !TSRequested )
		{
			blinkOutput(TSLED,LEDBLINK_FOUR,LEDBLINKNONSTOP); // start blinking to indicate ready.
		}
	} else
	{

	}

#ifdef SETDRIVEMODEINIDLE
	setCurConfig();
#endif

	float lastreq = CarState.Torque_Req;

    CarState.Torque_Req = PedalTorqueRequest();  // calculate request from APPS

    if ( abs(lastreq-CarState.Torque_Req) > 10 )
    {
    	DebugPrintf("Torquereq %d for Curve adj %d Act %d\r\n", CarState.Torque_Req , getTorqueReqCurve(ADCState.Torque_Req_R_Percent), ADCState.Torque_Req_R_Percent );
    }

// allow APPS checking before RTDM
	vectoradjust adj;
	speedadjust spd;

	doVectoring( CarState.Torque_Req, &adj, &spd );

	if ( CarState.APPSstatus ) setOutput(TSLED,On); else setOutput(TSLED,Off);

	InverterSetTorque(&adj, 0);

	if ( readystate == 0 )
	{
		blinkOutput(TSLED, LEDBLINK_ONE, 1000);
	} else
	{
		setOutput(TSLED, On);
	}

	if ( CheckTSActivationRequest() )
	{
		if ( readystate == 0 && CarState.allowtsactivation)
		{
			DebugMsg("TS Activation requested whilst ready.");
			TSRequested = 1;
			HVEnableTimer = gettimer();
			return TSActiveState;
//		CarState.HighVoltageReady = 1; // start timer, go to error state after 1am
		} else
		{
			DebugMsg("TS Activation requested whilst not ready.");
		}
	}

	return IdleState;
}
