/*
 * lcd.c
 *
 *  Created on: Apr 27, 2021
 *      Author: Visa
 */

#include "ecumain.h"
#include "i2c-lcd.h"

int lcd_updatedisplay( void );

#define ScrollLinesMax 40

char ScrollLines[ScrollLinesMax][21] = { 0 };
int ScrollLinesPos = 0;
int ScrollLinesRingStart = 0;
int ScrollLinesTop = 0;
char ScrollTitle[21] = "";

volatile static bool inerror = false;

SemaphoreHandle_t xLCDBuffer;

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

/* The queue is to be created to hold a maximum of 10 uint64_t
variables. */
#define LCDQUEUE_LENGTH    10
#define LCDITEMSIZE		sizeof( struct lcd_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t LCDStaticQueue;


/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t LCDQueueStorageArea[ LCDQUEUE_LENGTH * LCDITEMSIZE ];

QueueHandle_t LCDQueue;

int lcd_geterrors( void )
{
	return i2clcderrorcount();
}


void LCDTask(void *argument)
{
	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT( LCDQueue );

	vTaskDelay(5);

    const TickType_t xFrequency = 20;

	struct lcd_msg msg;

	while ( 1 )
	{
		while ( uxQueueMessagesWaiting( LCDQueue ) )
		{
        // UBaseType_t uxQueueMessagesWaiting( QueueHandle_t xQueue );
			xQueueReceive(LCDQueue,&msg,0);

			char str[LCDCOLUMNS+1] = "";

			sprintf(str,"Count: %.10lu", msg.data.count );

			lcd_send_stringline(3,str, 0);
	//		vTaskYield();

		}

		lcd_updatedisplay();

		vTaskDelay(xFrequency);

	//	vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}

	// shouldn't get here, but terminate thread cleanly if do.
	osThreadTerminate(NULL);
}

void strpad(char * str, int len){
	int strlength = strlen(str);
	if ( strlength > LCDCOLUMNS )
	{
		str[LCDCOLUMNS-2] = '.'; str[LCDCOLUMNS-1] = '.'; str[LCDCOLUMNS] = 0;
	} else
	{ // pad string out to clear rest of line if shorter.
		for (int i=strlength;i<LCDCOLUMNS;i++) str[i] = 32;
		str[LCDCOLUMNS] = 0;
	};
}


int lcd_updatedisplay( void ) // batch send buffered LCD commands
{
	if ( lcd_getstate() )
	{
		return 1;
	}


	// for every update, check if line display timeout has expired and reset.
	for ( int row=0;row<LCDROWS;row++)
	{
		if ( LinePriorityTime[row] < gettimer() )
		{
			 LinePriority[row]=255;
		}
	}

   if( xSemaphoreTake( xLCDBuffer, ( TickType_t ) 10 ) == pdTRUE )
   {
	   return lcd_dosend();
	   xSemamphoreGive( xLCDBuffer );
   } else
	   return 0;
}



int lcd_send_stringline( int row, char *str, uint8_t priority )
{
	if ( priority <= LinePriority[row]) // only allow update if not priority overridden
	{
		char line[21] = "                    ";

		int copylen = strlen(str);

		if ( copylen > LCDCOLUMNS )
			copylen = LCDCOLUMNS; // ensure don't print off screen.

		memcpy(line, str, copylen); // copy string into

		lcd_send_stringpos( row, 0, line );
		LinePriorityTime[row] = gettimer()+2000; // show for at least 200ms
		LinePriority[row]=priority;
		return 0;
	} else return 1; // no update allowed.
}


int lcd_send_stringpos( int row, int col, char *str )
{
    if( xSemaphoreTake( xLCDBuffer, ( TickType_t ) 10 ) == pdTRUE )
    {
    	int copylen = strlen(str);

    	if ( col + copylen > LCDCOLUMNS )
    		copylen = LCDCOLUMNS-copylen; // ensure don't print off screen.

    	memcpy(&LCDBuffer[col+row*LCDCOLUMNS], str, copylen); // copy string into buffer

        xSemaphoreGive( xSemaphore );
        return 1;
    }
    else
    {
    	return 0;
        /* We could not obtain the semaphore and can therefore not access
        the shared resource safely. */
    }
}


void lcd_setscrolltitle( char * str )
{
	for ( int i=0;i<LCDCOLUMNS;i++)
		ScrollTitle[i] = 32;
	ScrollTitle[LCDCOLUMNS] = 0;

	int copylen = strlen(str);
	if ( copylen > LCDCOLUMNS ) copylen = LCDCOLUMNS;

	memcpy(ScrollTitle, str, copylen);

	if ( LinePriority[0] == 255 ) // only allow update if not priority overridden
	{

		lcd_send_stringpos( 0, 0, ScrollTitle );
	}

}


void lcd_clearscroll( void )
{
	ScrollLinesPos = 0;
	ScrollLinesTop = 0;

//	char *lines = (char*)ScrollLines;

	for ( int i=0;i<ScrollLinesMax;i++){ 	// clear bottom of display buffer.
		for ( int j=0;j<LCDCOLUMNS;j++){
			ScrollLines[i][j] = 32;
		}
	}

	for ( int i=0;i<3;i++){
		if ( LinePriority[i] == 255 ) // only allow update if not priority overridden
		{
			lcd_send_stringpos( 1+i, 0, &ScrollLines[i][0] );
		}
	}
}


int lcd_printscroll( void )
{
    if( xSemaphoreTake( xLCDBuffer, ( TickType_t ) 10 ) == pdTRUE )
    {

		for ( int i=0;i<LCDCOLUMNS*3;i++){ 	// clear bottom of display buffer.
			LCDBuffer[LCDCOLUMNS+i] = 32;
		}

		for ( int i=0;i<3;i++){
			if ( LinePriority[i] == 255 ) // only allow update if not priority overridden
			{
				if ( i+ScrollLinesTop <= ScrollLinesPos )
					lcd_send_stringpos( 1+i, 0, &ScrollLines[i+ScrollLinesTop][0] );
			}
		}

		if ( ScrollLinesTop > 0 ) LCDBuffer[LCDCOLUMNS+LCDCOLUMNS-1] = 18;

		if ( ScrollLinesTop < ScrollLinesPos-3 ) LCDBuffer[LCDCOLUMNS*3+LCDCOLUMNS-1] = 19;

		xSemaphoreGive(xLCDBuffer);
		return 1;
    } else
    	return 0;
}


int lcd_processscroll( int direction )
{
	ScrollLinesTop+= direction;

	if ( ScrollLinesTop < 0 ) ScrollLinesTop = 0;
	else if ( ScrollLinesTop > ScrollLinesMax-1 ) ScrollLinesTop = ScrollLinesMax-1;

	return lcd_printscroll();
}


int lcd_send_stringscroll(char *str)
{
	if ( !inerror ) {
		int scroll = 0;

		if( ScrollLinesPos < ScrollLinesMax ){
			memcpy(&ScrollLines[ScrollLinesPos][0],str,LCDCOLUMNS); // copy string into lines
			ScrollLinesPos++;
			if ( ScrollLinesTop < ScrollLinesPos-3)
				scroll = 1;

		} else
		{
			memmove(&ScrollLines[0][0],&ScrollLines[1][0],21*(ScrollLinesMax-1)); // TODO shunt lines back. Should reimplement as ring buffer?
			memcpy(&ScrollLines[ScrollLinesPos-1][0],str,LCDCOLUMNS); // copy string into lines
		}

		lcd_processscroll( scroll );
	}
	return 0;
}


void lcd_errormsg(char *str)
{
	if ( !inerror ){
		lcd_send_stringposDIR( 0, 0,  str );

		inerror = true;
//		lcderrortime = gettimer();
	}
}


int initLCD( void )
{
#ifdef SCREEN
	MX_I2C3_Init();

	xLCDBuffer = xSemaphoreCreateMutex();

	for ( int i=0;i<LCDBUFSIZE;i++){ 	// clear display buffer.
		LCDBuffer[i] = 32;
	}

	for ( int i=0;i<LCDROWS;i++)
		LinePriority[i] = 255;

	if ( !lcd_init(&hi2c3) ){
		DeviceState.LCD = DISABLED;
	} else
	{
		lcd_send_stringposDIR(0,0,"Startup...   ");
		lcd_clearscroll();
	}

	LCDQueue = xQueueCreateStatic( LCDQUEUE_LENGTH,
							  LCDITEMSIZE,
							  LCDQueueStorageArea,
							  &LCDStaticQueue );

	vQueueAddToRegistry(LCDQueue, "LCDQueue" );

	LCDTaskHandle = osThreadNew(LCDTask, NULL, &LCDTask_attributes);
#endif
	return 0;
}
