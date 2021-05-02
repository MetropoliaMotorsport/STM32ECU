/*
 * ecumain.h
 *
 *  Created on: 14 Mar 2019
 *      Author: drago
 */

#ifndef ECUMAIN_H_
#define ECUMAIN_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "main.h"

#define WATCHDOG

#ifdef HPF19
	#include "wwdg.h"
#endif
#include "fdcan.h"
#include "tim.h"
#include "dma.h"
#include "i2c.h"
#include "adc.h"
#include "comp.h"

#include "ecu.h"

#ifdef RTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "cmsis_os.h"
#endif


#include "canecu.h"

extern CANData ECUCAN;

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


int realmain(void);

void configureTimerForRunTimeStats(void);

unsigned long getRunTimeCounterValue(void);

#endif /* ECUMAIN_H_ */
