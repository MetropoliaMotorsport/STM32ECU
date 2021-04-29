/*
 * timerecu.h
 *
 *  Created on: 29 Dec 2018
 *      Author: Visa
 */

#ifndef TIMERECU_H_
#define TIMERECU_H_

#include "ecumain.h"

uint32_t gettimer(void);
#ifdef RTOS
void TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
#endif

bool isRTCSet( void );
int setRTC( time_t time );

char * getCurTimeStr( void );
char * getTimeStr( time_t time );

time_t getTime( void );

int initRTC( void );

int initTimer( void );

#endif /* TIMERECU_H_ */
