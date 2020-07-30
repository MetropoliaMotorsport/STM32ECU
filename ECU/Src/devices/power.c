/*
 * power.c
 *
 *  Created on: Jul 17, 2020
 *      Author: visa
 */

#include "ecumain.h"

int setHV( bool buzzer )
{
#ifdef PDM
	return sendPDM( int buzzer );
#else
	bool HVR = true;
	for ( int i = 0;i<MOTORCOUNT;i++)
	{
		if ( ! CarState.Inverters[i].HighVoltageAllowed) HVR = false;
	} // HVR will be false if any of the inverters are not in true state.

	if ( ( HVR && CarState.HighVoltageReady ) || CarState.TestHV )
	{
// request HV on.
		ShutdownCircuitSet( true );
		setDevicePower( Buzzer, buzzer );
		return 1;
//		return CANSendPDM(10,buzzer); // send PDM message anyway as it's being monitored for HV state in SIM even though has no effect
	} else
	{
		ShutdownCircuitSet( false );
		return 0;
//	    return CANSendPDM(0,buzzer);
	}

	if ( ( ADCState.BrakeF > 5 ) ||      // ensure brake light power turned on if any indication brakes are being pressed.
		 ( ADCState.BrakeR > 5 ) || 		// TODO find minimum values to trigger on.
		 ( ADCState.Regen_Percent > 5) )
	{
		setDevicePower( Brake,  true);
	} else
	{
		setDevicePower( Brake,  false);
	};
#endif
}

bool CheckShutdown( void ) // returns true if shutdown circuit other than ECU is closed
{

#ifdef HPF20
	if ( !CarState.Shutdown.BOTS ) return false;
	if ( !CarState.Shutdown.BSPDAfter ) return false;
	if ( !CarState.Shutdown.BSPDBefore ) return false;
	if ( !CarState.Shutdown.CockpitButton ) return false;
	if ( !CarState.Shutdown.InertiaSwitch ) return false;
	if ( !CarState.Shutdown.LeftButton ) return false;
	if ( !CarState.Shutdown.RightButton ) return false;
	if ( !CarState.Shutdown.BMS ) return false;
	if ( !CarState.Shutdown.IMD ) return false;
#endif
	return true;

}

char * ShutDownOpenStr( void )
{
	static char str[255] = "";

	sprintf(str, "%s%s%s%s%s%s%s%s%s",
		(!CarState.Shutdown.CockpitButton)?"DRV,":"",
		(!CarState.Shutdown.LeftButton)?"LFT,":"",
		(!CarState.Shutdown.RightButton)?"RGT,":"",
		(!CarState.Shutdown.InertiaSwitch)?"INRT,":"",

		(!CarState.Shutdown.BMS)?"BMS,":"",
		(!CarState.Shutdown.IMD)?"IMD,":"",

		(!CarState.Shutdown.BOTS)?"BOTS,":"",
		(!CarState.Shutdown.BSPDAfter)?"BSPDA,":"",
		(!CarState.Shutdown.BSPDBefore)?"BSPDB,":""

	);

	int len=strlen(str);

	if ( len > 0)
		str[len-1] = 0;

	return str;
}

int errorPower( void )
{
#ifdef HPF19
	return errorPDM()
#endif

  return 0;
}

void resetPower( void )
{
	CarState.HighVoltageReady = 0;
	setHV( false ); // send high voltage off request to PDM.
}


int initPower( void )
{
	RegisterResetCommand(resetPower);

	resetPower();

	return 0;
}

