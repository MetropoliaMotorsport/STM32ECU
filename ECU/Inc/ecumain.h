/*
 * ecumain.h
 *
 *  Created on: 14 Mar 2019
 *      Author: drago
 */

#ifndef ECUMAIN_H_
#define ECUMAIN_H_

#include <stdlib.h>
#include <string.h>
#include "main.h"

#include "wwdg.h"
#include "fdcan.h"
#include "tim.h"
#include "adc.h"

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

#include "timerecu.h"
#include "dwt_delay.h"
#include "interrupts.h"
#include "sickencoder.h"
#include "pdm.h"
#include "ivt.h"
#include "inverter.h"
#include "bms.h"
#include "adcecu.h"
#include "STRAngle.h"


int realmain(void);

#endif /* ECUMAIN_H_ */
