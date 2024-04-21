/*
 * brake.c
 *
 *  Created on: 05 May 2021
 *      Author: Visa
 */

#include "ecumain.h"
#include "brake.h"
#include "node_device.h"


bool getBrakeLight(void) {
	return (CarState.RegenLight ||BrakeRear.data >= APPSBrakeLight
			|| BrakeFront.data >= APPSBrakeLight);
}

bool getBrakeLow(void) {
//	if ( CarState.AllowRegen && getEEPROMBlock(0)->Regen && BPPS.data_Percent < 500 ) return true;
	return (BrakeRear.data < APPSBrakeRelease
			|| BrakeFront.data < APPSBrakeRelease);
}

uint8_t getBrakeHigh(void) {
//	if ( CarState.AllowRegen && getEEPROMBlock(0)->Regen && BPPS.data_Percent > 500 ) return true;
	return BrakeRear.data >= APPSBrakeHard || BrakeFront.data >= APPSBrakeHard;
}

uint8_t getBrakeRTDM(void) {
//	if ( CarState.AllowRegen && getEEPROMBlock(0)->Regen && BPPS.data_Percent > 500 ) return true;
	return BrakeRear.data >= RTDMBRAKEPRESSURE
			|| BrakeFront.data >= RTDMBRAKEPRESSURE;
}

void resetBrake(void) {
	//DeviceState.BrakeLight = Offline;
}

int initBrake(void) {
	RegisterResetCommand(resetBrake);

	resetBrake();
	return 0;
}
