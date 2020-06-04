/*
 * eeprom.h
 *
 *  Created on: 20 Feb 2020
 *      Author: Visa
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#define EEPROMWC_GPIO_Port 			GPIOF
#define EEPROMWC_Pin				GPIO_PIN_2

#define EEPROM_ADDRESS             0xA0    /* EEPROM M24128 Address  */
#define EEPROM_PAGESIZE            32     /* EEPROM M24128 used     */

//#define EEPROM_DATAVERSION		   0x01
//#define EEPROM_LONG_TIMEOUT        1000    /* Long Timeout 1s */
//#define EEPROM_TIMING              0x00D00E28


#define EEPROMVERSIONSTR		("MMECUV0.1")

/*
 *
 block 0: data version 0 if doesn't match, don't load use read eeprom.
 block 1: current active bank
 block 2-9:reserved
 block 10-59: config 1
 block 60-109: config 2
 block 110-124: reserved
 block 125-128: emergency message. 4 blocks.
*/

/*
Config block: 16 blocks for current state { operating mode, any disabled devices?, torque steering max etc., 64 bytes.}
adc config block: 16-32blocks.
other config info: 2 blocks.

 // writeblock(0/1);

accelerator travel, linear L & R

: pedal profiles for modes->at least 5

*/

typedef struct pedalcurvestruct {
	//  uint8_t PedalCurveSize
	uint16_t PedalCurveInput[16];
	uint16_t PedalCurveOutput[16];//   64 bytes. * 5
} pedalcurve;

// uint8_t ADCSteeringSize; // don't need size, can use 0 to terminate.
typedef struct eepromdatastruct {
	char VersionString[10];

	uint16_t ADCSteeringInput[10]; // HPF19 compatibility, potential future use
	int16_t ADCSteeringOutput[10]; // min/max possible, further reaches, mid point. 40 bytes

	uint16_t ADCBrakeRPresInput[2];   // hpf19 compatibility, potential future use.
	uint16_t ADCBrakeRPresOutput[2]; // 8 bytes

	uint16_t ADCBrakeFPresInput[2];   // hpf19 compatibility, potential future use. linear scale Need more if non linear.
	uint16_t ADCBrakeFPresOutput[2]; // 8 bytes // 57 bytes.

	uint16_t ADCBrakeTravelInput[4]; // will always map to 0-100%.. min valid, zero value, 100% val, max val. // 8 bytes.

	uint16_t ADCTorqueReqLInput[4]; // min, zero reading, 100% reading[98%], max. // 8 bytes

	uint16_t ADCTorqueReqRInput[4]; // min, zero reading, 100% reading[98%], max. // 8 bytes

	pedalcurve pedalcurves[5];

	//uint8_t CoolantSize
	uint16_t CoolantInput[20];
	int16_t CoolantOutput[20];  // 81 bytes.

	uint16_t DrivingModeInput[8]; //  16 bytes

	// config data

	uint8_t MaxTorque;
	uint8_t PedalProfile;
	uint8_t LimpMode;
	uint8_t TorqueVectoring;

} eepromdata;

//uint16_t DrivingModeOutput[] // not needed, static

// 503 - 16 blocks. allocate 50 blocks : 128 blocks total

// create a simple EEPROM checksum.two blocks of 1.5k
// two banks of 1.9k data.
// 2016 bytes per bank.

// use last 32bytes as emergency data.


int initiliseEEPROM();

bool checkversion(char * data);

uint8_t * getEEPROMBuffer();

eepromdata * getEEPROMBlock(int block );

void commitEEPROM(); // function for timer callback to handle writing.

int writeFullEEPROM();

bool writeEEPROMDone();

#endif /* EEPROM_H_ */

