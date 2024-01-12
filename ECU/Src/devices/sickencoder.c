/*
 * sickencoder.c
 *
 *  Created on: 01 May 2019
 *      Author: Visa
 *
 *      Still needs rewrite to interrupt data handling.
 */

#include "ecumain.h"

#ifdef HPF19 // not present on HPF20, four motors with own encoders.

int sickState( uint8_t canid ) // returns current state, requests expected state over can.
{
	int State;

	switch (canid)
	{
		case FLSpeed_COBID : State = DeviceState.FLSpeed; break;
		case FRSpeed_COBID : State = DeviceState.FRSpeed; break;
		default : // invalid ID given.
			return ERROR;
	}

	switch ( State )
	{
		case PREOPERATION :
            CAN_NMT(1,canid);
            State = OPERATIONAL;
            break; // already in right state.
		case STOPPED :
		case BOOTUP :
            CAN_NMT(0x80,canid); State = PREOPERATION;
            break; // set to operational, is pre operational at bootup message.
		case OFFLINE :
            CAN_NMT(0x81,canid);
            break; // send reset
		case OPERATIONAL : break;
		case ERROR : // check error in error handler, and if allowed, try to reset.
		default :
			break;
	}

	switch (canid)
	{
		case FLSpeed_COBID :
			DeviceState.FLSpeed = State;
			break;
		case FRSpeed_COBID :
			DeviceState.FRSpeed = State;
			break;
	}

	return State;
}


uint8_t processSickNMT(uint8_t CANRxData[8], uint32_t DataLength, uint16_t encoder )
{
	switch (encoder)
	{
		case FLSpeed_COBID :
			 	CanState.FLeftSpeedNMT.time = gettimer();
				DeviceState.FLSpeed = BOOTUP; // received data without error bit set, so we can assume operational state
				CarState.SpeedFL = 0;

				break;
		case FRSpeed_COBID :
				CanState.FRightSpeedNMT.time = gettimer();
				DeviceState.FRSpeed = BOOTUP; // received data without error bit set, so we can assume operational state
				CarState.SpeedFR = 0;
				break;
	}
	return 1;
}


uint8_t processSickEncoder(uint8_t CANRxData[8], uint32_t DataLength, uint16_t encoder )
{
	if ( DeviceState.FrontSpeedSensors == ENABLED )
	{
		int32_t value = CANRxData[3]*16777216+CANRxData[2]*65536+CANRxData[1]*256+CANRxData[0];

		if ( CANRxData[4] == 0 && DataLength == FDCAN_DLC_BYTES_8 )
		{
			switch (encoder)
			{
				case FLSpeed_COBID :
					 	CanState.FLeftSpeedPDO1.time = gettimer();
						DeviceState.FLSpeed = OPERATIONAL; // received data without error bit set, so we can assume operational state
						if ( value != 0 ) value = value * -1;
						CarState.SpeedFL = value;
						return 1;
						break;
				case FRSpeed_COBID :
						CanState.FRightSpeedPDO1.time = gettimer();
						DeviceState.FRSpeed = OPERATIONAL; // received data without error bit set, so we can assume operational state
						CarState.SpeedFR = value;
						return 1;
						break;
			}
		}
		else // bad data.
		{
			Errors.CANError++;
			switch ( encoder ) // set state data.
			{
		//		IVTMsg_ID : ;
				case FLSpeed_COBID :
					Errors.FLSpeedReceive++;
#ifdef SENDBADDATAERROR
					CAN_SendErrorStatus(99,FLeftSpeedReceived,99);
#endif
					break;
				case FRSpeed_COBID :
					Errors.FRSpeedReceive++;
#ifdef SENDBADDATAERROR
					CAN_SendErrorStatus(99,FRightSpeedReceived,99);
#endif
					break;
			}
			reTransmitError(99,CANRxData, DataLength >> 16); // put into a retransmit queue?
			return 0; // bad data received, but still heard from device device.
		}
		return 1;
	} else // not enabled, just read 0;
	{
		CarState.SpeedFR = 0;
		CarState.SpeedFL = 0;
		return 1;
	}
}


int receiveSick( uint8_t canid ) // receives data from encoder. moved to interrupt.
{
    switch (canid)
    {
        case FLSpeed_COBID :
            if ( CanState.FLeftSpeedPDO1.time + SICKTIMEOUT > gettimer() )
                CarState.SpeedFL = 0; //  read 0 if timeout or not enabled, so that old data is not used, but don't go offline.
            break;
        case FRSpeed_COBID :
            if ( CanState.FRightSpeedPDO1.time + SICKTIMEOUT > gettimer() )
                CarState.SpeedFR = 0;
            break;
    }
    
  return 1; // always assume present.
}


int sickError( uint8_t canid )
//int sickError ( volatile struct CanData *data ) // receive sick sensor error.
{
	return 0;
}


int requestsick( int nodeid )
{
	return 0; // this is operating with CANopen sync, no extra needed.
}


int initSick( void )
{

}

#endif

