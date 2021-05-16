/*
 * adc_hpf19.h
 *
 *  Created on: May 15, 2021
 *      Author: visa
 */

#ifndef ADC_HPF19_H_
#define ADC_HPF19_H_

uint16_t SteeringInput[ sizeof(((eepromdata*)0)->ADCSteeringInput)/2 +2 ] = { 0 };
int16_t SteeringOutput[ sizeof(((eepromdata*)0)->ADCSteeringOutput)/2 +2 ] = { 0 };
uint8_t SteeringSize = 0;

// 19500 ~ -90  // 13300 full lock, 45000 ~ 90 deg right. 50000 full lock right. ~120
uint16_t SteeringInput[] = { 6539, 13300, 20000, 33500, 63019}; // going to pwm this year, no ADC needed.
int16_t SteeringOutput[] = { -210,  -120,  -90,   0,    210 };
uint8_t SteeringSize = sizeof(SteeringInput)/sizeof(SteeringInput[0]);

// -1 needs to be at minimum
// should be 0 to 25bar at 1-5v   0.6666v to 3.3v at adc -> 13107 -> 65536
uint16_t BrakeRInput[] = {1024, 1025, BRAKEZERO, BRAKEMAX, 65535 }; // at 240bar, should be 240 output, at 0 bar should be 0 // 62914
int16_t BrakeROutput[] = {-1,     0,    0,     240,  255 }; // output range // 240
uint8_t BrakeRSize = sizeof(BrakeRInput)/sizeof(BrakeRInput[0]);

uint16_t BrakeFInput[] = { 1024, 1025, BRAKEZERO,   BRAKEMAX, 65535 }; // calibrated input range //62914
int16_t BrakeFOutput[] = { -1,     0,    0,     240,    255 }; // output range // 240
uint8_t BrakeFSize = sizeof(BrakeFInput)/sizeof(BrakeFInput[0]);

// zero should be approx real pedal zero, zero is read below this to allow for some variance without triggering errors.
// ditto max value.

// define zero as 5% actual travel and 100% as 95% of actual travel
uint16_t TorqueReqLInput[] = { 1999,  2000, (ACCELERATORLMAX-ACCELERATORLZERO)/100*5+ACCELERATORLZERO,   (ACCELERATORLMAX-ACCELERATORLZERO)/100*98+ACCELERATORLZERO,  64000,  64001 }; // calibration values for left input // 5800
int16_t TorqueReqLOutput[] = {  -1,  0,     0,     1000,      1000,  1001 }; // range defined 0-1000 to allow percentage accuracy even if not using full travel range.
uint8_t TorqueReqLSize = sizeof(TorqueReqLInput)/sizeof(TorqueReqLInput[0]);

// TorqueRMin(6798) / TorqueRMax(54369)
uint16_t TorqueReqRInput[] = { 1999, 2000, (ACCELERATORRMAX-ACCELERATORRZERO)/100*5+ACCELERATORRZERO,  (ACCELERATORRMAX-ACCELERATORRZERO)/100*98+ACCELERATORRZERO,   64000,   64001 }; // calibration values for right input // 6200
int16_t TorqueReqROutput[] = { -1,      0,      0,      1000,   1000,   1001 };
uint8_t TorqueReqRSize = sizeof(TorqueReqRInput)/sizeof(TorqueReqRInput[0]);


uint16_t TorqueLinearInput[] = {50,950}; // start registered travel at 8%
int16_t TorqueLinearOutput[] = {0,1000};

uint16_t TorqueLowTravelInput[] = {50,500}; // start registered travel at 10%
int16_t TorqueLowTravelOutput[] = {0, 1000};

uint16_t TorqueLargelowRangeInput[] = {50,600, 950}; // start registered travel at 10%
int16_t TorqueLargelowRangeOutput[] = {0, 400,1000};

uint16_t CoolantInput[] = { 1000,4235, 4851, 5661, 6889, 8952, 11246, 14262, 18894, 22968, 27081, 33576, 39050, 44819, 49192, 54011, 58954,  64113, 64112};
int16_t CoolantOutput[] = { -1,   120,   115,  109,  101,   90,    82,    72,    60,    52,    46,    38,    32,    26,    22,    16,    11,    6, -1};
uint8_t CoolantLElements = sizeof(CoolantInput)/sizeof(CoolantInput[0]);


uint16_t DrivingModeInput[] = { 0 , 1022, 1023, 1024,4500, 13500, 23500, 33000, 40000, 49000, 57500, 65534, 65535 };
int16_t DrivingModeOutput[] = { 1 , 1,    0,     1,   1,    2,    3,      4,     5,     6,    7,      8,     0 };
uint8_t DriveModeSize = sizeof(DrivingModeInput)/sizeof(DrivingModeInput[0]);

#endif /* ADC_HPF19_H_ */
