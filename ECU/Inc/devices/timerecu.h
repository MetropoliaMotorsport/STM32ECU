/*
 * timerecu.h
 *
 *  Created on: 29 Dec 2018
 *      Author: Visa
 */

#ifndef TIMERECU_H_
#define TIMERECU_H_

#include "ecumain.h"
#include <time.h>

uint32_t gettimer(void);

void TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

time_t getTime( void );

int initTimer( void );

#endif /* TIMERECU_H_ */
