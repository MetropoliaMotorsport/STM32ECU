/*
 * powernode.h
 *
 *  Created on: 16 Jun 2020
 *      Author: Visa
 */

#ifndef POWERNODE_H_
#define POWERNODE_H_

#include "ecumain.h"

extern CanData PowerNodeErr;
extern CanData PowerNodeAck;

extern CanData PowerNode33; // [BOTS, inertia switch, BSPD.], Telemetry, front power
extern CanData PowerNode34;
extern CanData PowerNode35;
extern CanData PowerNode36;
extern CanData PowerNode37;

int receivePowerNodes( void );

#endif /* POWERNODE_H_ */

