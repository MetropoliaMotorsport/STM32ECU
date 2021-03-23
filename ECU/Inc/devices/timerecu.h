/*
 * timerecu.h
 *
 *  Created on: 29 Dec 2018
 *      Author: Visa
 */

#ifndef TIMERECU_H_
#define TIMERECU_H_

uint32_t gettimer(void);
#ifdef RTOS
void TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
#endif

int initTimer( void );

#endif /* TIMERECU_H_ */
