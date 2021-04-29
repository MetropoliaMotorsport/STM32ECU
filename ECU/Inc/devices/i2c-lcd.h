/*
 * i2c-lcd.h
 *
 *  Created on: 19 May 2020
 *      Author: Visa
 */

#ifndef I2CLCD_H_
#define I2CLCD_H_

#include "stm32h7xx_hal.h"

//int lcd_updatedisplay( void );


int lcd_init (I2C_HandleTypeDef *i2chandle);   // initialize lcd

int lcd_send_stringposDIR( int row, int col, char *str ); // send string straight to screen using polling.

void LCD_I2CError( void );

int lcd_getstate( void );
void lcd_resetbuffer();
int lcd_dosend( void );

int lcd_errorcount( void );

#endif /* I2CLCD_H */
