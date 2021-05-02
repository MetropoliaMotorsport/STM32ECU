/*
 * lcd.c
 *
 *  Created on: Apr 27, 2021
 *      Author: Visa
 */

#include "ecumain.h"
#include "i2c-lcd.h"
#include "semphr.h"

static int lcd_updatedisplay( void );
static int lcd_printscroll( void );
static void lcd_processscroll_internal( int direction );
static void lcd_set_stringline_internal( int row, char *str, uint8_t priority );

#define ScrollLinesMax 40
#define ScrollWidth    LCDCOLUMNS

#define USELCDQUEUE

char ScrollLines[ScrollLinesMax][ScrollWidth+1] = { 0 };
int ScrollLinesPos = 0;
int ScrollLinesRingStart = 0;
int ScrollLinesTop = 0;
char ScrollTitle[ScrollWidth+1] = "";

#if defined( __ICCARM__ )
  #define DMA_BUFFER \
      _Pragma("location=\".dma_buffer\"")
#else
  #define DMA_BUFFER \
      __attribute__((section(".dma_buffer")))
#endif

DMA_BUFFER ALIGN_32BYTES (uint8_t LCDBuffer[LCDBUFSIZE]);

static uint8_t LinePriority[4] = {0};
static uint32_t LinePriorityTime[4] = {0};

osThreadId_t LCDTaskHandle;
const osThreadAttr_t LCDTask_attributes = {
  .name = "LCDTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 8
};


// setup of the LCD Data queue
#define LCDQUEUE_LENGTH    10
#define LCDITEMSIZE		sizeof( struct lcd_msg )
static StaticQueue_t LCDStaticQueue;
uint8_t LCDQueueStorageArea[ LCDQUEUE_LENGTH * LCDITEMSIZE ];

QueueHandle_t LCDQueue;

int lcd_geterrors( void )
{
	return lcd_errorcount();
}


