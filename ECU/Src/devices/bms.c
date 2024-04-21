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

#ifdef HPF2023
bool processBMSSOC(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
#else
bool processBMSVoltageData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processBMSOpMode( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processBMSError( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
#endif

void BMSTimeout(uint16_t id);

#ifdef HPF2023
CANData BMSSOC = { &DeviceState.BMS, BMSSOC_ID, 8, processBMSSOC, BMSTimeout,
		6000 };
#else
CANData BMSVoltage = {&DeviceState.BMS, BMSVOLT_ID, 8, processBMSVoltageData, BMSTimeout, 2500};
CANData BMSOpMode = {&DeviceState.BMS, BMSBASE_ID, 8, processBMSOpMode, NULL, 0};
CANData BMSError = {&DeviceState.BMS, BMSBASE_ID+1, 8, processBMSError, NULL, 0};
#endif


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

int receiveBMS(void) {
	if (DeviceState.BMSEnabled) {
#ifdef HPF2023
		return receivedCANData(&BMSSOC);
#else
		return receivedCANData(&BMSVoltage);
#endif
	} else // BMS reading disabled, set 'default' values to allow operation regardless.
	{
		DeviceState.BMS = OPERATIONAL;
		CarState.VoltageBMS = 540; // set an assumed voltage that allows operation.
		return 1;
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
#ifdef HPF2023
	RegisterCan2Message(&BMSSOC);
#else
	RegisterCan2Message(&BMSVoltage);
	RegisterCan2Message(&BMSOpMode);
	RegisterCan2Message(&BMSError);
#endif
	return 0;
}

