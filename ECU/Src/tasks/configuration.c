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
#include "eeprom.h"
#include "node_device.h"
#include "semphr.h"
#include "taskpriorities.h"
#include "power.h"
#include "inverter.h"
#include "debug.h"
#include "canecu.h"
#include "torquecontrol.h"

// ADC conversion buffer, should be aligned in memory for faster DMA?
typedef struct {
	uint32_t msgval;
} ConfigInput_msg;
// this is input of human time input, very unlikely to be able to manage
// more than 2 inputs before processed.

#define ConfigInputQUEUE_LENGTH    2
#define ConfigInputITEMSIZE		sizeof( ConfigInput_msg )

static StaticQueue_t ConfigInputStaticQueue;
uint8_t ConfigInputQueueStorageArea[ConfigInputQUEUE_LENGTH
		* ConfigInputITEMSIZE];

QueueHandle_t ConfigInputQueue;

CANData ECUConfig;

#define ConfigSTACK_SIZE 128*8
#define ConfigTASKNAME  "ConfigTask"
StaticTask_t xConfigTaskBuffer;
RAM_D1 StackType_t xConfigStack[ConfigSTACK_SIZE];

TaskHandle_t ConfigTaskHandle = NULL;

static uint8_t ECUConfigdata[8] = { 0 };
static bool ECUConfignewdata = false;
static uint32_t ECUConfigDataTime = 0;

bool GetConfigCmd(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle);

CANData ECUConfig = { NULL, 0x21, 8, GetConfigCmd, NULL, 0 };

static bool configReset = false;
static bool redraw = false;
bool debugconfig;

bool checkConfigReset(void) {
	if (configReset) {
		configReset = false;
		return true;
	} else
		return false;
}

void ConfigReset(void) {
	configReset = false;
}

bool GetConfigCmd(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle) {
	if ((CANRxData[0] >= 8 && CANRxData[0] <= 11) || (CANRxData[0] == 30)) // eeprom command.
			{
		return GetEEPROMCmd(CANRxData, DataLength, datahandle);
	} else if (ECUConfignewdata) {
// TODO received data before processing old, send error?
	} else {
		ECUConfigDataTime = gettimer();
		memcpy(ECUConfigdata, CANRxData, 8);
		ECUConfignewdata = true; // moved to end to ensure data is not read before updated.
	}
	return true;
}

void setCurConfig(void) {
//	EEPROMdata

	if (DeviceState.EEPROM == ENABLED) {
		CarState.PedalProfile = getEEPROMBlock(0)->PedalProfile;
		SetupTorque(CarState.PedalProfile);
		CarState.LimpDisable = !getEEPROMBlock(0)->LimpMode;
		CarState.Torque_Req_Max = getEEPROMBlock(0)->MaxTorque;
		CarState.FanPowered = getEEPROMBlock(0)->Fans;

	} else {
		CarState.PedalProfile = 0;
		SetupTorque(CarState.PedalProfile);
		CarState.LimpDisable = 0;
		CarState.Torque_Req_Max = 5;
		CarState.FanPowered = true;
	}

	CarState.Torque_Req_CurrentMax = CarState.Torque_Req_Max;
}