void LCDTask(void *argument)
{
	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT( LCDQueue );

	vTaskDelay(5);

	bool inscroll = false;

    const TickType_t xFrequency = 20;

	struct lcd_msg msg;

	while ( 1 )
	{
		while ( uxQueueMessagesWaiting( LCDQueue ) )
		{
        // UBaseType_t uxQueueMessagesWaiting( QueueHandle_t xQueue );
			xQueueReceive(LCDQueue,&msg,0);

			int copylen = 0;
			switch ( msg.type )
			{
				case LCD_Txt :
			    	copylen = strlen(msg.string);

			    	if ( msg.col + copylen > LCDCOLUMNS )
			    		copylen = LCDCOLUMNS-copylen; // ensure don't print off screen.

			    	memcpy(&LCDBuffer[msg.col+msg.row*LCDCOLUMNS], msg.string, copylen); // copy string into buffer
			    	break;

				case LCD_Txt_Line :
					lcd_set_stringline_internal( msg.row, msg.string, msg.priority );
					break;

				case LCD_Clear :
					for ( int i=0;i<sizeof(LCDBuffer);i++){ 	// clear scroll buffer.
						LCDBuffer[i] = ' ';
					}
					break;

					//scrolling

				case LCD_Scroll_Start :
					// start and reset to Default
					inscroll = true;
					ScrollLinesPos = 0;
					ScrollLinesTop = 0;
					ScrollTitle[0] = 0;
					break;

				case LCD_Scroll_End :
					inscroll = false;
					break;

				case LCD_Scroll_Clear :

					ScrollLinesPos = 0;
					ScrollLinesTop = 0;

				//	char *lines = (char*)ScrollLines;

					for ( int i=0;i<ScrollLinesMax;i++){ 	// clear scroll buffer.
						for ( int j=0;j<LCDCOLUMNS;j++){
							ScrollLines[i][j] = 32;
						}
					}
					break;

				case LCD_Scroll_Title :
					inscroll = true;
					copylen = strlen(msg.string);
					memcpy(ScrollTitle, msg.string, copylen);
					strpad(ScrollTitle, LCDCOLUMNS, false);
					break;


				case LCD_Scroll_Add :
				{
					inscroll = true;

					int scroll = 0;

					if( ScrollLinesPos < ScrollLinesMax ){
						memcpy(&ScrollLines[ScrollLinesPos][0],msg.string,LCDCOLUMNS); // copy string into lines
						ScrollLinesPos++;
						if ( ScrollLinesTop < ScrollLinesPos-3)
							scroll = 1;

					} else
					{
						memmove(&ScrollLines[0][0],&ScrollLines[1][0],ScrollWidth+1*(ScrollLinesMax-1)); // TODO shunt lines back. Should reimplement as ring buffer?
						memcpy(&ScrollLines[ScrollLinesPos-1][0],msg.string,LCDCOLUMNS); // copy string into lines
					}

					lcd_processscroll_internal( scroll );
				}
				break;

				case LCD_Scroll_Up:
				case LCD_Scroll_Down:
					if ( msg.type == LCD_Scroll_Up)
						lcd_processscroll_internal( -1 );
					else
						lcd_processscroll_internal( 1 );
					break;

			}


		}

		if ( inscroll )
		{

			if ( LinePriority[0] == 255 ) // only allow update if not priority overridden
			{
				lcd_set_stringline_internal(0, ScrollTitle, 255);
			}


			lcd_printscroll();
		}

		lcd_updatedisplay();

		vTaskDelay(xFrequency);

	//	vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}

	// shouldn't get here, but terminate thread cleanly if do.
	osThreadTerminate(NULL);
}

void strpad(char * str, int len, bool adddots){
	int strlength = strlen(str);
	if ( strlength > LCDCOLUMNS )
	{
		if ( adddots )
		{
			str[LCDCOLUMNS-2] = '.'; str[LCDCOLUMNS-1] = '.';
		}
		str[LCDCOLUMNS] = 0;
	} else
	{ // pad string out to clear rest of line if shorter.
		for (int i=strlength;i<LCDCOLUMNS;i++) str[i] = 32;
		str[LCDCOLUMNS] = 0;
	};
}


static int lcd_updatedisplay( void ) // batch send buffered LCD commands
{
/*	if ( lcd_getstate() )
	{
		return 1;
	}

*/

	// for every update, check if line display timeout has expired and reset.
	for ( int row=0;row<LCDROWS;row++)
	{
		if ( LinePriorityTime[row] < gettimer() )
		{
			 LinePriority[row]=255;
		}
	}

	int result = lcd_dosend();
	return result;
}



int lcd_send_stringline( int row, char *str, uint8_t priority )
{
	struct lcd_msg msg;

	msg.type = LCD_Txt_Line;
	msg.row = row;
	msg.priority = priority;
	strncpy( msg.string, str, LCDCOLUMNS+1 );
	return xQueueSendToBack(LCDQueue,&msg,0);
}


static void lcd_set_stringline_internal( int row, char *str, uint8_t priority )
{
	if ( priority <= LinePriority[row]) // only allow update if not priority overridden
	{
		strpad(str, LCDCOLUMNS, false);
    	memcpy(&LCDBuffer[row*LCDCOLUMNS], str, LCDCOLUMNS); // copy string into buffer

		LinePriorityTime[row] = gettimer()+2000; // show for at least 200ms
		LinePriority[row]=priority;
	}

}



int lcd_settitle( char *str )
{
	struct lcd_msg msg;

	msg.type = LCD_Txt_Line;
	msg.row = 0;
	msg.priority = 255;
	strncpy( msg.string, str, LCDCOLUMNS+1 );
	return xQueueSendToBack(LCDQueue,&msg,0);
}


int lcd_send_stringpos( int row, int col, char *str, uint8_t priority )
{
	struct lcd_msg msg;

	msg.type = LCD_Txt;
	msg.col = col;
	msg.row = row;
	msg.priority = priority;
	strncpy( msg.string, str, LCDCOLUMNS+1 );
	return xQueueSendToBack(LCDQueue,&msg,0);
}


int lcd_setscrolltitle( char * str )
{
	struct lcd_msg msg;

	msg.type = LCD_Scroll_Title;
	strncpy( msg.string, str, LCDCOLUMNS+1 );
	return xQueueSendToBack(LCDQueue,&msg,0);
}


int lcd_clearscroll( void )
{

	struct lcd_msg msg;
	msg.type = LCD_Scroll_Clear;
	return xQueueSendToBack(LCDQueue,&msg,0);
}


int lcd_clear( void )
{

	struct lcd_msg msg;
	msg.type = LCD_Clear;
	return xQueueSendToBack(LCDQueue,&msg,0);
}



static int lcd_printscroll( void )
{
	for ( int i=0;i<LCDCOLUMNS*3;i++){ 	// clear bottom of display buffer.
		LCDBuffer[LCDCOLUMNS+i] = 32;
	}

	for ( int i=0;i<3;i++){
		if ( LinePriority[i] == 255 ) // only allow update if not priority overridden
		{
			if ( i+ScrollLinesTop <= ScrollLinesPos )
				lcd_set_stringline_internal(1+i, &ScrollLines[i+ScrollLinesTop][0], 255);
		}
	}

	if ( ScrollLinesTop > 0 ) LCDBuffer[LCDCOLUMNS+LCDCOLUMNS-1] = 18; // up arrow

	if ( ScrollLinesTop < ScrollLinesPos-3 ) LCDBuffer[LCDCOLUMNS*3+LCDCOLUMNS-1] = 19; // down arrow

	return 1;
}


int lcd_processscroll( int direction )
{
	struct lcd_msg msg;

	if ( direction)
		msg.type = LCD_Scroll_Up;
	else
		msg.type = LCD_Scroll_Down;
	return xQueueSendToBack(LCDQueue,&msg,0);
}

static void lcd_processscroll_internal( int direction )
{
	ScrollLinesTop+= direction;

	if ( ScrollLinesTop < 0 ) ScrollLinesTop = 0;
	else if ( ScrollLinesTop > ScrollLinesMax-1 ) ScrollLinesTop = ScrollLinesMax-1;
}


int lcd_send_stringscroll(char *str)
{
	struct lcd_msg msg;

	msg.type = LCD_Scroll_Add;
	strncpy( msg.string, str, LCDCOLUMNS+1 );
	return xQueueSendToBack(LCDQueue,&msg,0);
}

int lcd_startscroll( void )
{
	struct lcd_msg msg;
	msg.type = LCD_Scroll_Start;
	return xQueueSendToBack(LCDQueue,&msg,0);
}

int lcd_endscroll( void )
{
	struct lcd_msg msg;
	msg.type = LCD_Scroll_End;
	return xQueueSendToBack(LCDQueue,&msg,0);
}


void lcd_errormsg(char *str)
{
	lcd_send_stringposDIR( 0, 0,  str );
}


int initLCD( void )
{
	LCDQueue = xQueueCreateStatic( LCDQUEUE_LENGTH,
							  LCDITEMSIZE,
							  LCDQueueStorageArea,
							  &LCDStaticQueue );

	vQueueAddToRegistry(LCDQueue, "LCDQueue" );
#ifdef SCREEN

	MX_I2C3_Init();

	for ( int i=0;i<LCDBUFSIZE;i++){ 	// clear display buffer.
		LCDBuffer[i] = 32;
	}

	for ( int i=0;i<LCDROWS;i++)
		LinePriority[i] = 255;

	if ( !lcd_init(&hi2c3) ){
		DeviceState.LCD = DISABLED;
	} else
	{
		lcd_clear();
		lcd_send_stringposDIR(0,0,"LCD Init.");
	}

	LCDTaskHandle = osThreadNew(LCDTask, NULL, &LCDTask_attributes);
#endif
	return 0;
}
