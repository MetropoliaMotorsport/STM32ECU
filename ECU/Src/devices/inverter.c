/*
 * ivt.c
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */


// 1041 ( 411h )  52 1 0 0 -< turn on

// 1041 ( 411h )  49 0 175 0<- trigger message

#include "ecumain.h"

	// fail process, inverters go from 31->33h->60h->68h  when no HV supplied and request startup.
// states 3->1 ( stop )->-99 ( error )

int8_t GetInverterState ( uint16_t Status ) // status 104, failed to turn on HV 200, failure of encoders/temp
{
	// establish current state machine position from return status.
	if ( ( Status & 0b01001111 ) == 0b01000000) // 64
	{ // Switch on disabled
		return 1;
	}
	else if ( ( Status & 0b01101111 ) == 0b00100001 ) // 49
	{ // Ready to switch on
		return 2;
	}
	else if ( ( Status & 0b01101111 ) == 0b00100011 ) // 51
	{ // Switched on. HV?
		return 3;
	}
	else if ( ( Status & 0b01101111 ) == 0b00100111 ) // 55
	{ // Operation enabled.
		return 4;
	}
	else if ( ( ( Status & 0b01101111 ) == 0b00000111 )
			 || ( ( Status & 0b00011111 ) == 0b00010011 ) )
	{ // Quick Stop Active
		return -1;
	}
	else if  ( ( ( Status & 0b01001111 ) == 0b00001111 )
			 || ( ( Status & 0b01001111 ) == 0b00001001 ) )
	{ // fault reaction active, will move to fault status next
		return -2;
	}
	else if  ( ( ( Status & 0b01001111 ) == 0b00001000 )
			 || ( ( Status & 0b00001000 ) == 0b0001000 ) )
	{ // fault status
		return -99;
		// send reset
	} else
	{ // unknown state
		return 0; // state 0 will request reset to enter State 1,
		// will fall here at start of loop and if unknown status.
	}
}


int8_t InverterStateMachine( int8_t Inverter ) // returns response to send inverter based on current state.
{
	uint16_t State, TXState;

	char HighVoltageAllowed;//, ReadyToDriveAllowed; //, TsLED, RtdmLED;

	if( Inverter == LeftInverter ) // left inverter
	{
		State = CarState.LeftInvState;
		HighVoltageAllowed = CarState.HighVoltageAllowedL;
	}
	else if ( Inverter == RightInverter ) // right inverter
	{
		State = CarState.RightInvState;
		HighVoltageAllowed = CarState.HighVoltageAllowedR;
	}
	else return 0; // invalid inverter

	// first check for fault status, and issue reset.

	TXState = 0; // default  do nothing state.
	// process regular state machine sequence
	switch ( GetInverterState(State) )
	{
		case 0 : // state 0: Not ready to switch on, no can message. Internal state only at startup.
			HighVoltageAllowed = 0;  // High Voltage is not allowed
			TXState=0b10000000; // send bit 128 reset message to enter state 1 in case in fault. - fault reset.
			break;

		case 1 : // State 1: Switch on Disabled.
			HighVoltageAllowed = 0;
			TXState = 0b00000110; // send 0110 shutdown message to request move to State 2.
			break;

		case 2 : // State 2: Ready to switch on
			 // We are ready to turn on, so allow high voltage.
			// we are in state 2, process.
			// process shutdown request here, to move to move to state 1.
			if ( CarState.HighVoltageReady )
			{  // TS enable button pressed and both inverters are marked HV ready proceed to state 3.
				HighVoltageAllowed = 1;
				TXState = 0b00000111; // request Switch on message, State 3..
			} else
			{
				HighVoltageAllowed = 0;
				TXState = 0b00000110; // no change, continue to request State 2.
			}
			break;

		case 3 : // State 3: Switched on   <---- check this case.
			  // we are powered on, so allow high voltage.
			if ( CarState.HighVoltageReady )// IdleState ) <-
			{  // TS enable button has been pressed, proceed to request power on if both inverters on.
				HighVoltageAllowed = 1;
				TXState = 0b00001111; // Request Enable operation, State 4.
			}
			else if ( !CarState.HighVoltageReady )
			{ // return to switched on state.
				HighVoltageAllowed = 0;
				TXState = 0b00000110; // 0b00000000; // request Disable Voltage, drop to ready state., alternately Quick Stop 0b00000010
			}
			else
			{  // no change, continue to request State 3.
				TXState = 0b00000111;
			}
			break;

		case 4 : // State 4: Operation Enable
			 // we are powered on, so allow high voltage.
	/*		if ( CarState.HighVoltageReady ) //  && OperationalState == TSActiveState)
			{ // no longer in RTDM mode, but still got HV, so drop to idle.
				HighVoltageAllowed = 1;
				TXState = 0b00000111; // request state 3: Switched on.
			}
			else */
			if ( !CarState.HighVoltageReady )
			{ // full motor stop has been requested
				HighVoltageAllowed = 0; // drop back to ready to switch on.
				TXState = 0b00000110;//0b00000000; // request Disable Voltage., alternately Quick Stop 0b00000010 - test to see if any difference in behaviour.
			}
			else
			{ // no change, continue to request operation.
				TXState = 0b00001111;
				HighVoltageAllowed = 1;
			}
			break;

		case -1 : //5 Quick Stop Active - Fall through to default to reset state.

		case -2 : //98 Fault Reason Active

		case -99 : //99 Fault

		default : // unknown identifier encountered, ignore. Shouldn't be possible to get here due to filters.
			HighVoltageAllowed = 0;
			TXState = 0b10000000; // 128
			//TXState = 0b00000000; // 0
			break;
		}

	//  offset 0 length 32: power

	if( Inverter == 0 ) // left inverter
	{
		CarState.HighVoltageAllowedL=HighVoltageAllowed;
		return TXState;

	} else if ( Inverter == 1 ) // right inverter   // disabled so both inverter statuses mirrored for testing.
	{
		CarState.HighVoltageAllowedR=HighVoltageAllowed;
		return TXState;
	}

	return 1;
}


