/*
 * siemensinverter.c
 *
 *      Author: Visa
 */

// 1041 ( 411h )  52 1 0 0 -< turn on

// 1041 ( 411h )  49 0 175 0<- trigger message

#include "ecumain.h"
#ifdef SIEMENS
#include "siemensinverter.h"

bool processINVError(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle);
bool processINVStatus(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );
bool processINVTorque(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle);
bool processINVSpeed(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle);
bool processINVEmergency(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle);
bool processINVNMT(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle);

CANData InverterCANErr[MOTORCOUNT]= {
		{ &DeviceState.Inverters[0], InverterRL_COBID+COBERR_ID, 8, processINVError, NULL, 0, 0 },
		{ &DeviceState.Inverters[1], InverterRR_COBID+COBERR_ID, 8, processINVError, NULL, 0, 1 },
#if MOTORCOUNT > 2
		{ &DeviceState.Inverters[2], InverterFL_COBID+COBERR_ID, 8, processINVError, NULL, 0, 2 },
		{ &DeviceState.Inverters[3], InverterFR_COBID+COBERR_ID, 8, processINVError, NULL, 0, 3 }
#endif
};

CANData InverterCANNMT[MOTORCOUNT] = {
		{ &DeviceState.Inverters[0], InverterRL_COBID+COBNMT_ID, 2, processINVNMT, NULL, 0, 0 },
		{ &DeviceState.Inverters[1], InverterRR_COBID+COBNMT_ID, 2, processINVNMT, NULL, 0, 1 },
#if MOTORCOUNT > 2
		{ &DeviceState.Inverters[2], InverterFL_COBID+COBNMT_ID, 2, processINVNMT, NULL, 0, 2 },
		{ &DeviceState.Inverters[3], InverterFR_COBID+COBNMT_ID, 2, processINVNMT, NULL, 0, 3 }
#endif
};


CANData InverterCANPDO1[MOTORCOUNT] = { // status
		{ &DeviceState.Inverters[0], InverterRL_COBID+COBTPDO1_ID, 4, processINVStatus, NULL, 0, 0 },
		{ &DeviceState.Inverters[1], InverterRR_COBID+COBTPDO1_ID, 4, processINVStatus, NULL, 0, 1 },
#if MOTORCOUNT > 2
		{ &DeviceState.Inverters[2], InverterFL_COBID+COBTPDO1_ID, 4, processINVStatus, NULL, 0, 2 },
		{ &DeviceState.Inverters[3], InverterFR_COBID+COBTPDO1_ID, 4, processINVStatus, NULL, 0, 3 }
#endif
};

CANData InverterCANPDO2[MOTORCOUNT] = { // speed
		{ &DeviceState.Inverters[0], InverterRL_COBID+COBTPDO2_ID, 6, processINVSpeed, NULL, INVERTERTIMEOUT, 0 },
		{ &DeviceState.Inverters[1], InverterRR_COBID+COBTPDO2_ID, 6, processINVSpeed, NULL, INVERTERTIMEOUT, 1 },
#if MOTORCOUNT > 2
		{ &DeviceState.Inverters[2], InverterFL_COBID+COBTPDO2_ID, 6, processINVSpeed, NULL, INVERTERTIMEOUT, 2 },
		{ &DeviceState.Inverters[3], InverterFR_COBID+COBTPDO2_ID, 6, processINVSpeed, NULL, INVERTERTIMEOUT, 3 }
#endif
};


CANData InverterCANPDO3[MOTORCOUNT] = { // torque
		{ NULL, InverterRL_COBID+COBTPDO3_ID, 4, processINVTorque, NULL, 0, 0 },
		{ NULL, InverterRR_COBID+COBTPDO3_ID, 4, processINVTorque, NULL, 0, 1 },
#if MOTORCOUNT > 2
		{ NULL, InverterFL_COBID+COBTPDO3_ID, 4, processINVTorque, NULL, 0, 2 },
		{ NULL, InverterFR_COBID+COBTPDO3_ID, 4, processINVTorque, NULL, 0, 3 }
#endif
};

CANData InverterCANPDO4[MOTORCOUNT] = { // not currently used
		{ NULL, InverterRL_COBID+COBTPDO4_ID, 6, NULL, NULL, 0, 0, true },
		{ NULL, InverterRR_COBID+COBTPDO4_ID, 6, NULL, NULL, 0, 1, true },
#if MOTORCOUNT > 2
		{ NULL, InverterFL_COBID+COBTPDO4_ID, 6, NULL, NULL, 0, 2, true },
		{ NULL, InverterFR_COBID+COBTPDO4_ID, 6, NULL, NULL, 0, 3, true }
#endif
};


void InvResetError( volatile InverterState *Inverter )
{

}

char InvSend( volatile InverterState *Inverter, uint16_t cmd, int32_t vel, int16_t torque )
{
	uint8_t CANTxData[8] = {0};

	storeLEint16(cmd,&CANTxData[0]);
	storeLEint16(torque,&CANTxData[2]);

	return CAN2Send( COBRPDO3_ID + Inverter->COBID, 4, CANTxData );
}

bool InvStartupCfg( volatile InverterState *Inverter )
{

}


