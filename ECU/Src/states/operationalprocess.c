/**
  ******************************************************************************
  * @file           : operation.c
  * @brief          : Operational Loop and related function
  ******************************************************************************

  ******************************************************************************
  */

#include "ecumain.h"

/* Private includes ----------------------------------------------------------*/

#ifdef LCD
  #include "vhd44780.h"
#endif

static int LastOperationalState = 0;
static int NewOperationalState = 0;
int OperationalState  = StartupState;
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

	usecanADC = 0;

	CarState.Torque_Req_Max = 0;
	CarState.Torque_Req_CurrentMax = 0;
	CarState.LimpRequest = 0;
	CarState.LimpActive = 0;
    CarState.LimpDisable = 0;
    CarState.PedalProfile = 0;
    CarState.DrivingMode = 0;

    CarState.ActualSpeed = 0;

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
		CAN_SendStatus(1, StartupState, 0);
	}

	lcd_settitle("Startup init");

	// send startup state message here.
	// reset all state information.

	ResetStateData(); // set car state settings back to blank state.

	setOutput(RTDMLED,Off);
	setOutput(TSLED,Off);
	setOutput(TSOFFLED,On);
	blinkOutput(RTDMLED,Off,0); // ensure nothing blinking.
	blinkOutput(TSOFFLED,Off,0);
	blinkOutput(TSLED,Off,0);

	// set relay output LED's off
	setOutput(BMSLED,Off);
	setOutput(IMDLED,Off);
	setOutput(BSPDLED,Off);

	// Show status LED's for 2 seconds for rules compliance.

	blinkOutput(BMSLED,On,2);
	blinkOutput(IMDLED,On,2);
	blinkOutput(BSPDLED,On,2);

#ifdef FRONTSPEED
	CAN_NMT(0x81,FLSpeed_COBID);
	CAN_NMT(0x81,FRSpeed_COBID);
#endif

	ShutdownCircuitSet( false );

	vTaskDelay(5);

	if ( DeviceState.Inverter > OFFLINE )
	{
		return PreOperationalState;
	} else if ( CAN_NMT( 0x81, 0x0 ) ) // sent NMT reset packet to can buses if not received inverter status.
	{ // 0x81 orig, reset.
	 // NMT sent, enter wait and configure state.
		return PreOperationalState;

	} else
	{
	 // error sending NMT
		return OperationalErrorState;
	}

	 return PreOperationalState;
}

uint16_t CheckErrors( void )
{

#ifdef COOLANTSHUTDOWN // coolant limp instead?
	if ( ADCState.CoolantTempR > COOLANTMAXTEMP )
	{
		return 97;
	}
#endif

	if ( errorPower() )
	{
		return 98; // PDM error, stop operation.
	}

	for ( int i=0;i<MOTORCOUNT;i++) // speed isreceived
	{
		// send request to enter operational mode
		// TODO inverters check error.
		//if ( !CarState.TestHV && CarState.Inverters[i].InvState == INERROR )
		{
		//	return 99; // serious error, no operation allowed. -- inverter
		}

	}

	// inverter emergency message has been sent, halt.
/*	if ( CanState.InverterLERR.newdata == 1 || CanState.InverterRERR.newdata == 1 )
	{
		return 99;
	} */

	// check voltage/

/*	if ( CheckADCSanity() != 0 )
	{
		return 1;
	} */

	// BMS voltage, check against IVT, error if too different, calculation.

	return 0; // no errors.
}


int CheckCanError( void )
{
	int result = 0;

	FDCAN_ProtocolStatusTypeDef CAN1Status, CAN2Status;

	HAL_FDCAN_GetProtocolStatus(&hfdcan1, &CAN1Status);
	HAL_FDCAN_GetProtocolStatus(&hfdcan2, &CAN2Status);


	static uint8_t offcan1 = 0;
#ifndef ONECAN
	static uint8_t offcan2 = 0;
#endif
#ifdef RECOVERCAN
	static uint32_t offcan1time = 0;
#ifndef ONECAN
	static uint32_t offcan2time = 0;
#endif
#endif

	if ( CAN1Status.BusOff) // detect passive error instead and try to stay off bus till clears?
	{
	//	Errors.ErrorPlace = 0xAA;
		  blinkOutput(TSOFFLED, LEDBLINK_FOUR, 1);
		  HAL_FDCAN_Stop(&hfdcan1);
		  CAN_SendStatus(255,0,0);

		  if ( offcan1 == 0 )
		  {
#ifdef RECOVERCAN
			  offcan1time = gettimer();
#endif
			  offcan1 = 1;
			  DeviceState.CAN1 = OFFLINE;
//					  offcan1count++; // increment occurances of coming off bus. if reach threshhold, go to error state.
		  }
		  Errors.ErrorPlace = 0xF1;
		 LogError("CANBUS1 Down");
//		  result +=1;
	}

#ifdef RECOVERCAN
	if ( CAN1Status.BusOff && offcan1time+1000 >  gettimer() )
	{

		if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) // add a 5 cycle counter before try again? check in can1 send to disable whilst bus not active.
		{
		// Start Error
			 LogError("CANBUS1 Offline");
			 Errors.ErrorPlace = 0xF2;
			 result +=2;
		} else
		{
			offcan1 = 0;
			Errors.ErrorPlace = 0xF1;
			LogError("CANBUS1 Up");
			DeviceState.CAN1 = OPERATIONAL;
			CAN_SendStatus(254,0,0);
		}
	}