long getInvSpeedValue( volatile struct CanData *data )
{
	//		 Speed_Right_Inverter.data.longint * (1/4194304) * 60; - convert to rpm.
	return (data->data[5]*16777216+data->data[4]*65536+data->data[3]*256+data->data[2]) * ( 1.0/4194304 ) * 60;
}


uint8_t processINVNMT(uint8_t CANRxData[8], uint32_t DataLength, uint8_t Inverter ) // try to reread if possible?
{
	CanState.InverterNMT.time = gettimer();

	if ( CarState.LeftInvState == 0xFF ) // device marked offline, mark it online.
	{
		DeviceState.InverterL = BOOTUP;
		DeviceState.InverterR = BOOTUP;
	}

	CarState.RightInvState = 0xFF;
	CarState.LeftInvState = 0xFF;
	CarState.RightInvStateCheck = 0xFF;
	CarState.LeftInvStateCheck = 0xFF;
	CarState.RightInvStateCheck3 = 0xFF;
	CarState.LeftInvStateCheck3 = 0xFF;

	return 1;
}


uint8_t receiveINVNMT( uint8_t Inverter )
{
	if ( // CanState.InverterNMT.time > 0 || // switch to using device state, as set in interrupt.
			CarState.LeftInvState != 0xFF && CarState.RightInvState != 0xFF)
	{
		return 1;
	} else return 0;
}


