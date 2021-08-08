/*
 * timerecu.c
 *
 *  Created on: 30 Dec 2018
 *      Author: Visa
 */

#include "ecumain.h"
#include "timerecu.h"
#include "eeprom.h"
#include "memorator.h"
#include "interrupts.h"
//#include "stm32h7xx_hal_gpio.h"
#include "i2c-lcd.h"
#include "lcd.h"
#include <time.h>
#include "tim.h"

static volatile uint32_t stTick, timerticks;

time_t rtctime;

/**
 * returns total 0.1ms since turnon to use as comparison timestamp
 */
uint32_t gettimer(void)
{
	return stTick;
}

int setRTC( time_t time )
{
	rtctime = time;
	return 1;
}

bool isRTCSet( void )
{
	if ( rtctime == 0 )
		return true;
	else
		return false;
}

void HAL_IncTick(void)
{
//  timerticks++; // 10khz timer base.
//  if ( timerticks % 10 == 0)
  {
	  stTick++;
	  if ( stTick % 1000 == 0 && rtctime != 0 ) rtctime++;
  }
}

uint32_t HAL_GetTick(void)
{
  return stTick;
}


/**
 * @brief IRQ handler for timer3 used to keep timebase.
 */
void TIM3_IRQHandler()
{
    HAL_TIM_IRQHandler(&htim3);
}

void TIM6_IRQHandler()
{
    HAL_TIM_IRQHandler(&htim6);
}


/**
 * @brief timer interrupt to keep a timebase and process button debouncing.
 */
void TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM17) {
		HAL_IncTick();
	} else if ( htim->Instance == TIM6 )
	{

	} else if ( htim->Instance == TIM16) // used for eeprom write in background.
	{
		commitEEPROM();
	}
}


char * getCurTimeStr( void )
{
//	static char timestr[9] = "00:00:00";
//	static time_t lasttime = 0;
	if ( rtctime != 0 )
	{
//		if (lasttime != rtctime )
		{
			return getTimeStr( rtctime );
		}
	} else
	{
		return getTimeStr( gettimer()/1000 );
		//sprintf(timestr,"%.7lis", gettimer()/1000);
	}
}

char * getTimeStr( time_t time )
{
	static char timestr[9] = "00:00:00";

	if ( rtctime != 0 )
	{
		struct tm curtime;
		curtime = *localtime(&time);
		strftime(timestr, sizeof(timestr), "%H:%M:%S", &curtime);
	} else
	{
		sprintf(timestr,"%.7lis", (uint32_t)time);
	}
	return timestr;
}

time_t getTime( void )
{
	if ( rtctime != 0 )
	{
		return rtctime;
	} else
		return gettimer()/1000;
}

int initRTC( void )
{
	rtctime = 0;
	return initMemorator();
}


int initTimer( void )
{
	MX_TIM7_Init();

	MX_TIM6_Init();

	MX_TIM16_Init();

	if ( DeviceState.LCD == OPERATIONAL )
		lcd_send_stringscroll("Enable Interrupts");
	initInterrupts(); // start timers etc // move earlier to make display updating easier?
	return 0;
}
