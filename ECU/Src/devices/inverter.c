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


bool invertersStateCheck(int state )
{
	bool requestedstate = true;
	for ( int i=0;i<INVERTERCOUNT;i++){
		 if ( GetInverterState( CarState.Inverters[i].InvState ) != INVERTERREADY ) requestedstate = false;
	}
	return requestedstate;
}



int8_t InverterStateMachine( volatile InverterState *Inverter ) // returns response to send inverter based on current state.
{
	uint16_t State, TXState;

	char HighVoltageAllowed;//, ReadyToDriveAllowed; //, TsLED, RtdmLED;

	State = Inverter->InvState;
	HighVoltageAllowed = Inverter->HighVoltageAllowed;

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

	Inverter->HighVoltageAllowed = HighVoltageAllowed;
	return TXState;
}


long getInvSpeedValue( volatile struct CanData *data )
{
	//		 Speed_Right_Inverter.data.longint * (1/4194304) * 60; - convert to rpm.
	return (data->data[5]*16777216+data->data[4]*65536+data->data[3]*256+data->data[2]) * ( 1.0/4194304 ) * 60;
}


uint8_t processINVNMT(uint8_t CANRxData[8], uint32_t DataLength, volatile InverterState *Inverter ) // try to reread if possible?
{
	CanState.InverterNMT.time = gettimer();

	if ( Inverter->InvState == 0xFF ) // device marked offline, mark it online.
	{
		DeviceState.Inverters[Inverter->InverterNum] = BOOTUP;
	}

	// mark all inverter statuses as non operational for startup, so that they can be discovered and processed through state machine.

	Inverter->InvState = 0xFF;
	Inverter->InvStateCheck = 0xFF;
	Inverter->InvStateCheck3 = 0xFF;

	return 1;
}


uint8_t receiveINVNMT( volatile InverterState *Inverter)
{
	// TODO check also inverters for bootup state.

	if ( DeviceState.Inverters[Inverter->InverterNum] < OFFLINE // all valid operational states lower than offine.
// TODO fix nmt detection
			// CanState.InverterNMT.time > 0 || // switch to using device state, as set in interrupt.
//			CarState.Inverters[RearLeftInverter].InvState != 0xFF && CarState.Inverters[RearRightInverter].InvState != 0xFF
#ifdef HPF20
//			&& CarState.Inverters[FrontLeftInverter].InvState != 0xFF && CarState.Inverters[FrontLeftInverter].InvState != 0xFF
#endif
	)
	{
		return 1;
	} else return 0;
}


uint8_t processINVError(uint8_t CANRxData[8], uint32_t DataLength, volatile InverterState *Inverter ) // try to reread if possible?
{
	uint8_t errorid=0;

/*handle inverter errors here.
 * 0         $fe         8   0  16 129  51 117   2   0   0    1863.183700 R 30003 DC Underlink voltage, auto reset ok.
 * 0         $fe         8   0  16 129  93 117   2   0   0    1880.023720 R 30045 Power unit: Supply undervoltage
 * 0         $fe         8   0  16 129  93 117   3   0   0    1880.027050 R
 * 0         $fe         8   0  16 129  88 117   2   0   0    1880.047530 R 30040 Power unit: Undervolt 24 V
 * 0         $fe         8   0  16 129 165 120   2   0   0    1880.076030 R 30885 Encoder Cyclic data transfer error
 * 0         $fe         8   0  16 129 141 124   2   0   0    1880.080460 R 31885 Encoder Cyclic data transfer error
 *
 *				 1    000000FE   8  00  10  81      DD  1E     03   -   00  00
 *
 *							8  0    16 129      221  30     03
 */
// 4b 75

//	5d 75
//	a5 78
//	33 75


	CanState.InverterPDO1[Inverter->InverterNum].time = gettimer();
	errorid = 0xFF-INVERTERCOUNT+Inverter->InverterNum; // calculate so that inverter 4 = 0xFF, inverter 1 = 0xFC

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
        	case 30003 : // DC Underlink Voltage. HV dropped or dipped, allow reset attempt. //  33 117 hex
        	case 30040 : // Power unit: Undervolt 24 V
        	case 30045 : // Power unit: Supply undervoltage
                AllowReset = 1;
                break;
            default : // other unknown errors, don't allow auto reset attempt.
                AllowReset = 0;
        }
        

		if ( GetInverterState(Inverter->InvState) >= 0) //if inverter status not in error yet, put it there.
		{
			Inverter->InvState = 0xFE;
		}
		Inverter->InvBadStatus = 1;
		DeviceState.Inverters[Inverter->InverterNum] = ERROR;
        if ( Errors.InvAllowReset[Inverter->InverterNum] == 1 )
        {
            Errors.InvAllowReset[Inverter->InverterNum] = AllowReset;
        }
        Errors.INVReceiveStatus[Inverter->InverterNum]++;
