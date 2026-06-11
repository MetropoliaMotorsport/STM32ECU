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

bool GetConfigCmd(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData* datahandle);

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
#define MENU_INVEN (11)
#define MENU_REGEN (12)
#define MENU_REGENMAX (13)
#define MENU_REGENMAXR (14)
#define MENU_TELEMETRY (15)
#define MENU_HV (16)
#define MENU_LAST (MENU_HV)
#define MAINMENUSIZE (MENU_LAST + 1)

#define CAN_MENU_SET_VALUE 1
#define CAN_MENU_SAVE 2
#define CAN_MENU_FULLSAVE 3
#define CAN_MENU_APPLY 4
#define CAN_MENU_RESET 5

#endif
