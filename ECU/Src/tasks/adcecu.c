/*
 * adcecu.c
 *
 *      Author: Visa
 */

#include "ecumain.h"

#include "adc.h"
#include "eeprom.h"
#include "errors.h"
#include "input.h"
#include "adcecu.h"
#include "semphr.h"
#include "analognode.h"
#include <limits.h>
#include "taskpriorities.h"
#include "timerecu.h"
#include "debug.h"
#include "power.h"
#include "brake.h"

#define UINTOFFSET	360

#ifdef HPF20
#include "adc_hpf20.h"
#endif

volatile char minmaxADC;

volatile ADCState_t ADCState;
volatile ADCState_t ADCStateNew;

volatile ADCStateSensors_t ADCStateSensors;

volatile ADCInterpolationTables_t ADCInterpolationTables;

void ReadADC1(bool half);
void ReadADC3(bool half);
int getSteeringAnglePWM(void);

TaskHandle_t ADCTaskHandle = NULL;

#define ADCSTACK_SIZE 128*2
#define ADCTASKNAME  "ADCTask"
StaticTask_t xADCTaskBuffer;
StackType_t xADCStack[ADCSTACK_SIZE];

static SemaphoreHandle_t waitStr = NULL;
SemaphoreHandle_t ADCUpdate = NULL;

char ADCWaitStr[20] = "";

char* getADCWait(void) {
	static char ADCWaitStrRet[20] = "";
	xSemaphoreTake(waitStr, portMAX_DELAY);
	strcpy(ADCWaitStrRet, ADCWaitStr);
	xSemaphoreGive(waitStr);
	return ADCWaitStrRet;
}

uint32_t curanaloguenodesOnline = 0;
uint32_t analoguenodesOnline = 0;
uint32_t lastanaloguenodesOnline = 0;

uint32_t getAnalogueNodesOnline(void) {
	return curanaloguenodesOnline;
}

// Task shall monitor analogue node timeouts, and read data from local ADC/PWM to get all analogue values ready for main loop.
void ADCTask(void *argument) {
	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */

	xEventGroupSync(xStartupSync, 0, 1, portMAX_DELAY);

	DeviceState.ADC = OPERATIONAL;

	DeviceState.Sensors = OFFLINE; // not seen anything yet, so assume offline.

	uint32_t count = 0;

	uint32_t lastseenall = 0;

	uint32_t analoguenodesOnlineSince = 0;

	ADCWaitStr[0] = 0;

	while (1) {
		lastanaloguenodesOnline = analoguenodesOnline;
		// read the values from last cycle.
		xTaskNotifyWait( pdFALSE, ULONG_MAX, &analoguenodesOnline, 0);

		// block and wait for main cycle.
		xEventGroupSync(xCycleSync, 0, 1, portMAX_DELAY);

		// copy received critical sensor data from last cycle to
		xSemaphoreTake(ADCUpdate, portMAX_DELAY);
		memcpy(&ADCState, &ADCStateNew, sizeof(ADCState));
		ADCState.Oldest = getOldestANodeCriticalData();
		xSemaphoreGive(ADCUpdate);

		uint32_t curtime = gettimer();

		if (DeviceState.CriticalSensors == OPERATIONAL) {
			if (curtime - CYCLETIME * 2 - 1 > ADCState.Oldest) // TODO investigate, sometimes getting upto 39 ms
					{
				DebugPrintf("Oldest ANode data %d old at (%lu)",
						curtime - ADCState.Oldest, curtime);
			}
		}

		count++;

		if (lastanaloguenodesOnline != analoguenodesOnline) {
			char str[40];
			snprintf(str, 40, "Analogue diff %lu", count);
//			DebugMsg(str);
		}

		getSteeringAnglePWM();

		xSemaphoreTake(waitStr, portMAX_DELAY);

		analoguenodesOnlineSince |= analoguenodesOnline; // cumulatively add

		if ((curanaloguenodesOnline & AnodeCriticalBit) == AnodeCriticalBit) {
			DeviceState.CriticalSensors = OPERATIONAL;
			// we've received all the SCS data
		} else {
			DeviceState.CriticalSensors = INERROR;
		}

		if ((analoguenodesOnlineSince & ANodeAllBit) == ANodeAllBit) // all expected nodes reported in.
		{
			if (DeviceState.Sensors != OPERATIONAL) {
				DebugMsg("Analogue nodes all online");
			}
			DeviceState.Sensors = OPERATIONAL;
			setAnalogNodesStr(analoguenodesOnlineSince);
			ADCWaitStr[0] = 0;
			analoguenodesOnlineSince = 0;
			curanaloguenodesOnline = analoguenodesOnline;
			lastseenall = gettimer();
		} else if (gettimer() - lastseenall > NODETIMEOUT) // only update status to rest of code every timeout val.
		{
			lastseenall = gettimer();
			setAnalogNodesStr(analoguenodesOnlineSince);
			strcpy(ADCWaitStr, getAnalogNodesStr());

			// update the currently available nodes
			curanaloguenodesOnline = analoguenodesOnlineSince;

			if (analoguenodesOnlineSince == 0) {
				if (DeviceState.Sensors != OFFLINE) {
					DebugMsg("Analogue node timeout");
				}
				DeviceState.Sensors = OFFLINE; // can't see any nodes, so offline.
			} else {
				if (DeviceState.Sensors != INERROR) {
					DebugPrintf("Analogue nodes partially online (%lu)",
							curanaloguenodesOnline);
				}
				DeviceState.Sensors = INERROR; // haven't seen all needed, so in error.
			}

			analoguenodesOnlineSince = 0;
		}

#ifndef NOBRAKELIGHTCONTROL
		if (getBrakeLight()) {
			setDevicePower(Brake, true);
		} else {
			setDevicePower(Brake, false);
		};
#endif

		xSemaphoreGive(waitStr);

		DeviceState.ADCSanity = CheckADCSanity();
	}

	vTaskDelete(NULL);
}

