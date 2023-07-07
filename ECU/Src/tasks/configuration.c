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
#include "power.h"
#include "inverter.h"
#include "debug.h"

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

CANData ECUConfig;

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
static bool redraw = false;
bool debugconfig;

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


void doMenu8BitEdit( char * display, char * menuitem, bool selected, bool * editing,
		volatile uint8_t * value, const uint8_t * validvalues, uint16_t input, bool percentage )
{
	char strl[LCDCOLUMNS+1] = "";
	char strr[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(strl, "%c%s:", (selected) ? '>' :' ', menuitem);
	memcpy(display, strl, len);

	if ( selected  )
	{
		if ( input == KEY_ENTER )
	//	if ( CheckButtonPressed(Config_Input) )
		{
			*editing = !*editing;
			GetLeftRightPressed(); // clear out any buffered presses when weren't editing.
			redraw = true;
		}

		if ( *editing )
		{
			display[LCDCOLUMNS-1-5] = '<';
			display[LCDCOLUMNS-1] = '>';

			int change = 0;

			if ( input == KEY_LEFT )
				change-=1;

			if ( input == KEY_RIGHT )
				change+=1;

			int position = 0;

			// find current value position. will default to first item if an invalid value was given.
			for ( uint16_t i=1;validvalues[i]!=0;i++)
			{
				if ( *value == validvalues[i] ) position = i;
			}

			// work out new value change attempted.
			if ( change < 0 && position > 0)
			{
				redraw = true;
				*value = validvalues[position-1];
			} else if ( change > 0 && validvalues[position+1] != 0)
			{
				redraw = true;
				*value = validvalues[position+1];
			}
	//		if ( change + *value >= min && change + *value <= max ) *value+=change;
		}
	}

	// print value
	len = sprintf(strr, "%3d%s", *value, percentage?"%":"");
	memcpy(&display[LCDCOLUMNS-1-3-(percentage?1:0)], strr, len);
}


void doMenu16BitEdit( char * display, char * menuitem, bool selected, bool * editing,
		volatile uint16_t * value, const uint16_t * validvalues, uint16_t input )
{
	char strl[LCDCOLUMNS+1] = "";
	char strr[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(strl, "%c%s:", (selected) ? '>' :' ', menuitem);
	memcpy(display, strl, len);

	if ( selected  )
	{
		if ( input == KEY_ENTER )
	//	if ( CheckButtonPressed(Config_Input) )
		{
			*editing = !*editing;
			redraw = true;
			GetLeftRightPressed(); // clear out any buffered presses when weren't editing.
		}

		if ( *editing )
		{
			display[LCDCOLUMNS-1-6] = '<';
			display[LCDCOLUMNS-1] = '>';

			int change = 0;

			if ( input == KEY_LEFT )
				change-=1;

			if ( input == KEY_RIGHT )
				change+=1;

			int position = 0;

			// find current value position. will default to first item if an invalid value was given.
			for ( uint16_t i=1;validvalues[i]!=0;i++)
			{
				if ( *value == validvalues[i] ) position = i;
			}

			// work out new value change attempted.
			if ( change < 0 && position > 0)
			{
				redraw = true;
				*value = validvalues[position-1];
			} else if ( change > 0 && validvalues[position+1] != 0)
			{
				redraw = true;
				*value = validvalues[position+1];
			}
	//		if ( change + *value >= min && change + *value <= max ) *value+=change;
		}
	}

	// print value
	len = sprintf(strr, "%5d", *value);
	memcpy(&display[LCDCOLUMNS-1-5], strr, len);
}



void doMenuPedalEdit( char * display, char * menuitem, bool selected, bool * editing,
		volatile uint8_t * value, uint16_t input )
{
	char strl[LCDCOLUMNS+1] = "";
	char strr[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(strl, "%c%s", (selected) ? '>' :' ', menuitem);
	memcpy(display, strl, len);


	int max = 0;
	for ( max=0; GetPedalProfile(max, false)!=NULL;max++);
	if ( *value > max-1 ) *value = 0;

	if ( selected  )
	{
		if ( input == KEY_ENTER )
	//	if ( CheckButtonPressed(Config_Input) )
		{
			redraw = true;
			*editing = !*editing;
	//		GetLeftRightPressed(); // clear out any buffered presses when weren't editing.
		}

		if ( *editing )
		{
			display[LCDCOLUMNS-1-10] = '<';
			display[LCDCOLUMNS-1] = '>';

			int change = 0;

			if ( input == KEY_LEFT )
			{
				change-=1;
			}

			if ( input == KEY_RIGHT )
				change+=1;

			if ( change + *value >= 0 && change + *value <= max-1 )
			{
				*value+=change;
			}

			if ( change )
			{
				redraw = true;
			}
		}
	}

	// print value
	if ( GetPedalProfile(*value, false) )
		len = sprintf(strr, "%9s", GetPedalProfile(*value, false));
	else
		len = sprintf(strr, "Err %d",*value);
	if ( len > 9 ) len = 9;
	memcpy(&display[LCDCOLUMNS-1-9], strr, len);
}


char *RegSrc[3] = {"Off", "Orig", "Matlab"};

void doMenuRegSrcEdit( char * display, char * menuitem, bool selected, bool * editing,
		volatile uint8_t * value, uint16_t input )
{
	char strl[LCDCOLUMNS+1] = "";
	char strr[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(strl, "%c%s", (selected) ? '>' :' ', menuitem);
	memcpy(display, strl, len);

	if ( *value > 2 ) *value = 0;

	if ( selected )
	{
		if ( input == KEY_ENTER )
	//	if ( CheckButtonPressed(Config_Input) )
		{
			redraw = true;
			*editing = !*editing;
	//		GetLeftRightPressed(); // clear out any buffered presses when weren't editing.
		}

		if ( *editing )
		{
			display[LCDCOLUMNS-1-10] = '<';
			display[LCDCOLUMNS-1] = '>';

			int change = 0;

			if ( input == KEY_LEFT )
				change-=1;

			if ( input == KEY_RIGHT )
				change+=1;

			int max = 3;

			if ( change + *value >= 0 && change + *value <= max-1 )
			{
				redraw = true;
				*value+=change;
			}
		}
	}

	// print value
	len = sprintf(strr, "%9s", RegSrc[*value]);
	if ( len > 9 ) len = 9;
	memcpy(&display[LCDCOLUMNS-1-9], strr, len);
}



void doMenuBoolEdit( char * display, char * menuitem, bool selected, bool * editing,
		volatile uint8_t * value, uint8_t bit, uint16_t input )
{
	char strl[LCDCOLUMNS+1] = "";
	char strr[LCDCOLUMNS+1] = "";

	for ( int i=0;i<LCDCOLUMNS;i++) display[i] = ' ';

	int len = sprintf(strl, "%c%s", (selected) ? '>' :' ', menuitem);
	memcpy(display, strl, len);

	if ( selected  )
	{
		if ( input == KEY_ENTER )
		{
			redraw = true;
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
			{
				redraw = true;
				*value ^= (1 << bit);
			}
		}
	}

	// print value
	len = sprintf(strr, "%5s", (*value) & (1 << bit ) ? "True" : "False");
	memcpy(&display[LCDCOLUMNS-1-5], strr, len);
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
	static uint32_t count = 0;

	if ( count % 20 == 0)
		redraw = true;

	count++;

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
	int32_t REG_close = abs(REG_max-REG_min) < 50 ? 1 : 0;

	snprintf( str, 21, "L%5d R%5d B%5d", ADCState.APPSL, ADCState.APPSR, ADCState.Regen );

	if ( APPSL_close || APPSR_close )
	{
		lcd_send_stringline(1, "Press APPS & Regen", MENUPRIORITY-1);
		lcd_send_stringline(2," No brake pressure!", MENUPRIORITY-1);
		lcd_send_stringline(3, str, MENUPRIORITY-1);

		if ( debugconfig && redraw )
		{
			DebugPrintf("Press APPS & Regen");
			DebugPrintf(" No brake pressure!");
			DebugPrintf(str);
		}
	} else
	if ( REG_close )
	{
		lcd_send_stringline(1, "", MENUPRIORITY-1);
		lcd_send_stringline(2, "Press Regen", MENUPRIORITY-1);
		lcd_send_stringline(3, str, MENUPRIORITY-1);

		if ( debugconfig && redraw )
		{
			DebugPrintf("");
			DebugPrintf("Press Regen");
			DebugPrintf(str);
		}
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
		if ( debugconfig && redraw ) DebugPrintf(str);

		snprintf( str, 21, "Mn %5d %5d %5d", APPSL_min, APPSR_min, REG_min );
		lcd_send_stringline(2,str, MENUPRIORITY-1);
		if ( debugconfig && redraw ) DebugPrintf(str);

		snprintf( str, 21, "Mx %5d %5d %5d", APPSL_max, APPSR_max, REG_max );
		lcd_send_stringline(3,str, MENUPRIORITY-1);
		if ( debugconfig && redraw ) DebugPrintf(str);
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
			data->ADCTorqueReqLInput[2] = 0;
			data->ADCTorqueReqLInput[3] = 0;

			data->ADCTorqueReqRInput[0] = APPSR_min;
			data->ADCTorqueReqRInput[1] = APPSR_max;
			data->ADCTorqueReqRInput[2] = 0;
			data->ADCTorqueReqRInput[3] = 0;


			// store new APPS calibration to memory.
		}

		if ( REG_max == 0 )
		{

		} else
		{
			data->ADCBrakeTravelInput[0] = REG_min;
			data->ADCBrakeTravelInput[1] = REG_max;
			data->ADCBrakeTravelInput[2] = 0;
			data->ADCBrakeTravelInput[3] = 0;
			// store new Regen calibration to memory.
		}

		return false;
	}
	else
		return true;

}

#define MENU_NM		 	(1)
#define MENU_NMBAL		(2)
#define MENU_TORQUE		(3)
#define MENU_RPM	 	(4)
#define MENU_ACCEL 	 	(5)
#define MENU_LIMPDIS 	(6)
#define MENU_FANS	 	(7)
#define MENU_FANMAX	 	(8)
#define MENU_CALIB   	(9)
#define MENU_STEERING   (10)
#define MENU_INVEN	 	(11)
#define MENU_REGEN	 	(12)
#define MENU_REGENMAX	(13)
#define MENU_REGENMAXR  (14)
#define MENU_TELEMETRY  (15)
#define MENU_HV		 	(16)
#define MENU_LAST	 	(MENU_HV)

#define MAINMENUSIZE	(MENU_LAST+1)

// struct to track menu positioning and status.
typedef struct {
	uint8_t top;
	uint8_t menusize;
	uint8_t selection;
	bool inedit;
} menustruct_t;


void MenuInput( menustruct_t *menu, uint16_t *input )
{
	if ( !menu->inedit )
	{
		if ( *input == KEY_DOWN )
		{
			menu->selection += 1;
			*input = 0;
			redraw = true;
		}
		if ( *input == KEY_UP )
		{
			if ( menu->selection > 0)
				menu->selection -= 1;
			*input = 0;
			redraw = true;
		}

		if ( menu->selection <  0) menu->selection=0;
		if ( menu->selection > menu->menusize-1) menu->selection=menu->menusize-1;

		if ( menu->top + 2 < menu->selection ) menu->top +=1;
		if ( menu->selection < menu->top ) menu->top -=1;
	}
}

bool DoMenuMain ( uint16_t input )
{
	return false;
}

bool DoMenuInv ( uint16_t input )
{
	return false;
}


bool DoMenuTorque ( uint16_t input )
{
#define TORQUEMENU_WHEELS		(1)
#define TORQUEMENU_TCS			(2)
#define TORQUEMENU_VECTORING	(3)
#define TORQUEMENU_TRACTION   	(4)
#define TORQUEMENU_VELOCITY   	(5)
#define TORQUEMENU_FEEDBACK		(6)
#define TORQUEMENU_FEEDACT		(7)
#define TORQUEMENU_VELSOURCE	(8)
#define TORQUEMENU_LAST	 		(TORQUEMENU_FEEDACT)
#define TORQUEMENUSIZE			(TORQUEMENU_LAST+1)

	static menustruct_t menu = {
			.inedit = false,
			.top = 0,
			.selection = 0,
			.menusize = TORQUEMENUSIZE
	};

	static char MenuLines[TORQUEMENUSIZE+1][21] = { 0 };

	if ( menu.selection == 0 && input == KEY_ENTER ) // CheckButtonPressed(Config_Input) )
	{
		redraw = true;
		DebugPrintf("Leaving torque menu");
		menu.inedit = false;
		return false;
	}

	MenuInput(&menu, &input); // handle menu movement.

	strcpy(MenuLines[0], "Vectoring Menu:");
	sprintf(MenuLines[1], "%cBack...", (menu.selection==0) ? '>' :' ');
	doMenuBoolEdit( MenuLines[1+TORQUEMENU_WHEELS], "ToWheels", (menu.selection==TORQUEMENU_WHEELS), &menu.inedit, &getEEPROMBlock(0)->TorqueVectoring, TORQUE_VECTORINGENABLEDBIT, input);
	doMenuBoolEdit( MenuLines[1+TORQUEMENU_TCS], "TCS RPM", (menu.selection==TORQUEMENU_TCS), &menu.inedit, &getEEPROMBlock(0)->TorqueVectoring, TORQUE_TCSENABLEDBIT, input);
	doMenuBoolEdit( MenuLines[1+TORQUEMENU_VECTORING], "Vectoring", (menu.selection==TORQUEMENU_VECTORING), &menu.inedit, &getEEPROMBlock(0)->TorqueVectoring, TORQUE_VECTORINGBIT, input);
	doMenuBoolEdit( MenuLines[1+TORQUEMENU_TRACTION], "Traction", (menu.selection==TORQUEMENU_TRACTION), &menu.inedit, &getEEPROMBlock(0)->TorqueVectoring, TORQUE_TRACTIONBIT, input);
	doMenuBoolEdit( MenuLines[1+TORQUEMENU_VELOCITY], "Velocity", (menu.selection==TORQUEMENU_VELOCITY), &menu.inedit, &getEEPROMBlock(0)->TorqueVectoring, TORQUE_VELOCITYBIT, input);
	doMenuBoolEdit( MenuLines[1+TORQUEMENU_FEEDBACK], "Feedback", (menu.selection==TORQUEMENU_FEEDBACK), &menu.inedit, &getEEPROMBlock(0)->TorqueVectoring, TORQUE_FEEDBACKBIT, input);
	doMenuBoolEdit( MenuLines[1+TORQUEMENU_FEEDACT], "FeedbackAct", (menu.selection==TORQUEMENU_FEEDACT), &menu.inedit, &getEEPROMBlock(0)->TorqueVectoring, TORQUE_FEEDACTBIT, input);

	lcd_send_stringline( 0, MenuLines[0], MENUPRIORITY );
	if ( debugconfig && redraw ) DebugPrintf(MenuLines[0]);

	for ( int i=0;i<3;i++)
	{
		lcd_send_stringline( i+1, MenuLines[i+menu.top+1], MENUPRIORITY );
		if ( debugconfig && redraw ) DebugPrintf(MenuLines[i+menu.top+1]);
	}
	redraw = false;

	return true; // done with menu
}

bool DoMenuSetting ( uint16_t input )
{
	return false;
}



bool DoMenu( uint16_t input )
{
	static bool inmenu = false;
	static bool incalib = false;
	static bool dofullsave = false;
	static int8_t submenu = 0;

	static menustruct_t menu = {
			.inedit = false,
			.top = 0,
			.selection = 0,
			.menusize = MAINMENUSIZE
	};

	static char MenuLines[MAINMENUSIZE+1][21] = { 0 };

	const uint8_t torquevals[] = {0, 5, 10, 15, 18, 20, 25,0}; // zero terminated so function can find end.

	const uint8_t torquebalvals[] = {50, 60, 70, 80, 0}; // zero terminated so function can find end.

	const uint8_t fanvals[] = {10,20,30,40,50,60,70,80,90,100,0};

	const uint16_t rpmvals[] = {500, 3000, 5000, 10000, 15000, 19000, 20000, 0};

	const uint8_t regenvals[] = {5,10,12,15,0}; // zero terminated so function can find end.

	if ( inmenu )
	{
		if ( submenu == 0 && menu.selection == 0 && input == KEY_ENTER ) // CheckButtonPressed(Config_Input) )
		{
			inmenu = false;
			menu.inedit = false;

			lcd_send_stringline( 1, "", MENUPRIORITY );
			lcd_send_stringline( 2, "Saving settings.", MENUPRIORITY );
			lcd_send_stringline( 3, "", MENUPRIORITY );

			DebugPrintf("\nSaving settings\n");

			if ( dofullsave )
			{
				writeFullConfigEEPROM();
			} else
			{
				writeEEPROMCurConf(); // enqueue write the data to eeprom.
			}

			return false;
		}

		if ( submenu == 0 ) // only check for entering a menu if not in one.
		{
			if ( menu.selection == MENU_TORQUE && input == KEY_ENTER )
			{
				redraw = true;
				submenu = MENU_TORQUE;
				input = 0;
			}

		}

		if ( submenu != 0 ) // we're in a sub menu, process it instead of current menu.
		{
			switch ( menu.selection ) // run the sub menu.
			{
				case MENU_TORQUE :
						if ( !DoMenuTorque(input) )
						{
							redraw = true;
							submenu = 0; // check if sub menu is done.
						}
						break;
				default :
						submenu = 0;
			}
			input = 0; // in a sub menu, no input processing here.
			return true;
		}


		if ( !incalib && menu.selection == MENU_CALIB && input == KEY_ENTER ) // CheckButtonPressed(Config_Input) )
		{
			if ( DeviceState.CriticalSensors == OPERATIONAL )
			{
				redraw = true;
				incalib = true;
				input = 0;

				APPSL_min = UINT16_MAX;
				APPSL_max = 0;
				APPSR_min = UINT16_MAX;
				APPSR_max = 0;
				REG_min = UINT16_MAX;
				REG_max = 0;
			} else
			{
				DebugPrintf("Err: ADC Not ready.");
				lcd_send_stringline( 3, "Err: ADC Not ready.", 3);
				input = 0; // input has been seen, null it.
			}
		}

		if ( incalib )
		{
			if ( !doPedalCalibration(input) )
			{
				redraw = true;
				incalib = false;
				dofullsave = true;
				SetupADCInterpolationTables(getEEPROMBlock(0));

				// set the current pedal calibration after calibration exited.
			}
			else
				return true;
		}

		MenuInput(&menu, &input);

		strcpy(MenuLines[0], "Config Menu:");

		sprintf(MenuLines[1], "%cBack & Save", (menu.selection==0) ? '>' :' ');

		doMenu8BitEdit( MenuLines[1+MENU_NM], "Max Nm", (menu.selection==MENU_NM), &menu.inedit, &getEEPROMBlock(0)->MaxTorque, torquevals, input, false );
		doMenu8BitEdit( MenuLines[1+MENU_NMBAL], "Nm Bal", (menu.selection==MENU_NMBAL), &menu.inedit, &getEEPROMBlock(0)->TorqueBal, torquebalvals, input, false );


		snprintf(MenuLines[1+MENU_TORQUE], sizeof(MenuLines[0]), "%cTorqueVect...", (menu.selection==MENU_TORQUE) ? '>' :' ');

		uint16_t currpm = getEEPROMBlock(0)->maxRpm;
		doMenu16BitEdit( MenuLines[1+MENU_RPM], "RPM Max", (menu.selection==MENU_RPM), &menu.inedit, &currpm, rpmvals, input );

		if ( currpm != getEEPROMBlock(0)->maxRpm )
		{
			getEEPROMBlock(0)->maxRpm = currpm;
// add rr
		}

        doMenuPedalEdit( MenuLines[1+MENU_ACCEL], "Accel", (menu.selection==MENU_ACCEL), &menu.inedit, &getEEPROMBlock(0)->PedalProfile, input );
		doMenuBoolEdit( MenuLines[1+MENU_LIMPDIS], "LimpDisable", (menu.selection==MENU_LIMPDIS), &menu.inedit, (uint8_t*)&getEEPROMBlock(0)->LimpMode, 0, input);


		bool curfans = getEEPROMBlock(0)->Fans;
		doMenuBoolEdit( MenuLines[1+MENU_FANS], "Fans", (menu.selection==MENU_FANS), &menu.inedit, (uint8_t*)&curfans, 0, input);
		if ( curfans != getEEPROMBlock(0)->Fans )
		{
			getEEPROMBlock(0)->Fans = curfans;
			setDevicePower(LeftFans, curfans);
			setDevicePower(RightFans, curfans);
		}

		uint8_t curfanmaxcur = ceil((100.0 / 255 * getEEPROMBlock(0)->FanMax )); // convert to %

		uint8_t curfanmax = curfanmaxcur;
		doMenu8BitEdit( MenuLines[1+MENU_FANMAX], "Fan Max", (menu.selection==MENU_FANMAX), &menu.inedit, &curfanmax, fanvals, input, true );
		if ( curfanmax != curfanmaxcur ) // value changed.
		{
			getEEPROMBlock(0)->FanMax = floor(curfanmax * 2.55);
			FanPWMControl( getEEPROMBlock(0)->FanMax, getEEPROMBlock(0)->FanMax );
//			DebugPrintf("Setting fanmax to %d", getEEPROMBlock(0)->FanMax);
		}

		snprintf(MenuLines[1+MENU_CALIB], sizeof(MenuLines[0]), "%cAPPS Calib", (menu.selection==MENU_CALIB) ? '>' :' ');

		snprintf(MenuLines[1+MENU_STEERING], sizeof(MenuLines[0]), "%cSteeringCalib %+4d", (menu.selection==MENU_STEERING) ? '>' :' ', ADCState.SteeringAngle);

		if ( menu.selection == MENU_STEERING && input == KEY_ENTER )
		{
			if ( ADCState.SteeringAngleAct != 0xFFFF )
			{
				getEEPROMBlock(0)->steerCalib = 180-ADCState.SteeringAngleAct;
			// value should update on display. add a set message.
				DebugPrintf("Steering angle calibrated to offset %d", 180-ADCState.SteeringAngleAct);
			} else
			{
				DebugPrintf("Steering angle no data to calibrate");
			}
		}

		doMenuBoolEdit( MenuLines[1+MENU_INVEN], "InvEnabled", (menu.selection==MENU_INVEN), &menu.inedit, (uint8_t*)&getEEPROMBlock(0)->InvEnabled, 0, input);

		uint8_t regenon = getEEPROMBlock(0)->Regen;
		//doMenuBoolEdit( MenuLines[1+MENU_REGEN], "Regen", (menu.selection==MENU_REGEN), &menu.inedit, (uint8_t*)&regenon, 0, input);
		doMenuRegSrcEdit( MenuLines[1+MENU_REGEN], "RegSrc", (menu.selection==MENU_REGEN), &menu.inedit, &regenon, input );

		if ( regenon != getEEPROMBlock(0)->Regen )
		{
			getEEPROMBlock(0)->Regen = regenon;
#if 0
			for ( int i=0; i<MOTORCOUNT;i++)
			{
				InverterState_t * invs = getInvState(i);
				invs->AllowRegen = getEEPROMBlock(0)->Regen>0?true:false;
			}
#endif
		}

		uint8_t regenmax = getEEPROMBlock(0)->regenMax;
		doMenu8BitEdit( MenuLines[1+MENU_REGENMAX], "Regen MaxF", (menu.selection==MENU_REGENMAX), &menu.inedit, &regenmax, regenvals, input, false );
		if ( regenmax != getEEPROMBlock(0)->regenMax ) // value changed.
		{
			getEEPROMBlock(0)->regenMax = regenmax;
		}

		uint8_t regenmaxR = getEEPROMBlock(0)->regenMaxR;
		doMenu8BitEdit( MenuLines[1+MENU_REGENMAXR], "Regen MaxR", (menu.selection==MENU_REGENMAXR), &menu.inedit, &regenmaxR, regenvals, input, false );
		if ( regenmaxR != getEEPROMBlock(0)->regenMaxR ) // value changed.
		{
			getEEPROMBlock(0)->regenMaxR = regenmaxR;
		}

		bool curTM = getEEPROMBlock(0)->Telemetry;
		doMenuBoolEdit( MenuLines[1+MENU_TELEMETRY], "Telemetry", (menu.selection==MENU_TELEMETRY), &menu.inedit, (uint8_t*)&curTM, 0, input);
		if ( curTM != getEEPROMBlock(0)->Telemetry )
		{
			getEEPROMBlock(0)->Telemetry = curTM;
			setDevicePower(Telemetry, curTM);
		}

#if (MENU_LAST == MENU_HV)
		bool curhvState = getEEPROMBlock(0)->alwaysHV;
		doMenuBoolEdit( MenuLines[1+MENU_HV], "HV@Startup", (menu.selection==MENU_HV), &menu.inedit,(uint8_t*)&curhvState, 0, input);
		if ( curhvState != getEEPROMBlock(0)->alwaysHV )
		{
			getEEPROMBlock(0)->alwaysHV = curhvState;
			ShutdownCircuitSet(curhvState);
		}
#endif

		lcd_send_stringline( 0, MenuLines[0], MENUPRIORITY );

		if ( debugconfig && redraw ) DebugPrintf(MenuLines[0]);

		for ( int i=0;i<3;i++)
		{
			lcd_send_stringline( i+1, MenuLines[i+menu.top+1], MENUPRIORITY );
			if ( debugconfig && redraw ) DebugPrintf(MenuLines[i+menu.top+1]);
		}

		if ( debugconfig && redraw ) DebugPrintf("------\n");

		redraw = false; // updated, unflag till something changes.
		return true;
	}

	if ( !inmenu )
	{
		inmenu = true;
		submenu = 0;
		dofullsave = false;

		if ( debugconfig )
		{
			redraw = true; // starting menu, draw it.
			DebugPrintf("------\n");
		}

		return true;
	}

	redraw = false;
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

uint8_t configstate = 0;

bool inConfig( void )
{
	return configstate;//uxSemaphoreGetCount( xInConfig );
}

// checks if device initial values appear OK.
void ConfigTask(void *argument)
{
	xEventGroupSync( xStartupSync, 0, 1, portMAX_DELAY ); // ensure that tasks don't start before all initialisation done.

	ConfigInput_msg confinp;

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

