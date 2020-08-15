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
	for ( int i=0;i<ResetCount;i++) (*ResetCommands[i])();

	DeviceState.CAN1 = OPERATIONAL;
	DeviceState.CAN2 = OPERATIONAL;

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

	Errors.ErrorPlace = 0;
	Errors.ErrorReason = 0;
	Errors.CANSendError1 = 0;
	Errors.CANSendError2 = 0;
	Errors.ADCSent = false;
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
		lcd_setscrolltitle("Startup init");

		readystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.

		// send startup state message here.
		// reset all state information.

		ResetStateData(); // set car state settings back to blank state.

//		ResetCanReceived(); // clear any previously received candata, incase we're doing a complete new startup.

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

		// Show status LED's for 2 seconds for rules compliance.

		blinkOutput(BMSLED_Output,LEDON,2);
		blinkOutput(IMDLED_Output,LEDON,2);
		blinkOutput(BSPDLED_Output,LEDON,2);

		CAN_NMT(0x81,FLSpeed_COBID);
		CAN_NMT(0x81,FRSpeed_COBID);

		ShutdownCircuitSet( false );

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
//	uint32_t looptimer = 0;

#ifndef RTOS
	DWT_Delay((PROCESSLOOPTIME-MS1*5-(gettimer()-loopstart))*1000); // wait for 5ms
#else
	vTaskDelay(5);
