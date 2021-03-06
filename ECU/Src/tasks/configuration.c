/*
 * configuration.c
 *
 *  Created on: 13 Apr 2019
 *      Author: Visa
 */

#include "ecumain.h"
#include "configuration.h"
#include "input.h"
#include "timerecu.h"
#include "ecumain.h"
#include "lcd.h"
#include "eeprom.h"
#include "adcecu.h"
#include "semphr.h"
#include "taskpriorities.h"

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
		volatile uint8_t * value, const uint8_t * validvalues, uint16_t input )
{
	char str[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(str, "%c%s:", (selected) ? '>' :' ', menuitem);
	memcpy(display, str, len);

	if ( selected  )
	{
		if ( input == KEY_ENTER )
	//	if ( CheckButtonPressed(Config_Input) )
		{
			*editing = !*editing;
			GetLeftRightPressed(); // clear out any buffered presses when weren't editing.
		}


		if ( *editing )
		{
			display[LCDCOLUMNS-1-5] = '<';
			display[LCDCOLUMNS-1] = '>';

			int change = 0;

			if ( input == KEY_LEFT )
				change+=1;

			if ( input == KEY_RIGHT )
				change-=1;

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
		volatile uint8_t * value, uint16_t input )
{
	char str[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(str, "%c%s", (selected) ? '>' :' ', menuitem);
	memcpy(display, str, len);

	if ( selected  )
	{
		if ( input == KEY_ENTER )
	//	if ( CheckButtonPressed(Config_Input) )
		{
			*editing = !*editing;
	//		GetLeftRightPressed(); // clear out any buffered presses when weren't editing.
		}


		if ( *editing )
		{
			display[LCDCOLUMNS-1-10] = '<';
			display[LCDCOLUMNS-1] = '>';

			int change = 0;

			if ( input == KEY_LEFT )
				change+=1;

			if ( input == KEY_RIGHT )
				change-=1;

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
		volatile bool * value, uint16_t input )
{
	char str[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(str, "%c%s", (selected) ? '>' :' ', menuitem);
	memcpy(display, str, len);

	if ( selected  )
	{
		if ( input == KEY_ENTER )
		{
			*editing = !*editing;
		}

		if ( *editing )
		{
			display[LCDCOLUMNS-1-6] = '<';
			display[LCDCOLUMNS-1] = '>';

			int change = 0;

			if ( input == KEY_LEFT || input == KEY_RIGHT )
			{
				change = 1;
				input = 0;
			}

			if ( change != 0 )
				*value= !*value;
		}
	}

	// print value
	len = sprintf(str, "%5s", (*value)? "True" : "False");
	memcpy(&display[LCDCOLUMNS-1-5], str, len);
}

uint16_t APPSL_min;
uint16_t APPSL_max;
uint16_t APPSR_min;
uint16_t APPSR_max;
uint16_t REG_min;
uint16_t REG_max;

// values to define sane input range on APPS ADC's

#define ADCMAXTHRESH (0.95)
#define ADCMINTHRESH (0.5)

void setMin( uint16_t * min, uint16_t minval)
{
	if ( minval < *min ) *min = minval;
}

void setMax( uint16_t * max, uint16_t maxval)
{
	if ( maxval > *max ) *max = maxval;
}



bool doPedalCalibration( uint16_t input )
{
	char str[21];

	bool baddata = false;

	if ( ADCState.APPSL > (UINT16_MAX*ADCMAXTHRESH)
		|| ADCState.APPSL < 0 // (UINT16_MAX*ADCMINTHRESH)
		)
	{
		lcd_send_stringline(1, "APPSL not sane", MENUPRIORITY);
		baddata = true;
	}

	if ( ADCState.APPSR > (UINT16_MAX*ADCMAXTHRESH)
		|| ADCState.APPSR < 0 //  (UINT16_MAX*ADCMINTHRESH)
		)
	{
		lcd_send_stringline(2, "APPSR not sane", MENUPRIORITY);
		baddata = true;
	}

	if ( ADCState.Regen > (UINT16_MAX*ADCMAXTHRESH)
		|| ADCState.Regen < 0 //(UINT16_MAX*ADCMINTHRESH)
		)
	{
		lcd_send_stringline(3, "Regen not sane", MENUPRIORITY);
		baddata = true;
	}

	if ( baddata )
	{
		lcd_send_stringline(0, "APPS Calib <close>", MENUPRIORITY);

		if ( input == KEY_ENTER)
			return false;
		else
			return true;
	} else
		lcd_send_stringline( 0, "APPS Calib <save>", MENUPRIORITY );


	setMin(&APPSL_min, ADCState.APPSL);
	setMin(&APPSR_min, ADCState.APPSR);
	setMin(&REG_min, ADCState.Regen);

	setMax(&APPSL_max, ADCState.APPSL);
	setMax(&APPSR_max, ADCState.APPSR);
	setMax(&REG_max, ADCState.Regen);

	int32_t APPSL_close = abs(APPSL_max-APPSL_min) < 500 ? 1 : 0;
	int32_t APPSR_close = abs(APPSR_max-APPSR_min) < 500 ? 1 : 0;
	int32_t REG_close = abs(REG_max-REG_min) < 500 ? 1 : 0;

	if ( APPSL_close || APPSR_close )
	{
		lcd_send_stringline(1, "", MENUPRIORITY-1);
		lcd_send_stringline(2, "Press APPS & Brake", MENUPRIORITY-1);

		lcd_send_stringline(3, "", MENUPRIORITY-1);
	} else
	if ( REG_close )
	{
		lcd_send_stringline(1, "", MENUPRIORITY-1);
		lcd_send_stringline(2, "Press Brake", MENUPRIORITY-1);
		lcd_send_stringline(3, "", MENUPRIORITY-1);
	} else
	{
		int APPSL = 100.0/(APPSL_max-APPSL_min) * (ADCState.APPSL-APPSL_min);
		if ( APPSL > 99 ) APPSL = 99;

		int APPSR = 100.0/(APPSR_max-APPSR_min) * (ADCState.APPSR-APPSR_min);
		if ( APPSR > 99 ) APPSR = 99;

		int REGEN = 100.0/(REG_max-REG_min) * (ADCState.Regen-REG_min);
		if ( APPSR > 99 ) APPSR = 99;

		snprintf( str, 21, "Cur L%2d%%  R%2d%%  B%2d%%", APPSL, APPSR, REGEN );

		lcd_send_stringline(1,str, MENUPRIORITY-1);

		snprintf( str, 21, "Mn %5d %5d %5d", APPSL_min, APPSR_min, REG_min );

		lcd_send_stringline(2,str, MENUPRIORITY-1);

		snprintf( str, 21, "Mx %5d %5d %5d", APPSL_max, APPSR_max, REG_max );

		lcd_send_stringline(3,str, MENUPRIORITY-1);
	}

	if ( input == KEY_ENTER)
	{

		eepromdata * data = getEEPROMBlock(0);

		if ( APPSL_max == 0 || APPSR_max == 0 )
		{

		} else
		{
			data->ADCTorqueReqLInput[0] = APPSL_min;
			data->ADCTorqueReqLInput[1] = APPSL_max;

			data->ADCTorqueReqRInput[0] = APPSR_min;
			data->ADCTorqueReqRInput[1] = APPSR_max;

			// store new APPS calibration to memory.
		}

		if ( REG_max == 0 )
		{

		} else
		{
			data->ADCBrakeTravelInput[0] = REG_min;
			data->ADCBrakeTravelInput[1] = REG_max;
			// store new Regen calibration to memory.
		}

		return false;
	}
	else
		return true;

}

bool DoMenu( uint16_t input )
{
	static bool inmenu = false;
	static int8_t selection = 0;
	static int8_t top = 0;
	static bool inedit = false;
	static bool incalib = false;
	static bool dofullsave = false;

#define menusize	(8)

	static char MenuLines[menusize+1][21] = { 0 };

	const uint8_t torquevals[] = {0,5,10,25,65,0}; // zero terminated so function can find end.

	if ( inmenu )
	{
		if ( selection == 0 && input == KEY_ENTER ) // CheckButtonPressed(Config_Input) )
		{
			inmenu = false;
			inedit = false;

			lcd_send_stringline( 1, "", MENUPRIORITY );
			lcd_send_stringline( 2, "Saving settings.", MENUPRIORITY );
			lcd_send_stringline( 3, "", MENUPRIORITY );

			if ( dofullsave )
			{
				writeFullConfigEEPROM();
			} else
			{
				writeEEPROMCurConf(); // enqueue write the data to eeprom.
			}

			return false;
		}

		if ( !incalib && selection == 5 && input == KEY_ENTER ) // CheckButtonPressed(Config_Input) )
		{
			incalib = true;
			input = 0;

			APPSL_min = UINT16_MAX;
			APPSL_max = 0;
			APPSR_min = UINT16_MAX;
			APPSR_max = 0;
			REG_min = UINT16_MAX;
			REG_max = 0;
		}

		if ( incalib )
		{
			if ( !doPedalCalibration(input) )
			{
				incalib = false;
				dofullsave = true;
				SetupADCInterpolationTables(getEEPROMBlock(0));

				// set the current pedal calibration after calibration exited.
			}
			else
				return true;
		}

		if ( !inedit )
		{
			if ( input == KEY_DOWN )
			{
				selection += 1;
				input = 0;
			}
			if ( input == KEY_UP )
			{
				selection -= 1;
				input = 0;
			}

			if ( selection <  0) selection=0;
			if ( selection > menusize-1) selection=menusize-1;

			if ( top + 2 < selection ) top +=1;
			if ( selection < top ) top -=1;
		}

		strcpy(MenuLines[0], "Config Menu:");

		sprintf(MenuLines[1], "%cBack & Save", (selection==0) ? '>' :' ');
		doMenuIntEdit( MenuLines[2], "Max Nm", (selection==1), &inedit, &getEEPROMBlock(0)->MaxTorque, torquevals, input );
		doMenuPedalEdit( MenuLines[3], "Accel", (selection==2), &inedit, &getEEPROMBlock(0)->PedalProfile, input );
		doMenuBoolEdit( MenuLines[4], "LimpDisable", (selection==3), &inedit, &getEEPROMBlock(0)->LimpMode, input);
		doMenuBoolEdit( MenuLines[5], "Fans", (selection==4), &inedit, &getEEPROMBlock(0)->Fans, input);
		doMenuIntEdit( MenuLines[6], "Fan Max", (selection==5), &inedit, &getEEPROMBlock(0)->MaxTorque, torquevals, input );

		sprintf(MenuLines[7], "%cAPPS Calib", (selection==6) ? '>' :' ');
		doMenuBoolEdit( MenuLines[8], "InvEnabled", (selection==7), &inedit, &getEEPROMBlock(0)->InvEnabled, input);

//		doMenuBoolEdit( MenuLines[9], "TestHV", (selection==8), &inedit, &CarState.TestHV, input);

		lcd_send_stringline( 0, MenuLines[0], MENUPRIORITY );

		for ( int i=0;i<3;i++)
		{
			 lcd_send_stringline( i+1, MenuLines[i+top+1], MENUPRIORITY );
		}
		return true;
	}

	if ( !inmenu )
	{

		inmenu = true;
		dofullsave = false;

		return true;
	}

	return false;

}

// Add message to uart message queue. Might be called from ISR so add a check.
bool ConfigInput( uint16_t input )
{
	ConfigInput_msg confmsg;

	confmsg.msgval = input;

	if ( xPortIsInsideInterrupt() )
		return xQueueSendFromISR( ConfigInputQueue, &confmsg, 0 );
	else
		return xQueueSendToBack( ConfigInputQueue, &confmsg, 0); // send it to error state handler queue for display to user.
}


char ConfStr[40] = "";

char * getConfStr( void )
{
	// TODO add a mutex
	if ( ConfStr[0] == 0 ) return NULL;
	else return ConfStr;
}

SemaphoreHandle_t xInConfig = NULL;
StaticSemaphore_t xInConfigBuffer;

bool inConfig( void )
{
	return uxSemaphoreGetCount( xInConfig );
}

// checks if device initial values appear OK.
void ConfigTask(void *argument)
{
	xEventGroupSync( xStartupSync, 0, 1, portMAX_DELAY ); // ensure that tasks don't start before all initialisation done.

	ConfigInput_msg confinp;

	uint8_t configstate = 0;

	while ( 1 )
	{
		 // config menu does not need to run very real time.
		if ( xQueueReceive(ConfigInputQueue,&confinp,20) )
		{
			if ( confinp.msgval == 0xFFFF )
			{
				xSemaphoreTake(xInConfig, 0);
				configstate = 1;
			}
		} else
		{
			confinp.msgval = 0;
		}

		switch ( configstate )
		{
		case 0 : break;

		case 1:
			if ( !EEPROMBusy() )
			{
				if ( !DoMenu(confinp.msgval) )
				{
					configstate = 0;
					xSemaphoreGive(xInConfig);
				}
			}

			// check if new can data received.
			if ( ECUConfignewdata )
			{
				ECUConfignewdata = false;

				if ( ECUConfigdata[0] != 0)
				{
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
							// toggle HV.
							break;

						default : // unknown request.
							break;
					}
				} else
				{
		// deal with local data.
				}
			}

			break;
		}

		snprintf(ConfStr, 40, "Conf: %dnm %s %c %s", CarState.Torque_Req_Max,
				GetPedalProfile(CarState.PedalProfile, true),
				(!CarState.LimpDisable)?'T':'F',
				(CarState.FanPowered)?"Fan":""  );

	}

	// clean up if we somehow get here.
	vTaskDelete(NULL);
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
//		doPedalCalibration();
	}

    xInConfig = xSemaphoreCreateBinaryStatic( &xInConfigBuffer );

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

