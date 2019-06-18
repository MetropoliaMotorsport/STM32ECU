/*
 * vhd44780.c
 *
 *  Created on: 5 Mar 2019
 *      Author: visa
 *
 *      call hd44780_Init() once before starting timer to init display
 *      then call hd44780_isr from timer interrupt, will constantly push buffer to screen.
 *
 *      Add screen functions, clear, write at pos x, y, etc.
 */
#ifdef LCD

#include <stdio.h>
#include <string.h>

#include "main.h"
#include "vhd44780.h"

#define HD44780_NR_OF_ROWS                2
#define HD44780_NR_OF_COLUMNS             16
#define HD44780_DATA_BUFFER_SIZE          HD44780_NR_OF_ROWS*HD44780_NR_OF_COLUMNS

volatile static uint8_t hd44780_data_buffer[HD44780_DATA_BUFFER_SIZE];

// LCD stuff

 // functions used for init
 static void lcd_clock_pulse(void) // toggle e to get display to read data
 {
 	HAL_Delay(1);
 	HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, 0);
 	HAL_Delay(1);
 	HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, 1);
 }

 static void write8_4bitmode( char command, char rs_value)
 {
	  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, rs_value);

	  HAL_Delay(1);

	  HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (command >> 4) & 1);
	  HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (command >> 5) & 1);
	  HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (command >> 6) & 1);
	  HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (command >> 7) & 1);

      lcd_clock_pulse();  // toggles e to read high data.

	  HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (command >> 0) & 1);
	  HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (command >> 1) & 1);
	  HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (command >> 2) & 1);
	  HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (command >> 3) & 1);

	  lcd_clock_pulse();  // toggles e to read low data.

 }

 void hd44780_Clear(void)
 {
   for ( int i = 0; i < HD44780_DATA_BUFFER_SIZE; i++ ) { hd44780_data_buffer[0]; }
 }

 void hd44780_writexy(uint8_t data, uint8_t x, uint8_t y)
 {

 }

 void hd44780_writeline1(char data[])
 {
	int i;
	i = strlen(data);
    if ( i > 16 ) { i = 16; }
    for ( int j = 0; j <= i; j++ ) { hd44780_data_buffer[j] = data[j]; };
 }

 void hd44780_writeline2(char data[])
  {
	int i;
	i = strlen(data);
	if ( i > 16 ) { i = 16; }
	for ( int j = 0; j <= i; j++ ) { hd44780_data_buffer[j+16] = data[j]; };
  }

 void hd44780_integerWrite(int data, int x, int y)
 {
 	int divisor = 10000000;

 	while(divisor){
 		hd44780_dataWrite('0' + ((data/divisor) % 10));
 		divisor /= 10;
 	}
 }

 void hd44780_Init(void)
 {
 	/* Initialize display */
 	lcd_clock_pulse();
 	HAL_Delay(1);
 	write8_4bitmode(0b00110011,0);
 	HAL_Delay(1);
 	write8_4bitmode(0b00110010,0); //  send reset
 	HAL_Delay(1);
 	write8_4bitmode(0b00101100,0); //  init two line display
 	HAL_Delay(1);
 	write8_4bitmode(0b00001100,0); // no blink or cursor.
 	HAL_Delay(1);
 	write8_4bitmode(0b00000001,0);
 	HAL_Delay(1);
 	write8_4bitmode(0b00000110,0);
 	HAL_Delay(1);

 	// write an init message to allow visual confirm initialised successfully.

	write8_4bitmode('S',1);
	write8_4bitmode('T',1);
	write8_4bitmode('M',1);
	write8_4bitmode(' ',1);
	write8_4bitmode('H',1);
	write8_4bitmode('7',1);
	write8_4bitmode('4',1);
	write8_4bitmode('3',1);
 	write8_4bitmode('Z',1);
 	write8_4bitmode('I',1);
	write8_4bitmode(0x80,0); // home cursor.
	HAL_Delay(1);
 }

 void hd44780_Isr(void)
 {
   static uint8_t state;
   static uint8_t column;
   static uint8_t row;
   static uint8_t rsstate = 0;
   static uint8_t command = 0x80;
   switch( state )  //  first loop will be sending go home.
   {
   	   case 0 : // set e low, write high data
   		  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, rsstate);
   		  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, 0);

   		  HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (command >> 4) & 1);
   		  HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (command >> 5) & 1);
   		  HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (command >> 6) & 1);
   		  HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (command >> 7) & 1);
   		  state = 1;
	   	  break;
   	   case 1 : // set e high to read data
   		   HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, 1);
   		   state = 2;
	   	   break;
   	   case 2 :  // set e low, write low data
   		   HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, 0);
   		   HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (command >> 0) & 1);
   		   HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (command >> 1) & 1);
   		   HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (command >> 2) & 1);
   		   HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (command >> 3) & 1);
   		   state = 3;
	   	   break;
   	   case 3 :  // set e high to read data, fetch next character/command.
   		   HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, 1);
   		   rsstate = 1;
   		   column++;
   		   if ( column > HD44780_NR_OF_COLUMNS ) {
   			   column = 0;
   			   row++;
   			   if ( row > HD44780_NR_OF_ROWS-1 ){
   				   row = 0;
   			   }
			   command = 0x80 | ( 0x40 * row);
			   rsstate = 0;
   		   } else
   		   {
   	   		   command = hd44780_data_buffer[column-1 + (16 * row)];
   	   		   if ( command == 0 ) command = 32;
			   rsstate = 1;
   		   }
   		   state = 0;
   		   break;
   }
 }

#endif