#ifdef SENDBADDATAERROR
		CAN_SendErrorStatus(99,INVERTERRECEIVED+Inverter->InverterNum,99);
#endif

		return 1;
	} else // bad data, even if bad data, assume error state as this is emergency message.
	{
		Errors.CANError++;

		Inverter->InvState = 104;
		DeviceState.Inverters[Inverter->InverterNum] = ERROR;
		Errors.INVReceiveStatus[Inverter->InverterNum]++;
#ifdef SENDBADDATAERROR
		CAN_SendErrorStatus(99,INVERTERRECEIVED+Inverter->InverterNum,100);
#endif
#ifdef RETRANSMITBADDATAERROR
		reTransmitError(99, CANRxData, DataLength);
#endif
		return 0;
	}

#ifdef errorLED
			blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif
}


uint8_t processINVStatus(uint8_t CANRxData[8], uint32_t DataLength, volatile InverterState *Inverter ) // try to reread if possible?
{
	int statusreceived = 0;

	CanState.InverterPDO2[Inverter->InverterNum].time = gettimer();

	uint16_t status = CANRxData[0];//*256+CANRxData[1]; - second byte not actually used publically, definition private.

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
		Inverter->InvState = status;
		Inverter->InvBadStatus = 0;
		DeviceState.Inverters[Inverter->InverterNum] = GetInverterState( status );

		return 1;
	} else // bad data.
	{
		//				CarState.LeftInvState = 0xFE; // flag up that an invalid status was received.
		//				DeviceState.InverterL = OPERATIONAL;
		Inverter->InvBadStatus = 1;
		Errors.INVReceiveStatus[Inverter->InverterNum]++;
#ifdef SENDBADDATAERROR
		CAN_SendErrorStatus(99,INVERTERRECEIVED+Inverter->InverterNum,99);
#endif

#ifdef errorLED
		blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif

		Errors.CANError++;

#ifdef RETRANSMITBADDATAERROR
		reTransmitError(99, CANRxData, DataLength); // put into a retransmit queue?
#endif
		return 0;
	}
}


uint8_t receiveINVStatus( volatile InverterState *Inverter  )
{
	uint32_t pdotime = 0;
	uint32_t operationalstatus =0;
	uint32_t validdata = 0;

	pdotime = CanState.InverterPDO2[Inverter->InverterNum].time = gettimer();
	operationalstatus = DeviceState.Inverters[Inverter->InverterNum];
	if ( Inverter->InvState != 0xFF) validdata = 1;

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


uint8_t processINVTorque(uint8_t CANRxData[8], uint32_t DataLength, volatile InverterState *Inverter  ) // try to reread if possible?
{
	int statusreceived = 0;

	CanState.InverterPDO3[Inverter->InverterNum].time = gettimer();

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

	// value

	if ( DataLength == FDCAN_DLC_BYTES_4
			&& ( CANRxData[1]==22 ||CANRxData[1]==6 )
			&& statusreceived
			// && torque < maxtorquepossible )
		)
	{
		Inverter->InvTorque = torque;
		Inverter->InvStateCheck3 = status;
		DeviceState.Inverters[Inverter->InverterNum] = OPERATIONAL;

		return 1;
	} else // bad data.
	{
		Errors.CANError++;

		//				DeviceState.InverterL = OPERATIONAL;
		Errors.INVReceiveTorque[Inverter->InverterNum]++;
		Inverter->InvStateCheck3 = 0xFE; // flag up that an invalid status was received.
		#ifdef SENDBADDATAERROR
		CAN_SendErrorStatus(99,INVERTERRECEIVED+Inverter->InverterNum+25,99);
					//	CAN_SendErrorStatus(99,InverterLReceived+Inverter->InverterNum25,99);
		#endif

		#ifdef errorLED
		blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
		#endif

	#ifdef RETRANSMITBADDATAERROR
		reTransmitError(99, CANRxData, DataLength); // put into a retransmit queue?
	#endif
		return 0;
	}
}

uint8_t receiveINVTorque( volatile InverterState *Inverter  )
{
	uint32_t operationalstatus = 0;

	/*
	operationalstatus = DeviceState.Inverters[Inverter->InverterNum];
	if ( Inverter->StateCheck3 != 0xFF) validdata = 1;
	 */

	if (// time - pdotime <=  && // can't timeout on torque message.
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
						CAN_SendStatus(99,INVERTERRECEIVED+Inverter->InverterNum+57,99);
					}
					errorsentl = 1;
					break;
				case RightInverter :
					if ( errorsentr == 0 )
					{
						CAN_SendErrorStatus(99,INVERTERRECEIVED+Inverter->InverterNum+57,99);
						}
					errorsentr = 1;
					break;
			}
		}
		return 0;
	}
