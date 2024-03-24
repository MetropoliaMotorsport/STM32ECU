/*
 * brake.c
 *
 *  Created on: 05 May 2021
 *      Author: Visa
 */

#include "ecumain.h"
#include "brake.h"


bool getBrakeLight(void) {
	return (CarState.RegenLight || ADCState.BrakeR >= APPSBrakeLight
			|| ADCState.BrakeF >= APPSBrakeLight);
}

bool getBrakeLow(void) {
//	if ( CarState.AllowRegen && getEEPROMBlock(0)->Regen && ADCState.Regen_Percent < 500 ) return true;
	return (ADCState.BrakeR < APPSBrakeRelease
			|| ADCState.BrakeF < APPSBrakeRelease);
}

uint8_t getBrakeHigh(void) {
//	if ( CarState.AllowRegen && getEEPROMBlock(0)->Regen && ADCState.Regen_Percent > 500 ) return true;
	return ADCState.BrakeR >= APPSBrakeHard || ADCState.BrakeF >= APPSBrakeHard;
}

uint8_t getBrakeRTDM(void) {
//	if ( CarState.AllowRegen && getEEPROMBlock(0)->Regen && ADCState.Regen_Percent > 500 ) return true;
	return ADCState.BrakeR >= RTDMBRAKEPRESSURE
			|| ADCState.BrakeF >= RTDMBRAKEPRESSURE;
}

uint8_t getBrake(void) {
	if (DeviceState.BrakeLight == OPERATIONAL)
		return (ADCState.BrakeR + ADCState.BrakeF) / 2;
	else
		return APPSBrakeHard;
}

void resetBrake(void) {
	//DeviceState.BrakeLight = Offline;
}

int initBrake(void) {
	RegisterResetCommand(resetBrake);

	resetBrake();
	return 0;
}