bool processINVNMT(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle) // try to reread if possible?
{
	volatile InverterState *Inverter = &CarState.Inverters[datahandle->index];

	if ( Inverter->InvState == 0xFF ) // device marked offline, mark it online.
	{
		DeviceState.Inverters[Inverter->InverterNum] = BOOTUP;
	}

	// mark all inverter statuses as non operational for startup, so that they can be discovered and processed through state machine.

	Inverter->InvState = 0xFF;
	Inverter->InvStateCheck = 0xFF;
	Inverter->InvStateCheck3 = 0xFF;

	return true;
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




bool processINVError(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle)
{
	volatile InverterState *Inverter = &CarState.Inverters[datahandle->index];

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


	//	CanState.

//	InverterCANPDO1[datahandle->index].time = gettimer();
	errorid = 0xFF-MOTORCOUNT+datahandle->index; // calculate so that inverter 4 = 0xFF, inverter 1 = 0xFC

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
		DeviceState.Inverters[datahandle->index] = ERROR;
        if ( Errors.InvAllowReset[datahandle->index] == 1 )
        {
            Errors.InvAllowReset[datahandle->index] = AllowReset;
        }
        Errors.INVReceiveStatus[datahandle->index]++;
#ifdef SENDBADDATAERROR
//		CAN_SendErrorStatus(99,INVERTERRECEIVED+Inverter->InverterNum,99);
#endif

		return true;
	} else // bad data, even if bad data, assume error state as this is emergency message.
	{
		Inverter->InvState = 104;
		//		DeviceState.Inverters[Inverter->InverterNum] = ERROR;
		return false;
	}

#ifdef errorLED
			blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif
}


bool processINVStatus(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle)
{
	volatile InverterState *Inverter = &CarState.Inverters[datahandle->index];
	uint16_t status = CANRxData[0];//*256+CANRxData[1]; - second byte not actually used publically, definition private.

	if ( ( CANRxData[1]==22 || CANRxData[1]==6 )
			&& checkStatusCode(CANRxData[0]) )
	{
		Inverter->InvState = status;
		Inverter->InvBadStatus = 0;
		DeviceState.Inverters[datahandle->index] = GetInverterState( status );

		return true;
	} else // bad data.
	{
		Inverter->InvBadStatus = 1;
		return false;
	}
}


uint8_t receiveINVStatus( volatile InverterState *Inverter  )
{
	uint32_t pdotime = 0;
	uint32_t operationalstatus =0;
	uint32_t validdata = 0;

	pdotime = InverterCANPDO1[Inverter->InverterNum].time = gettimer();
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


bool processINVTorque(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle)
{
	volatile InverterState *Inverter = &CarState.Inverters[datahandle->index];
	uint16_t status = CANRxData[0];//*256+CANRxData[1]; - second byte not actually used publically, definition private.
	uint16_t torque = CANRxData[2]*256+CANRxData[3]; // not yet converter to correct value, reverse of torque request.

	if ( ( CANRxData[1]==22 ||CANRxData[1]==6 )
			&& checkStatusCode(CANRxData[0])
			// && torque < maxtorquepossible )
		)
	{
		Inverter->InvTorque = torque;
		Inverter->InvStateCheck3 = status;
		return true;
	} else // bad data.
	{
		Inverter->InvStateCheck3 = 0xFE; // flag up that an invalid status was received.
		#ifdef errorLED
		blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
		#endif
		return false;
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
			switch ( Inverters )
			{
				case LeftInverter :
					if ( errorsentl == 0 )
					{
						CAN_SendStatus(99,INVERTERRECEIVED+Inverters->InverterNum+57,99);
					}
					errorsentl = 1;
					break;
				case RightInverter :
					if ( errorsentr == 0 )
					{
						CAN_SendErrorStatus(99,INVERTERRECEIVED+Inverters->InverterNum+57,99);
						}
					errorsentr = 1;
					break;
			}
		}
		return 0;
	}
#endif
}



bool processINVSpeed(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle) // try to reread if possible?
{
	volatile InverterState *Inverter = &CarState.Inverters[datahandle->index];

	uint16_t status = CANRxData[0];//*256+CANRxData[1];

//	int32_t Speed = (CANRxData[5]*16777216+CANRxData[4]*65536+CANRxData[3]*256+CANRxData[2]) * ( 1.0/4194304 ) * 60;

	int32_t Speed = getLEint32(&CANRxData[2]);

	if ( ( CANRxData[1]==22 ||CANRxData[1]==6 )
			&& checkStatusCode(CANRxData[0])
			&& abs(Speed) < 15000 )
	{
		Inverter->Speed = Speed;
		Inverter->InvStateCheck = status;

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

		return true;
	} else // bad data.
	{
		Inverter->InvStateCheck = 0xFE;

#ifdef errorLED
		blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif

		return false;
	}
}

uint8_t receiveINVSpeed( volatile InverterState *Inverter )
{
	return receivedCANData(&InverterCANPDO2[Inverter->InverterNum]);
}


bool registerInverterCAN( void )
{
	for( int i=0;i<MOTORCOUNT;i++)
	{
		RegisterCan2Message(&InverterCANErr[i]);
		RegisterCan2Message(&InverterCANNMT[i]);

		RegisterCan2Message(&InverterCANPDO1[i]);
		RegisterCan2Message(&InverterCANPDO2[i]);
		RegisterCan2Message(&InverterCANPDO3[i]);
		RegisterCan2Message(&InverterCANPDO4[i]);
	}

	return true;
}

#endif
