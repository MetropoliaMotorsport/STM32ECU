/*
 * lenzeinverter.c
 *
 *  Created on: 21 Mar 2021
 *      Author: Visa
 */

// 1041 ( 411h )  52 1 0 0 -< turn on

// 1041 ( 411h )  49 0 175 0<- trigger message

#include "ecumain.h"
#include "eeprom.h"
#include "errors.h"
#include "inverter.h"
#include "timerecu.h"

#ifdef LENZE
#include "lenzeinverter.h"

extern volatile InverterState_t InverterState[MOTORCOUNT];

bool processINVError( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processINVStatus( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processINVValues1( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processINVValues2( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processINVEmergency( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processINVNMT( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

bool processAPPCOnline( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

CANData InverterCANErr[MOTORCOUNT]= {
		{ NULL, InverterRL_COBID+COBERR_ID, 8, processINVError, NULL, 0, 0 },
		{ NULL, InverterRR_COBID+COBERR_ID, 8, processINVError, NULL, 0, 1 },
#if MOTORCOUNT > 2
		{ NULL, InverterFL_COBID+COBERR_ID, 8, processINVError, NULL, 0, 2 },
		{ NULL, InverterFR_COBID+COBERR_ID, 8, processINVError, NULL, 0, 3 }
#endif
};

CANData InverterCANNMT[MOTORCOUNT] = {
		{ NULL, InverterRL_COBID+COBNMT_ID, 2, processINVNMT, NULL, 0, 0 },
		{ NULL, InverterRR_COBID+COBNMT_ID, 2, processINVNMT, NULL, 0, 1 },
#if MOTORCOUNT > 2
		{ NULL, InverterFL_COBID+COBNMT_ID, 2, processINVNMT, NULL, 0, 2 },
		{ NULL, InverterFR_COBID+COBNMT_ID, 2, processINVNMT, NULL, 0, 3 }
#endif
};


#define TPDStatus 			( 0x1C0 )
#define TPDVal1				( 0x240 )
#define TPDVal2				( 0x280 )


CANData InverterCANMotorStatus[MOTORCOUNT] = { // status values
//		InverterRL_COBID+COBTPDO1_ID
//		InverterRL_COBID+COBTPDO1_ID
		{ NULL, InverterRL_COBID + TPDStatus + ( InverterRL_Channel * 0x100 ), 8, processINVStatus, NULL, 0, 0 },
		{ NULL, InverterRR_COBID + TPDStatus + ( InverterRR_Channel * 0x100 ), 8, processINVStatus, NULL, 0, 1 },
#if MOTORCOUNT > 2
		{ NULL, InverterFL_COBID + TPDStatus + ( InverterFL_Channel * 0x100 ), 8, processINVStatus, NULL, 0, 2 },
		{ NULL, InverterFR_COBID + TPDStatus + ( InverterFR_Channel * 0x100 ), 8, processINVStatus, NULL, 0, 3 }
#endif
};


CANData InverterCANMotorValues1[MOTORCOUNT] = { // speed/torque
		{ NULL, InverterRL_COBID + TPDVal1 + ( InverterRL_Channel * 0x100 ), 8, processINVValues1, NULL, INVERTERTIMEOUT, 0 },
		{ NULL, InverterRR_COBID + TPDVal1 + ( InverterRR_Channel * 0x100 ), 8, processINVValues1, NULL, INVERTERTIMEOUT, 1 },
#if MOTORCOUNT > 2
		{ NULL, InverterFL_COBID + TPDVal1 + ( InverterFL_Channel * 0x100 ), 8, processINVValues1, NULL, INVERTERTIMEOUT, 2 },
		{ NULL, InverterFR_COBID + TPDVal1 + ( InverterFR_Channel * 0x100 ), 8, processINVValues1, NULL, INVERTERTIMEOUT, 3 }
#endif
};

CANData InverterCANMotorValues2[MOTORCOUNT] = { // speed
		{ NULL, InverterRL_COBID + TPDVal2 + ( InverterRL_Channel * 0x100 ), 8, processINVValues2, NULL, 0, 0 },
		{ NULL, InverterRR_COBID + TPDVal2 + ( InverterRR_Channel * 0x100 ), 8, processINVValues2, NULL, 0, 1 },
#if MOTORCOUNT > 2
		{ NULL, InverterFL_COBID + TPDVal2 + ( InverterFL_Channel * 0x100 ), 8, processINVValues2, NULL, 0, 2 },
		{ NULL, InverterFR_COBID + TPDVal2 + ( InverterFR_Channel * 0x100 ), 8, processINVValues2, NULL, 0, 3 }
#endif
};

CANData InverterCANAPPCStatus[INVERTERCOUNT] = { // torque
//		{ NULL, InverterRL_COBID+COBTPDO1_ID, 8, processINVStatus, NULL, 0, 0 },
#if INVERTERCOUNT > 1
//		{ NULL, InverterRR_COBID+COBTPDO1_ID, 8, processINVStatus, NULL, 0, 1 }
#endif
};

// use APPC RDO1 sending as trigger to signify online.
CANData InverterCANAPPCSRDO1[INVERTERCOUNT] = { // torque
//		{ NULL, InverterRL_COBID+COBTPDO1_ID, 8, processAPPCOnline, NULL, 0, 0 },
#if INVERTERCOUNT > 1
//		{ NULL, InverterRR_COBID+COBTPDO1_ID, 8, processAPPCOnline, NULL, 0, 1 }
#endif
};


// two per MC
//char InvSend( uint16_t response, uint16_t request, uint8_t inverter )

void InvResetError( volatile InverterState_t *Inverter )
{
	uint8_t msg[8] = {0};
	msg[0] = 0x80;

	if ( Inverter->MCChannel == false )
	{
		CAN2Send(COBRPDO1_ID + Inverter->COBID, 8, msg);
	} else
	{
		CAN2Send(COBRPDO2_ID + Inverter->COBID, 8, msg);
	}
}

char InvSend( volatile InverterState_t *Inverter, int32_t vel, int16_t torque )
{
	uint8_t msg1[8] = {0};
	uint8_t msg2[8] = {0};
	uint8_t msgblank[8] = {0};

  //  msg[0] = cmd;// cmd shr 8;
  //  msg[1] = cmd >> 8;  // most significant byte;

    vel = vel * 0x4000; // rpm multiplied out.

    // store values for primary request.
    storeLEint16(Inverter->InvCommand, &msg1[0]);
    storeLEint32(vel, &msg1[2]);
    storeLEint16(torque, &msg1[6]);


    // secondary values, what units are these in? they presumably need multiplying up. by 16?
    storeLEint16(620, &msg2[0]);
    storeLEint16(400, &msg2[2]);
    storeLEint16(20, &msg2[4]); // max power
    storeLEint16(0, &msg2[6]); // max regeneration

//	resetCanTx(CANTxData);

	if ( Inverter->MCChannel == false )
	{
		CAN2Send( Inverter->COBID+COBRPDO5_ID, 8, msgblank ); // this should probably be disabled, not needed?

		CAN2Send( Inverter->COBID+COBRPDO1_ID, 8, msg1 );
		CAN2Send( Inverter->COBID+COBRPDO2_ID, 8, msg2 );

	} else
	{
		CAN2Send( Inverter->COBID+COBRPDO3_ID, 8, msg1 );
		CAN2Send( Inverter->COBID+COBRPDO4_ID, 8, msg2 );
	}

	return 0;
}


bool InvStartupCfg( volatile InverterState_t *Inverter )
{
	if ( Inverter->MCChannel == false )
	{
		DebugMsg("Sending inverter sdo setup.");
		// TODO verify correct way this ID should be calculated.
//		CANSendSDO(bus0, 32, 0x4004, 1, 1234);    // disable APPC PDO to take manual control
		CANSendSDO(bus0, Inverter->COBID+31, 0x4004, 1, 1234);
 // set sdo to 10ms cycle
		vTaskDelay(10);
 //   sendSDO(bus0, Inverter->COBID, 0x1801, 5, 10);

 // set SDO's to sync   0x1801-1806  = lenze TPDO 2 through 7
		CANSendSDO(bus0, Inverter->COBID, 0x1801, 2, 1);
		CANSendSDO(bus0, Inverter->COBID, 0x1802, 2, 1);
		CANSendSDO(bus0, Inverter->COBID, 0x1803, 2, 1);

		CANSendSDO(bus0, Inverter->COBID, 0x1804, 2, 1);
		CANSendSDO(bus0, Inverter->COBID, 0x1805, 2, 1);
		CANSendSDO(bus0, Inverter->COBID, 0x1806, 2, 1);
	}

	return true;
}


bool processINVNMT( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle ) // try to reread if possible?
{
	uint8_t inv=datahandle->index;

	if ( InverterState[inv].InvStateAct == OFFLINE ) // device marked offline, mark it online.
	{
	//	InverterState[inv].InvStateAct = BOOTUP;
	}

	// mark all inverter statuses as non operational for startup, so that they can be discovered and processed through state machine.

	return true;
}


uint8_t receiveINVNMT( volatile InverterState_t *Inverter)
{
	// TODO check also inverters for bootup state.

	if ( InverterState[Inverter->InverterNum].InvStateAct < OFFLINE // all valid operational states lower than offine.
// TODO fix nmt detection
			// CanState.InverterNMT.time > 0 || // switch to using device state, as set in interrupt.
//			InvState.Inverter[RearLeftInverter].InvState != 0xFF && InvState.Inverter[RearRightInverter].InvState != 0xFF
#ifdef HPF20
//			&& InvState.Inverter[FrontLeftInverter].InvState != 0xFF && InvState.Inverter[FrontLeftInverter].InvState != 0xFF
#endif
	)
	{
		return 1;
	} else return 0;
}


bool processINVError( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	uint8_t inv=datahandle->index;

	uint8_t errorid=0;

	/*handle inverter errors here.
	 * 0         $fe         8   0  16 129  51 117   2   0   0    1863.183700 R 30003 DC Underlink voltage, auto reset ok.
	 * 0         $fe         8   0  16 129  93 117   2   0   0    1880.023720 R 30045 Power unit: Supply undervoltage
	 * 0         $fe         8   0  16 129  93 117   3   0   0    1880.027050 R
	 * 0         $fe         8   0  16 129  88 117   2   0   0    1880.047530 R 30040 Power unit: Undervolt 24 V
	 * 0         $fe         8   0  16 129 165 120   2   0   0    1880.076030 R 30885 Encoder Cyclic data transfer error
	 * 0         $fe         8   0  16 129 141 124   2   0   0    1880.080460 R 31885 Encoder Cyclic data transfer error
	 *
	 */


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
	if ( 1 ) //CANRxData[0] == 0 && CANRxData[1] == 0x10 && CANRxData[2] == 0x81 && CANRxData[6] == 0x00 && CANRxData[7] == 0 )
	{
		DebugMsg("Inverter error received.");

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


		if ( InverterState[inv].InvStateAct >= OFFLINE) //if inverter status not in error yet, put it there.
		{
			InverterState[inv].InvStateAct = INERROR;
		}

		InverterState[datahandle->index].InvStateAct = INERROR;
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
		InverterState[inv].InvStateAct = INERROR;
		return false;
	}

#ifdef errorLED
			blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif
}


bool processAPPCStatus( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	uint8_t inv=datahandle->index;

	if ( InverterState[inv].MCChannel && inv > 0 ) inv--; // set the voltage available on the the first channel of inverter.

	int16_t InvInputVoltage = getLEint16(&CANRxData[0])/16;
	int16_t InvTemperature = getLEint16(&CANRxData[2])/16;

	if ( InvInputVoltage > 5 && InvInputVoltage < 640 )
	{
		if ( InvInputVoltage > 60 )
			InverterState[inv].HighVoltageAvailable = true;
		else
			InverterState[inv].HighVoltageAvailable = false;

		return true;
	} else // bad data.
	{
		return false;
	}
}


bool processINVStatus( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	uint8_t inv=datahandle->index;

	uint16_t status = CANRxData[0];

//	uint16_t statusword = getLEint16(&CANRxData[0]);
	uint32_t latchedStatus1 = getLEint32(&CANRxData[2]);
	uint16_t latchedStatus2 = getLEint16(&CANRxData[6]);

	if ( true ) // ( CANRxData[1]==22 || CANRxData[1]==6 )
	//	 checkStatusCode(CANRxData[0])
	{
		volatile DeviceStatus curinvState = InternalInverterState(status);
		if ( curinvState == INERROR && InverterState[inv].InvStateAct != INERROR )
		{
			// first error, record time for error time handling.
			InverterState[inv].errortime = xTaskGetTickCount();
			InverterState[inv].latchedStatus1 = latchedStatus1;
			InverterState[inv].latchedStatus2 = latchedStatus2;
		}

		InverterState[inv].InvStateAct = curinvState;

		xTaskNotify( InvTaskHandle, ( 0x1 << (InverterState[inv].InverterNum*3+0) ), eSetBits);

		return true;
	} else // bad data.
	{
		return false;
	}
}


uint8_t receiveINVStatus( volatile InverterState_t *Inverter  )
{
	uint32_t pdotime = 0;
	uint32_t operationalstatus =0;
	uint32_t validdata = 0;

	pdotime = InverterCANMotorStatus[Inverter->InverterNum].time = gettimer();
	operationalstatus = InverterState[Inverter->InverterNum].InvStateAct;
	if ( Inverter->InvStateAct != OFFLINE) validdata = 1;

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


bool processINVValues1( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle ) // try to reread if possible?
{
	uint8_t inv=datahandle->index;

//	int32_t Speed = (CANRxData[5]*16777216+CANRxData[4]*65536+CANRxData[3]*256+CANRxData[2]) * ( 1.0/4194304 ) * 60;

	int32_t Speed = getLEint32(&CANRxData[0]);
	int16_t Torque = getLEint16(&CANRxData[4]);
	int16_t Current = getLEint16(&CANRxData[6]);

	if ( 1 ) //abs(Speed) < 15000 && abs(Torque) < 1000 && abs(Current) < 1000 )
	{
		xTaskNotify( InvTaskHandle, ( 0x1 << (InverterState[inv].InverterNum*3+1) ), eSetBits);
		InverterState[inv].Speed = Speed;
		InverterState[inv].InvTorque = Torque;
		InverterState[inv].InvCurrent = Torque;

		return true;
	} else // bad data.
	{
//		Inverter->InvStateCheck = 0xFE;

#ifdef errorLED
		blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif

		return false;
	}
}


bool processINVValues2( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle ) // try to reread if possible?
{
	uint8_t inv=datahandle->index;

	int16_t MotorTemp = getLEint16(&CANRxData[0])/16;
//	int16_t powerActFiltered = getLEint16(&CANRxData[2]);
//	int16_t volSActFiltered = getLEint16(&CANRxData[4]);
	int16_t PowerModTemp = getLEint16(&CANRxData[6])/16;

	xTaskNotify( InvTaskHandle, ( 0x1 << (InverterState[inv].InverterNum*3+2) ), eSetBits);
	// don't actually have anything to do with these right now.
	return true;

	if ( abs(MotorTemp) > 0 && abs(MotorTemp) < 200 && abs(PowerModTemp) > 0 && abs(PowerModTemp) < 200 )
	{

	} else // bad data.
	{
#ifdef errorLED
		blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif
		return false;
	}
}

bool processAPPCOnline( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
//	volatile InverterState_t *Inverter = &InverterState[datahandle->index];
//	return InvStartupCfg( Inverter );
	return true;
}


uint8_t receiveINVSpeed( volatile InverterState_t *Inverter )
{
	return receivedCANData(&InverterCANMotorValues1[Inverter->InverterNum]);
}

uint8_t receiveINVTorque( volatile InverterState_t *Inverter  )
{
	return receivedCANData(&InverterCANMotorValues1[Inverter->InverterNum]);
}

void SpeedCalculation( int32_t leftdata )
{
/*	CarState.Wheel_Speed_Right_Calculated = Speed_Right_Inverter.data.longint * (1/4194304) * 60; // resolution 4194304 for one revolution
	CarState.Wheel_Speed_Left_Calculated = Speed_Left_Inverter.data.longint * (1/4194304) * 60;
	CarState.Wheel_Speed_Rear_Average = (CarState.Wheel_Speed_Right_Calculated  + CarState.Wheel_Speed_Left_Calculated)/2;
*/
}

// disable public can and move to private.
//  sendSDO(APPC_id, $4004, 1, 1234);
// set sdo to 10ms cycle
//   sendSDO(INVA_id, $1801, 5, 10);

// set SDO's to sync, shouldn't need to do this if settings stored.
//   sendSDO(INVA_id, $1801, 2, 1);
//   sendSDO(INVA_id, $1802, 2, 1);
//   sendSDO(INVA_id, $1802, 2, 1);

bool registerInverterCAN( void )
{
	for( int i=0;i<MOTORCOUNT;i++)
	{
		RegisterCan2Message(&InverterCANErr[i]);
		RegisterCan2Message(&InverterCANNMT[i]);

		// TPDO 2 - status from inverter A
		// TPDO 3 - actual values (1) from motor A
		// TPDO 4 - actual values (2) from motor A

		// TPDO 5 - status from inverter B
		// TPDO 6 - actual values (1) from motor B
		// TPDO 7 - actual values (2) from motor B
		RegisterCan2Message(&InverterCANMotorStatus[i]);
		RegisterCan2Message(&InverterCANMotorValues1[i]);
		RegisterCan2Message(&InverterCANMotorValues2[i]);

	}

	for( int i=0;i<INVERTERCOUNT;i++)
	{
		// TPDO 1 - actual values from device
		RegisterCan2Message(&InverterCANAPPCStatus[i]);
		RegisterCan2Message(&InverterCANAPPCSRDO1[i]);
	}

	return true;
}

#endif
