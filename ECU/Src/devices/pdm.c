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
		&& CANRxData[5] < 2
		&& CANRxData[6] < 2
		&& CANRxData[7] < 2
		&& ( CANRxData[5] == CANRxData[6] && CANRxData[6] == CANRxData[7] ) // all three last bytes are sending AIR status to help verification.
		)
	{
		receiveerror=0;

		DeviceState.PDM = OPERATIONAL; // received data without error bit set, so we can assume operational state
		CarState.BMS_relay_status = CANRxData[0];
		CarState.IMD_relay_status = CANRxData[1];
		CarState.BSPD_relay_status = CANRxData[2];

		CarState.AIROpen = CANRxData[7];

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

void processPDMVolts(uint8_t CANRxData[8], uint32_t DataLength )
{
	static uint8_t receiveerror = 0;
	CanState.PDM.time = gettimer();

	if ( DataLength == FDCAN_DLC_BYTES_8
//		&& CANRxData[0] < 2
//		&& CANRxData[1] < 2
		&& CANRxData[2] == 0
		&& CANRxData[3] == 0
		&& CANRxData[4] == 0
		&& CANRxData[5] == 0
		&& CANRxData[6] == 0
		&& CANRxData[7] == 0
		)
	{
		receiveerror=0;

		DeviceState.PDM = OPERATIONAL; // received data without error bit set, so we can assume operational state
		CarState.AIROpen = CANRxData[0];
		CarState.LVVoltage = CANRxData[1];

		CarState.AIROpen = CANRxData[7];

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

	if ( CarState.VoltageINV  > 59 || CarState.AIROpen == 0 ) // doesn't effect error state, just updates as other PDM derived LED's updated here. SCS Signal, move to PDM ideally.
        // currently will default to showing HV if timeout, which should make correct.
	{
		 setOutput(TSOFFLED_Output,LEDOFF);
	} else setOutput(TSOFFLED_Output,LEDON);

	return returnval;
}

int requestPDM( int nodeid )
{
	return 0; // this is operating with cansync, no extra needed.
}

int sendPDM( int buzzer )
{
	if ( CarState.HighVoltageAllowedL && CarState.HighVoltageAllowedR && CarState.HighVoltageReady )
		return CANSendPDM(10,buzzer);
	else
		return CANSendPDM(0,buzzer);
}

