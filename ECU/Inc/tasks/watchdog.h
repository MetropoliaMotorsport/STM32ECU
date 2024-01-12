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

// register a bit to be kicked.
uint8_t registerWatchdogBit( char * taskname );
int initWatchdog( void );

#endif /* WATCHDOG_H_ */

