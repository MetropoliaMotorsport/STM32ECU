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
#include "main.h"

#ifdef HPF19
	#include "wwdg.h"
#endif
#include "fdcan.h"
#include "tim.h"
#include "adc.h"
#include "comp.h"

#include "ecu.h"

#include "output.h"
#include "input.h"
#include "canecu.h"

#include "preoperation.h"
#include "operationreadyness.h"
#include "operationalprocess.h"
#include "idleprocess.h"
#include "tsactiveprocess.h"
#include "runningprocess.h"
#include "configuration.h"

#include "timerecu.h"
#include "dwt_delay.h"
#include "interrupts.h"
#include "sickencoder.h"
#include "pdm.h"
#include "ivt.h"
#include "inverter.h"
#include "bms.h"
#include "adcecu.h"
#include "i2c-lcd.h"
#include "eeprom.h"
#include "memorator.h"
#include "powernode.h"


int realmain(void);

#endif /* ECUMAIN_H_ */