#endif

/*			if ( HAL_FDCAN_IsRestrictedOperationMode(&hfdcan1) )
	{
		Errors.ErrorPlace = 0xF2;
		NewOperationalState = OperationalErrorState;
	} */
#ifndef ONECAN
	if ( CAN2Status.BusOff) // detect passive error instead and try to stay off bus till clears?
	{
	//	Errors.ErrorPlace = 0xAA;
		  blinkOutput(BMSLED, LEDBLINK_FOUR, 1);
		  HAL_FDCAN_Stop(&hfdcan2);
		  CAN_SendStatus(255,0,0);
		  DeviceState.CAN2 = OFFLINE;

		  if ( offcan2 == 0 )
		  {
#ifdef RECOVERCAN
			  offcan2time = gettimer();
#endif
			  offcan2 = 1;
		  }
		  Errors.ErrorPlace = 0xF3;
		  LogError("CANBUS2 Down");
//		  result +=4;
	}

#ifdef RECOVERCAN
	if ( CAN2Status.BusOff && offcan2time+5000 >  gettimer() )
	{
		if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK) // add a 5 cycle counter before try again? check in can1 send to disable whilst bus not active.
		{
		// Start Error
			 Errors.ErrorPlace = 0xF4;
			 LogError("CANBUS2 Offline");
			 result +=8;
		} else
		{
			offcan2 = 0;
			LogError("CANBUS1 Up");
			DeviceState.CAN2 = OPERATIONAL;
			CAN_SendStatus(254,0,0);
		}
	}
#endif
#endif

/*			if ( HAL_FDCAN_IsRestrictedOperationMode(&hfdcan2) )
	{
		Errors.ErrorPlace = 0xF4;
		NewOperationalState = OperationalErrorState;
	}
*/
	return result;

}


int LimpProcess( uint32_t OperationLoops  )
{
	lcd_settitle("LimpProcess(NA)");
	CAN_SendStatus(1, LimpState, 0 );
	setRunningPower( true, false );
	return LimpState;
}

int TestingProcess( uint32_t OperationLoops  )
{
	lcd_settitle("TestingProcess(NA)");
	CAN_SendStatus(1, TestingState, 0 );
	setRunningPower( true, false );
	return TestingState;
}



int OperationalProcess( void )
{
	// initialise loop timer variable.
	uint32_t looptimer = gettimer(); // was volatile, doesn't need to be

	static uint16_t loopoverrun = 0;

	cancount = 0;

	CAN_NMTSyncRequest();

	if ( NewOperationalState != OperationalState ) // state has changed.
	{
		LastOperationalState = OperationalState;
		OperationalState = NewOperationalState;
		loopcount = 0;
		clearButtons(); // don't let any user input pass between states
		loopoverrun = 0; // reset over run counter.
	}

	uint32_t currenttimer = gettimer();

	//calculate right delay to wait for loop.

//		vTaskDelayUntil( &xLastCycleTime, MS1*10 );

//		if( looptimer + PROCESSLOOPTIME < currenttimer ) // once per 10ms second process state
	{
		// check how much past 10ms timer is, if too far, soft error. Allow a few times, but not too many before entering an error state?
		int lastlooplength = currenttimer-looptimer;

		looptimer = gettimer(); // start timing loop


		if ( lastlooplength > PROCESSLOOPTIME*1.1 )
		{
			CAN_SendStatus(1, OperationalStateOverrun, lastlooplength);

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
				LastOperationalState = OperationalState;
				OperationalState = NewOperationalState;
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
				NewOperationalState = RunningProcess(loopcount, looptimer + PROCESSLOOPTIME );
				break;

			case TestingState : // testing state, can only enter from state 1.
				NewOperationalState = TestingProcess(loopcount);
				break;

			case LimpState : // limping state, allow car to operate with limited input/motor power in slow mode.
				NewOperationalState = LimpProcess(loopcount);
				break;

			case OperationalErrorState : // critical error or unknown state.					CAN_SendStatus(1, OperationalState, OperationalErrorState);
				NewOperationalState = OperationalErrorHandler( loopcount );
				break;

			default : // 99
				NewOperationalState = OperationalErrorHandler( loopcount );
			//	return FatalErrorState; // error state, quit state machine.
		}


		if ( CheckCanError() )
		{
			NewOperationalState = OperationalErrorState;
		}

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
			if ( Errors.OperationalReceiveError == 0) CAN_SendADC(ADC_Data, 0);
#endif
			if ( DeviceState.LoggingEnabled ) CANLogDataSlow();
		}
//		clearButtons();
		loopcount++;
		totalloopcount++;
	}

	return 0;
}