uint8_t processINVError(uint8_t CANRxData[8], uint32_t DataLength, uint8_t Inverter ) // try to reread if possible?
{
	uint8_t errorid=0;

/*handle inverter errors here.
 * 0         $fe         8   0  16 129  51 117   2   0   0    1863.183700 R
 * 0         $fe         8   0  16 129  93 117   2   0   0    1880.023720 R
 * 0         $fe         8   0  16 129  93 117   3   0   0    1880.027050 R
 * 0         $fe         8   0  16 129  88 117   2   0   0    1880.047530 R
 * 0         $fe         8   0  16 129 165 120   2   0   0    1880.076030 R
 * 0         $fe         8   0  16 129 141 124   2   0   0    1880.080460 R
 *
 *				 1    000000FE   8  00  10  81      DD  1E     03   -   00  00
 *
 *							8  0    16 129      221  30     03
 */
// 4b 75

//	5d 75
//	a5 78
//	33 75


	switch ( Inverter )
	{
		case LeftInverter :	CanState.InverterLPDO1.time = gettimer(); errorid = 0xFE; break;
		case RightInverter : CanState.InverterRPDO1.time = gettimer(); errorid = 0xFF; break;
	}

	if ( Errors.InverterErrorHistoryPosition < 8) // add error data to log.
	{
		for( int i=0;i<8;i++){
			Errors.InverterErrorHistory[Errors.InverterErrorHistoryPosition][i] = CANRxData[i];
			Errors.InverterErrorHistoryID[i] = errorid;
		}
		Errors.InverterErrorHistoryPosition++;
	}

	Errors.InverterError++;

	// 00  10  81      DD  1E     03   -   00  00
	if ( CANRxData[0] == 0 && CANRxData[1] == 0x10 && CANRxData[2] == 0x81 && CANRxData[6] == 0x00 && CANRxData[7] == 0 )
	{
        uint16_t ErrorCode = CANRxData[4]*256+CANRxData[3];
        
        uint8_t AllowReset = 0;
        
        switch ( ErrorCode ) // 29954
        {
        	case 30003 : // DC Underlink Voltage. HV dropped or dipped, allow reset attempt.
                AllowReset = 1;
                break;
            default : // other unknown errors, don't allow reset attempt.
                AllowReset = 0;
        }
        
		switch ( Inverter )
		{
			case LeftInverter :
				if ( GetInverterState(CarState.LeftInvState) >= 0) //if inverter status not in error yet, put it there.
				{
					CarState.LeftInvState = 0xFE;
				}
				CarState.LeftInvBadStatus = 1;
				DeviceState.InverterL = ERROR;
                if ( Errors.LeftInvAllowReset == 1 )
                {
                    Errors.LeftInvAllowReset = AllowReset;
                }
				Errors.INVLReceiveStatus++;
#ifdef SENDBADDATAERROR
				CAN_SendErrorStatus(99,InverterLReceived+20,99);
#endif
				break;
			case RightInverter :
				if ( GetInverterState(CarState.RightInvState) >= 0)
				CarState.RightInvState = 0xFE;
				CarState.RightInvBadStatus = 1;
				DeviceState.InverterR = ERROR;
                if ( Errors.RightInvAllowReset == 1 )
                {
                    Errors.RightInvAllowReset = AllowReset;
                }
				Errors.INVRReceiveStatus++;
#ifdef SENDBADDATAERROR
                CAN_SendErrorStatus(99,InverterRReceived+20,99);
#endif
				break;
		}

		return 1;
	} else // bad data, even if bad data, assume error state as this is emergency message.
	{
		Errors.CANError++;
		switch ( Inverter )
		{
			case LeftInverter :
				CarState.LeftInvState = 104;
				DeviceState.InverterL = ERROR;
				Errors.INVLReceiveStatus++;
#ifdef SENDBADDATAERROR
				CAN_SendErrorStatus(99,InverterLReceived,100);
#endif
				break;
			case RightInverter :
				CarState.RightInvState = 104;
				DeviceState.InverterR = ERROR;
#ifdef SENDBADDATAERROR
				CAN_SendErrorStatus(99,InverterRReceived,100);
#endif
				Errors.INVRReceiveStatus++;
				break;
		}
		reTransmitError(99, CANRxData, DataLength);
		return 0;
	}

#ifdef errorLED
			blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif
}


