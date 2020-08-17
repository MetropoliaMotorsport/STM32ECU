/*
 * i2c-lcd.h
 *
 *  Created on: 19 May 2020
 *      Author: Visa
 */

#ifndef I2CLCD_H_
#define I2CLCD_H_

#include "stm32h7xx_hal.h"

#define LCDCOLUMNS (20)
#define LCDROWS 	(4)

enum lcd_msg_type { LCD_Title, LCD_Line, LCD_Scroll, LCD_Cmd };

enum lcd_cmd_type { Clear, ScrollUp, ScrollDown };


struct lcd_msg {

  enum lcd_msg_type type;

  union {


    char stringptr[41];

    enum lcd_cmd_type cmd;

    uint32_t count;

  } data;

  uint8_t line;

  uint8_t priority;

};

extern QueueHandle_t LCDQueue;

#define LCDBUFSIZE (LCDCOLUMNS*LCDROWS)


int lcd_update( void );
int lcd_updatedisplay( void );

void strpad(char * str, int len);

void lcd_errormsg(char *str); // send an

int lcd_clearerror( void ); // clear error message and allow normal display again.

int lcd_init (I2C_HandleTypeDef *i2chandle);   // initialize lcd

int lcd_send_cmd (char cmd);  // send command to the lcd

void lcd_setscrolltitle( char * str );
void lcd_clearscroll( void );
int lcd_send_stringscroll(char *str);

int lcd_send_data (char data);  // send data to the lcd

int lcd_processscroll( int direction );

int lcd_send_stringpos( int row, int col, char *str );
int lcd_send_stringline( int row, char *str, uint8_t priority );

int lcd_send_stringposDIR( int row, int col, char *str ); // send string straight to screen using polling.

void lcd_clear (void);

void LCD_I2CError( void );

int initLCD( void );

#endif /* I2CLCD_H */
