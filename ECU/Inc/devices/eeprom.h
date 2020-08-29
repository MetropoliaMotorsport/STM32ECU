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

typedef enum EEPROM_cmd { send, receive, read, write } EEPROM_cmd;

typedef struct EEPROM_msg {
	EEPROM_cmd cmd;
} EEPROM_msg;

bool GetEEPROMCmd(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle );

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
	int16_t CoolantOutput[20];  // 80 bytes.

	uint16_t DrivingModeInput[8]; //  16 bytes -- 506 bytes to here

	// config data

	uint8_t MaxTorque;
	uint8_t PedalProfile;
	bool LimpMode;
	bool TorqueVectoring;
	bool Fans;						// 511 bytes

} eepromdata; // max 1600bytes=50*32byte blocks.

// 503 - 16 blocks. allocate 50 blocks : 128 blocks total

int initEEPROM( void );

bool checkversion(char * data);

uint8_t * getEEPROMBuffer( void );

eepromdata * getEEPROMBlock(int block );

int readEEPROMAddr( uint16_t address, uint16_t size );

void commitEEPROM( void ); // function for timer callback to handle writing.

int writeFullEEPROM( void );
int writeConfigEEPROM( void );
int writeEEPROMCurConf( void );

bool writeEEPROMDone( void );

int EEPROMSend( void );

bool stopEEPROM( void );
bool EEPROMBusy( void );

int DoEEPROM( void );

#endif /* EEPROM_H_ */