uint8_t processINVStatus(uint8_t CANRxData[8], uint32_t DataLength, uint8_t Inverter ) // try to reread if possible?
{
	int statusreceived = 0;

	switch ( Inverter ){
		case LeftInverter :	CanState.InverterLPDO2.time = gettimer(); break;
		case RightInverter : CanState.InverterRPDO2.time = gettimer(); break;
	}

	uint16_t status = CANRxData[0];//*256+CANRxData[1];

	switch ( CANRxData[0] )
	{
	    case 49 : // ready to switch on.
		case 51 : // on
		case 55 : // operation
		case 64 : // startup
		case 96 : //
		case 104 : // error
		case 200 : // very error // c0   c8     192-200 errors.
			statusreceived = 1;
			break;
		default :
			statusreceived = 0;
	}

	if ( DataLength == FDCAN_DLC_BYTES_2
			&& ( CANRxData[1]==22 || CANRxData[1]==6 )
			&& statusreceived )
	{
		switch ( Inverter ){
			case LeftInverter :
				CarState.LeftInvState = status;
				CarState.LeftInvBadStatus = 0;
				DeviceState.InverterL = GetInverterState(CarState.LeftInvState );
				break;
			case RightInverter :
				CarState.RightInvState = status;
				CarState.RightInvBadStatus = 0;
				DeviceState.InverterR = GetInverterState(CarState.RightInvState );
				break;
		}
		return 1;
	} else // bad data.
	{
		switch ( Inverter )
		{
			case LeftInverter :
//				CarState.LeftInvState = 0xFE; // flag up that an invalid status was received.
//				DeviceState.InverterL = OPERATIONAL;
				CarState.LeftInvBadStatus = 1;
				Errors.INVLReceiveStatus++;
#ifdef SENDBADDATAERROR
				CAN_SendErrorStatus(99,InverterLReceived,99);
#endif

#ifdef errorLED
			blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif
				break;
			case RightInverter :
//				CarState.RightInvState = 0xFE; //flag up that an invalid status was received.
				CarState.RightInvBadStatus = 1;
				Errors.INVRReceiveStatus++;
//				DeviceState.InverterR = OPERATIONAL;
#ifdef SENDBADDATAERROR
				CAN_SendErrorStatus(99,InverterRReceived,99);
#endif

#ifdef errorLED
			blinkOutput(BSPDLED_Output,LEDBLINK_FOUR,255);
#endif
				break;
		}
		Errors.CANError++;

		reTransmitError(99, CANRxData, DataLength); // put into a retransmit queue?
		return 0;
	}
}


uint8_t receiveINVStatus( uint8_t Inverter  )
{
	uint32_t pdotime = 0;
	uint32_t operationalstatus =0;
	uint32_t validdata = 0;

	switch ( Inverter )
	{
		case LeftInverter :
			pdotime = CanState.InverterLPDO1.time;
			operationalstatus = DeviceState.InverterL;
			if ( CarState.LeftInvState != 0xFF) validdata = 1;
			break;
		case RightInverter :
			pdotime = CanState.InverterRPDO1.time;
			operationalstatus = DeviceState.InverterR;
			if ( CarState.RightInvState != 0xFF) validdata = 1;
			break;
	}

	if ( operationalstatus != OFFLINE && validdata ) // no timeout on inverter status pdo, not sent frequently.
	{
		return 1; // data received within windows
	} else
	{
		if ( pdotime > 0 )
		{
			return 1; // we have received data, and not OFFLINE
		} else return 0;
	}
}


