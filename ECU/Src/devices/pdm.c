/*
 * interrupts.c
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#include "ecumain.h"

//	0x520,0,8 -> BMS_relay_status
//	0x520,8,8 -> IMD_relay_status
//	0x520,16,8 -> BSPD_relay_status
void processPDM(uint8_t CANRxData[8], uint32_t DataLength )
{
	static uint8_t receiveerror = 0;
	CanState.PDM.time = gettimer();

	if ( DataLength == FDCAN_DLC_BYTES_8
		&& CANRxData[0] < 2
		&& CANRxData[1] < 2
		&& CANRxData[2] < 2
//		&& CANRxData[3] < 2 AIR voltage - should be between ~10-16v
//		&& CANRxData[4] < 2 PDM voltage  - should be between
//		&& CANRxData[5] < 2 PDM Current - should be between 0 - to maybe 6?
		&& CANRxData[6] < 2 // shutdown switch status.
		&& CANRxData[7] < 2 // shutdown switch status.
		&& ( CANRxData[6] == CANRxData[7] ) // all three last bytes are sending AIR status to help verification.
		)
	{
		receiveerror=0;

		DeviceState.PDM = OPERATIONAL; // received data without error bit set, so we can assume operational state
		CarState.BMS_relay_status = CANRxData[0];
		CarState.IMD_relay_status = CANRxData[1];
		CarState.BSPD_relay_status = CANRxData[2];
		CarState.VoltageLV = (CANRxData[4] * 1216)/10;
		CarState.CurrentLV = CANRxData[5];
		CarState.ShutdownSwitchesClosed = CANRxData[6];
		if (!CarState.ShutdownSwitchesClosed)
		{
			setOutputNOW(LED4_Output, LEDON);
		} else setOutputNOW(LED4_Output, LEDOFF);

		CarState.VoltageAIRPDM = (CANRxData[3] * 200);

		if ( ( CarState.VoltageAIRPDM < CarState.VoltageLV*0.54 ) && ( CarState.VoltageAIRPDM > CarState.VoltageLV*0.46 ) ) //  63% == contactors fully closed ~50% when all open., try to establish 1/2 closed?
			CarState.AIROpen = 1;
		else
			CarState.AIROpen = 0;

		DeviceState.PDM = OPERATIONAL;
	} else // bad data.
	{
		receiveerror++;
		Errors.CANError++;
		Errors.PDMReceive++;
/*		if ( receiveerror > 10 ), device is still responding, so don't put it offline.
		{
			CarState.VoltageBMS=0;
			DeviceState.BMS = OFFLINE;
			return 0; // returnval = 0;
		} */
#ifdef SENDBADDATAERROR
		CAN_SendStatus(99,PDMReceived,99);
#endif
		reTransmitError(99,CANRxData, DataLength);
	}
}

int receivedPDM( void )
{
	uint32_t time = gettimer();
	if ( CanState.PDM.time+PDMTIMEOUT >= time )
    {
		return 1;
    }
	else
	{
		return 0;
	}
}


int receivePDM( void )
{
	uint32_t time=gettimer();
	static uint8_t errorsent;

#ifdef NOTIMEOUT
		if ( DeviceState.PDM == OPERATIONAL )
		{
			errorsent = 0;
			return 1;
		} else return 0;
#endif

	if ( time - CanState.PDM.time <= PDMTIMEOUT && DeviceState.PDM == OPERATIONAL )
	{
		errorsent = 0;
		return 1;
	} else
	{
        /* T 11.9.3
         * Safe state is defined depending on the signals as follows:
         • signals only influencing indicators – Indicating a failure of its own function or of the connected system
         */

	//	if ( time - CanState.PDM.time > PDMTIMEOUT )
        if ( DeviceState.PDM == OPERATIONAL )
		{
            CarState.BMS_relay_status = 1;
            CarState.IMD_relay_status = 1;
            CarState.BSPD_relay_status = 1;
            CarState.AIROpen = 0;

			if ( errorsent == 0 )
			{
				CAN_SendStatus(200,PDMReceived,(time-CanState.PDM.time)/10);
				errorsent = 1;
				Errors.CANTimeout++;
				Errors.PDMTimeout++;
				DeviceState.PDM = OFFLINE;
			}
			return 0;
		}
		return 0; // PDM is SCS, must always time out.
	}
	return 0;
}

int errorPDM( void )
{
	int returnval = 0;

    if ( receivePDM() == 0 )
    {
    	returnval +=1;
    }

	if ( CarState.BMS_relay_status == 1 )
	{
		returnval +=2;
        blinkOutput(BMSLED_Output,LEDON,0); // ensure potential limp mode blinking disabled.
		setOutputNOW(BMSLED_Output,LEDON);
	} else setOutput(BMSLED_Output,LEDOFF);

	if ( CarState.IMD_relay_status == 1 )
	{
		returnval +=4;
		setOutputNOW(IMDLED_Output,LEDON);
	} else setOutput(IMDLED_Output,LEDOFF);

	if ( CarState.BSPD_relay_status == 1 )
	{
		returnval +=8;
		setOutputNOW(BSPDLED_Output,LEDON);
	} else setOutput(BSPDLED_Output,LEDOFF);

	/* EV 4.10.3
	 * The TS is deactivated when ALL of the following conditions are true:
	 * All accumulator isolation relays are opened.
	 * The pre-charge relay, see EV 5.7.3, is opened.
	 * The voltage outside the accumulator container(s) does not exceed 60V DC or 25V AC RMS.
	 *
	 * AIROpen indicates all relays
	 * VoltageINV & VoltageIVTAccu indicate voltage outside accumulator, with fallback for IVT not working.
	 */

#ifdef SHUTDOWNSWITCHSTATUS // use mid dash led for shutdown switch
#ifndef TORQUEVECTOR
	if ( CarState.ShutdownSwitchesClosed )
	{
		setOutput(TSOFFLED_Output,LEDON);
	} else
	{
		setOutput(TSOFFLED_Output,LEDOFF);
	}
#endif
#else // use mid dash led for TSOFF status.

	if (
#ifdef IVTEnable
			CarState.VoltageINV  > 59 || CarState.VoltageIVTAccu > 59 ||
#endif
		CarState.AIROpen == 0 || DeviceState.IVT == OFFLINE ) // doesn't effect error state, just updates as other PDM derived LED's updated here. SCS Signal, move to PDM ideally.
        // currently will default to showing HV if timeout, which should make correct.
	{
		 setOutput(TSOFFLED_Output,LEDOFF);
	} else setOutput(TSOFFLED_Output,LEDON);
#endif
	return returnval;
}

int requestPDM( int nodeid )
{
	return 0; // this is operating with cansync, no extra needed.
}

int sendPDM( int buzzer )
{
#ifdef PDMSECONDMESSAGE
	CANSendPDMFAN();
#endif
	bool HVR = true;
	for ( int i = 0;i<INVERTERCOUNT;i++)
	{
		if ( ! CarState.Inverters[i].HighVoltageAllowed) HVR = false;
	} // HVR will be false if any of the inverters are not in true state.

	if ( ( HVR && CarState.HighVoltageReady ) || CarState.TestHV )
		return CANSendPDM(10,buzzer);
	else
		return CANSendPDM(0,buzzer);

}

