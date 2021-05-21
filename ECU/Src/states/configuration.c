/*
 * configuration.c
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#include "ecumain.h"
#include "configuration.h"
#include "input.h"
#include "timerecu.h"
#include "ecumain.h"
#include "lcd.h"
#include "eeprom.h"
#include "adcecu.h"

static uint8_t ECUConfigdata[8] = {0};
static bool	   ECUConfignewdata = false;
static uint32_t ECUConfigDataTime = 0;

bool GetConfigCmd( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

CANData ECUConfig = { NULL, 0x21, 8, GetConfigCmd, NULL, 0 };

static bool configReset = false;


int initConfig( void )
{
	RegisterCan1Message(&ECUConfig);
	return 0;
}

bool checkConfigReset( void )
{
	if ( configReset )
	{
/*
		CanState.ECU.newdata = 0; // we've seen message in error state loop.
		if ( ( CanState.ECU.data[0] == 0x99 ) && ( CanState.ECU.data[1] == 0x99 ))
		{
			return true;
		}
*/
		configReset = false;
		return true;
	} else return false;
}

void ConfigReset( void )
{
	configReset = false;
}


bool GetConfigCmd( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	if ( ( CANRxData[0] >= 8 && CANRxData[0] <= 11 ) || ( CANRxData[0] == 30 ) ) // eeprom command.
	{
		return GetEEPROMCmd( CANRxData, DataLength, datahandle );
	} else if ( ECUConfignewdata )
	{
// TODO received data before processing old, send error?
	} else
	{
		ECUConfigDataTime= gettimer();
		memcpy(ECUConfigdata, CANRxData, 8);
		ECUConfignewdata = true; // moved to end to ensure data is not read before updated.
	}
	return true;
}

void setCurConfig(void)
{
#ifndef HPF19

//	EEPROMdata

	if ( DeviceState.EEPROM == ENABLED )
	{
		CarState.PedalProfile = getEEPROMBlock(0)->PedalProfile;
		SetupTorque(CarState.PedalProfile);
		CarState.LimpDisable = !getEEPROMBlock(0)->LimpMode;
		CarState.Torque_Req_Max = getEEPROMBlock(0)->MaxTorque;
		CarState.FanPowered = getEEPROMBlock(0)->Fans;

	} else
	{
		CarState.PedalProfile = 0;
		SetupTorque(CarState.PedalProfile);
		CarState.LimpDisable = 0;
		CarState.Torque_Req_Max = 5;
		CarState.FanPowered = true;
	}


#else
	SetupNormalTorque();
	CarState.LimpDisable = 0;
	CarState.DrivingMode = ADCState.DrivingMode;

	switch ( ADCState.DrivingMode )
	{
		case 1: // 5nm  5 , 5,    0,     5,   5,    10,    15,    20,   25,     30,    64,    65,   0
			CarState.Torque_Req_Max = 5;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 0;
#endif
			break;
		case 2: // 10nm
			CarState.Torque_Req_Max = 25;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 0;
#endif
			break;
		case 3: // 15nm
			CarState.Torque_Req_Max = 25;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 1;
#endif
			break;
		case 4: // 20nm
			CarState.Torque_Req_Max = 35;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 0;
#endif
			break;
		case 5: // 25nm
			CarState.Torque_Req_Max = 35;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 1;
#endif
			break;
		case 6: // 30nm
			CarState.Torque_Req_Max = 65;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 0;
#endif
			#ifdef EEPROM
				SetupTorque(0);
			#else
				SetupLargeLowRangeTorque();
			#endif
			break;
		case 7: // 65nm Track
			CarState.Torque_Req_Max = 65;
#ifdef TORQUEVECTOR
			CarState.TorqueVectoring = 1;
#endif

		#ifdef EEPROM
			SetupTorque(0);
		#else
			SetupLargeLowRangeTorque();
		#endif
			break;
		case 8: // 65nm Accel
			CarState.Torque_Req_Max = 65;
			CarState.LimpDisable = 1;
			#ifdef EEPROM
				SetupTorque(0);
			#else
				SetupLowTravelTorque();
			#endif

			break;

	}

#endif

	CarState.Torque_Req_CurrentMax = CarState.Torque_Req_Max;
}