uint8_t processINVTorque(uint8_t CANRxData[8], uint32_t DataLength, uint8_t Inverter ) // try to reread if possible?
{
	int statusreceived = 0;

	switch ( Inverter ){
		case LeftInverter :	CanState.InverterLPDO3.time = gettimer(); break;
		case RightInverter : CanState.InverterRPDO3.time = gettimer(); break;
	}

	uint16_t status = CANRxData[0];//*256+CANRxData[1];
	uint16_t torque = CANRxData[2]*256+CANRxData[3]; // not yet converter to correct value, reverse of torque request.

	switch ( CANRxData[0] )
	{
	    case 49 : // ready to switch on.
		case 51 : // on
		case 55 : // operation
		case 64 : // startup
		case 96 : //
		case 104 : // error
		case 200 : // very error
			statusreceived = 1;
			break;
		default :
			statusreceived = 0;
	}

	// value =

	if ( DataLength == FDCAN_DLC_BYTES_4
			&& ( CANRxData[1]==22 ||CANRxData[1]==6 )
			&& statusreceived
			// && torque < maxtorquepossible )
		)
	{
		switch ( Inverter )
		{
			case LeftInverter :
				CarState.LeftInvTorque = torque;
				CarState.LeftInvStateCheck3 = status;
				DeviceState.InverterL = OPERATIONAL;
				break;
			case RightInverter :
				CarState.RightInvTorque = torque;
				CarState.RightInvStateCheck3 = status;
				DeviceState.InverterR = OPERATIONAL;
				break;
		}
		return 1;
	} else // bad data.
	{
		Errors.CANError++;
		switch ( Inverter )
		{
			case LeftInverter :
//				DeviceState.InverterL = OPERATIONAL;
				Errors.INVLReceiveTorque++;
				CarState.LeftInvStateCheck3 = 0xFE; // flag up that an invalid status was received.
#ifdef SENDBADDATAERROR
				CAN_SendErrorStatus(99,InverterLReceived+25,99);
#endif

#ifdef errorLED
			blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif
				break;
			case RightInverter :
//				DeviceState.InverterR = OPERATIONAL;
				Errors.INVRReceiveTorque++;
				CarState.RightInvStateCheck3 = 0xFE; //flag up that an invalid status was received.
#ifdef SENDBADDATAERROR
				CAN_SendErrorStatus(99,InverterRReceived+25,99);
#endif

#ifdef errorLED
			blinkOutput(BSPDLED_Output,LEDBLINK_FOUR,255);
#endif
				break;
		}
		reTransmitError(99, CANRxData, DataLength); // put into a retransmit queue?
		return 0;
	}
}

uint8_t receiveINVTorque( uint8_t Inverter )
{
	uint32_t operationalstatus =0;

/*	switch ( Inverter )
	{
		case LeftInverter :
			operationalstatus = DeviceState.InverterL;
			if ( CarState.LeftInvStateCheck3 != 0xFF) validdata = 1;
			break;
		case RightInverter :
			operationalstatus = DeviceState.InverterR;
			if ( CarState.RightInvStateCheck3 != 0xFF) validdata = 1;
			break;
	} */

	if (// time - pdotime <=  &&
			operationalstatus != OFFLINE )
	{
		return 1; // data received within windows
	} else return 0;

#ifdef pdo3timeout
	else
	{
		if ( time - pdotime > INVERTERTIMEOUT )
		{
			switch ( Inverter )
			{
				case LeftInverter :
					if ( errorsentl == 0 )
					{
						CAN_SendStatus(99,InverterLReceived+57,99);
					}
					errorsentl = 1;
					break;
				case RightInverter :
					if ( errorsentr == 0 )
					{
						CAN_SendErrorStatus(99,InverterRReceived+57,99);
						}
					errorsentr = 1;
					break;
			}
		}
		return 0;
	}
#endif

}


