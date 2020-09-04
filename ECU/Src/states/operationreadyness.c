/*
 * operationreadyness.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#include "ecumain.h"

static uint8_t InverterSent;

int ReadyRequest( void )   // request data / invalidate existing data to ensure we have fresh data from this cycle.
{
	uint16_t command;
	int error = 0;

	// Inverter Pre Operation preparedness.

//	ResetCanReceived(); // reset can data before operation to ensure we aren't checking old data from previous cycle.
	CAN_NMTSyncRequest(); // anything working with cansync will respond with
	sendPDM( 0 );

	// request ready states from devices.

	if ( ( CarState.LeftInvState != 0xFF && GetInverterState( CarState.LeftInvState ) != INVERTERREADY) || InverterSent == 0  )
	{
		command = InverterStateMachine( LeftInverter ); // request left inv state machine to pre operation readyness if not already
		CANSendInverter( command, 0, LeftInverter );
	}

	if (( CarState.RightInvState != 0xFF && GetInverterState( CarState.RightInvState ) != INVERTERREADY ) || InverterSent == 0 )
	{
		command = InverterStateMachine( RightInverter ); //  request right inv state machine to pre operation readyness if not already
		CANSendInverter( command, 0, RightInverter );
	}

	InverterSent = 0;

	if (DeviceState.FrontSpeedSensors == ENABLED )
	{
		DeviceState.FLSpeed = sickState( FLSpeed_COBID );
		DeviceState.FRSpeed = sickState( FRSpeed_COBID );
	}
	// send BMS - currently no sync request, just sending data

	// send PDM - currently no sync request, just sending data.
//	requestIVT();

	return error;
}

int AllowLimp( void ) // function to determine if limp mode is allowable
{
   return 0;
}


uint16_t ReadyReceive( uint16_t returnvalue )
{
	if ( returnvalue == 0xFFFF)
	{
		returnvalue = (0x1 << InverterLReceived)+
					(0x1 << InverterRReceived)+
					(0X1 << PDMReceived)+
					(0X1 << BMSReceived)+
					(0X1 << IVTReceived)+
					(0x1 << FLeftSpeedReceived)+
					(0x1 << FRightSpeedReceived)+
					(0x1 << PedalADCReceived);//
			//(0x1 << YAWOnlineBit);
	}

	// change order, get status from pdo3, and then compare against pdo2?, 2 should be more current being higher priority

	receiveINVStatus(LeftInverter);
	receiveINVStatus(RightInverter);

	if ( receiveINVSpeed(LeftInverter) ) returnvalue &= ~(0x1 << InverterLReceived); // speed should always be seen every cycle in RTDM
	if ( receiveINVSpeed(RightInverter) ) returnvalue &= ~(0x1 << InverterRReceived);

	if ( DeviceState.FrontSpeedSensors == ENABLED)
	{

		if ( DeviceState.FLSpeed == OPERATIONAL )
		{
			returnvalue &= ~(0x1 << FLeftSpeedReceived);
		}

		if ( DeviceState.FRSpeed == OPERATIONAL )
		{
			returnvalue &= ~(0x1 << FRightSpeedReceived);
		}
	} else // not using sensors.
	{
		returnvalue &= ~(0x1 << FLeftSpeedReceived); // currently setting all to zero to allow quick testing
		returnvalue &= ~(0x1 << FRightSpeedReceived); // currently setting all to zero to allow quick testing
	}

	receiveBMS();

	if ( DeviceState.BMS != OFFLINE && ( CarState.VoltageBMS > 460 && CarState.VoltageBMS < 600) )
	{
		// check voltages, temperatures.
		returnvalue &= ~(0x1 << BMSReceived); // return voltage in OK range.
	}

	receiveIVT();

	if ( DeviceState.IVT != OFFLINE )
	{
		returnvalue &= ~(0x1 << IVTReceived); // check values
	}

	if ( DeviceState.PDM != OFFLINE )
	{
		returnvalue &= ~(0x1 << PDMReceived);
// ensure in communication with PDM.

	}

#ifdef STMADC
    if ( CheckADCSanity() == 0 ) returnvalue &= ~(0x1 << PedalADCReceived); // change this to just indicate ADC received in some form.
#else
	if ( DeviceState.ADC == OPERATIONAL )
	{
		returnvalue &= ~(0x1 << PedalADCReceived); // using local adc, already established online in initialisation.
	}
#endif

	return returnvalue;
}

// 1.	Testing the functionality/readings from the different sensors
// 2.	Checking that all expected CAN bus messages were received within specified time-interval
//      (Both Inverters, both encoders, both accelerator pedal sensors, brake pressure sensor/s, BMS
//      IVT-MOD, steering angle sensor, acceleration/yaw sensor�.)
// 3.	Checking that the HV and LV batteries voltages and temperatures are within defined limits

int OperationReadyness( uint32_t OperationLoops ) // process function for operation readyness state
{
//	static uint16_t sanitystate;
	static uint16_t received;

	if ( OperationLoops == 0) // reset state on entering/rentering.
	{
//		sanitystate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state to ensure can't proceed yet.
		received = 0xFFFF;
		InverterSent = 0;
		CarState.HighVoltageReady = 0; // no high voltage allowed in this state.
	}
#ifndef everyloop
	if ( ( OperationLoops % STATUSLOOPCOUNT ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		CAN_SendStatus(1, OperationalReadyState, received );
//		CAN_SendStatus(1, , sanitystate+(devicestate<<16));
	}
//	OperationLoops++; // counter for sending can status messages.

	if ( OperationLoops > 500 )	// how many loops allow to get all data on time?, failure timeout.
	{
		Errors.ErrorPlace = 0xBA;
		return OperationalErrorState; // error, too long waiting for data
	}

	uint32_t loopstart = gettimer();
	uint32_t looptimer = 0;

	ReadyRequest(); // request state updates / get devices into ready mode / receiving any sensor information for checking sanity.

	do
	{
		received = ReadyReceive( received );
        // check for incoming data, break when all received.
		looptimer = gettimer() - loopstart;
	} while ( received != 0 && looptimer < PROCESSLOOPTIME-50 ); // check

	// process data.

	uint16_t Errorcode = CheckErrors();

	if ( Errorcode )
	{
		CAN_SendStatus(5, OperationalReadyState, received);
		Errors.State = OperationalReadyState;
	//	Errors.error = 1; // checkerrors;
		Errors.ErrorPlace = 0xBB;
		Errors.ErrorReason = Errorcode;
		return OperationalErrorState; // something has triggered an error, drop to error state to deal with it.
	}

	if (received != 0 )
	{ // activation requested but not everything is in satisfactory condition to continue

		// show error state but allow to continue in some state if non critical sensor fails sanity.

	//	setOutput(TSLED_Output,0);
	//	blinkOutput(TSLED_Output,LEDBLINK_TWO,0);
#ifdef movetoerrorstate
		if ( AllowLimp() )
		{
		//	blinkOutput(STOPLED_Output,LEDBLINK_TWO,0); //indicate that limp mode is allowable.
			if ( CheckLimpActivationRequest() ) // check if limp mode requested if possible.
			{
				return LimpState; // enter limp mode?, or set variable to toggle limp mode elsewhere.
			}
		}
		else
#endif
		{
			blinkOutput(TSLED_Output,LEDBLINK_FOUR,1); // indicate TS was requested before system ready.
	//		blinkOutput(STOPLED_Output,LEDOFF,0); // disable limp mode alert
			return OperationalReadyState; // maintain current state.
		}
	}
	else if ( ( GetInverterState(CarState.LeftInvState) == INVERTERREADY && GetInverterState(CarState.RightInvState) == INVERTERREADY )  // 2  // Ready to switch on
	//	 || ( GetInverterState(CarState.LeftInvState) == INVERTERONLINE && GetInverterState(CarState.RightInvState) == INVERTERONLINE )
		 )
		// everything is ok to continue.
	{
//		CarState.Torque_Req_Max = ADCState.DrivingMode; // set max torque request before enter operational state.
//		CarState.Torque_Req_CurrentMax = ADCState.DrivingMode;
		return IdleState; // ready to move onto TS activated but not operational state, idle waiting for RTDM activation.
		// set TS ready for activation light.
	}

	return OperationalReadyState;
}