char* GetPedalProfile(uint8_t profile, bool shortform) {
	switch (profile) {
	case 0: // Full EEPROM
		if (shortform)
			return "Lin";
		else
			return "Linear";
	case 1: // Full EEPROM
		if (shortform)
			return "Low";
		else
			return "Low Range";
	case 2: // Full EEPROM
		if (shortform)
			return "Acc";
		else
			return "Acceleration";
	}
	return NULL;
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

void setMin(uint16_t *min, uint16_t minval) {
	if (minval < *min)
		*min = minval;
}

void setMax(uint16_t *max, uint16_t maxval) {
	if (maxval > *max)
		*max = maxval;
}

bool doPedalCalibration(uint16_t input) {
	static uint32_t count = 0;

	if (count % 20 == 0)
		redraw = true;

	count++;

	char str[21];

	bool baddata = false;

	//TODO implement
	if (APPS1.data > (UINT16_MAX * ADCMAXTHRESH) || APPS1.data < 0 // (UINT16_MAX*ADCMINTHRESH)
			) {
		baddata = true;
	}

	if (APPS2.data > (UINT16_MAX * ADCMAXTHRESH) || APPS2.data < 0 //  (UINT16_MAX*ADCMINTHRESH)
			) {
		baddata = true;
	}

	if (BPPS.data > (UINT16_MAX * ADCMAXTHRESH) || BPPS.data < 0 //(UINT16_MAX*ADCMINTHRESH)
			) {
		baddata = true;
	}

	if (baddata) {
		return input != KEY_ENTER;
	}

	setMin(&APPSL_min, APPS1.data);
	setMin(&APPSR_min, APPS2.data);
	setMin(&REG_min, BPPS.data);

	setMax(&APPSL_max, APPS1.data);
	setMax(&APPSR_max, APPS2.data);
	setMax(&REG_max, BPPS.data);

	int32_t APPSL_close = abs(APPSL_max - APPSL_min) < 500 ? 1 : 0;
	int32_t APPSR_close = abs(APPSR_max - APPSR_min) < 500 ? 1 : 0;
	int32_t REG_close = abs(REG_max - REG_min) < 50 ? 1 : 0;

	snprintf(str, 21, "L%5d R%5d B%5d", APPS1.data, APPS2.data,
			BPPS.data);

	if (APPSL_close || APPSR_close) {
		if (debugconfig && redraw) {
			DebugPrintf("Press APPS & Regen");
			DebugPrintf(" No brake pressure!");
			DebugPrintf(str);
		}
	} else if (REG_close) {
		if (debugconfig && redraw) {
			DebugPrintf("");
			DebugPrintf("Press Regen");
			DebugPrintf(str);
		}
	} else {
		int APPSL = 100.0 / (APPSL_max - APPSL_min)
				* (APPS1.data - APPSL_min);
		if (APPSL > 99)
			APPSL = 99;

		int APPSR = 100.0 / (APPSR_max - APPSR_min)
				* (APPS2.data - APPSR_min);
		if (APPSR > 99)
			APPSR = 99;

		int REGEN = 100.0 / (REG_max - REG_min) * (BPPS.data - REG_min);
		if (APPSR > 99)
			APPSR = 99;

		snprintf(str, 21, "Cur L%2d%%  R%2d%%  B%2d%%", APPSL, APPSR, REGEN);
		if (debugconfig && redraw)
			DebugPrintf(str);

		snprintf(str, 21, "Mn %5d %5d %5d", APPSL_min, APPSR_min, REG_min);
		if (debugconfig && redraw)
			DebugPrintf(str);

		snprintf(str, 21, "Mx %5d %5d %5d", APPSL_max, APPSR_max, REG_max);
		if (debugconfig && redraw)
			DebugPrintf(str);
	}

	if (input == KEY_ENTER) {

		eepromdata *data = getEEPROMBlock(0);

		if (APPSL_max == 0 || APPSR_max == 0) {

		} else {
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

		if (REG_max == 0) {

		} else {
			data->ADCBrakeTravelInput[0] = REG_min;
			data->ADCBrakeTravelInput[1] = REG_max;
			data->ADCBrakeTravelInput[2] = 0;
			data->ADCBrakeTravelInput[3] = 0;
			// store new Regen calibration to memory.
		}

		return false;
	} else
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

bool DoMenuTorque(uint16_t input) {
#define TORQUEMENU_WHEELS		(1)
#define TORQUEMENU_TCS			(2)
#define TORQUEMENU_VECTORING	(3)
#define TORQUEMENU_TRACTION   	(4)
#define TORQUEMENU_VELOCITY   	(5)
#define TORQUEMENU_FEEDBACK		(6)
#define TORQUEMENU_FEEDFWD		(7)
#define TORQUEMENU_VELSOURCE	(8)
#define TORQUEMENU_LAST	 		(TORQUEMENU_FEEDFWD)
#define TORQUEMENUSIZE			(TORQUEMENU_LAST+1)

	static menustruct_t menu = { .inedit = false, .top = 0, .selection = 0,
			.menusize = TORQUEMENUSIZE };

	static char MenuLines[TORQUEMENUSIZE + 1][21] = { 0 };

	if (menu.selection == 0 && input == KEY_ENTER) // CheckButtonPressed(Config_Input) )
	{
		redraw = true;
		DebugPrintf("Leaving torque menu");
		menu.inedit = false;
		return false;
	}

	strcpy(MenuLines[0], "Vectoring Menu:");
	sprintf(MenuLines[1], "%cBack...", (menu.selection == 0) ? '>' : ' ');
	if (debugconfig && redraw)
		DebugPrintf(MenuLines[0]);

	for (int i = 0; i < 3; i++) {
		if (debugconfig && redraw)
			DebugPrintf(MenuLines[i + menu.top + 1]);
	}
	redraw = false;

	return true; // done with menu
}

bool DoMenu(uint16_t input) {
	static bool inmenu = false;
	static bool incalib = false;
	static bool dofullsave = false;
	static int8_t submenu = 0;

	static menustruct_t menu = { .inedit = false, .top = 0, .selection = 0,
			.menusize = MAINMENUSIZE };

	static char MenuLines[MAINMENUSIZE + 1][21] = { 0 };

	if (inmenu) {
		if (submenu == 0 && menu.selection == 0 && input == KEY_ENTER) // CheckButtonPressed(Config_Input) )
		{
			inmenu = false;
			menu.inedit = false;
			DebugPrintf("\nSaving settings\n");

			writeFullConfigEEPROM();

			if (dofullsave) {
				writeFullConfigEEPROM();
			} else {
				writeEEPROMCurConf(); // enqueue write the data to eeprom.
			}

			return false;
		}

		if (submenu == 0) // only check for entering a menu if not in one.
				{
			if (menu.selection == MENU_TORQUE && input == KEY_ENTER) {
				redraw = true;
				submenu = MENU_TORQUE;
				input = 0;
			}

		}

		if (submenu != 0) // we're in a sub menu, process it instead of current menu.
				{
			switch (menu.selection) // run the sub menu.
			{
			case MENU_TORQUE:
				if (!DoMenuTorque(input)) {
					redraw = true;
					submenu = 0; // check if sub menu is done.
				}
				break;
			default:
				submenu = 0;
			}
			input = 0; // in a sub menu, no input processing here.
			return true;
		}

		if (!incalib && menu.selection == MENU_CALIB && input == KEY_ENTER) // CheckButtonPressed(Config_Input) )
		{
			if (DeviceState.CriticalSensors == OPERATIONAL) {
				redraw = true;
				incalib = true;
				input = 0;

				APPSL_min = UINT16_MAX;
				APPSL_max = 0;
				APPSR_min = UINT16_MAX;
				APPSR_max = 0;
				REG_min = UINT16_MAX;
				REG_max = 0;
			} else {
				DebugPrintf("Err: ADC Not ready.");
				input = 0; // input has been seen, null it.
			}
		}

		if (incalib) {
			if (!doPedalCalibration(input)) {
				redraw = true;
				incalib = false;
				dofullsave = true;
				SetupInterpolationTables(getEEPROMBlock(0));

				// set the current pedal calibration after calibration exited.
			} else
				return true;
		}

		strcpy(MenuLines[0], "Config Menu:");

		sprintf(MenuLines[1], "%cBack & Save",
				(menu.selection == 0) ? '>' : ' ');
		snprintf(MenuLines[1 + MENU_TORQUE], sizeof(MenuLines[0]),
				"%cTorqueVect...", (menu.selection == MENU_TORQUE) ? '>' : ' ');

		uint16_t currpm = getEEPROMBlock(0)->maxRpm;

		if (currpm != getEEPROMBlock(0)->maxRpm) {
			getEEPROMBlock(0)->maxRpm = currpm;
// add rr
		}

		bool curfans = getEEPROMBlock(0)->Fans;
		if (curfans != getEEPROMBlock(0)->Fans) {
			getEEPROMBlock(0)->Fans = curfans;
			setDevicePower(LeftFans, curfans);
			setDevicePower(RightFans, curfans);
		}

		uint8_t curfanmaxcur = ceil((100.0 / 255 * getEEPROMBlock(0)->FanMax)); // convert to %

		uint8_t curfanmax = curfanmaxcur;
		if (curfanmax != curfanmaxcur) { // value changed.
			getEEPROMBlock(0)->FanMax = floor(curfanmax * 2.55);
			FanPWMControl(getEEPROMBlock(0)->FanMax, getEEPROMBlock(0)->FanMax);
		}

		snprintf(MenuLines[1 + MENU_CALIB], sizeof(MenuLines[0]),
				"%cAPPS Calib", (menu.selection == MENU_CALIB) ? '>' : ' ');

		snprintf(MenuLines[1 + MENU_STEERING], sizeof(MenuLines[0]),
				"%cSteeringCalib %+4d",
				(menu.selection == MENU_STEERING) ? '>' : ' ',
				SteeringAngle.data);

		if (menu.selection == MENU_STEERING && input == KEY_ENTER) {
			if (SteeringAngle.data != 0xFFFF) {
				getEEPROMBlock(0)->steerCalib = 180 - SteeringAngle.data;
				// value should update on display. add a set message.
				DebugPrintf("Steering angle calibrated to offset %d",
						180 - SteeringAngle.data);
			} else {
				DebugPrintf("Steering angle no data to calibrate");
			}
		}

		uint8_t regenon = getEEPROMBlock(0)->Regen;

		if (regenon != getEEPROMBlock(0)->Regen) {
			getEEPROMBlock(0)->Regen = regenon;
		}

		uint8_t regenmax = getEEPROMBlock(0)->regenMax;
		if (regenmax != getEEPROMBlock(0)->regenMax) { // value changed.
			getEEPROMBlock(0)->regenMax = regenmax;
		}

		uint8_t regenmaxR = getEEPROMBlock(0)->regenMaxR;
		if (regenmaxR != getEEPROMBlock(0)->regenMaxR) { // value changed.
			getEEPROMBlock(0)->regenMaxR = regenmaxR;
		}

		bool curTM = getEEPROMBlock(0)->Telemetry;
		if (curTM != getEEPROMBlock(0)->Telemetry) {
			getEEPROMBlock(0)->Telemetry = curTM;
			setDevicePower(Telemetry, curTM);
		}

#if (MENU_LAST == MENU_HV)
		bool curhvState = getEEPROMBlock(0)->alwaysHV;
		if (curhvState != getEEPROMBlock(0)->alwaysHV) {
			getEEPROMBlock(0)->alwaysHV = curhvState;
			ShutdownCircuitSet(curhvState);
		}
#endif

		if (debugconfig && redraw)
			DebugPrintf(MenuLines[0]);

		for (int i = 0; i < 3; i++) {
			if (debugconfig && redraw)
				DebugPrintf(MenuLines[i + menu.top + 1]);
		}

		if (debugconfig && redraw)
			DebugPrintf("------\n");

		redraw = false; // updated, unflag till something changes.
		return true;
	}

	if (!inmenu) {
		inmenu = true;
		submenu = 0;
		dofullsave = false;

		if (debugconfig) {
			redraw = true; // starting menu, draw it.
			DebugPrintf("------\n");
		}

		return true;
	}

	redraw = false;
	return false;

}

// Add message to uart message queue. Might be called from ISR so add a check.
bool ConfigInput(uint16_t input) {
	ConfigInput_msg confmsg;

	confmsg.msgval = input;

	if (xPortIsInsideInterrupt())
		return xQueueSendFromISR(ConfigInputQueue, &confmsg, 0);
	else
		return xQueueSendToBack(ConfigInputQueue, &confmsg, 0); // send it to error state handler queue for display to user.
}

char ConfStr[40] = "";

char* getConfStr(void) {
	// TODO add a mutex
	if (ConfStr[0] == 0)
		return NULL;
	else
		return ConfStr;
}

SemaphoreHandle_t xInConfig = NULL;
StaticSemaphore_t xInConfigBuffer;

uint8_t configstate = 0;

bool inConfig(void) {
	return configstate; //uxSemaphoreGetCount( xInConfig );
}

// checks if device initial values appear OK.
void ConfigTask(void *argument) {
	xEventGroupSync(xStartupSync, 0, 1, portMAX_DELAY); // ensure that tasks don't start before all initialisation done.

	ConfigInput_msg confinp;

	while (1) {
		// config menu does not need to run very real time.
		if (xQueueReceive(ConfigInputQueue, &confinp, 20)) {
			if (confinp.msgval == 0xFFFF) {
				xSemaphoreTake(xInConfig, 0);
				configstate = 1;
			}
		} else {
			confinp.msgval = 0;
		}

		switch (configstate) {
		case 0:
			break;

		case 1:
			if (!EEPROMBusy()) {
				if (!DoMenu(confinp.msgval)) {
					configstate = 0;
					xSemaphoreGive(xInConfig);
				}
			}

			// check if new can data received.
			if (ECUConfignewdata) {
				ECUConfignewdata = false;

				if (ECUConfigdata[0] != 0) {
					switch (ECUConfigdata[0]) {
					case 2:
						//CAN_SendADCminmax();
						break;
					case 3:
						// toggle HV.
						break;

					default: // unknown request.
						break;
					}
				} else {
					// deal with local data.
				}
			}

			break;
		}

		snprintf(ConfStr, 40, "Conf: %dnm %s %c %s", CarState.Torque_Req_Max,
				GetPedalProfile(CarState.PedalProfile, true),
				(!CarState.LimpDisable) ? 'T' : 'F',
				(CarState.FanPowered) ? "Fan" : "");

	}

	// clean up if we somehow get here.
	vTaskDelete(NULL);
}

bool initConfig(void) {
	RegisterCan1Message(&ECUConfig);

	ConfigInputQueue = xQueueCreateStatic(ConfigInputQUEUE_LENGTH,
			ConfigInputITEMSIZE, ConfigInputQueueStorageArea,
			&ConfigInputStaticQueue);

	vQueueAddToRegistry(ConfigInputQueue, "Config Input");

	xInConfig = xSemaphoreCreateBinaryStatic(&xInConfigBuffer);

	CarState.Torque_Req_Max = 0;
	CarState.Torque_Req_CurrentMax = 0;

	ConfigTaskHandle = xTaskCreateStatic(ConfigTask,
	ConfigTASKNAME,
	ConfigSTACK_SIZE, (void*) 1,
	ConfigTASKPRIORITY, xConfigStack, &xConfigTaskBuffer);

	return true;
}

