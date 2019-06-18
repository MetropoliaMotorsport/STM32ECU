/*
 * vhd44780.c
 *
 *  Created on: 5 Mar 2019
 *      Author: visa
 *
 *  Very basic LCD Driver for HD44780 for use with stm32.
 *  Define pins LCD_E, LCD_RS, LCD_D4-D7 as gpio
 *  call hd44780_Init to initialise LCD, then call
 *  hd44780_Isr from a timer to continually refresh.
 *
 */

#ifndef VHD44780_C_
#define VHD44780_C_

void hd44780_Init(void);
void hd44780_Isr(void);

void hd44780_integerWrite(int data, int x, int y);

void hd44780_writeline1(char data[]);

#endif /* VHD44780_C_ */
