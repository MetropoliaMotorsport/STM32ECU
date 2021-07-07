/*
 * lenzeinverter.c
 *
 *  Created on: 21 Mar 2021
 *      Author: Visa
 */

#define DEBUGAPPCSDO

#include "ecumain.h"
#include "eeprom.h"
#include "errors.h"
#include "inverter.h"
#include "timerecu.h"
#include "debug.h"

#ifdef LENZE
#include "lenzeinverter.h"

extern volatile InverterState_t InverterState[MOTORCOUNT];

bool processINVError( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processINVStatus( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processINVValues1( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processINVValues2( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processINVEmergency( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processINVNMT( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

bool processAPPCRDO( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

bool processINVRDO( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

//bool processAPPCOnline( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processAPPCError( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );


bool getInvSDOSet( void );

//		{ TimeoutFunction, ID, DLC, receivefunction, dotimeout, timeout, index.
CANData InverterCANErr[MOTORCOUNT]= {
		{ NULL, InverterRL_COBID+COBERR_ID + ( InverterRL_Channel * LENZE_MOTORB_OFFSET ), 8, processINVError, NULL, 0, 0 },
		{ NULL, InverterRR_COBID+COBERR_ID + ( InverterRR_Channel * LENZE_MOTORB_OFFSET ), 8, processINVError, NULL, 0, 1 },
#if MOTORCOUNT > 2
		{ NULL, InverterFL_COBID+COBERR_ID + ( InverterFL_Channel * LENZE_MOTORB_OFFSET ), 8, processINVError, NULL, 0, 2 },
		{ NULL, InverterFR_COBID+COBERR_ID + ( InverterFR_Channel * LENZE_MOTORB_OFFSET ), 8, processINVError, NULL, 0, 3 }
#endif
};

CANData InverterCANNMT[MOTORCOUNT] = {
		{ NULL, InverterRL_COBID+COBNMT_ID + ( InverterRL_Channel * 0x100 ), 2, processINVNMT, NULL, 0, 0 },
		{ NULL, InverterRR_COBID+COBNMT_ID + ( InverterRR_Channel * 0x100 ), 2, processINVNMT, NULL, 0, 1 },
#if MOTORCOUNT > 2
		{ NULL, InverterFL_COBID+COBNMT_ID + ( InverterFL_Channel * 0x100 ), 2, processINVNMT, NULL, 0, 2 },
		{ NULL, InverterFR_COBID+COBNMT_ID + ( InverterFR_Channel * 0x100 ), 2, processINVNMT, NULL, 0, 3 }
#endif
};

#define TPDSTATUS_ID 			( LENZE_TPDO2_ID )
CANData InverterCANMotorStatus[MOTORCOUNT] = { // status values
		{ NULL, InverterRL_COBID + TPDSTATUS_ID + ( InverterRL_Channel * 0x100 ), 8, processINVStatus, NULL, 0, 0 },
		{ NULL, InverterRR_COBID + TPDSTATUS_ID + ( InverterRR_Channel * 0x100 ), 8, processINVStatus, NULL, 0, 1 },
#if MOTORCOUNT > 2
		{ NULL, InverterFL_COBID + TPDSTATUS_ID + ( InverterFL_Channel * 0x100 ), 8, processINVStatus, NULL, 0, 2 },
		{ NULL, InverterFR_COBID + TPDSTATUS_ID + ( InverterFR_Channel * 0x100 ), 8, processINVStatus, NULL, 0, 3 }
#endif
};

#define TPDVal1_ID				( LENZE_TPDO3_ID )
CANData InverterCANMotorValues1[MOTORCOUNT] = { // speed/torque
		{ NULL, InverterRL_COBID + TPDVal1_ID + ( InverterRL_Channel * 0x100 ), 8, processINVValues1, NULL, INVERTERTIMEOUT, 0 },
		{ NULL, InverterRR_COBID + TPDVal1_ID + ( InverterRR_Channel * 0x100 ), 8, processINVValues1, NULL, INVERTERTIMEOUT, 1 },
#if MOTORCOUNT > 2
		{ NULL, InverterFL_COBID + TPDVal1_ID + ( InverterFL_Channel * 0x100 ), 8, processINVValues1, NULL, INVERTERTIMEOUT, 2 },
		{ NULL, InverterFR_COBID + TPDVal1_ID + ( InverterFR_Channel * 0x100 ), 8, processINVValues1, NULL, INVERTERTIMEOUT, 3 }
#endif
};

#define TPDVal2_ID				( LENZE_TPDO4_ID )
CANData InverterCANMotorValues2[MOTORCOUNT] = { // speed
		{ NULL, InverterRL_COBID + TPDVal2_ID + ( InverterRL_Channel * 0x100 ), 8, processINVValues2, NULL, 0, 0 },
		{ NULL, InverterRR_COBID + TPDVal2_ID + ( InverterRR_Channel * 0x100 ), 8, processINVValues2, NULL, 0, 1 },
#if MOTORCOUNT > 2
		{ NULL, InverterFL_COBID + TPDVal2_ID + ( InverterFL_Channel * 0x100 ), 8, processINVValues2, NULL, 0, 2 },
		{ NULL, InverterFR_COBID + TPDVal2_ID + ( InverterFR_Channel * 0x100 ), 8, processINVValues2, NULL, 0, 3 }
#endif
};

/*
CANData InverterCANAPPCStatus[INVERTERCOUNT] = { // torque
		{ NULL, Inverter1_NodeID+COBTPDO1_ID+LENZE_APPC_OFFSET, 8, processAPPCOnline, NULL, 0, 0 },
#if INVERTERCOUNT > 1
		{ NULL, Inverter2_NodeID+COBTPDO1_ID+LENZE_APPC_OFFSET, 8, processAPPCOnline, NULL, 0, 1 }
#endif
};
*/


// use APPC RDO1 sending as trigger to signify online.
CANData InverterCANMotorRDO[MOTORCOUNT] = { // torque
		{ NULL, InverterRL_COBID+LENZE_RDO_ID + ( InverterRL_Channel * LENZE_MOTORB_OFFSET ), 8, processINVRDO, NULL, 0, 0 },
		{ NULL, InverterRR_COBID+LENZE_RDO_ID + ( InverterRR_Channel * LENZE_MOTORB_OFFSET ), 8, processINVRDO, NULL, 0, 1 },
#if INVERTERCOUNT > 1
		{ NULL, InverterFL_COBID+LENZE_RDO_ID + ( InverterFL_Channel * LENZE_MOTORB_OFFSET ), 8, processINVRDO, NULL, 0, 2 },
		{ NULL, InverterFR_COBID+LENZE_RDO_ID + ( InverterFR_Channel * LENZE_MOTORB_OFFSET ), 8, processINVRDO, NULL, 0, 3 },
#endif
};

CANData InverterCANAPPCRDO[INVERTERCOUNT] = { // torque
		{ NULL, Inverter1_NodeID+LENZE_RDO_ID+LENZE_APPC_OFFSET, 8, processAPPCRDO, NULL, 0, 0 },
#if INVERTERCOUNT > 1
		{ NULL, Inverter2_NodeID+LENZE_RDO_ID+LENZE_APPC_OFFSET, 8, processAPPCRDO, NULL, 0, 1 }
#endif
};

CANData InverterCANAPPCErr[INVERTERCOUNT]= {
		{ NULL, InverterRL_COBID+COBERR_ID+LENZE_APPC_OFFSET + ( InverterRL_Channel * 0x100 ), 8, processAPPCError, NULL, 0, 0 },
#if MOTORCOUNT > 2
		{ NULL, InverterFL_COBID+COBERR_ID+LENZE_APPC_OFFSET + ( InverterFL_Channel * 0x100 ), 8, processAPPCError, NULL, 0, 1 },
#endif
};



// two per MC
void InvResetError( volatile InverterState_t *Inverter )
{
	uint8_t msg[8] = {0};
	msg[0] = 0x80;

	CAN2Send(LENZE_RPDO1_ID + Inverter->COBID + ( Inverter->MCChannel * 0x200 ), 8, msg);
}


void InvReset( volatile InverterState_t *Inverter )
{
	uint8_t msg[8] = {0};
	msg[0] = 0x81;
	msg[0] = Inverter->COBID;

	CAN2Send(0x0, 2, msg);
}

uint8_t InvSend( volatile InverterState_t *Inverter, bool reset )
{
	uint8_t msg1[8] = {0};
	uint8_t msg2[8] = {0};
	uint8_t msgblank[8] = {0};

	int32_t vel = 0;
	int16_t torque = 0;

	if ( Inverter->AllowTorque )
	{
		vel = Inverter->MaxSpeed * 0x4000; //*16; // TODO add gear ratio, rpm multiplied out.  div by 16

		if ( Inverter->Torque_Req < 0 && !Inverter->AllowRegen )
			torque = 0;
		else
			torque = Inverter->Torque_Req;
	}

    // store values for primary request.
	if ( !reset )
		storeLEint16(Inverter->InvCommand, &msg1[0]);
	else
		storeLEint16(0x80, &msg1[0]);
    storeLEint32(vel, &msg1[2]);
    storeLEint16(torque, &msg1[6]);

    // secondary values, what units are these in? they presumably need multiplying up. by 16?
#ifndef BENCHTEST
    storeLEint16(0, &msg2[0]);
    storeLEint16(0, &msg2[2]);
    storeLEint16(0, &msg2[4]); // max power
    storeLEint16(0, &msg2[6]); // max regeneration
#else
    storeLEint16(620*16, &msg2[0]); //max DC voltage
    storeLEint16(400*16, &msg2[2]); // min DC voltage.
    storeLEint16(20*16, &msg2[4]); // max power
    storeLEint16(0, &msg2[6]); // max regeneration
#endif

	if ( Inverter->MCChannel == false )
	{
		CAN2Send( Inverter->COBID+LENZE_RPDO5_ID, 8, msgblank ); // this should probably be disabled, not needed?

		CAN2Send( Inverter->COBID+LENZE_RPDO1_ID, 8, msg1 );
		CAN2Send( Inverter->COBID+LENZE_RPDO2_ID, 8, msg2 );
	} else
	{
		CAN2Send( Inverter->COBID+LENZE_RPDO3_ID, 8, msg1 );
		CAN2Send( Inverter->COBID+LENZE_RPDO4_ID, 8, msg2 );
	}

	return 0;
}


bool processINVNMT( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle ) // try to reread if possible?
{
	uint8_t inv=datahandle->index;

	if ( InverterState[inv].InvState == OFFLINE ) // device marked offline, mark it online.
	{
	//	InverterState[inv].InvStateAct = BOOTUP;
	}

	// mark all inverter statuses as non operational for startup, so that they can be discovered and processed through state machine.

	return true;
}


uint8_t receiveINVNMT( volatile InverterState_t *Inverter)
{
	// TODO check also inverters for bootup state.

	if ( InverterState[Inverter->Motor].InvState < OFFLINE // all valid operational states lower than offine.
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

	if ( Errors.InverterErrorHistoryPosition < 8 ) // add error data to log.
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


		if ( InverterState[inv].InvState >= OFFLINE) //if inverter status not in error yet, put it there.
		{
			InverterState[inv].InvState = INERROR;
		}

		InverterState[datahandle->index].InvState = INERROR;
        if ( Errors.InvAllowReset[datahandle->index] == 1 )
        {
            Errors.InvAllowReset[datahandle->index] = AllowReset;
        }
        Errors.INVReceiveStatus[datahandle->index]++;

		return true;
	} else // bad data, even if bad data, assume error state as this is emergency message.
	{
		InverterState[inv].InvState = INERROR;
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
		{
			InverterState[inv].HighVoltageAvailable = true;
			// ensure that both motors on inverter are updated.
			if ( !InverterState[inv].MCChannel )
				InverterState[inv+1].HighVoltageAvailable = true;
		}
		else
		{
			InverterState[inv].HighVoltageAvailable = false;
			if ( !InverterState[inv].MCChannel )
				InverterState[inv+1].HighVoltageAvailable = false;
		}

		return true;
	} else // bad data.
	{
		return false;
	}
}

// 0x2900:0x05 - Inverter A Supervision: latched status 1
// 0x3100:0x05 - Inverter B Supervision: latched status 1
char * LenzeErrorBitTypeStatus1Str( uint8_t bit )
{
	switch ( bit )
	{
	case 0  : return "New entry in event memory"; // Since the last upload, there is a new entry in the event memory.
	case 1  : return "Event mem full"; // Event memory is full, at least one entry got lost.
	case 2  : return "HW Overcurrent"; // Power section: Hardware has detected overcurrent.
	case 3  : return "Current offset calib fail"; // Power section: Current offset calibration has failed.
	case 4  : return "Thermal sens bad"; // Power section: Thermal sensor is defective.
	case 5  : return "PWR Temp warning"; // Power section: Temperature has reached warning threshold.
	case 6  : return "PWR Temp error"; // Power section: Temperature has reached error threshold.
	case 7  : return "Ixt overload"; // Power section: Ixt overload
	case 8  : return "SW Overcurrent"; // Power section: Software has detected overcurrent.
	case 9  : return "Bad PWM Pattern"; // Power section: Inconsistent PWM pattern.
	case 10 : return "HW Overvolt"; // DC bus: Hardware has detected overvoltage.
	case 11 : return "SW Overvolt"; // DC bus: Software has detected overvoltage.
	case 12 : return "SW Undervolt"; // DC bus: Software has detected undervoltage.
	case 13 : return "Other Inv Fault"; // There is a fault in the other inverter in the MOBILE.
	case 14 : return "Motor Temp sens bad"; // Motor thermal sensor is defective.
	case 15 : return "Motor temp warn"; // Motor temperature has reached warning threshold.
	case 16 : return "Motor temp err"; // Motor temperature has reached error threshold.
	case 17 : return "Motor stator freq high"; // Motor stator frequency is too high.
	case 18 : return "LV supply fail"; // Voltage supply of the MOBILE has failed or is interrupted.
	case 19 : return "PDO Timeout"; // No PDO has been received (time-out).
	case 20 : return "NMT not in operational"; // Network management (NMT) is not in "Operational" state.
	case 21 : return "Program time overflow"; // Program time overflow
	case 22 : return "Mains sync error"; // Mains synchronisation error
	case 23 : return "Position enc signal weak"; // Position encoder signal too weak.
	case 24 : return "Position enc signal too strong"; // Position encoder signal is too strong.
	case 25 : return "Resolver calib fail"; // Resolver calibration has failed.
	case 26 : return "System error in analog or feedback"; // System error, error in analog inputs or motor feedback.
	case 27 : return "Cover open"; // MOBILE cover is open (InterLock).
	case 28 : return "Power locked by APPC"; // Power section has been locked by the Application Controller.
	case 29 : return "ESM err from APPC"; // ESM error has been detected by the Application Controller.
	case 30 : return "MOB Temp warn"; // MOBILE interior temperature has reached warning threshold.
	case 31 : return "MOB Temp err"; // MOBILE interior temperature has reached error threshold.
	default:
		return "Bad bit";
	}
}


//Diagnostic parameter:
//0x2900:0x07 - Inverter A Supervision: latched status 1
//0x3100:0x07 - Inverter B Supervision: latched status 2
char * LenzeErrorBitTypeStatus2Str( uint8_t bit )
{
	switch ( bit )
	{
	case 0  : return "MOB Therm sens bad"; // MOBILE interior thermal sensor is defective.
	case 1  : return "PWR Clamp timeout"; // Power section: Clamping time-out
	case 2  : return "Too high AC voltage on DC"; // Too high superimposed AC voltage in the DC bus.
	case 3  : return "Motor utilisation warn"; // Motor utilisation (I2xt) has reached warning threshold.
	case 4  : return "Motor utilisation err"; // Motor utilisation (I2xt) has reached error threshold.
	case 5  : return "Motor switch off on field weaken"; // Motor was switched off upon active field weakening.
	case 6  : return "Invalid param combination"; // Invalid combination of parameters selected
	case 7  : return "MOB Cover sensor signal weak"; // MOBILE cover sensor: signal too weak
	case 8  : return "Motor conn test fail"; // Motor connection test has failed
	case 9  : return "PWR Ixt warn"; // Power section utilisation (Ixt) has reached the warning threshold

	default:
		return "Bad bit";

	}
}

bool processINVStatus( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	uint8_t inv=datahandle->index;

	uint16_t status = CANRxData[0];

//	uint16_t statusword = getLEint16(&CANRxData[0]);
	uint32_t latchedStatus1 = getLEint32(&CANRxData[2]);
	uint16_t latchedStatus2 = getLEint16(&CANRxData[6]);

	if ( InverterState[inv].SetupState == 0xFF ) // only process once inverter PrivateCan control taken.
	{
		volatile DeviceStatus curinvState = InternalInverterState(status);

		if ( curinvState == INERROR )
		{
			// first error, record time for error time handling.
			if ( InverterState[inv].InvState != INERROR )
				InverterState[inv].errortime = xTaskGetTickCount();

			// new error bits
			if ( InverterState[inv].latchedStatus1 != latchedStatus1 )
			{
				uint32_t newbits = (InverterState[inv].latchedStatus1 ^ latchedStatus1 );

				for ( int i=0;i<32;i++)
				{
					if ( newbits & ( 0x1 << i ) )
					{
						char str[40] = "";
						snprintf(str, 40, "Inv%d %s at (%lu)", inv, LenzeErrorBitTypeStatus1Str(i), gettimer());
						DebugMsg(str);
					}
				}

				InverterState[inv].latchedStatus1 = latchedStatus1;
			}

			if ( InverterState[inv].latchedStatus2 != latchedStatus2 )
			{
				uint32_t newbits = (InverterState[inv].latchedStatus2 ^ latchedStatus2 );

				for ( int i=0;i<9;i++) // only 9 current valid error bits instatus2
				{
					if ( newbits & ( 0x1 << i ) )
					{
						char str[40] = "";
						snprintf(str, 40, "Inv%d %s  at (%lu)", inv, LenzeErrorBitTypeStatus2Str(i), gettimer());
						DebugMsg(str);
					}
				}

				InverterState[inv].latchedStatus2 = latchedStatus2;
			}

//			if ( InverterState[inv].latchedStatus2 & LenzeErrorType1bits2 ) // we've got type errors
		}

		InverterState[inv].InvState = curinvState;

		xTaskNotify( InvTaskHandle, ( 0x1 << (InverterState[inv].Motor*3+0) ), eSetBits);

		return true;
	} else // bad data.
	{
		return false;
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
		xTaskNotify( InvTaskHandle, ( 0x1 << (InverterState[inv].Motor*3+1) ), eSetBits);
		InverterState[inv].Speed = Speed;
		InverterState[inv].InvTorque = Torque;
		InverterState[inv].InvCurrent = Torque;

		return true;
	} else // bad data.
	{
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

	xTaskNotify( InvTaskHandle, ( 0x1 << (InverterState[inv].Motor*3+2) ), eSetBits);
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

bool processAPPCError( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	return true;
}


bool InvStartupState( volatile InverterState_t *Inverter, const uint8_t CANRxData[8] )
{
	uint32_t time = gettimer();
	static char str[60];
	snprintf(str, 60, "Inv %d [%2X %2X %2X %2X state %d] (last %lu) (%lu)", Inverter->Motor, CANRxData[0], CANRxData[1], CANRxData[2], CANRxData[3], Inverter->SetupState,Inverter->SetupLastSeenTime, time);
	DebugMsg(str);

	// PDO timeout set in lenze software right now.
	 // set SDO's to sync   0x1800-1806  = lenze TPDO 1 through 7
	if ( Inverter->MCChannel == false )
	{
		// state gets moved by reply.
		switch ( Inverter->SetupState )
		{
		case 0:
			DebugMsg("called in state 0"); // don't do anything in state 0
			break;

		case 1:
			// received startup message, start timer on last seen SDO message.
			Inverter->SetupLastSeenTime = time;
			break;

		case 2:
		{
			char str[40];
			snprintf(str, 40, "Lenze inverter %d state 2 at (%lu)", Inverter->Motor, time);
			DebugMsg(str);
			Inverter->SetupLastSeenTime = time;
			InverterState[Inverter->Motor+1].SetupLastSeenTime = time;
			CANSendSDO(bus0, Inverter->COBID+31, 0x4004, 1, 1234); // sets private can.
			Inverter->SetupState = 3;
			return true;
		}
		case 3:
		{
			uint8_t RDODone[8] = { 0x60, 0x04, 0x40, 0x01, 0, 0, 0, 0};

			if ( memcmp(RDODone, CANRxData, 4) == 0 )
			{
				Inverter->SetupLastSeenTime = time;
				InverterState[Inverter->Motor+1].SetupLastSeenTime = time;
#if 1
				char str[40];
				snprintf(str, 40, "Lenze inverter %d rcv APPC SDO in state 3 at (%lu)", Inverter->Motor, time);
				DebugMsg(str);
#endif
				Inverter->SetupState = 3; // start the setup state machine
				CANSendSDO(bus0, Inverter->COBID, 0x1800, 2, 1);
				return true;
			}
			break;

		}

		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		{
			uint8_t RDODone[8] = { 0x60, Inverter->SetupState-3, 0x18, 0x02 };
			if ( memcmp(RDODone, CANRxData, 8) == 0 )
			{
				Inverter->SetupLastSeenTime = time;
				InverterState[Inverter->Motor+1].SetupLastSeenTime = time;
#if 1
				char str[40];
				snprintf(str, 40, "Lenze inverter %d rcv SDO in state %d at (%lu)", Inverter->Motor,  Inverter->SetupState, time );
				DebugMsg(str);
#endif
				if ( Inverter->SetupState < 9)
				{
					CANSendSDO(bus0, Inverter->COBID, 0x1800+Inverter->SetupState-2, 2, 1); // sets TPDO's to sync mode.
					Inverter->SetupState++;
				}
				else
				{
					char str[60];
					snprintf(str, 60, "Lenze inverter %d ready at (%lu)", Inverter->Motor, gettimer());
					DebugMsg(str);
					InvSend(Inverter, true);
					Inverter->SetupState = 0xFF; // Done!
					InvSend(&InverterState[Inverter->Motor+1], true);
					InverterState[Inverter->Motor+1].SetupState = 0xFF; // set the other inverter as configured too.
				}
				return true;
			}
			break;
		}

		default:
			Inverter->SetupState = 0;
			InverterState[Inverter->Motor+1].SetupState = 0;
		}
	}



//	Inverter->SetupState = 0;
//	InverterState[Inverter->Motor+1].SetupState = 0;

	return true;
}

bool InvStartupCfg( volatile InverterState_t *Inverter )
{
	// forcibly start the setup state machine.
//	Inverter->SetupState = 1;
//	InvStartupState( Inverter, (uint8_t[8]){0,0,0,0,0,0,0,0} );
	return true;
}


bool processAPPCRDO( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	if ( InverterState[datahandle->index].SetupState > 0)
	{
//		InvStartupState( &InverterState[datahandle->index], CANRxData );
	}

	return true;
}


bool processINVRDO( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
//	uint32_t bitset=(0x1 << datahandle->index);

	if ( InverterState[datahandle->index].SetupState > 0 && InverterState[datahandle->index].SetupState < 0xFE)
	{

		uint8_t INVREBOOT[8] = {0x43, 0x56, 0x1F, 0x01 };

		// check if APPC has just restarted
		if ( memcmp(INVREBOOT, CANRxData, 4) == 0 )
		{
			char str[80];
			snprintf(str, 80, "Lenze inverter %d starting APPC setup again unexpectedly at (%lu)", datahandle->index, gettimer());
			DebugMsg(str);
			// set inv offline here.
			InverterState[datahandle->index].SetupState = 0;
//			InvStartupState( &InverterState[datahandle->index], CANRxData );
			return true;
		} else
			InvStartupState( &InverterState[datahandle->index], CANRxData );
	}
	else
	{
		if ( InverterState[datahandle->index].SetupState == 0 )
		{
			uint8_t RDODone[8] = { 0x43, 0x56, 0x1F, 0x01 }; // ID query only done once at startup.

			if ( memcmp(RDODone, CANRxData, 4) == 0 )
			{
				if ( !InverterState[datahandle->index].MCChannel )
				{
				//	if ( CanRxData[] )  // 0x1801
		#ifdef DEBUGAPPCSDO
					char str[80];
					snprintf(str, 80, "Lenze inverter %d starting APPC setup, waiting at (%lu)", datahandle->index, gettimer());
					DebugMsg(str);
		#endif
					InverterState[datahandle->index].SetupLastSeenTime = gettimer();
					InverterState[InverterState[datahandle->index].Motor+1].SetupLastSeenTime = gettimer();
					// ensure timer is set before state changed.
					InverterState[datahandle->index].SetupState = 1; // start the setup state machine
					InvStartupState(&InverterState[datahandle->index], CANRxData);
				}
			}
		}
	}

	return true;
}


void SpeedCalculation( int32_t leftdata )
{
/*	CarState.Wheel_Speed_Right_Calculated = Speed_Right_Inverter.data.longint * (1/4194304) * 60; // resolution 4194304 for one revolution
	CarState.Wheel_Speed_Left_Calculated = Speed_Left_Inverter.data.longint * (1/4194304) * 60;
	CarState.Wheel_Speed_Rear_Average = (CarState.Wheel_Speed_Right_Calculated  + CarState.Wheel_Speed_Left_Calculated)/2;
*/
}

uint32_t getInvExpected( uint8_t inv )
{
	return (0b111 << (inv * 3)); // three message flags per motor, status, vals1, vals2
}

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
		RegisterCan2Message(&InverterCANMotorRDO[i]);
	}

	RegisterCan2Message(&InverterCANAPPCRDO[0]);

	return true;
}

#endif