#endif

}


uint8_t processINVSpeed(uint8_t CANRxData[8], uint32_t DataLength, volatile InverterState *Inverter ) // try to reread if possible?
{
	int statusreceived = 0;

	CanState.InverterPDO2[Inverter->InverterNum].time = gettimer();

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

		Inverter->Speed = Speed;
		Inverter->InvStateCheck = status;

//				if ( Inverter->InvState == 0xFE || Inverter->InvState == 0xFF )
		if ( Inverter->InvBadStatus == 1 || Inverter->InvState == 0xFF )
		{
			Inverter->InvState = status;
			Inverter->InvBadStatus = 0;
			DeviceState.Inverters[Inverter->InverterNum] = GetInverterState(Inverter->InvState ); // correct state if wrong.
		} // this will mark as online if offline.
			else if ( Inverter->InvState != Inverter->InvStateCheck ) // was commented out for RL
		{
			Inverter->InvState = status; // was commented out for RL
		}

		return 1;
	} else // bad data.
	{
		Errors.CANError++;

		Errors.INVReceiveSpd[Inverter->InverterNum]++;
		Inverter->InvStateCheck = 0xFE;
#ifdef SENDBADDATAERROR
		CAN_SendErrorStatus(99,INVERTERRECEIVED,99);
#endif

#ifdef errorLED
		blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif

#ifdef RETRANSMITBADDATAERROR
		reTransmitError(99, CANRxData, DataLength); // put into a retransmit queue?
#endif
		return 0;
	}
}

uint8_t receiveINVSpeed( volatile InverterState *Inverter )
{
	uint32_t time = gettimer();
	uint32_t pdotime = 0;
	uint32_t operationalstatus = 0;
	uint32_t validdata = 0;

	static uint8_t errorsent[INVERTERCOUNT] = { 0 } ;

	pdotime = CanState.InverterPDO2[Inverter->InverterNum].time;
	operationalstatus = DeviceState.Inverters[Inverter->InverterNum];
	if ( CarState.Inverters[Inverter->InverterNum].InvStateCheck != 0xFF) validdata = 1;

	if ( time - pdotime < 200 && operationalstatus != OFFLINE && validdata )
	{
		errorsent[Inverter->InverterNum] = 0;

		return 1; // data received within windows
	} else if ( operationalstatus != OFFLINE )
	{
		if ( time - pdotime > INVERTERTIMEOUT )
		{
			if ( errorsent[Inverter->InverterNum] == 0 )
			{
				CAN_SendErrorStatus(200,InverterReceived+58+Inverter->InverterNum,99);
			}
			DeviceState.Inverters[Inverter->InverterNum] = OFFLINE;
			errorsent[Inverter->InverterNum] = 1;
		}
		return 0;
	}
	return 1;
}

uint8_t requestINV( uint8_t Inverter  )
{
	return 0; // this is operating with cansync, no extra needed.
}

