/*
 * ivt.c
 *
 *  Created on: 01 May 2019
 *      Author: Visa
 */

// 1041 ( 411h )  52 1 0 0 -< turn on
// 1041 ( 411h )  49 0 175 0<- trigger message
#include "ecumain.h"
#include "ivt.h"
#include "timerecu.h"
#include "errors.h"
#include "eeprom.h"

bool processIVTData(const uint8_t *CANRxData, const uint32_t DataLength,
		CANData *datahandle);

void IVTTimeout(uint16_t id);


CANData IVT_I = { &DeviceState.IVT, IVTI_ID, 6, processIVTData, IVTTimeout, IVTTIMEOUT };
CANData IVT_U1 = { &DeviceState.IVT, IVTU1_ID, 6, processIVTData, IVTTimeout, IVTTIMEOUT };
CANData IVT_U2 = { &DeviceState.IVT, IVTU2_ID, 6, processIVTData, IVTTimeout, IVTTIMEOUT };
CANData IVT_W = { &DeviceState.IVT, IVTW_ID, 6, processIVTData, IVTTimeout, IVTTIMEOUT };
CANData IVT_Wh = { &DeviceState.IVT, IVTWh_ID, 6, processIVTData, IVTTimeout, IVTTIMEOUT };


// IVTWh = 0x528
bool processIVTData(const uint8_t CANRxData[8], const uint32_t DataLength, //TODO add safety checks for IVT data
		CANData *datahandle) {
		
	datahandle->data = (uint32_t)(CANRxData[5] | (CANRxData[4] << 8) | (CANRxData[3] << 16) | (CANRxData[2] << 24) );

	switch (datahandle->id)
	{
	case IVTI_ID:
		CarState.Current = datahandle->data;
		break;
	case IVTU1_ID:
		CarState.VoltageIVTAccu = datahandle->data;
		break;
	case IVTU2_ID:
		CarState.VoltageINV = datahandle->data;

		((CarState.VoltageINV > 450000) ? (CarState.HV_on = true) : (CarState.HV_on = false)); // if voltage is greater than 450V, set HV_on to true (1
		
		break;
	case IVTW_ID:
		CarState.Power = datahandle->data;
		break;
	case IVTWh_ID:
		CarState.Wh = datahandle->data;
		break;	
	default:
		break;
	}

	DeviceState.IVT = OPERATIONAL;

	if (DeviceState.IVTEnabled) {

		return true;
	} else
		return true;
}



void IVTTimeout(uint16_t id) {

	CarState.Power = 0;
	CarState.Current = 0;
	CarState.VoltageINV = 0;

	SetCriticalError(CRITERIVT);
}

int receiveIVT(void) {
	if (DeviceState.IVTEnabled) {

		int returnval = 1;

		return returnval;
	} else // IVT reading disabled, set 'default' values to allow operation regardless.
	{

		CarState.VoltageINV = 540; // set an assumed voltage that forces TSOFF indicator to go out on timeout for SCS.
		CarState.VoltageIVTAccu = 540;
		CarState.Power = 0;
		CarState.Current = 0;
		return 1;
	}
}


void resetIVT(void) {

	DeviceState.IVTEnabled = ENABLED;
	DeviceState.IVT = OFFLINE;
	CarState.Power = 0;
	CarState.Current = 0;
}

int initIVT(void) {
	RegisterResetCommand(resetIVT);

	resetIVT();

	RegisterCan1Message(&IVT_I);
	RegisterCan1Message(&IVT_U1);
	RegisterCan1Message(&IVT_U2);
	RegisterCan1Message(&IVT_W);
	RegisterCan1Message(&IVT_Wh);

	return 0;
}
