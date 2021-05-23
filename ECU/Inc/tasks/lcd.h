/*
 * lcd.h
 *
 *  Created on: 27 Apr 2021
 *      Author: visa
 */

#ifndef DEVICES_LCD_H_
#define DEVICES_LCD_H_


#define LCDCOLUMNS (20)
#define LCDROWS 	(4)

enum lcd_msg_type { LCD_Txt, LCD_Clear, LCD_Txt_Line, LCD_Scroll_Start, LCD_Scroll_End, LCD_Scroll_Clear, LCD_Scroll_Up, LCD_Scroll_Down, LCD_Scroll_Title, LCD_Scroll_Add };

struct lcd_msg {

  enum lcd_msg_type type;

  char string[LCDCOLUMNS+1];

  uint8_t row;

  uint8_t col;

  uint8_t priority;
};

extern QueueHandle_t LCDQueue;

#define LCDBUFSIZE (LCDCOLUMNS*LCDROWS)


void strpad(char * str, int len, bool adddots);

int lcd_send_stringpos( int row, int col, char *str, uint8_t priority );
int lcd_send_stringline( int row, char *str, uint8_t priority );


void lcd_errormsg(char *str); // send an

int lcd_clearerror( void ); // clear error message and allow normal display again.

int lcd_startscroll( void );
int lcd_setscrolltitle( char * str );
int lcd_clearscroll( void );
int lcd_send_stringscroll(char *str);
int lcd_endscroll( void );

int lcd_send_data (char data);  // send data to the lcd

int lcd_settitle( char *str );

int lcd_processscroll( int direction );
int lcd_clear (void);

int lcd_geterrors( void );

int initLCD( void );


#endif /* DEVICES_LCD_H_ */
