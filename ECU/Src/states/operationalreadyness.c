/*
 * operationreadyness.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#include "ecumain.h"
#include "operationalreadyness.h"
#include "inverter.h"
#include "output.h"
#include "power.h"
#include "errors.h"
#include "lcd.h"
#include "debug.h"


int AllowLimp( void ) // function to determine if limp mode is allowable
{
   return 0;
}


uint16_t ReadyReceive( uint16_t returnvalue )
{
	if ( returnvalue == 0xFFFF)
	{
		returnvalue =
#ifndef POWERNODES
					(0x1 << PDMReceived)+
#else
					(0x1 << PNodeReceived)+
#endif
					(0x1 << BMSReceived)+
					(0x1 << IVTReceived)+
#ifdef HPF19
					(0x1 << FLeftSpeedReceived)+
					(0x1 << FRightSpeedReceived)+
#endif
					(0x1 << PedalADCReceived);

			//(0x1 << YAWOnlineBit);
	}


#ifdef HPF19
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
#endif

	//receiveBMS();

	if ( DeviceState.BMS != OFFLINE && ( CarState.VoltageBMS > 460 && CarState.VoltageBMS < 600) )
	{
		// check voltages, temperatures.
		returnvalue &= ~(0x1 << BMSReceived); // return voltage in OK range.
	} else
	{
		static bool first = false;
		if ( !first )
		{
			first = true;
			DebugMsg("Readyness BMS fail");
		}
	}

	//receiveIVT();

	if ( DeviceState.IVT != OFFLINE )
	{
		returnvalue &= ~(0x1 << IVTReceived); // check values
	} else
	{
		static bool first = false;
		if ( !first )
		{
			first = true;
			DebugMsg("Readyness IVT fail");
		}
	}

#ifndef POWERNODES
	if ( DeviceState.PDM != OFFLINE )
	{
		returnvalue &= ~(0x1 << PDMReceived);
// ensure in communication with PDM.

	}
#else
	if ( DeviceState.CriticalPower == OPERATIONAL )
	{
		returnvalue &= ~(0x1 << PNodeReceived);
	} else
	{
//		static bool first = false;
//		if ( !first )
		{
//			first = true;
			DebugPrintf("Readyness Power fail: %s", getDeviceStatusStr(DeviceState.CriticalPower));
		}
	}

#endif

#ifdef STMADC

    if ( DeviceState.ADCSanity == 0 ) returnvalue &= ~(0x1 << PedalADCReceived); // change this to just indicate ADC received in some form.


#else
	if ( DeviceState.ADCSanity == 0 )
	{
		returnvalue &= ~(0x1 << PedalADCReceived); // using local adc, already established online in initialisation.
	} else
	{
//		static bool first = false;
//		if ( !first )
		{
//			first = true;
			DebugPrintf("Readyness ADC fail ADCSanity %d", DeviceState.ADCSanity);
		}
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
		DebugMsg("Entering Readyness check State");
		SetErrorLogging(true);
		lcd_clear();
		lcd_settitle("Readying Devices");
		received = 0xFFFF;
	}

	CAN_SendStatus(1, OperationalReadyState, received );

	if ( OperationLoops > 5 ) // 500 )	// how many loops allow to get all data on time?, failure timeout.
	{
		DebugMsg("Errorplace 0xBA Too many loops.");
		Errors.ErrorPlace = 0xBA;
		return OperationalErrorState; // error, too long waiting for data. Go to error state to inform and allow restart of process.
	}

//	invRequestState(STOPPED); // not working for lenze.

	vTaskDelay(5);

	// this state is just to allow devices to get ready, don't need to check any data.

	received = ReadyReceive( received );

        // check for incoming data, break when all received.

	// process data.

	if ( CheckCriticalError() )
	{
	//	CAN_SendErrorStatus(5, OperationalReadyState, received);
	//	Errors.State = OperationalReadyState;
		DebugMsg("Errorplace 0xBB critical error.");
		Errors.ErrorReason = ReceivedCriticalError | ( CheckCriticalError() << 8 );
		Errors.ErrorPlace = 0xBB;
		return OperationalErrorState; // something has triggered an unacceptable error ( inverter error state etc ), drop to error state to deal with it.

	}

	int invcount = 0;

	if (received != 0 )
	{ // activation requested but not everything is in satisfactory condition to continue

		DebugPrintf("Received %d", received);
		// show error state but allow to continue in some state if non critical sensor fails sanity.

		blinkOutput(TSLED,LEDBLINK_FOUR,1000); // indicate TS was requested before system ready.
		return OperationalReadyState; // maintain current state.
	}
	else // if ( GetInverterState() >= STOPPED  ) // Ready to switch on
	{
		for ( int i=0;i<MOTORCOUNT; i++)
		{
			if ( getInvState(i)->Device != OFFLINE )
			{
				invcount++;
			}
		}

		DebugPrintf("Invc: %d %s %s %s %s", invcount,
				getDeviceStatusStr(getInvState(0)->Device),
				getDeviceStatusStr(getInvState(1)->Device),
				getDeviceStatusStr(getInvState(2)->Device),
				getDeviceStatusStr(getInvState(3)->Device) );

		if ( invcount == MOTORCOUNT )
		{
			// everything is ok to continue.
			return IdleState; // ready to move onto TS activated but not operational state, idle waiting for RTDM activation.
		}
	}

	return OperationalReadyState;
}
