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

bool CheckHV = false;

uint32_t OperationalReceive( void )
{
	uint32_t returnvalue = 0;
/*	if (returnvalue == 0xFF)
	{ returnvalue =
#ifdef HPF19
			(0x1 << FLeftSpeedReceived) + // initialise return value to all devices in error state ( bit set )
				  (0x1 << FRightSpeedReceived) +
#endif
				  (0x1 << BMSReceived)+
#ifndef POWERNODES
				  (0x1 << PDMReceived)+
#endif
				  (0x1 << InverterReceived)+ // TODO inverter receive
				  (0x1 << PedalADCReceived);
#ifdef HPF20

//				  (0x1 << InverterLReceived)+
//				  (0x1 << InverterRReceived);
#endif
	}
	*/

	// change order, get status from pdo3, and then compare against pdo2?, 2 should be more current being higher priority

	int invcount = 0;
	for ( int i=0;i<MOTORCOUNT; i++)
	{
		if ( getInvState(i)->Device == OFFLINE )
		{
			invcount++;
		}
	}

	if ( invcount == MOTORCOUNT )// GetInverterState() > OFFLINE )
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
//	if ( receiveBMS() )
		returnvalue &= ~(0x1 << BMSReceived);

//	 receiveIVT();
	// if ( receiveIVT() )
		returnvalue &= ~(0x1 << IVTReceived); // assume IVT is present, don't go to error state if missing.

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
		DebugMsg("Entering Idle State");
							 //12345678901234567890
		setRunningPower( CheckHV, false );
		invRequestState( PREOPERATIONAL ); // request to go into ready for HV

		lcd_clear();
		//lcd_settitle("Ready to activate TS");
		InverterAllowTorqueAll(false);

		CheckHV = false;
		HVEnableTimer = 0;
		TSRequested = 0;
		setOutput(RTDMLED, Off);
		blinkOutput(RTDMLED, Off, 0);
	}
	readystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, IdleState, readystate );
	}

	PrintRunning("Ready");

	//readystate = OperationalReceiveLoop();


//		vTaskDelay(5);

	uint32_t received = OperationalReceive();

	// check what not received here, only error for inverters

	if ( received != 0 ) // not all expected data received in window.
	{
		DebugMsg("Errorplace 0x9A not received data");
		CAN_SendStatus(1, OperationalState, received);
		Errors.ErrorPlace = 0x9A;
		Errors.OperationalReceiveError = received;
		Errors.State = OperationalState;
		return OperationalErrorState;
	}

	if ( CheckCriticalError() )
	{
		DebugMsg("Errorplace 0xCA Critical error.");
		Errors.ErrorPlace = 0xCA;
		Errors.ErrorReason = 0;
		return OperationalErrorState;
	}

	// at this state, everything is ready to be powered up.

	if ( invertersStateCheck(STOPPED) // returns true if all inverters match state
	  && CarState.VoltageBMS > MINHV
#ifdef IVTEnable
	#ifndef NOTSAL
	  && CarState.VoltageINV > -5 // 18
	#endif
#endif
#ifdef SHUTDOWNSWITCHCHECK
	  && CheckShutdown() // only allow TS enabling if shutdown switches are all closed, as it would otherwise fail
#endif
	  ) // minimum accumulator voltage to allow TS, set a little above BMS limit, so we can
	{
		readystate = 0;
		if ( !TSRequested )
		{
			blinkOutput(TSLED,LEDBLINK_FOUR,LEDBLINKNONSTOP); // start blinking to indicate ready.
		}
	}

#ifdef SETDRIVEMODEINIDLE
	setCurConfig();
#endif

// allow APPS checking before RTDM
	vectoradjust adj;

	doVectoring( CarState.Torque_Req, &adj );

	if ( CarState.APPSstatus ) setOutput(TSLED,On); else setOutput(TSLED,Off);

	InverterSetTorque(&adj, 0);

	uint8_t InvHVPresent = 0;

	if ( DeviceState.IVTEnabled )
	{
		if ( CarState.VoltageINV > 60 ) InvHVPresent = 1;
	} else InvHVPresent = 1; // just assume HV present after request if IVT not enabled.

	if ( readystate == 0 && TSRequested && CarState.VoltageBMS > MINHV && InvHVPresent )
	{
		return TSActiveState;
	}

	if ( TSRequested == 1
		&& HVEnableTimer+MS1000*9 < gettimer()
		&& !InvHVPresent // make this optional so IVT can be disabled.
		)
	{
        // error enabling high voltage, stop trying and alert.
		CheckHV = false;
//		CarState.HighVoltageReady = 0;
//		blinkOutput(TSLED_Output,LEDBLINK_FOUR,1);
		TSRequested = 0;

		// SHOW ERROR.

		if ( CheckShutdown() )
		{
			lcd_send_stringline(1,"Error Activating TS", 255);
			lcd_send_stringline(2,"Check TSMS & HVD.", 255);
		}
	}

	if ( readystate == 0 && CheckTSActivationRequest() )
	{
		TSRequested = 1;
		HVEnableTimer = gettimer();
		CheckHV=true;
//		CarState.HighVoltageReady = 1; // start timer, go to error state after 1am
	} else
	{

	}

	return IdleState;
}
