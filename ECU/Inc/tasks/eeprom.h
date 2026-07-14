/*
 * eeprom.h
 *
 *  Created on: 20 Feb 2020
 *      Author: Visa
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include "canecu.h"
#include <stdbool.h>
#include <stdint.h>

#define EEPROMWC_GPIO_Port GPIOF
#define EEPROMWC_Pin GPIO_PIN_2

#define EEPROM_ADDRESS 0xA0 /* EEPROM M24128 Address  */
#define EEPROM_PAGESIZE 32  /* EEPROM M24128 used     */

#define EEPROMVERSIONSTR ("MMECUV0.1")

typedef enum EEPROM_cmd
{
  EEPROMCurConf,
  EEPROMRunningData,
  writeEEPROM1,
  writeEEPROM2,
  writeEEPROMC,
  FullConfigEEPROM,
  FullEEPROM,
  eraseEEPROM,
} EEPROM_cmd;

typedef struct EEPROM_msg
{
  EEPROM_cmd cmd;
} EEPROM_msg;

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
 Config block: 16 blocks for current state { operating mode, any disabled devices?, torque steering
 max etc., 64 bytes.} adc config block: 16-32blocks. other config info: 2 blocks.

 writeblock(0/1);

 accelerator travel, linear L & R

 : pedal profiles for modes->at least 5

 */

typedef struct pedalcurvestruct pedalcurve;
typedef struct eepromdatastruct eepromdata;
typedef struct runtimedata_t runtimedata_t;

struct pedalcurvestruct
{
  //  uint8_t PedalCurveSize
  uint16_t PedalCurveInput[16];
  uint16_t PedalCurveOutput[16]; //   64 bytes. * 5
};

// uint8_t ADCSteeringSize; // don't need size, can use 0 to terminate.

struct runtimedata_t
{
  uint32_t time;
  uint16_t maxIVTI;
  uint16_t maxMotorI[4];
};

struct eepromdatastruct
{
  union
  {
    char VersionString[10];
    uint8_t BlockStart;
  };

  uint16_t SteeringInput[10]; // HPF19 compatibility, potential future use
  int16_t SteeringOutput[10]; // min/max possible, further reaches, mid point. 40 bytes

  uint16_t BrakeRPresInput[2];  // hpf19 compatibility, potential future use.
  uint16_t BrakeRPresOutput[2]; // 8 bytes

  uint16_t BrakeFPresInput[2];  // hpf19 compatibility, potential future use. linear scale Need more
                                // if non linear.
  uint16_t BrakeFPresOutput[2]; // 8 bytes // 57 bytes.

  uint16_t BrakeTravelInput[4]; // will always map to 0-100%.. min valid, zero value, 100% val, max
                                // val. // 8 bytes.

  uint16_t TorqueReqLInput[4]; // min, zero reading, 100% reading[98%], max. // 8 bytes

  uint16_t TorqueReqRInput[4]; // min, zero reading, 100% reading[98%], max. // 8 bytes

  pedalcurve pedalcurves[5];

  // uint8_t CoolantSize
  uint16_t CoolantInput[20];
  int16_t CoolantOutput[20]; // 80 bytes.

  uint16_t DrivingModeInput[8]; //  16 bytes -- 506 bytes to here

  uint8_t AlignmentPadding[6];

  // config data start at 512 for alignment and easy writing.
  union
  {
    uint8_t MaxTorque;
    uint8_t ConfigStart;
  };

  uint8_t PedalProfile;
  bool LimpMode;
  uint8_t LimpNM;

  uint8_t TorqueVectoring;
  uint8_t TractionControl;

  bool Fans;
  uint8_t FanMax;

  bool InvEnabled;
  uint8_t EnabledMotors;

  uint8_t DrivingMode;

  uint16_t AccelRpms;
  uint16_t maxRpm;

  uint8_t regenMax;
  uint8_t regenMaxR;
  uint8_t Regen;

  uint16_t TorqueSlope;
  uint16_t DecelRpms;

  bool alwaysHV;
  int16_t steerCalib;

  bool Telemetry;

  uint8_t TorqueBal;

  uint8_t APPSBrakeLightCfg;
  uint8_t APPSBrakeHardCfg;
  uint8_t APPSBrakeReleaseCfg;
  uint8_t RTDMBrakePressureCfg;

  uint8_t MaxOutputPower;
  uint8_t MaxDrivePower;

  uint8_t ReservedConfigBytes[16];

  uint8_t Blockend;
}; // max 1600bytes=50*32byte blocks.

typedef union
{ // EEPROMU
  uint8_t buffer[4096];
  struct
  {
    char version[32]; // block 0  32 bytes
    uint8_t active;   // block 1 32 bytes
    uint8_t paddingact[31];
    union
    {
      uint8_t reserved1[32 * 8]; // blocks 2-9 256 bytes.
      runtimedata_t runtimedata;
    };
    union
    {
      uint8_t padding1[32 * 50]; // force the following structure to be aligned to start of a 50
                                 // block area.
      eepromdata block1;         // block 10-59
    };
    union
    {
      uint8_t padding2[32 * 50];
      eepromdata block2; // block 60-109
    };

    uint8_t reserved2[32 * 14]; // block 110-123  448 bytes
    uint8_t errorlogs[32 * 4];  // block 124-127  128 bytes
  };
} EEPROMdataType;

extern runtimedata_t* runtimedata_p;

// 503 - 16 blocks. allocate 50 blocks : 128 blocks total

bool GetEEPROMCmd(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData* datahandle);

bool initEEPROM(void);
bool resetEEPROM(void);
bool clearEEPROM(void);

bool checkversion(char* data);

uint8_t* getEEPROMBuffer(void);

eepromdata* getEEPROMBlock(int block);

int readEEPROMAddr(uint16_t address, uint16_t size);

void commitEEPROM(
    void); // function for timer callback to handle writing, not meant for public calling.

int writeFullEEPROM(void);
int writeFullConfigEEPROM(void);
int writeEEPROMCurConf(void);

bool writeEEPROMDone(void);

void clearRunningData(void);

int EEPROMSend(void);

bool stopEEPROM(void);
bool EEPROMBusy(void);

int DoEEPROM(void);

#endif /* EEPROM_H_ */