bool SetupADCInterpolationTables(eepromdata *data) {
	// calibrated input range for steering, from left lock to center to right lock.
	// check if this can be simplified?

	if (checkversion(data->VersionString)) {

		int i = 0;

		BrakeRInput[2] = data->ADCBrakeRPresInput[0];
		BrakeRInput[3] = data->ADCBrakeRPresInput[1];

		BrakeROutput[2] = data->ADCBrakeRPresOutput[0];
		BrakeROutput[3] = data->ADCBrakeRPresOutput[1];

		BrakeFInput[2] = data->ADCBrakeFPresInput[0];
		BrakeFInput[3] = data->ADCBrakeFPresInput[1];

		BrakeFOutput[2] = data->ADCBrakeFPresOutput[0];
		BrakeFOutput[3] = data->ADCBrakeFPresOutput[1];

		i = 0;

		int TravMin = data->ADCTorqueReqLInput[0];
		int TravMax = data->ADCTorqueReqLInput[1];
		if (TravMax == 0) // not calibrated, force 0 output
				{
			TravMin = 64000;
			TravMax = 64000;
		}
		int TravMinOffset = 10;
		int TravMaxOffset = 98;

		if (data->ADCTorqueReqLInput[3] != 0) {
			TravMinOffset = data->ADCTorqueReqLInput[2];
			TravMaxOffset = data->ADCTorqueReqLInput[3];
		}

		TorqueReqLInput[2] = (TravMax - TravMin) / 100 * TravMinOffset
				+ TravMin;
		TorqueReqLInput[3] = (TravMax - TravMin) / 100 * TravMaxOffset
				+ TravMin;

#ifdef TORQUEERRORCHECK
		uint32_t absolutemax = TravMax*1.1;
		if ( absolutemax > 0xFFFF )
			absolutemax + 64000;

		TorqueReqLInput[4] = absolutemax;
		TorqueReqLInput[5] = TorqueReqLInput[4]+1;
#endif

		TravMin = data->ADCTorqueReqRInput[0];
		TravMax = data->ADCTorqueReqRInput[1];
		if (TravMax == 0) {
			TravMin = 64000;
			TravMax = 64000;
		}
		TravMinOffset = 10;
		TravMaxOffset = 98;

		if (data->ADCTorqueReqRInput[3] != 0) {
			TravMinOffset = data->ADCTorqueReqRInput[2];
			TravMaxOffset = data->ADCTorqueReqRInput[3];
		}

		TorqueReqRInput[2] = (TravMax - TravMin) / 100 * TravMinOffset
				+ TravMin;
		TorqueReqRInput[3] = (TravMax - TravMin) / 100 * TravMaxOffset
				+ TravMin;

#ifdef TORQUEERRORCHECK
		absolutemax = TravMax*1.1;
		if ( absolutemax > 0xFFFF )
			absolutemax + 64000;

		TorqueReqRInput[4] = absolutemax;
		TorqueReqRInput[5] = TorqueReqRInput[4]+1;
#endif

		TravMin = data->ADCBrakeTravelInput[0];
		TravMax = data->ADCBrakeTravelInput[1];
		if (TravMax == 0) {
			TravMin = 64000;
			TravMax = 64000;
		}
		TravMinOffset = 10;
		TravMaxOffset = 98;

		if (data->ADCBrakeTravelInput[3] != 0) {
			TravMinOffset = data->ADCBrakeTravelInput[2];
			TravMaxOffset = data->ADCBrakeTravelInput[3];
		}

		// regen
		BrakeTravelInput[2] = (TravMax - TravMin) / 100 * TravMinOffset
				+ TravMin;
		BrakeTravelInput[3] = (TravMax - TravMin) / 100 * TravMaxOffset
				+ TravMin;

#ifdef TORQUEERRORCHECK
		absolutemax = TravMax*1.1;
		if ( absolutemax > 0xFFFF )
			absolutemax + 64000;

		BrakeTravelInput[4] = absolutemax;
		BrakeTravelInput[5] = BrakeTravelInput[4]+1;
#endif

		if (data->pedalcurves[i].PedalCurveInput[1] != 0) {
			TorqueCurveCount = 0;

			i = 0;

			for (; data->pedalcurves[i].PedalCurveInput[1] != 0; i++) // first number could be 0, but second will be non zero.
					{
				TorqueCurveCount++;

				int j = 0;
				do {
					TorqueInputs[i][j] =
							data->pedalcurves[i].PedalCurveInput[j];
					TorqueOutputs[i][j] =
							data->pedalcurves[i].PedalCurveOutput[j];
					j++;

				} while (data->pedalcurves[i].PedalCurveInput[j] != 0);
				//			if ( j < 3 ) j = 0;
				TorqueCurveSize[i] = j;
			}
		}

		ADCInterpolationTables.BrakeR.Input = BrakeRInput;
		ADCInterpolationTables.BrakeR.Output = BrakeROutput;

		ADCInterpolationTables.BrakeR.Elements = BrakeRSize;

		ADCInterpolationTables.BrakeF.Input = BrakeFInput;
		ADCInterpolationTables.BrakeF.Output = BrakeFOutput;

		ADCInterpolationTables.BrakeF.Elements = BrakeFSize;

		ADCInterpolationTables.Regen.Input = BrakeTravelInput;
		ADCInterpolationTables.Regen.Output = BrakeTravelOutput;

		ADCInterpolationTables.Regen.Elements = BrakeTravelSize;

		ADCInterpolationTables.AccelL.Input = TorqueReqLInput;
		ADCInterpolationTables.AccelL.Output = TorqueReqLOutput;

		ADCInterpolationTables.AccelL.Elements = TorqueReqLSize;

		ADCInterpolationTables.AccelR.Input = TorqueReqRInput;
		ADCInterpolationTables.AccelR.Output = TorqueReqROutput;

		ADCInterpolationTables.AccelR.Elements = TorqueReqRSize;

		ADCInterpolationTables.TorqueCurve.Input = TorqueInputs[0];
		ADCInterpolationTables.TorqueCurve.Output = TorqueOutputs[0];

		ADCInterpolationTables.TorqueCurve.Elements = TorqueCurveSize[0];

		ADCInterpolationTables.Coolant.Input = CoolantInput;
		ADCInterpolationTables.Coolant.Output = CoolantOutput;

		ADCInterpolationTables.Coolant.Elements = CoolantSize;

		ADCInterpolationTables.ModeSelector.Input = DrivingModeInput;
		ADCInterpolationTables.ModeSelector.Output = DrivingModeOutput;

		ADCInterpolationTables.ModeSelector.Elements = DriveModeSize;

		return true;
	} else
		return false;
}

