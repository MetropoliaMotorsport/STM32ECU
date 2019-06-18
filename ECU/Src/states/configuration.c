/*
 * configuration.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#include "main.h"
#include "ecumain.h"

// checks if device initial values appear OK.
int CheckConfigurationRequest( void )
{
//	static int configstart = 0;
	int returnvalue = 0;

	static int initialconfig = 0;

	if ( !initialconfig )
	{
		SetupADCInterpolationTables(); // setup default ADC lookup tables.
		initialconfig = 1; // call interpolation table setup once only.
		CarState.Torque_Req_Max = 5;
		CarState.Torque_Req_CurrentMax = 5;
	}

	// check for config change messages.
	if ( CanState.ECUConfig.newdata )
	{
		CanState.ECUConfig.newdata = 0;
		if ( CanState.ECUConfig.data[0] != 0)
		{
	//		returnvalue = ReceivingConfig;
			switch ( CanState.ECUConfig.data[0] )
			{
				case 1 : // send ADC
					CAN_SendADC(ADC_Data,0);
					break;
				case 2 :
					CAN_SendADCminmax();
					break;
				break;
				default :
					break;
			}
		}

		// config data packet received, process.
	}

	// if can config request testing mode, send acknowledgement, then return 10;

	return returnvalue; // return a requested driving state, or that in middle of config?
}

