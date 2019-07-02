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

void ResetStateData( void ) // set default startup values for global state values.
{
	DeviceState.CAN1 = OPERATIONAL;
	DeviceState.CAN2 = OPERATIONAL;

	Errors.LeftInvAllowReset = 1;
    Errors.RightInvAllowReset = 1;
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

#ifdef IVTEnable
	DeviceState.IVTEnabled = ENABLED;
#else
	DeviceState.IVTEnabled = DISABLED;
#endif

#ifdef BMSEnable
	DeviceState.BMSEnabled = ENABLED;
#else
	DeviceState.BMSEnabled = DISABLED;
#endif

	DeviceState.ADC = OFFLINE;
	DeviceState.InverterL = OFFLINE;
	DeviceState.InverterR = OFFLINE;
	DeviceState.BMS = OFFLINE;
	DeviceState.PDM = OFFLINE;

	CarState.BMS_relay_status = 0; // these are latched
	CarState.IMD_relay_status = 0;
	CarState.BSPD_relay_status = 0;

	CarState.AIROpen = 0;
	CarState.ShutdownSwitchesClosed = 1;

	CarState.brake_balance = 0;

	usecanADC = 0;

	CarState.HighVoltageAllowedR = 0;
	CarState.HighVoltageAllowedL = 0;

	CarState.HighVoltageReady = 0;

	CarState.LeftInvState = 0xFF;
	CarState.RightInvState = 0xFF;
	CarState.LeftInvStateCheck = 0xFF;
	CarState.RightInvStateCheck = 0xFF;
	CarState.LeftInvStateCheck3 = 0xFF;
	CarState.RightInvStateCheck3 = 0xFF;
	CarState.LeftInvBadStatus = 1;
	CarState.RightInvBadStatus = 1;

	CanState.InverterLERR.time = 0;
	CanState.InverterRERR.time = 0;
	CanState.InverterLPDO1.time = 0;
	CanState.InverterRPDO1.time = 0;

	CanState.InverterLPDO2.time = 0;
	CanState.InverterRPDO2.time = 0;

	CanState.InverterLPDO3.time = 0;
	CanState.InverterRPDO3.time = 0;

	CarState.Torque_Req_Max = 0;
	CarState.Torque_Req_CurrentMax = 0;
	CarState.LimpRequest = 0;
	CarState.LimpActive = 0;
    CarState.LimpDisable = 0;
    


	CarState.SpeedRL = 0;
	CarState.SpeedRR = 0;
	CarState.SpeedFL = 0;
	CarState.SpeedFR = 0;
	CarState.SpeedRR = 0;
	CarState.SpeedFL = 0;
	CarState.SpeedFR = 0;
	CarState.Torque_Req_L = 0;
	CarState.Torque_Req_R = 0;

	Errors.InverterError = 0; // reset logged errors.
	Errors.ErrorPlace = 0;
	Errors.ErrorReason = 0;
	Errors.CANSendError1 = 0;
	Errors.CANSendError2 = 0;

}