void SetupTorque(int request) {
	ADCInterpolationTables.TorqueCurve.Input = TorqueInputs[request];
	ADCInterpolationTables.TorqueCurve.Output = TorqueOutputs[request];
	ADCInterpolationTables.TorqueCurve.Elements = TorqueCurveSize[request];
}

/**
 * function to perform a linear interpolation using given input/output value arrays and raw data.
 */
int16_t linearInterpolate(uint16_t Input[], int16_t Output[], uint16_t count,
		uint16_t RawADCInput) {
	int i;

	if (Input == NULL) {
		return 0;
	}

	if (count < 2) {
		return 0;
	}

	if (RawADCInput < Input[0]) { // if input less than first table value return first.
		return Output[0];
	}

	if (RawADCInput > Input[count - 1]) { // if input larger than last table value return last.
		return Output[count - 1];
	}

	// loop through input values table till we find space where requested fits.
	for (i = 0; i < count - 1; i++) {
		if (Input[i + 1] > RawADCInput) {
			break;
		}
	}

	int dx, dy;

	/* interpolate */
	dx = Input[i + 1] - Input[i];
	dy = Output[i + 1] - Output[i];
	return Output[i] + ((RawADCInput - Input[i]) * dy / dx);
}

int getSteeringAnglePWM(void) {
	volatile int angle = 0;

	if (receivePWM()) {
		angle = getPWMDuty();
		ADCState.SteeringDuty = angle;
		ADCState.SteeringFreq = getPWMFreq();
		angle = (angle * 360.0);
		angle = angle / 10000; // center around 180;
		ADCState.SteeringAngleAct = angle;
		angle += getEEPROMBlock(0)->steerCalib; // account for calibration
		if (angle < 0)
			angle = angle + 360;
		if (angle > 359)
			angle = angle - 360;
		angle = (angle - 180);
		ADCState.SteeringAngle = -1 * angle; // flip positive.
		xTaskNotify(ADCTaskHandle, ( 0x1 << SteeringAngleReceivedBit ),
				eSetBits);
		return true;
	} else {
		ADCState.SteeringAngle = 0xFFFF; // not read, return impossible angle for sanity check.
		ADCState.SteeringAngleAct = 0xFFFF;
		return false;
	}
}

