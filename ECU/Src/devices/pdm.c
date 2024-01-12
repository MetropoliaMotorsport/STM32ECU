/*
 * pdm.c
 *
 *  Created on: 01 May 2019
 *      Author: Visa
 */

#include "ecumain.h"
#include "pdm.h"
#include "output.h"
#include "power.h"


bool processPDMData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
void PDMTimeout( uint16_t id );


CANData PDMCanData = { &DeviceState.PDM, PDM_ID, 8, processPDMData, PDMTimeout, PDMTIMEOUT };


//	0x520,0,8 -> BMS_relay_status
//	0x520,8,8 -> IMD_relay_status
//	0x520,16,8 -> BSPD_relay_status
bool processPDMData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
#ifdef PDM
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
		CarState.Shutdown.BMS = CANRxData[0];
		CarState.Shutdown.IMD = CANRxData[1];
		CarState.Shutdown.BSPDBefore = CANRxData[2];
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
		return true;
	}
	else
		return false;
#else
	return true;
#endif
}


void PDMTimeout( uint16_t id )
{
    /* T 11.9.3
     * Safe state is defined depending on the signals as follows:
     • signals only influencing indicators – Indicating a failure of its own function or of the connected system

  	  -- thus show a timeout as error status.

     */

    Shutdown.BMS = true;
    Shutdown.IMD = true;
    Shutdown.BSPDBefore = true;
    Shutdown.AIRm = false;
    Shutdown.AIRp = false;
    Shutdown.PRE = false;
}

int receivePDM( void )
{
	return receivedCANData(&PDMCanData);
}

/*
CanData * PDMCAN( void )
{
	return PDMCanData;
}
*/

int errorPDM( void )
{
	int returnval = 0;

#ifndef POWERNODES
    if ( receivePDM() == 0 )
    {
    	returnval +=1;
    }
#endif

	if ( Shutdown.BMS )
	{
		returnval +=2;
		//stopBlinkOutput(BMSLED); // ensure potential limp mode blinking disabled.
		//setOutputNOW(BMSLED,On);
	} //else setOutput(BMSLED,Off);

	if ( Shutdown.IMD )
	{
		returnval +=4;
		setOutputNOW(IMDLED,On);
	} else setOutput(IMDLED,Off);

	if ( Shutdown.BSPDBefore )
	{
		returnval +=8;
		setOutputNOW(BSPDLED,On);
	} else setOutput(BSPDLED,Off);

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
	if ( CheckShutdown() )
	{
		setOutput(TSOFFLED,On);
	} else
	{
		setOutput(TSOFFLED,Off);
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
		 setOutput(TSOFFLED,Off);
	} else setOutput(TSOFFLED,On);
#endif
	return returnval;
}

int requestPDM( int nodeid )
{
	return 0; // this is operating with cansync, no extra needed.
}

int sendPDM( bool buzzer )
{
#ifdef PDMSECONDMESSAGE
	CANSendPDMFAN();
#endif
	bool HVR = true;
	for ( int i = 0;i<MOTORCOUNT;i++)
	{
		// TODO inverter check HVA
		//if ( ! CarState.Inverters[i].HighVoltageAllowed) HVR = false;
	} // HVR will be false if any of the inverters are not in true state.

	if ( HVR && getPowerHVReady() )
		return CANSendPDM(10,buzzer);
	else
		return CANSendPDM(0,buzzer);

}

void resetPDM ( void )
{
//	PDMCanData.seen = false;
	DeviceState.PDM = OFFLINE;

//	setShutdown(shtBMS, true);
	Shutdown.BMS = true; // these are latched
	Shutdown.IMD = true;
	Shutdown.BSPDBefore = true;

	Shutdown.AIRm = false;
	Shutdown.AIRp = false;
	Shutdown.PRE = false;
	Shutdown.CockpitButton = true;
}

void initPDM( void )
{

	RegisterResetCommand(resetPDM);
	resetPDM();

	RegisterCan1Message(&PDMCanData);
}

