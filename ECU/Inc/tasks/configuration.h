#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "canecu.h"
#include <stdbool.h>
#include <stdint.h>
//  conversion buffer, should be aligned in memory for faster DMA?
typedef struct
{
  uint8_t top;
  uint8_t menusize;
  uint8_t selection;
  bool inedit;
} menustruct_t;

#define ConfigInputQUEUE_LENGTH 2
#define ConfigInputITEMSIZE sizeof(uint32_t)
#define ConfigSTACK_SIZE 128 * 8
#define ConfigTASKNAME "ConfigTask"

#define MENU_NM (1)
#define MENU_NMBAL (2)
#define MENU_TORQUE (3)
#define MENU_RPM (4)
#define MENU_ACCEL (5)
#define MENU_LIMPDIS (6)
#define MENU_FANS (7)
#define MENU_FANMAX (8)
#define MENU_CALIB (9)
#define MENU_STEERING (10)
#define MENU_INV (11)
#define MENU_REGEN (12)
#define MENU_REGENMAX (13)
#define MENU_REGENMAXR (14)
#define MENU_TELEMETRY (15)
#define MENU_HV (16)

#define MENU_ENABLED_MOTORS (17)
#define MENU_LIMPNM (18)
#define MENU_PEDAL_PROFILE (19)
#define MENU_DRIVING_MODE (20)

#define MENU_APPS_BRAKE_LIGHT (21)
#define MENU_APPS_BRAKE_HARD (22)
#define MENU_APPS_BRAKE_RELEASE (23)
#define MENU_RTDM_BRAKE_PRESSURE (24)

#define MENU_TV_ENABLE (25)
#define MENU_TC_ENABLE (26)

#define MENU_TORQUE_SLOPE (27)

#define MENU_POWER_BALANCE (28)
#define MENU_MAX_POWER (29)
#define MENU_MAX_DRIVE_POWER (30)

#define MENU_LAST (MENU_MAX_DRIVE_POWER)
#define MAINMENUSIZE (MENU_LAST + 1)

#define CAN_MENU_SET_VALUE 1
#define CAN_MENU_SAVE 2
#define CAN_MENU_FULLSAVE 3
#define CAN_MENU_APPLY 4
#define CAN_MENU_RESET 5

bool GetConfigCmd(const uint8_t CANRxData[8], const uint32_t DataLength, CANData* datahandle);
bool checkConfigReset(void);
void ConfigReset(void);
bool initConfig(void);

#endif