/**
 * convert raw front brake reading into calibrated brake position
 */
int getBrakeF(uint16_t RawADCInput) {
#ifdef NOBRAKES
	return 0;
#else
	struct ADCTable ADC = ADCInterpolationTables.BrakeF;
	return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInput);
#endif
}

/**
 * convert raw rear brake reading into calibrated brake position
 */
int getBrakeR(uint16_t RawADCInput) {
#ifdef NOBRAKES
	return 0;
#else
	struct ADCTable ADC = ADCInterpolationTables.BrakeR;
	return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInput);
#endif
}

/**
 * convert front/rear brake inputs into brake balance percentage.
 */
int getBrakeBalance(uint16_t ADCInputF, uint16_t ADCInputR) {
	if (ADCInputF > 5 && ADCInputR > 5) // too small a value to get accurate reading
			{
		return (ADCInputF * 100) / (ADCInputF + ADCInputR);
	} else
		return 0;

}

int getTorqueReqPercR(uint16_t RawADCInputR) {
	struct ADCTable ADC = ADCInterpolationTables.AccelR;
	return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInputR);
}

int getTorqueReqPercL(uint16_t RawADCInputF) {
#ifdef NOAPPS
	return 0;
#else
	struct ADCTable ADC = ADCInterpolationTables.AccelL;
	return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInputF);
#endif
}

int getBrakeTravelPerc(uint16_t RawADCInputF) {
#ifdef NOAPPS
	return 0;
#else
	struct ADCTable ADC = ADCInterpolationTables.Regen;
	return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInputF);
#endif
}

int getTorqueReqCurve(uint16_t ADCInput) {
#ifdef NOAPPS
	return 0;
#else
	struct ADCTable ADC = ADCInterpolationTables.TorqueCurve;
	int returnval = linearInterpolate(ADC.Input, ADC.Output, ADC.Elements,
			ADCInput);
	return returnval;
#endif
}