uint8_t processINVSpeed(uint8_t CANRxData[8], uint32_t DataLength, uint8_t Inverter ) // try to reread if possible?
{
	int statusreceived = 0;

	switch ( Inverter ){
		case LeftInverter :	CanState.InverterLPDO2.time = gettimer(); break;
		case RightInverter : CanState.InverterRPDO2.time = gettimer(); break;
	}

	uint16_t status = CANRxData[0];//*256+CANRxData[1];
	int32_t Speed = (CANRxData[5]*16777216+CANRxData[4]*65536+CANRxData[3]*256+CANRxData[2]) * ( 1.0/4194304 ) * 60;

	switch ( CANRxData[0] )
	{
	    case 49 : // ready to switch on.
		case 51 : // on
		case 55 : // operation
		case 64 : // startup
		case 96 : //
		case 104 : // error
		case 200 : // very error
			statusreceived = 1;
			break;
		default :
			statusreceived = 0;
	}

	if ( DataLength == FDCAN_DLC_BYTES_6
			&& ( CANRxData[1]==22 ||CANRxData[1]==6 )
			&& statusreceived
			&& abs(Speed) < 15000 )
	{
		switch ( Inverter )
		{
			case LeftInverter :
				CarState.SpeedRL = Speed;
				CarState.LeftInvStateCheck = status;
				if ( CarState.LeftInvBadStatus == 1 || CarState.LeftInvState == 0xFF )
//				if ( CarState.LeftInvState == 0xFE || CarState.LeftInvState == 0xFF )
				{
					CarState.LeftInvState = status;
					CarState.LeftInvBadStatus = 0;
					DeviceState.InverterL = OPERATIONAL;
					DeviceState.InverterL = GetInverterState(CarState.LeftInvState ); // correct state if wrong.
				} // this will mark as online if offline.
	/*			else if ( CarState.LeftInvState != CarState.LeftInvStateCheck )
				{
					CarState.LeftInvState = status;
				} */

				break;

			case RightInverter :
				CarState.SpeedRR = Speed;
				CarState.RightInvStateCheck = status;
//				if ( CarState.RightInvBadStatus == 1 )
				if ( CarState.RightInvBadStatus == 1 || CarState.RightInvState == 0xFF )
//				if ( CarState.RightInvState == 0xFE || CarState.RightInvState == 0xFF )
				{
					CarState.RightInvBadStatus = 0;
					DeviceState.InverterR = OPERATIONAL;
					CarState.RightInvState = status;
					DeviceState.InverterR = GetInverterState(CarState.RightInvState );
				} else if ( CarState.RightInvState != CarState.RightInvStateCheck )
				{
					CarState.RightInvState = status;
				}
				break;
		}
		return 1;
	} else // bad data.
	{
		Errors.CANError++;
		switch ( Inverter )
		{
			case LeftInverter :
//				DeviceState.InverterL = OPERATIONAL;
				//set speed to 0?
				Errors.INVLReceiveSpd++;
				CarState.LeftInvStateCheck = 0xFE; // flag up that an invalid status was received.
#ifdef SENDBADDATAERROR
				CAN_SendErrorStatus(99,InverterLReceived,99);
#endif

#ifdef errorLED
			blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif
				break;
			case RightInverter :
//				DeviceState.InverterR = OPERATIONAL;
				Errors.INVRReceiveSpd++;
				CarState.RightInvStateCheck = 0xFE; //flag up that an invalid status was received.
#ifdef SENDBADDATAERROR
				CAN_SendErrorStatus(99,InverterRReceived,99);
#endif

#ifdef errorLED
			blinkOutput(BSPDLED_Output,LEDBLINK_FOUR,255);
#endif
				break;
		}
		reTransmitError(99, CANRxData, DataLength); // put into a retransmit queue?
		return 0;
	}
}

uint8_t receiveINVSpeed( uint8_t Inverter  )
{
	uint32_t time=gettimer();
	uint32_t pdotime = 0;
	uint32_t operationalstatus =0;
	uint32_t validdata = 0;

	static uint8_t errorsentl = 0;
	static uint8_t errorsentr = 0;

	switch ( Inverter )
	{
		case LeftInverter :
			pdotime = CanState.InverterLPDO2.time;
			operationalstatus = DeviceState.InverterL;
			if ( CarState.LeftInvStateCheck != 0xFF) validdata = 1;
			break;
		case RightInverter :
			pdotime = CanState.InverterRPDO2.time;
			operationalstatus = DeviceState.InverterR;
			if ( CarState.RightInvStateCheck != 0xFF) validdata = 1;
			break;
	}

	if ( time - pdotime < 200 && operationalstatus != OFFLINE && validdata )
	{
		switch ( Inverter )
		{
			case LeftInverter :
				errorsentl = 0;
				break;
			case RightInverter :
				errorsentr = 0;
				break;
		}

		return 1; // data received within windows
	} else if ( operationalstatus != OFFLINE )
	{
		if ( time - pdotime > INVERTERTIMEOUT )
		{
			switch ( Inverter )
			{
				case LeftInverter :
					if ( errorsentl == 0 )
					{
						CAN_SendErrorStatus(200,InverterLReceived+58,99);
					}
					DeviceState.InverterL = OFFLINE;
					errorsentl = 1;
					break;
				case RightInverter :
					if ( errorsentr == 0 )
					{
						CAN_SendErrorStatus(200,InverterRReceived+58,99);
					}
					DeviceState.InverterR = OFFLINE;
					errorsentr = 1;
					break;
			}
		}
		return 0;
	}
	return 1;
}

uint8_t requestINV( uint8_t Inverter  )
{
	return 0; // this is operating with cansync, no extra needed.
}

