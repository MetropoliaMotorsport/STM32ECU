/*
 * watchdog.h
 *
 *  Created on: 30 Jul 2020
 *      Author: Visa
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

bool watchdogRebooted( void );
void setWatchdogBit(uint8_t bit); // kick the watchdog.
void registerWatchdogBit(uint8_t bit); // register a bit to be kicked.
int initWatchdog( void );

#endif /* WATCHDOG_H_ */