/**
 * process driving mode selector input, adjusts max possible torque request. Equivalent to gears.
 */
int getDrivingMode(uint16_t RawADCInput)
// this will become a configuration setting, changed at config portion of bootup only.
{ // torq_req_max, call once a second
	return 5;
}

int getCoolantTemp(uint16_t RawADCInput) {
	return 20;
}

int getCoolantTemp2(uint16_t RawADCInput) {
	return 20;
}

HAL_StatusTypeDef startADC(void) {
	minmaxADC = 1;
	minmaxADCReset();

	// in RTOS mode ADC poll is requested per cycle.
	DeviceState.ADC = OPERATIONAL;

	return 0;
}

// checks all expected data is present and within acceptable range -- allow occasional error, remove inverter.
uint32_t CheckADCSanity(void) {
	uint16_t returnvalue = 0;

	// check adc's are giving values in acceptable range.
	if (abs(ADCState.SteeringAngle) >= 181) // if impossible angle.
			{
//            returnvalue &= ~(0x1 << SteeringAngleReceivedBit);
		//   returnvalue |= 0x1 << SteeringAngleReceivedBit;
	} else
		returnvalue &= ~(0x1 << SteeringAngleReceivedBit);

	if (DeviceState.CriticalSensors == OPERATIONAL) // string will be empty if everything expected received.
			{

		if (ADCState.BrakeF < 0 || ADCState.BrakeF >= 240)
			returnvalue |= 0x1 << BrakeFReceivedBit; // Received
		else
			returnvalue &= ~(0x1 << BrakeFReceivedBit); // ok

		if (ADCState.BrakeR < 0 || ADCState.BrakeR >= 240)
			returnvalue |= 0x1 << BrakeRReceivedBit;
		else
			returnvalue &= ~(0x1 << BrakeRReceivedBit);

		if (ADCState.Torque_Req_R_Percent < 0
				|| ADCState.Torque_Req_R_Percent > 1000) // if value is not between 0 and 100 then out of range Received
			returnvalue |= 0x1 << AccelRReceivedBit;
		else
			returnvalue &= ~(0x1 << AccelRReceivedBit);

		if (ADCState.Torque_Req_L_Percent < 0
				|| ADCState.Torque_Req_L_Percent > 1000) // if value is not between 0 and 100 then out of range Received
			returnvalue |= 0x1 << AccelLReceivedBit;
		else
			returnvalue &= ~(0x1 << AccelLReceivedBit);

	} else { // not received data, set Received bits.
		returnvalue |= 0x1 << BrakeFReceivedBit; // Received
		returnvalue |= 0x1 << BrakeRReceivedBit;
		returnvalue |= 0x1 << AccelRReceivedBit;
		returnvalue |= 0x1 << AccelLReceivedBit;
		ADCState.BrakeF = 0; // if value is not between 0 and 255 then out of range Received
		ADCState.BrakeR = 0;
		ADCState.Torque_Req_R_Percent = 0;
		ADCState.Torque_Req_L_Percent = 0;
	}

	// calculate brake balance, if theres some pressure.
	if (ADCState.BrakeF > 1 && ADCState.BrakeR > 1 && ADCState.BrakeF < 255
			&& ADCState.BrakeR < 255) {
		CarState.brake_balance = getBrakeBalance(ADCState.BrakeF,
				ADCState.BrakeR);
	} else
		CarState.brake_balance = -1;

	if (returnvalue) { // if error in adc data check if it's yet to be treated as fatal.
		Errors.ADCError++; // increase adc error counter
		Errors.ADCErrorState = returnvalue; // store current error value for checking.
		DeviceState.ADC = INERROR;

	} else // no errors, clear flags.
	{
		if (Errors.ADCError > 0)
			Errors.ADCError--;
		Errors.ADCErrorState = 0;
	}

	return returnvalue;
}

int initADC(void) {
	waitStr = xSemaphoreCreateMutex();

	ADCUpdate = xSemaphoreCreateMutex();

	ADCTaskHandle = xTaskCreateStatic(ADCTask,
	ADCTASKNAME,
	ADCSTACK_SIZE, (void*) 1,
	ADCTASKPRIORITY, xADCStack, &xADCTaskBuffer);

	configASSERT(ADCTaskHandle);

	return 0;
}