int Startup( uint32_t OperationLoops  )
{
	static uint16_t readystate;
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, StartupState, readystate);
	}

	if ( OperationLoops == 0) // reset state on entering/rentering.
	{
		readystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.

		// send startup state message here.
		// reset all state information.

		ResetStateData(); // set car state settings back to blank state.
		ResetCanReceived(); // clear any previously received candata, incase we're doing a complete new startup.

		setOutput(RTDMLED_Output,LEDOFF);
		setOutput(TSLED_Output,LEDOFF);
		setOutput(TSOFFLED_Output,LEDON);
		blinkOutput(RTDMLED_Output,LEDOFF,0);
		blinkOutput(TSOFFLED_Output,LEDOFF,0);
		blinkOutput(TSLED_Output,LEDOFF,0);

		// set relay output LED's off
		setOutput(BMSLED_Output,LEDOFF);
		setOutput(IMDLED_Output,LEDOFF);
		setOutput(BSPDLED_Output,LEDOFF);

		blinkOutput(BMSLED_Output,LEDON,2);
		blinkOutput(IMDLED_Output,LEDON,2);
		blinkOutput(BSPDLED_Output,LEDON,2);

		CAN_NMT(0x81,FLSpeed_COBID);
		CAN_NMT(0x81,FRSpeed_COBID);


		if ( CAN_NMTSyncRequest() ) // sent NMT sync packet to can to ensure we hear from any awake devices.
		{
		 // NMT sent, enter wait and configure state.
			return StartupState;

		} else
		{
		 // error sending NMT
			return OperationalErrorState;
		}

	}

	uint32_t loopstart = gettimer();
	uint32_t looptimer = 0;

	do
	{
        // check for incoming data, break when all received.
		looptimer = gettimer() - loopstart;
	} while ( looptimer < PROCESSLOOPTIME-50 ); // check

//	if ( readystate == 1 ) return StartupState;

	receiveINVStatus(LeftInverter);
	receiveINVStatus(RightInverter);

	receiveINVSpeed(LeftInverter);
	receiveINVSpeed(RightInverter);

	if (  (CarState.LeftInvState != 0xFF && CarState.RightInvState != 0xFF )
		|| ( CarState.LeftInvStateCheck != 0xFF && CarState.RightInvStateCheck != 0xFF )	) // we've received status from inverters, don't send reset.
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

#ifdef COOLANTSHUTDOWN
	if ( ADCState.CoolantTempR > COOLANTMAXTEMP )
	{
		return 97;
	}
