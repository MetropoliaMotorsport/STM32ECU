/*
 * power.c
 *
 *  Created on: Jul 17, 2020
 *      Author: visa
 */

#include "ecumain.h"

int sendHV( bool buzzer )
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
#endif
}
