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

void strpad(char * str, int len);

int lcd_send_stringpos( int row, int col, char *str );
int lcd_send_stringline( int row, char *str, uint8_t priority );


void lcd_errormsg(char *str); // send an

int lcd_clearerror( void ); // clear error message and allow normal display again.

void lcd_setscrolltitle( char * str );
void lcd_clearscroll( void );
int lcd_send_stringscroll(char *str);

int lcd_send_data (char data);  // send data to the lcd

int lcd_processscroll( int direction );
void lcd_clear (void);

int lcd_geterrors( void );

int initLCD( void );


#endif /* DEVICES_LCD_H_ */
