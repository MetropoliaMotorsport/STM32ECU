/**
  ******************************************************************************
  * @file           : operation.c
  * @brief          : Operational Loop and related function
  ******************************************************************************

  ******************************************************************************
  */

#include "ecumain.h"
#include "operationalprocess.h"
#include "idleprocess.h"
#include "runningprocess.h"
#include "tsactiveprocess.h"
#include "preoperation.h"
#include "operationalreadyness.h"
#include "errors.h"
#include "lcd.h"
#include "input.h"
#include "output.h"
#include "timerecu.h"
#include "power.h"
#include "inverter.h"
#include "debug.h"

static int LastOperationalState = 0;
static int NewOperationalState = 0;
int OperationalState = StartupState;
uint32_t loopcount = 0;
uint32_t totalloopcount = 0;

//	The driver must be able to activate and deactivate the TS, see EV 4.10.2 and EV 4.10.3,
//	from within the cockpit without the assistance of any other person. - deactivation not handled atm?

//	Closing the shutdown circuit by any part defined in EV 6.1.2 must not (re-)activate the TS.
//	Additional action must be required.

// detect shutdown, move back to pre operation state


ResetCommand * ResetCommands[64] = { NULL };

uint32_t ResetCount = 0;

int RegisterResetCommand( ResetCommand * Handler )
{
	if ( Handler != NULL )
	{
		ResetCommands[ResetCount] = Handler;
		ResetCount++;
		return 0;
	} else return 1;
}

void ResetStateData( void ) // set default startup values for global state values.
{
	for ( int i=0;i<ResetCount;i++)
	{
		if ( ResetCommands[i] != NULL )
				(*ResetCommands[i])();
	}

	DeviceState.timeout = false;

#ifdef FANCONTROL
	CarState.FanPowered = 0;
#else
	CarState.FanPowered = 1;
#endif

#ifdef FRONTSPEED
	DeviceState.FrontSpeedSensors = ENABLED;
	DeviceState.FLSpeed = OFFLINE;
	DeviceState.FRSpeed = OFFLINE;
#else
	DeviceState.FrontSpeedSensors = DISABLED;
	DeviceState.FLSpeed = OPERATIONAL;
	DeviceState.FRSpeed = OPERATIONAL;
#endif

#ifdef LOGGINGON
	DeviceState.LoggingEnabled = ENABLED;
#else
	DeviceState.LoggingEnabled = DISABLED;
#endif

	DeviceState.ADC = OFFLINE;

	CarState.Torque_Req_Max = 0;
	CarState.Torque_Req_CurrentMax = 0;
	CarState.LimpRequest = 0;
	CarState.LimpActive = 0;
    CarState.LimpDisable = 0;
    CarState.PedalProfile = 0;
    CarState.DrivingMode = 0;
    CarState.AllowRegen = false;

	Errors.ErrorPlace = 0;
	Errors.ErrorReason = 0;
	Errors.CANSendError1 = 0;
	Errors.CANSendError2 = 0;
	Errors.ADCSent = false;
}


int Startup( uint32_t OperationLoops  )
{
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		//CarState.allowtsactivation = true;
		DebugMsg("Entering Startup State");
		CAN_SendStatus(1, StartupState, 0);
	}

	lcd_settitle("Startup init");

	// send startup state message here.
	// reset all state information.

	ResetStateData(); // set car state settings back to blank state.

	resetOutput(RTDMLED, Off);
	resetOutput(TSOFFLED, Off);
	resetOutput(TSLED, Off);
	resetOutput(LED6, Off);

	// set relay output LED's off
	//setOutput(BMSLED,Off);
	setOutput(IMDLED,Off);
	setOutput(BSPDLED,Off);

	// Show status LED's for 2 seconds for rules compliance.

	//timeOutput(BMSLED, 2000);
	//timeOutput(IMDLED, 2000);
	//timeOutput(BSPDLED, 2000);

#ifdef FRONTSPEED
	CAN_NMT(0x81,FLSpeed_COBID);
	CAN_NMT(0x81,FRSpeed_COBID);
#endif

	vTaskDelay(5);

	return PreOperationalState;
}


