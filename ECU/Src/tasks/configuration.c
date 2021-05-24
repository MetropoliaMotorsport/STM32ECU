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

// ADC conversion buffer, should be aligned in memory for faster DMA?
typedef struct {
	uint32_t msgval;
} ConfigInput_msg;
// this is input of human time input, very unlikely to be able to manage
// more than 2 inputs before processed.

#define ConfigInputQUEUE_LENGTH    2
#define ConfigInputITEMSIZE		sizeof( ConfigInput_msg )

static StaticQueue_t ConfigInputStaticQueue;
uint8_t ConfigInputQueueStorageArea[ ConfigInputQUEUE_LENGTH * ConfigInputITEMSIZE ];

QueueHandle_t ConfigInputQueue;


#define ConfigSTACK_SIZE 128*8
#define ConfigTASKNAME  "ConfigTask"
#define ConfigTASKPRIORITY 1
StaticTask_t xConfigTaskBuffer;
RAM_D1 StackType_t xConfigStack[ ConfigSTACK_SIZE ];

TaskHandle_t ConfigTaskHandle = NULL;


static uint8_t ECUConfigdata[8] = {0};
static bool	   ECUConfignewdata = false;
static uint32_t ECUConfigDataTime = 0;

bool GetConfigCmd( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

CANData ECUConfig = { NULL, 0x21, 8, GetConfigCmd, NULL, 0 };

static bool configReset = false;


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
bool ConfigTask( void )
{

	while ( 1 )
	{
		vTaskDelay(10);
	}

	if ( !EEPROMBusy() )
	{
		DoMenu();
	}

	char str[40];

	snprintf(str, 40, "Conf: %dnm %s %c %s", CarState.Torque_Req_Max, GetPedalProfile(CarState.PedalProfile, true), (!CarState.LimpDisable)?'T':'F', (CarState.FanPowered)?"Fan":""  );
	lcd_send_stringline(3,str, 255);


	// check if new can data received.
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

	return true; // return if config changed.
}


bool doPedalCalibration( void )
{



}


bool initConfig( void )
{
	RegisterCan1Message(&ECUConfig);

	ConfigInputQueue = xQueueCreateStatic( ConfigInputQUEUE_LENGTH,
											ConfigInputITEMSIZE,
											ConfigInputQueueStorageArea,
											&ConfigInputStaticQueue );

	vQueueAddToRegistry(ConfigInputQueue, "Config Input" );

	if ( !SetupADCInterpolationTables(getEEPROMBlock(0)) )
	{
			// bad config, fall back to some kind of of default, force recalibration of pedal?
		doPedalCalibration();
	}

	CarState.Torque_Req_Max = 0;
	CarState.Torque_Req_CurrentMax = 0;

	ConfigTaskHandle = xTaskCreateStatic(
						  ConfigTask,
						  ConfigTASKNAME,
						  ConfigSTACK_SIZE,
						  ( void * ) 1,
						  ConfigTASKPRIORITY,
						  xConfigStack,
						  &xConfigTaskBuffer );

	return true;
}