#endif

	if ( errorPDM() )
	{
		return 98; // PDM error, stop operation.
	}

	if ( GetInverterState( CarState.LeftInvState ) == INVERTERERROR
		  || GetInverterState( CarState.RightInvState ) == INVERTERERROR )
	{
		return 99; // serious error, no operation allowed. -- inverter
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


int LimpProcess( uint32_t OperationLoops  )
{
	CAN_SendStatus(1, LimpState, 0 );
	sendPDM( 0 );
	return LimpState;
}

int TestingProcess( uint32_t OperationLoops  )
{
	CAN_SendStatus(1, TestingState, 0 );
	sendPDM( 0 );
	return TestingState;
}

int OperationalErrorHandler( uint32_t OperationLoops )
{
	// determine whether occurred error is totally or partially recoverable, and what to do. Try to reset device in question.
	// If still not co-operating either disable, suggest limp mode, or if absolutely critical, total failure.

	// essentially if eg front speed sensors, or yaw sensor, or steering angle, or a single brake sensor
	// then allow near normal operation without those sensors operating.

	// if one inverter

	static uint16_t errorstate;

	static uint32_t errorstatetime = 0;

#ifndef everyloop
	if ( ( OperationLoops % LOGLOOPCOUNTSLOW ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
	//	if ( Errors.ADCErrorState )
	//	  CAN_SendStatus(32, OperationalErrorState, Errors.ADCErrorState+(errorstate<<16));
	//	else
		  CAN_SendStatus(1, OperationalErrorState, Errors.OperationalReceiveError+(errorstate<<16));
	//	devicestate+(sanitystate<<16)
	}

	if ( OperationLoops == 0) // reset state on entering/rentering.
	{

		CarState.HighVoltageReady = 0; // no high voltage allowed in this state.
        CANSendInverter( 0b00000110, 0, LeftInverter );  // request inverters go to non operational state before cutting power.

        CANSendInverter( 0b00000110, 0 , RightInverter );
        
        sendPDM( 0 ); //disable high voltage on error state;
        CAN_SendTimeBase();
        
		errorstate = CheckErrors();
		// send cause of error state.
		CanState.ECU.newdata = 0;
        
//		CAN_NMT( 2, 0x0 ); // send stop command to all nodes.  /// verify that this stops inverters.
		blinkOutput(TSLED_Output,LEDBLINK_FOUR,LEDBLINKNONSTOP);
		blinkOutput(RTDMLED_Output,LEDBLINK_FOUR,LEDBLINKNONSTOP);
		errorstatetime = gettimer();
	}

	if ( Errors.InverterError )	CAN_SENDINVERTERERRORS();

	if ( Errors.ErrorPlace ){
		CAN_SendErrors();
	}

	receivePDM();

	if ( !CarState.ShutdownSwitchesClosed ) // indicate shutdown switch status with blinking rate.
	{
		blinkOutput(TSLED_Output,LEDBLINK_ONE,LEDBLINKNONSTOP);
		blinkOutput(RTDMLED_Output,LEDBLINK_ONE,LEDBLINKNONSTOP);
	} else
	{
		blinkOutput(TSLED_Output,LEDBLINK_FOUR,LEDBLINKNONSTOP);
		blinkOutput(RTDMLED_Output,LEDBLINK_FOUR,LEDBLINKNONSTOP);
	}

//	CheckErrors();

	// wait for restart request if allowed by error state.
	if ( errorstatetime + 20000 < gettimer() // ensure error state is seen for at least 2 seconds.
		&& errorPDM() == 0
        && CheckADCSanity() == 0
        && CarState.ShutdownSwitchesClosed
        && ( checkReset() == 1 // manual reset
#ifdef AUTORESET
        		|| ( ( DeviceState.InverterL == ERROR || DeviceState.InverterR == ERROR ) // or automatic reset if allowed inverter error.
        			 && ( Errors.LeftInvAllowReset && Errors.RightInvAllowReset )
				   )
#endif
           )
		)
	{
		setupButtons();
		setupLEDs();
		//loopcount = 0;
		return StartupState;
		// try to perform a full reset back to startup state here on user request.
	}

		// check for restart request. -> pre operation.
  return OperationalErrorState;

  //return FatalErrorState; // only go to fatal error if error deemed unrecoverable.
}

int OperationalProcess( void )
{
	// initialise loop timer variable.
	uint32_t looptimer = gettimer(); // was volatile, doesn't need to be

	static uint16_t loopoverrun = 0;

	cancount = 0;

	OperationalState = StartupState; // start with setup/waiting for devices state.

	// if( ADCState.newdata )
	while( 1 ) // start main process loop, exit by return statement from error or request only.
	{
		if ( NewOperationalState != OperationalState ) // state has changed.
		{
			LastOperationalState = OperationalState;
			OperationalState = NewOperationalState;
			loopcount = 0;
			clearButtons(); // don't let any user input pass between states
			loopoverrun = 0; // reset over run counter.
		}

		uint32_t currenttimer = gettimer();

		if( looptimer + PROCESSLOOPTIME < currenttimer ) // once per 10ms second process state
		{
			// check how much past 10ms timer is, if too far, soft error. Allow a few times, but not too many before entering an error state?
			int lastlooplength = currenttimer-looptimer;

			looptimer = gettimer(); // start timing loop


#ifdef WATCHDOG
			// tickle the watchdog
		    if (HAL_WWDG_Refresh(&hwwdg1) != HAL_OK)
		    {
		      Error_Handler();
		    }
		    if ( loopcount % 100 == 0 ) toggleOutput(LED2_Output);
#endif

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
					NewOperationalState = PreOperation(loopcount);
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

/*				case 90 : // non critical error, limp potentially possible.? // currently looking to move to limp mode from operational ready state.
					CANSendPDM(0,0);
			//		CAN_NMT( 0x80, 0x0 ); // request pre operation to restart everything.

					NewOperationalState = NMTReset();

					// set limp mode.
*/
				case OperationalErrorState : // critical error or unknown state.					CAN_SendStatus(1, OperationalState, OperationalErrorState);
					NewOperationalState = OperationalErrorHandler( loopcount );
					break;

				default : // 99
					NewOperationalState = OperationalErrorHandler( loopcount );
				//	return FatalErrorState; // error state, quit state machine.
			}

	/*		if ( Errors.InverterError > 0)
			{
				Errors.ErrorPlace = 0xF0;
				NewOperationalState = OperationalErrorState;
			} */


			FDCAN_ProtocolStatusTypeDef CAN1Status, CAN2Status;

			HAL_FDCAN_GetProtocolStatus(&hfdcan1, &CAN1Status);
			HAL_FDCAN_GetProtocolStatus(&hfdcan2, &CAN2Status);


			static uint8_t offcan1 = 0;
			static uint8_t offcan2 = 0;

			static uint32_t offcan1time = 0;
			static uint32_t offcan2time = 0;

			if ( CAN1Status.BusOff) // detect passive error instead and try to stay off bus till clears?
			{
			//	Errors.ErrorPlace = 0xAA;
				  blinkOutput(BMSLED_Output, LEDBLINK_FOUR, 1);
				  HAL_FDCAN_Stop(&hfdcan1);
				  CAN_SendStatus(255,0,0);

				  if ( offcan1 == 0 )
				  {
					  offcan1time = gettimer();
					  offcan1 = 1;
					  DeviceState.CAN1 = OFFLINE;
//					  offcan1count++; // increment occurances of coming off bus. if reach threshhold, go to error state.
				  }
				  Errors.ErrorPlace = 0xF1;
				  NewOperationalState = OperationalErrorState;
			}

#ifdef RECOVERCAN
			if ( CAN1Status.BusOff && offcan1time+5000 >  gettimer() )
			{

				if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) // add a 5 cycle counter before try again? check in can1 send to disable whilst bus not active.
				{
				// Start Error
					 Errors.ErrorPlace = 0xF2;
					 NewOperationalState = OperationalErrorState;
				} else
				{
					offcan1 = 0;
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

			if ( CAN2Status.BusOff) // detect passive error instead and try to stay off bus till clears?
			{
			//	Errors.ErrorPlace = 0xAA;
				  blinkOutput(BMSLED_Output, LEDBLINK_FOUR, 1);
				  HAL_FDCAN_Stop(&hfdcan2);
				  CAN_SendStatus(255,0,0);
				  DeviceState.CAN2 = OFFLINE;

				  if ( offcan2 == 0 )
				  {
					  offcan2time = gettimer();
					  offcan2 = 1;
				  }
				  Errors.ErrorPlace = 0xF2;
				  NewOperationalState = OperationalErrorState;
			}

#ifdef RECOVERCAN
			if ( CAN2Status.BusOff && offcan2time+5000 >  gettimer() )
			{

				if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK) // add a 5 cycle counter before try again? check in can1 send to disable whilst bus not active.
				{
				// Start Error
					 Errors.ErrorPlace = 0xF4;
					 NewOperationalState = OperationalErrorState;
				} else
				{
					offcan2 = 0;
					DeviceState.CAN2 = OPERATIONAL;
					CAN_SendStatus(254,0,0);
				}
			}
#endif

/*			if ( HAL_FDCAN_IsRestrictedOperationMode(&hfdcan2) )
			{
				Errors.ErrorPlace = 0xF4;
				NewOperationalState = OperationalErrorState;
			}
*/

#ifndef everyloop
			if ( ( loopcount % LOGLOOPCOUNTFAST ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
			{
				if ( DeviceState.LoggingEnabled ) CANLogDataFast();
			}

//#ifndef everyloop
			if ( ( loopcount % LOGLOOPCOUNTSLOW ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
//#endif
			{
				CAN_SendLED(); // send LED statuses for debug, make toggleable.
				if ( Errors.OperationalReceiveError == 0) CAN_SendADC(ADC_Data, 0);
				if ( DeviceState.LoggingEnabled ) CANLogDataSlow();
			}
			clearButtons();
			loopcount++;
			totalloopcount++;
		}
	}
}
