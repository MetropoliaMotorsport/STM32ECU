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
		const uint16_t field);

void IVTTimeout(uint16_t id);

CANData IVTMsg = { &DeviceState.IVT, IVTMsg_ID, 6, processIVT, NULL, 0 };

uint8_t processIVT(uint8_t CANRxData[8], uint32_t DataLength, uint16_t field) {
	CANData *datahandle = &IVTCan[field - IVTBase_ID];
	processCANData(datahandle, CANRxData, DataLength);
	return 0;
}


CANData IVTCan[8] = { 
	 { &DeviceState.IVT, IVTBase_ID, 6, processIVTData, IVTTimeout, IVTTIMEOUT },
	 { &DeviceState.IVT, IVTBase_ID + 1, 6,	processIVTData, NULL, 0 },
	 { &DeviceState.IVT, IVTBase_ID + 2, 6, processIVTData, NULL, 0 },
	 { &DeviceState.IVT, IVTBase_ID + 3, 6, processIVTData, NULL, 0 },
	 { &DeviceState.IVT, IVTBase_ID + 4, 6, processIVTData, NULL, 0 },
	 { &DeviceState.IVT, IVTBase_ID + 5, 6, processIVTData, NULL, 0 },
	 { &DeviceState.IVT, IVTBase_ID + 6, 6, processIVTData, NULL, 0 },
	 { &DeviceState.IVT, IVTBase_ID + 7, 6, processIVTData, NULL, 0 } };

// IVTWh = 0x528
bool processIVTData(const uint8_t CANRxData[8], const uint32_t DataLength, //TODO add safety checks for IVT data
		const CANData *datahandle) {
		
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

		return processIVTData(CANRxData, DataLength, IVTI_ID);
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

		int returnval = receivedCANData(&IVTCan[0]);

		return returnval;
	} else // IVT reading disabled, set 'default' values to allow operation regardless.
	{
		*IVTCan[0].devicestate = OPERATIONAL;
		CarState.VoltageINV = 540; // set an assumed voltage that forces TSOFF indicator to go out on timeout for SCS.
		CarState.VoltageIVTAccu = 540;
		CarState.Power = 0;
		CarState.Current = 0;
		return 1;
	}
}


int IVTstate(void) // not currently being used, not properly functional
{
	if (IVTMsg.time > 0) // packet has been received
			{
		{
			DeviceState.IVT = OPERATIONAL;
		}
		return 1;

	} else if (gettimer() - IVTCan[IVTBase_ID - IVTU1_ID].time < IVTTIMEOUT) {
		return 1;
	} else
		return 0;
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

	for (int i = 0; i < 8; i++)
		RegisterCan1Message(&IVTCan[i]);

	return 0;
}