char * GetPedalProfile( uint8_t profile, bool shortform )
{
	switch ( profile )
	{
		case 0 : // Full EEPROM
			if ( shortform )
				return "Lin";
			else return "Linear";
		case 1 : // Full EEPROM
			if ( shortform )
				return "Low";
			else return "Low Range";
		case 2 : // Full EEPROM
			if ( shortform )
				return "Acc";
			else return "Acceleration";
	}
	return NULL;
}


void doMenuIntEdit( char * display, char * menuitem, bool selected,	bool * editing,
		volatile uint8_t * value, const uint8_t * validvalues )
{
	char str[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(str, "%c%s:", (selected) ? '>' :' ', menuitem);
	memcpy(display, str, len);

	if ( selected  )
	{
		if ( CheckButtonPressed(Config_Input) )
		{
			*editing = !*editing;
			GetLeftRightPressed(); // clear out any buffered presses when weren't editing.
		}


		if ( *editing )
		{
			display[LCDCOLUMNS-1-5] = '<';
			display[LCDCOLUMNS-1] = '>';
			int change = GetLeftRightPressed();
			int position = 0;

			// find current value position. will default to first item if an invalid value was given.
			for ( int i=1;validvalues[i]!=0;i++)
			{
				if ( *value ==  validvalues[i] ) position = i;
			}

			// work out new value change attempted.
			if ( change < 0 && position > 0)
			{
				*value = validvalues[position-1];
			} else if ( change > 0 && validvalues[position+1] != 0)
			{
				*value = validvalues[position+1];
			}
	//		if ( change + *value >= min && change + *value <= max ) *value+=change;
		}
	}

	// print value
	len = sprintf(str, "%3d", *value);
	memcpy(&display[LCDCOLUMNS-1-3], str, len);
}



void doMenuPedalEdit( char * display, char * menuitem, bool selected, bool * editing,
		volatile uint8_t * value )
{
	char str[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(str, "%c%s", (selected) ? '>' :' ', menuitem);
	memcpy(display, str, len);

	if ( selected  )
	{
		if ( CheckButtonPressed(Config_Input) )
		{
			*editing = !*editing;
			GetLeftRightPressed(); // clear out any buffered presses when weren't editing.
		}


		if ( *editing )
		{
			display[LCDCOLUMNS-1-10] = '<';
			display[LCDCOLUMNS-1] = '>';
			int change = GetLeftRightPressed();
			int max = 0;

			for ( max=0; GetPedalProfile(max, false)!=NULL;max++);

			if ( change + *value >= 0 && change + *value <= max-1 ) *value+=change;
		}
	}

	// print value
	len = sprintf(str, "%9s", GetPedalProfile(*value, false));
	if ( len > 9 ) len = 9;
	memcpy(&display[LCDCOLUMNS-1-9], str, len);
}



void doMenuBoolEdit( char * display, char * menuitem, bool selected, bool * editing,
		volatile bool * value )
{
	char str[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(str, "%c%s", (selected) ? '>' :' ', menuitem);
	memcpy(display, str, len);

	if ( selected  )
	{
		if ( CheckButtonPressed(Config_Input) )
		{
			*editing = !*editing;
			GetLeftRightPressed(); // clear out any buffered presses when weren't editing.
		}


		if ( *editing )
		{
			display[LCDCOLUMNS-1-6] = '<';
			display[LCDCOLUMNS-1] = '>';
			int change = GetLeftRightPressed();
			if ( change != 0 )
				*value= !*value;
		}
	}

	// print value
	len = sprintf(str, "%5s", (*value)? "True" : "False");
	memcpy(&display[LCDCOLUMNS-1-5], str, len);
}


bool DoMenu()
{
	static bool inmenu = false;
	static int8_t selection = 0;
	static int8_t top = 0;
	static bool inedit = false;

#define menusize	(6)

	static char MenuLines[menusize+1][21] = { 0 };

	const uint8_t torquevals[] = {0,5,10,25,65,0}; // zero terminate so function can find end.

	if ( inmenu )
	{
		if ( selection == 0 && CheckButtonPressed(Config_Input) )
		{
			inmenu = false;
			inedit = false;

			writeEEPROMCurConf(); // enqueue write the data to eeprom.

			return false;
		}

		if ( !inedit )
		{
			selection+=GetUpDownPressed(); // allow position adjustment if not editing item.

			if ( selection <  0) selection=0;
			if ( selection > menusize-1) selection=menusize-1;

			if ( top + 2 < selection ) top +=1;
			if ( selection < top ) top -=1;
		}

		strcpy(MenuLines[0], "Config Menu:");

		sprintf(MenuLines[1], "%cBack & Save", (selection==0) ? '>' :' ');
		doMenuIntEdit( MenuLines[2], "Max Nm", (selection==1), &inedit, &getEEPROMBlock(0)->MaxTorque, torquevals );
		doMenuPedalEdit( MenuLines[3], "Accel", (selection==2), &inedit, &getEEPROMBlock(0)->PedalProfile );
		doMenuBoolEdit( MenuLines[4], "LimpDisable", (selection==3), &inedit, &getEEPROMBlock(0)->LimpMode);
		doMenuBoolEdit( MenuLines[5], "Fans", (selection==4), &inedit, &getEEPROMBlock(0)->Fans);

//		doMenuBoolEdit( MenuLines[6], "TestHV", (selection==5), &inedit, &CarState.TestHV);

//		sprintf(MenuLines[3], "%cAccel: %s", (selection==2) ? '>' :' ', GetPedalProfile(CarState.PedalProfile, false));

		lcd_send_stringline( 0, MenuLines[0], 4 );

		for ( int i=0;i<3;i++)
		{
			 lcd_send_stringline( i+1, MenuLines[i+top+1], 4 );
		}
		return true;
	}

	if ( !inmenu )
	{
		if ( CheckButtonPressed(Config_Input) )
		{
			inmenu = true;
			GetUpDownPressed(); // clear any queued actions.
			return true;
		}
	}

	return false;

}


// checks if device initial values appear OK.
int CheckConfigurationRequest( void )
{

	char str[40] = "";

//	static int configstart = 0;
	int returnvalue = 0;

	static bool initialconfig = true;

	static uint8_t testingactive = 0;

	if ( initialconfig )
	{
		if ( !SetupADCInterpolationTables(getEEPROMBlock(0)) )
		{
				// bad config.
		}
		// TODO move to better place. setup default ADC lookup tables.
		initialconfig = false; // call interpolation table setup once only.
		CarState.Torque_Req_Max = 0;
		CarState.Torque_Req_CurrentMax = 0;
	}


	if ( !EEPROMBusy() )
	{
		DoMenu();
	}

	sprintf(str,"Conf: %dnm %s %c %s", CarState.Torque_Req_Max, GetPedalProfile(CarState.PedalProfile, true), (!CarState.LimpDisable)?'T':'F', (CarState.FanPowered)?"Fan":""  );
	lcd_send_stringline(3,str, 255);

	// check for config change messages. - broken?

	// data receive block [ block sequence[2], data size[1] ]

	if ( ECUConfignewdata )
	{
		ECUConfignewdata = false;

		if ( ECUConfigdata[0] != 0)
		{

	//		returnvalue = ReceivingConfig;
			switch ( ECUConfigdata[0] )
			{
#ifdef STMADC
				case 1 : // send ADC
					CAN_SendADC(ADC_Data,0);
					break;
#endif
				case 2 :
					CAN_SendADCminmax();
					break;
				case 3 :
					// toggle HV force loop.
/*					if ( CarState.TestHV )
					{
						CarState.TestHV = 0;
						testingactive = 0;
						CAN_ConfigRequest(3, 0 );
					}
					else
					{
						CarState.TestHV = 1;
						testingactive = 1;
						CAN_ConfigRequest( 3, 1 );
					} */
					break;

				default : // unknown request.
					break;
			}
		} else
		{
// deal with local data.
		}
		// not transferring data.

		// config data packet received, process.
	}

	// if can config request testing mode, send acknowledgement, then return 10;

	if ( testingactive ) returnvalue = TestingState;

	return returnvalue; // return a requested driving state, or that in middle of config?
}

