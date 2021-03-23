/*
 * vhd44780.c
 *
 *  Created on: 5 Mar 2019
 *      Author: Visa
 *
 *
 */

#ifndef VHD44780_C_
#define VHD44780_C_

#include "main.h"


void hd44780_Init(void);
void hd44780_Isr(void);

void hd44780_integerWrite(int data, int x, int y);

void hd44780_writeline1(char data[]);

#endif /* VHD44780_C_ */
