/*
 * timerecu.c
 *
 *  Created on: 30 Dec 2018
 *      Author: Visa
 */

#include "ecumain.h"
#include "stm32h7xx_hal_gpio.h"
#include "output.h"
#include "input.h"
#include "i2c-lcd.h"

static volatile uint32_t stTick, timerticks;


/**
 * returns total 0.1ms since turnon to use as comparison timestamp
 */
uint32_t gettimer(void)
{
	return timerticks;
}

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{

	// TODO check why wrong systemcoreclock variable, = default 64mhz hsi.

  uint16_t volatile * const chiprevision = (uint16_t *) DEBUGMCU+1;

  if ( *chiprevision==REVV ) // revision V
	  SysTick_Config(48000);// SystemCoreClock giving wrong value?, is definitely running at right value though.

  else // revision Y
	  SysTick_Config(40000);// Right now bodging with chip detection

  /* Configure the SysTick IRQ priority */
  if (TickPriority < (1UL << __NVIC_PRIO_BITS))
  {
    HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority, 0U);
    uwTickPrio = TickPriority;
  }
  else
  {
    return HAL_ERROR;
  }

  /* Return function status */
  return HAL_OK;
}

void HAL_IncTick(void)
{
  timerticks++; // 10khz timer base.
  if ( timerticks % 10 == 0)
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
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if ( htim->Instance == TIM3 ){
//		timerticks++; // increment global time counter
		static uint8_t blinkcounter = 0;

		for ( int i = 0; i<OUTPUTCount; i++)
		{
			if ( LEDs[i].blinktime > 0 )
			{
				if ( LEDs[i].blinkingrate == 9 )
				{
					HAL_GPIO_WritePin(getGpioPort(i), getGpioPin(i), LEDON);
				} else if ( LEDs[i].blinkingrate == 0 )
				{
					HAL_GPIO_WritePin(getGpioPort(i), getGpioPin(i), LEDOFF);
				} else if ( blinkcounter % LEDs[i].blinkingrate == 0 )
				{
					toggleOutputMetal(i);
				};

				if ( LEDs[i].blinktime != 255 )
				{
					LEDs[i].blinktime--;
			//		if ( LEDs[i].blinktime == 0 ) LEDs[i].blinkingrate = 0;
				}
			} else
			{
				if ( LEDs[i].state == 9 )
					HAL_GPIO_WritePin(getGpioPort(i), getGpioPin(i), LEDON);
				else
					HAL_GPIO_WritePin(getGpioPort(i), getGpioPin(i), LEDOFF);
				LEDs[i].blinkingrate = 0;
			}
		}

		if ( ( blinkcounter == 8) )
		{
			blinkcounter = 0;
//			CAN_SendTimeBase();
			Errors.CANCount1 = 0;
			Errors.CANCount2 = 0;
		}

		blinkcounter++;
	} else if ( htim->Instance == TIM7 )
	{
		InputTimerCallback();
	} else if ( htim->Instance == TIM6 )
	{

	} else if ( htim->Instance == TIM16) // used for eeprom write in background.
	{
		commitEEPROM();
	}
#ifdef LCD
	else if ( htim->Instance == TIM17 )
	{
			hd44780_Isr(); // call interrupt handler to run next clock tick for display.
//	 		HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
	}
#endif

}



int initTimer( void )
{
	MX_TIM3_Init(); // at this point LED status should work.

	MX_TIM7_Init();

	MX_TIM6_Init();

	MX_TIM16_Init();

	if ( DeviceState.LCD == OPERATIONAL )
		lcd_send_stringscroll("Enable Interrupts");
	initInterrupts(); // start timers etc // move earlier to make display updating easier?
	return 0;
}
