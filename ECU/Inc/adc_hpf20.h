/*
 * adc_hpf19.h
 *
 *  Created on: May 15, 2021
 *      Author: visa
 */

#ifndef ADC_HPF20_H_
#define ADC_HPF20_H_


uint16_t BrakeRInput[sizeof(((eepromdata*)0)->ADCBrakeRPresInput)/2 +3 ] = { 0, 0, 	  0, 	 0, 65535 }; // at 240bar, should be 240 output, at 0 bar should be 0 // 62914
int16_t BrakeROutput[sizeof(((eepromdata*)0)->ADCBrakeRPresOutput)/2 +3 ] = { 0,     0,    0,     240,  255 }; // output range // 240
uint8_t BrakeRSize = sizeof(BrakeRInput)/sizeof(BrakeRInput[0]);


uint16_t BrakeFInput[sizeof(((eepromdata*)0)->ADCBrakeFPresInput)/2 +3 ] = { 0, 0,    0,	 0, 65535 }; // at 240bar, should be 240 output, at 0 bar should be 0 // 62914
int16_t BrakeFOutput[sizeof(((eepromdata*)0)->ADCBrakeFPresOutput)/2 +3 ] = { 0,     0,    0,     240,  255 }; // output range // 240
uint8_t BrakeFSize = sizeof(BrakeFInput)/sizeof(BrakeFInput[0]);


// define zero as 5% actual travel and 100% as 95% of actual travel
uint16_t TorqueReqLInput[sizeof(((eepromdata*)0)->ADCTorqueReqLInput)/2+4] = {  0,  0, 0,     0,     64000,  64001 }; // calibration values for left input // 5800
int16_t TorqueReqLOutput[sizeof(((eepromdata*)0)->ADCTorqueReqLInput)/2+4] = {  0,  0,      0,     1000,   1000,  1001 }; // range defined 0-1000 to allow percentage accuracy even if not using full travel range.
uint8_t TorqueReqLSize = 6; //sizeof(TorqueReqLInput)/sizeof(TorqueReqLInput[0]);


// TorqueRMin(6798) / TorqueRMax(54369)
uint16_t TorqueReqRInput[sizeof(((eepromdata*)0)->ADCTorqueReqRInput)/2+4] =  {  0,  0, 0,     0,      64000,  64001 };; // calibration values for right input // 6200
int16_t TorqueReqROutput[sizeof(((eepromdata*)0)->ADCTorqueReqRInput)/2+4] = { 0,      0,      0,   1000,   1000,   1001 };

uint8_t TorqueReqRSize = 6; //sizeof(TorqueReqRInput)/sizeof(TorqueReqRInput[0]);

// Regen sensor.
uint16_t BrakeTravelInput[sizeof(((eepromdata*)0)->ADCTorqueReqRInput)/2+4] =  {  0,  0, 0,     0,      64000,  64001 };; // calibration values for right input // 6200
int16_t BrakeTravelOutput[sizeof(((eepromdata*)0)->ADCTorqueReqRInput)/2+4] = { 0,      0,      0,   1000,   1000,   1001 };

uint8_t BrakeTravelSize = 6; //sizeof(TorqueReqRInput)/sizeof(TorqueReqRInput[0]);

uint16_t TorqueInputs[5][sizeof(((eepromdata*)0)->pedalcurves[0].PedalCurveInput)/2] = {{50,950}}; // start registered travel at 8%
int16_t TorqueOutputs[5][sizeof(((eepromdata*)0)->pedalcurves[0].PedalCurveOutput)/2] = {{0,1000}};
uint8_t TorqueCurveSize[5] = { 2, 0 };
uint8_t TorqueCurveCount = 1;


// Following deprecated in HPF20, but kept defined for potential backwards compatibility.

uint16_t CoolantInput[sizeof(((eepromdata*)0)->CoolantInput)/2] =  { 0 }; // { 1000,4235, 4851, 5661, 6889, 8952, 11246, 14262, 18894, 22968, 27081, 33576, 39050, 44819, 49192, 54011, 58954,  64113, 64112};
int16_t CoolantOutput[sizeof(((eepromdata*)0)->CoolantOutput)/2] = { 0 }; // { -1,   120,   115,  109,  101,   90,    82,    72,    60,    52,    46,    38,    32,    26,    22,    16,    11,    6, -1};
uint8_t CoolantSize = 0;

uint16_t DrivingModeInput[sizeof(((eepromdata*)0)->DrivingModeInput)/2+4] = { 0 }; // { 0 , 1022, 1023, 1024,4500, 13500, 23500, 33000, 40000, 49000, 57500, 65534, 65535 };
int16_t DrivingModeOutput[sizeof(((eepromdata*)0)->DrivingModeInput)/2+4] = { 0 }; // { 1 , 1,    0,     1,   1,    2,    3,      4,     5,     6,    7,      8,     0 };
uint8_t DriveModeSize = 0;

#endif /* ADC_HPF20_H_ */