#endif
/*	do
	{
        // check for incoming data, break when all received.
		looptimer = gettimer() - loopstart;
	} while ( looptimer < PROCESSLOOPTIME-50 ); // check
*/
//	if ( readystate == 1 ) return StartupState;

	uint8_t invertercount = 0;

	for ( int i=0;i<MOTORCOUNT;i++) // speed is received
	{
		receiveINVStatus(&CarState.Inverters[i]);
		receiveINVSpeed(&CarState.Inverters[i]);
		receiveINVTorque(&CarState.Inverters[i]);

		if ( CarState.Inverters[i].InvState != 0xFF ) invertercount++;
	}

	if ( invertercount == MOTORCOUNT ) // we've received status from inverters, don't send reset.
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
		if ( !CarState.TestHV && CarState.Inverters[i].InvState == INVERTERERROR )
		{
			return 99; // serious error, no operation allowed. -- inverter
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


int LimpProcess( uint32_t OperationLoops  )
{
	lcd_setscrolltitle("LimpProcess(NA)");
	CAN_SendStatus(1, LimpState, 0 );
	setHV( false );
	return LimpState;
}

int TestingProcess( uint32_t OperationLoops  )
{
	lcd_setscrolltitle("TestingProcess(NA)");
	CAN_SendStatus(1, TestingState, 0 );
	setHV( false );
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
		char str[20];
		lcd_setscrolltitle("ERROR State");
		lcd_clearscroll();

		sprintf(str,"Loc:%.2X Code:%.4X", Errors.ErrorPlace, Errors.ErrorReason);
		lcd_send_stringscroll(str);

		CarState.HighVoltageReady = 0; // no high voltage allowed in this state.

		for ( int i=0;i<MOTORCOUNT;i++){
	        CANSendInverter( 0b00000110, 0, i );  // request inverters go to non operational state before cutting power.
		}

        sendPDM( 0 ); //disable high voltage on error state;

        CAN_SendTimeBase();

		errorstate = CheckErrors();

		sprintf(str,"Errorstate: %.4X", errorstate);
		lcd_send_stringscroll(str);
		// send cause of error state.

		ConfigReset();


		switch ( Errors.ErrorPlace )
		{
		case 0xF1 : 	sprintf(str,"CANBUS1 Offline"); break;
		case 0xF2 : 	sprintf(str,"CANBUS2 Offline"); break;
		}
		lcd_send_stringscroll(str);

//		CAN_NMT( 2, 0x0 ); // send stop command to all nodes.  /// verify that this stops inverters.
		blinkOutput(TSLED_Output,LEDBLINK_FOUR,LEDBLINKNONSTOP);
		blinkOutput(RTDMLED_Output,LEDBLINK_FOUR,LEDBLINKNONSTOP);
		errorstatetime = gettimer();
	}

#ifdef POWERNODES
	CAN_NMTSyncRequest();
#endif

	if ( Errors.InverterError ){
		CAN_SENDINVERTERERRORS();
		lcd_send_stringscroll("Inverter error");
	}

	if ( Errors.ErrorPlace ){
		CAN_SendErrors();
	}

	receivePDM();

	if ( !CheckShutdown() ) // indicate shutdown switch status with blinking rate.
	{
		lcd_send_stringscroll("Shutdown switches");
		blinkOutput(TSLED_Output,LEDBLINK_ONE,LEDBLINKNONSTOP);
		blinkOutput(RTDMLED_Output,LEDBLINK_ONE,LEDBLINKNONSTOP);
	} else
	{
		blinkOutput(TSLED_Output,LEDBLINK_FOUR,LEDBLINKNONSTOP);
		blinkOutput(RTDMLED_Output,LEDBLINK_FOUR,LEDBLINKNONSTOP);
	}

#ifdef AUTORESET
	bool allowautoreset = true;
	bool invertererror = false;

	for ( int i=0;i<MOTORCOUNT;i++)
	{
		if ( DeviceState.Inverters[i] == INERROR ) invertererror = true;
		if ( !Errors.InvAllowReset[i] ) allowautoreset = false;
	}

	// ( DeviceState.InverterRL == INERROR || DeviceState.InverterRR == INERROR )
	// ( Errors.LeftInvAllowReset && Errors.RightInvAllowReset )
#endif


	int allowreset = 0; // allow reset if this is still 0 after checks.

	char str[80] = "ERROR: ";

	if ( errorstatetime + MS1000*2 > gettimer() ) // ensure error state is seen for at least 2 seconds.
	{
		allowreset += 1;
		strcat(str, "" );
	}

	if ( ! ( CheckErrors() == 0 || CheckErrors() == 99 ) ) // inverter error checked in next step.
	{
//		allowreset += 2;
		strcat(str, "PDM " );
	}

	if ( ! ( CheckADCSanity() == 0 ))
	{
		allowreset +=4;
		strcat(str, "PDL " );

	}

#ifdef SHUTDOWNSWITCHCHECK
    if ( !CheckShutdown() )
    {
    	allowreset +=8;  // only allow exiting error state if shutdown switches closed. - maybe move to only for auto
    	strcat(str, "SHT " );
    }
#endif

    strpad(str,20);

    if ( allowreset == 0 )
    {
    	lcd_setscrolltitle("ERROR (Can Reset) ");
    } else
    {
    	lcd_setscrolltitle(str);
    }


	// wait for restart request if allowed by error state.
	if ( allowreset == 0 && ( checkReset() == 1 // manual reset
#ifdef AUTORESET
        		|| ( invertererror  && allowautoreset ) // or automatic reset if allowed inverter error.
#endif
           )
		)
	{
//		setupButtons();
//		setupLEDs();
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
#ifndef RTOS
	lcd_setscrolltitle("Starting Main Loop");
#endif
	// initialise loop timer variable.
	uint32_t looptimer = gettimer(); // was volatile, doesn't need to be

	static uint16_t loopoverrun = 0;


//	TickType_t xLastCycleTime;
    const TickType_t xFrequency = 20;

	// Initialise the xLastWakeTime variable with the current time.
//    xLastCycleTime = xTaskGetTickCount();

	cancount = 0;

	OperationalState = StartupState; // start with setup/waiting for devices state.

	// if( ADCState.newdata )
#ifndef RTOS
	while( 1 ) // start main process loop, exit by return statement from error or request only.
#endif
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

		//calculate right delay to wait for loop.
#ifndef RTOS
		DWT_Delay((PROCESSLOOPTIME-(currenttimer-looptimer))*1000);
#else
//		vTaskDelayUntil( &xLastCycleTime, MS1*10 );
#endif

//		if( looptimer + PROCESSLOOPTIME < currenttimer ) // once per 10ms second process state
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
				  blinkOutput(TSOFFLED_Output, LEDBLINK_FOUR, 1);
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
#ifndef ONECAN
			if ( CAN2Status.BusOff) // detect passive error instead and try to stay off bus till clears?
			{
			//	Errors.ErrorPlace = 0xAA;
				  blinkOutput(BMSLED_Output, LEDBLINK_FOUR, 1);
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
#ifndef ANALOGNODES
				if ( Errors.OperationalReceiveError == 0) CAN_SendADC(ADC_Data, 0);
#endif
				if ( DeviceState.LoggingEnabled ) CANLogDataSlow();
			}
	//		clearButtons();
			loopcount++;
			totalloopcount++;
		}


		static int lcdupdatecounter = 0;
		// update LCD at end of loop rather than timer so that all possible updates have been done, and there won't be priority clashes on what to display.
		if ( DeviceState.LCD == OPERATIONAL && ( lcdupdatecounter % 5 ) == 0 )
		{
			lcd_update();
		}
		lcdupdatecounter++;

#ifdef WFI
		__WFI(); // sleep till interrupt, avoid loading cpu doing nothing.
#endif
	}
}
