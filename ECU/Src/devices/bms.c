/*
 * bms.c
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#include "ecumain.h"
#include "errors.h"
#include "canecu.h"
#include "bms.h"
#include "output.h"
#include "power.h"
#include "debug.h"

// bms operation mode, byte 4   normal mode, data logging.
// byte 5, cell with min voltage - mv, use to trigger
// 0x9   byte 6-7 last two.


bool processBMSSOC(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
void BMSTimeout(uint16_t id);

CANData BMSSOC = { &DeviceState.BMS, BMSSOC_ID, 8, processBMSSOC, BMSTimeout,
		6000 };

void BMSTimeout(uint16_t id) {
	setOutputNOW(BMSLED, On);
	Shutdown.BMS = true;
	DebugMsg("BMS Timeout");
	CAN_SendErrorStatus(199, 0, 0);
	if (DeviceState.BMS != OFFLINE) {
		CarState.VoltageBMS = 0;
		SetCriticalError(CRITERRBMSTIMEOUT);
	}
}


void resetBMS(void) {
#ifdef BMSEnable
	DeviceState.BMSEnabled = ENABLED;
#else
	DeviceState.BMSEnabled = DISABLED;
#endif

	DeviceState.BMS = OFFLINE;
	CarState.VoltageBMS = 0;
}

int initBMS(void) {
	RegisterResetCommand(resetBMS);
	resetBMS();
	RegisterCan2Message(&BMSSOC);

	return 0;
}

