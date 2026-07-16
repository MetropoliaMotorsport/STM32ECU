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
	return (CarState.RegenLight ||BrakeRear.data >= DefaultAPPSBrakeLight
			|| BrakeFront.data >= DefaultAPPSBrakeLight);
}

bool getBrakeLow(void) {
//	if ( CarState.AllowRegen && getEEPROMBlock(0)->Regen && BPPS.data_Percent < 500 ) return true;
	return (BrakeRear.data < DefaultAPPSBrakeRelease
			|| BrakeFront.data < DefaultAPPSBrakeRelease);
}

uint8_t getBrakeHigh(void) {
//	if ( CarState.AllowRegen && getEEPROMBlock(0)->Regen && BPPS.data_Percent > 500 ) return true;
	return BrakeRear.data >= DefaultAPPSBrakeHard || BrakeFront.data >= DefaultAPPSBrakeHard;
}

uint8_t getBrakeRTDM(void) {
//	if ( CarState.AllowRegen && getEEPROMBlock(0)->Regen && BPPS.data_Percent > 500 ) return true;
	return BrakeRear.data >= DefaultRTDMBRAKEPRESSURE
			|| BrakeFront.data >= DefaultRTDMBRAKEPRESSURE;
}

void resetBrake(void) {
	DeviceState.BrakeLight = OFFLINE;
}

int initBrake(void) {
	RegisterResetCommand(resetBrake);

	resetBrake();
	return 0;
}
