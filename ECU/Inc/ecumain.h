/*
 * ecumain.h
 *
 *  Created on: 14 Mar 2019
 *      Author: drago
 */

#ifndef ECUMAIN_H_
#define ECUMAIN_H_

#include "main.h"
#include "ecu.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "stm32h7xx_hal.h"

#include "event_groups.h"

#include "canecu.h"

extern CANData ECUCAN;

extern volatile CarStateType CarState;

extern volatile DeviceStateType DeviceState;

extern EventGroupHandle_t xStartupSync;
extern EventGroupHandle_t xCycleSync;

typedef void (ResetCommand)( void );

int RegisterResetCommand( ResetCommand Handler );

/*
#include "output.h"
#include "input.h"

#include "preoperation.h"
#include "operationreadyness.h"
#include "operationalprocess.h"
#include "idleprocess.h"
#include "tsactiveprocess.h"
#include "runningprocess.h"
#include "configuration.h"
#include "errors.h"

#include "timerecu.h"
#include "dwt_delay.h"
#include "interrupts.h"
#include "sickencoder.h"
#include "pdm.h"
#include "ivt.h"
#include "inverter.h"
#include "bms.h"
#include "adcecu.h"
#include "lcd.h"
#include "eeprom.h"
#include "memorator.h"
#include "power.h"
#include "analognode.h"
#include "powernode.h"
#include "node.h"
#include "imu.h"
#include "torquecontrol.h"
#include "watchdog.h"
#include "brake.h"
*/

int realmain(void);

void configureTimerForRunTimeStats(void);

unsigned long getRunTimeCounterValue(void);

#endif /* ECUMAIN_H_ */
