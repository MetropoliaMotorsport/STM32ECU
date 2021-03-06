/*
 * brake.c
 *
 *  Created on: 05 May 2021
 *      Author: Visa
 */

#include "ecumain.h"
#include "brake.h"
#include "adcecu.h"


bool getBrakeLow( void )
{
	return ( ADCState.BrakeR < APPSBrakeRelease || ADCState.BrakeF < APPSBrakeRelease );
}

uint8_t getBrakeHigh( void )
{
	return ADCState.BrakeR >= APPSBrakeHard || ADCState.BrakeF >= APPSBrakeHard; // TODO add regen sensor.
}

uint8_t getBrake( void )
{
	if ( DeviceState.BrakeLight == OPERATIONAL )
		return ( ADCState.BrakeR + ADCState.BrakeF ) / 2;
	else
		return APPSBrakeHard;
}

void resetBrake( void )
{
	//DeviceState.BrakeLight = Offline;
}

int initBrake( void )
{
	RegisterResetCommand(resetBrake);

	resetBrake();
	return 0;
}
