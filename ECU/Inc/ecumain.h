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

#if defined( __ICCARM__ )
  #define DMA_BUFFER \
      _Pragma("location=\".dma_buffer\"")
#else
  #define DMA_BUFFER \
      __attribute__((section(".dma_buffer")))
#endif

#if defined( __ICCARM__ )
  #define RAM_D1 \
      _Pragma("location=\".dma_buffer\"")
#else
  #define RAM_D1 \
      __attribute__((section(".dma_buffer")))
#endif

extern CANData ECUCAN;

extern volatile CarStateType CarState;

extern volatile DeviceStateType DeviceState;

extern EventGroupHandle_t xStartupSync;
extern EventGroupHandle_t xCycleSync;

typedef void (ResetCommand)( void );

int RegisterResetCommand( ResetCommand Handler );

int realmain(void);

void configureTimerForRunTimeStats(void);

unsigned long getRunTimeCounterValue(void);

#endif /* ECUMAIN_H_ */