int LimpProcess( uint32_t OperationLoops  )
{
	lcd_settitle("LimpProcess(NA)");
	CAN_SendStatus(1, LimpState, 0 );
	return LimpState;
}

int TestingProcess( uint32_t OperationLoops  )
{
	lcd_settitle("TestingProcess(NA)");
	CAN_SendStatus(1, TestingState, 0 );
	return TestingState;
}

int OperationalProcess( void )
{
	// initialise loop timer variable.
	uint32_t looptimer = gettimer(); // was volatile, doesn't need to be

	static uint16_t loopoverrun = 0;

	cancount = 0;

	if ( NewOperationalState != OperationalState ) // state has changed.
	{
		LastOperationalState = OperationalState;
		OperationalState = NewOperationalState;
		loopcount = 0;
		clearButtons(); // don't let any user input pass between states
		loopoverrun = 0; // reset over run counter.
	}

	 DeviceState.LoggingEnabled = true;

	uint32_t currenttimer = gettimer();

	// check how much past 10ms timer is, if too far, soft error. Allow a few times, but not too many before entering an error state?
	int lastlooplength = currenttimer-looptimer;

	looptimer = gettimer(); // start timing loop

	// check loop timing.
	if ( lastlooplength > CYCLETIME*1.1 )
	{
		CAN_SendErrorStatus(1, OperationalStateOverrun, lastlooplength);

		loopoverrun++; // bms

		if ( ( loopcount % 100 ) == 0) // allow one loop overrun every 100 by decrementing overrun counter if over 0
		{
		   if ( loopoverrun > 0 )
		   {
			   loopoverrun--;
		   }
		}

		if ( loopoverrun > 10 )
		{
			// if too many overruns,  do something?
		}
	}

	switch ( OperationalState )
	{
		case StartupState : // NMT, initial startup.
			NewOperationalState = Startup(loopcount); // run NMT state machine till got responses.
			break;

		case PreOperationalState : // pre operation - configuration, wait for device presence announcements in pre operation state.
			NewOperationalState = PreOperationState(loopcount);
			break;

		case OperationalReadyState : // operation has been requested, get all devices to operational ready state and check sanity.
			NewOperationalState = OperationReadyness(loopcount);
			break;

		case IdleState:  // idle, inverters on. Ready to enter TS, everything should be ready to go at this stage.
			NewOperationalState = IdleProcess(loopcount);
			break;

		case TSActiveState : // TS active state OperationalState, 0); // can return to state 3 ( stop button ) or go to 5
			NewOperationalState = TSActiveProcess(loopcount);
			break;

		case RunningState : // Running    // can return to state 3
			NewOperationalState = RunningProcess(loopcount, looptimer + CYCLETIME );
			break;

		case TestingState : // testing state, can only enter from state 1.
			NewOperationalState = TestingProcess(loopcount);
			break;

		case LimpState : // limping state, allow car to operate with limited input/motor power in slow mode.
			NewOperationalState = LimpProcess(loopcount);
			break;

		case OperationalErrorState : // critical error or unknown state.
			CAN_SendStatus(1, OperationalState, OperationalErrorState);
		default : // unknown state, assume it's an error and go into error
			NewOperationalState = OperationalErrorHandler( loopcount );
			break;
	}

#ifndef BENCH
	if ( CheckCanError() )
	{
		NewOperationalState = OperationalErrorState;
	}
#endif

#ifndef everyloop
	if ( ( loopcount % LOGLOOPCOUNTFAST ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		if ( DeviceState.LoggingEnabled ) CANLogDataFast();
	}

	if ( ( loopcount % LOGLOOPCOUNTSLOW ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
	{
		CAN_SendLED(); // send LED statuses for debug, make toggleable.
#ifndef ANALOGNODES
		// no point in sending raw adc state if using nodes.
		if ( Errors.OperationalReceiveError == 0) CAN_SendADC(ADC_Data, 0);
#endif
		if ( DeviceState.LoggingEnabled ) CANLogDataSlow();
	}

	loopcount++;
	totalloopcount++;

	return 0;
}
