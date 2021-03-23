// Heavily modified i2c lcd library.

/** Put this in the src folder **/

#include "ecumain.h"
#include "i2c-lcd.h"
#include <stdbool.h>
#include "stm32h7xx_hal.h"

I2C_HandleTypeDef * lcdi2c;  // change your handler here accordingly
extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef hspi4;

#define US2066

#define LCDBUFFER

#ifdef US2066

//#define SPI

#if defined( __ICCARM__ )
  #define DMA_BUFFER \
      _Pragma("location=\".dma_buffer\"")
#else
  #define DMA_BUFFER \
      __attribute__((section(".dma_buffer")))
#endif


#define SLAVE_ADDRESS_LCD 0x3C // 3D if change address pin pulled high
#define OLED_Command_Mode 0x80
#define OLED_Data_Mode 0x40

#define ScrollLinesMax 40

char ScrollLines[ScrollLinesMax][21] = { 0 };
int ScrollLinesPos = 0;
int ScrollLinesRingStart = 0;
int ScrollLinesTop = 0;
char ScrollTitle[21] = "";


DMA_BUFFER ALIGN_32BYTES (static uint8_t LCDBuffer[LCDBUFSIZE]);

DMA_BUFFER ALIGN_32BYTES (static uint8_t sendbuffer[LCDBUFSIZE*2]); // allow for command codes etc.
static int     sendbufferpos = 0;

static uint8_t LinePriority[4] = {0};
static uint32_t LinePriorityTime[4] = {0};

volatile static bool inerror = false;

volatile static bool readytosend = true;

volatile static uint32_t sendtime = 0;

volatile static uint32_t lcderrorcount = 0;

uint32_t lcderrortime = 0;

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

void LCDTask(void *argument)
{
	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT( LCDQueue );

	vTaskDelay(5);

    const TickType_t xFrequency = 20;

//	unsigned long counter;

	struct lcd_msg msg;

	for(;;)
	{

		while ( uxQueueMessagesWaiting( LCDQueue ) )
		{
        // UBaseType_t uxQueueMessagesWaiting( QueueHandle_t xQueue );
			xQueueReceive(LCDQueue,&msg,0);

			char str[LCDCOLUMNS+1] = "";

			sprintf(str,"Count: %.10lu", msg.data.count );

			lcd_send_stringline(3,str, 0);
	//		vTaskYield();

		} // portMAX_DELAY

		lcd_updatedisplay();

		vTaskDelay(xFrequency);

	//	vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}



	osThreadTerminate(NULL);
}

void strpad(char * str, int len){
	int strlength = strlen(str);
	if ( strlength > 20 )
	{
		str[18] = '.'; str[19] = '.'; str[20] = 0;
	} else
	{ // pad string out to clear rest of line if shorter.
		for (int i=strlength;i<20;i++) str[i] = 32;
		str[20] = 0;
	};
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if ( hi2c->Instance == I2C3 ){
		sendtime = gettimer();
		readytosend = true;
		sendbufferpos = 0;
		inerror = false;
	}
}

void LCD_I2CError( void )
{
//	volatile uint32_t err = HAL_I2C_ERROR_NONE;
	// check error in i2c handle HAL_I2C_ERROR_AF

	lcderrorcount++;
	lcderrortime = gettimer();
	readytosend = false;
	inerror = true;

//	if ( lcdi2c->ErrorCode & ( 1 << HAL_I2C_ERROR_BERR )

/*	if ( lcdi2c->ErrorCode == 4 )
	{
		readytosend = false;
		inerror = true;

		//== not connected);
	} else
	{
		readytosend = true;
		inerror = false;
	}
*/
}

int lcd_update( void )
{
  return 0;
}

int lcd_send( void )
{
  return 0;
}

int lcd_updatedisplay( void ) // batch send buffered LCD commands
{
//	static lastcall;

	if (HAL_I2C_GetState(lcdi2c) != HAL_I2C_STATE_READY)
	{
		return 1;
	}

//	lastcall = gettimer();
#ifdef LCDBUFFER

	for ( int row=0;row<LCDROWS;row++)
	{
		if ( LinePriorityTime[row] < gettimer() )
		{
			 LinePriority[row]=255;
		}
	}

	if ( sendbufferpos != 0 && readytosend ) // used for initialisation, and any other special commands. send blocking to ensure works.
	{
		if ( sendtime+10 < gettimer() ) HAL_Delay(5);

		if ( lcdi2c->State == HAL_I2C_STATE_READY  )
		{

			if ( HAL_I2C_Master_Transmit_IT(lcdi2c, SLAVE_ADDRESS_LCD<<1,(uint8_t *) sendbuffer, sendbufferpos) != HAL_OK ){
//			if ( HAL_I2C_Master_Transmit(lcdi2c, SLAVE_ADDRESS_LCD<<1,(uint8_t *) sendbuffer, sendbufferpos, 10) != HAL_OK ){
					sendbufferpos=0;
					inerror = true;
					lcderrortime = gettimer();
					readytosend = false;
					return 1;
			}
			inerror = false;
			readytosend = false;
			sendbufferpos=0;
			return 0;
		}
//		sendbufferpos=0;
//		readytosend = true;
		return 1;
	} else if ( readytosend )
	{
		if ( sendtime+10 < gettimer() ) HAL_Delay(5);

		readytosend = false;
		sendbufferpos = 0;

		sendbuffer[0] = 0x80;
		sendbuffer[1] = 0x80; // position.
		sendbuffer[2] = 0x40;

		int len = 3;

		memcpy(&sendbuffer[len], LCDBuffer, 80);

/* 		for ( int i=0;i<40;i++){
			sendbuffer[len+i] = LCDBuffer[i];
		} */

/*
		sendbuffer[sendbufferpos] = 0x80;
		sendbuffer[sendbufferpos+1] = cmd;
		sendbufferpos +=2;

		sendbuffer[sendbufferpos] = 0xC0;
		sendbuffer[sendbufferpos+1] = data;
		sendbufferpos +=2;

*/
		// send whole screen buffer.
#ifdef SCREEN

		if ( HAL_I2C_Master_Transmit_IT(lcdi2c, SLAVE_ADDRESS_LCD<<1,(uint8_t *) sendbuffer, 83) != HAL_OK ){
				sendbufferpos=0;
				inerror = true;
				lcderrortime = gettimer();
				return 1;
		}
#endif
	#endif
		sendbufferpos=0;
	} else if ( inerror && gettimer() > lcderrortime+100 )
		// been one second since lcd error, try again.
	{	// avoid i2c constantly failing if lcd dropped off, but try to recover.
		if ( lcd_init(lcdi2c) ) // TODO, lots of i2c lcd errors suddenly. can't figure out why, use probe later.
		{
			inerror = false;
			readytosend = true;
		} else lcderrortime = gettimer();

	}
	return 0;

}

/*
10 = 0x80  command byte next, continue processing control
11 = 0xC0 data byte next, continue processing control
01 = 0x40  continous data next, command bit ignored.
00 = 0x00 continous data next.
*/


/*
 * After the transmission of the slave address, either the control byte or the data byte may be sent across the SDA.
 * A control byte mainly consists of Co and D/C# bits following by six �0��s.
a. If the Co bit is set as logic �0�, the transmission of the following information will contain data bytes only.
b. The D/C# bit determines the next data byte is acted as a command or a data. If the D/C# bit is set to logic �0�,
it defines the following data byte as a command. If the D/C# bit is set to logic �1�,
it defines the following data byte as a data which will be stored at the DDRAM. The DDRAM address counter will be increased by one
automatically after each data write.
 */

uint8_t reverse_byte(uint8_t byte)
 {
     byte = (byte & 0xF0) >> 4 | (byte & 0x0F) << 4;
     byte = (byte & 0xCC) >> 2 | (byte & 0x33) << 2;
     byte = (byte & 0xAA) >> 1 | (byte & 0x55) << 1;
     return byte;
 }


int lcd_send_cmd (char cmd)
{

#ifdef SPI
	uint8_t data_t[3];
	cmd = reverse_byte(cmd);

	data_t[0] = 0b11111000;;//OLED_Command_Mode;  //en=1, rs=0
	data_t[1] = cmd & 0b11110000;;  //en=0, rs=0
	data_t[2] = cmd << 4;


	if ( HAL_SPI_Transmit(&hspi4, data_t, 3, 50)  != HAL_OK ){
		return 1;
	}
#else

#ifdef LCDBUFFER
	sendbuffer[sendbufferpos] = 0x80;
	sendbuffer[sendbufferpos+1] = cmd;
	sendbufferpos +=2;
#else

	data_t[0] = 0;//OLED_Command_Mode;  //en=1, rs=0
	data_t[1] = cmd;  //en=0, rs=0
	if ( HAL_I2C_Master_Transmit(lcdi2c, SLAVE_ADDRESS_LCD<<1,(uint8_t *) data_t, 2, 100) != HAL_OK ){
		return 1;
	}
#endif
#endif

	return 0;
}


int lcd_send_data (char data)
{

#ifdef SPI
	uint8_t data_t[2];
	data = reverse_byte(data);

	data_t[0] = 0b11111010;//OLED_Command_Mode;  //en=1, rs=0
	data_t[1] = data & 0b11110000;;  //en=0, rs=0
	data_t[2] = data << 4;

	if ( HAL_SPI_Transmit(&hspi4, data_t, 3, 50)  != HAL_OK ){
		return 1;
	}
#else

#ifdef LCDBUFFER
	sendbuffer[sendbufferpos] = 0xC0;
	sendbuffer[sendbufferpos+1] = data;
	sendbufferpos +=2;
#else

	data_t[0] = OLED_Data_Mode;  //en=1, rs=0
	data_t[1] = data;  //en=0, rs=0
	if ( HAL_I2C_Master_Transmit(lcdi2c, SLAVE_ADDRESS_LCD<<1,(uint8_t *) data_t, 2, 100) != HAL_OK ){
		return 1;
	}
#endif
#endif
	return 0;
}
#else

#define SLAVE_ADDRESS_LCD 0x4E //0x4E // change this according to your setup NHD=3C or 3D, standard lcd = 4E


void lcd_send_cmd (char cmd)
{
  char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0
	data_t[1] = data_u|0x08;  //en=0, rs=0
	data_t[2] = data_l|0x0C;  //en=1, rs=0
	data_t[3] = data_l|0x08;  //en=0, rs=0

	//
	// #define OLED_Command_Mode 0x80
//#define OLED_Data_Mode 0x40

	HAL_I2C_Master_Transmit(lcdi2c, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=0
	data_t[1] = data_u|0x09;  //en=0, rs=0
	data_t[2] = data_l|0x0D;  //en=1, rs=0
	data_t[3] = data_l|0x09;  //en=0, rs=0
	HAL_I2C_Master_Transmit (lcdi2c, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}
#endif


void lcd_clear (void)
{
	lcd_send_cmd (0x80);
	for (int i=0; i<70; i++)
	{
		lcd_send_data (' ');
	}
}


void lcd_put_cur(int row, int col)
{
#ifdef US2066

	 int row_offsets[] = { 0x00, 0x20, 0x40, 0x60 };//THIS WAS THE CHANGE FOR 4 LINES
	 col = (0x80 | (col + row_offsets[row])); //was 0x80
#else
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;

        case 1:
            col |= 0xC0;
            break;
    }
#endif

    lcd_send_cmd (col);
}


int lcd_init (I2C_HandleTypeDef *i2chandle)
{
	lcdi2c = i2chandle;

#ifdef US2066

#ifdef LCDBUFFER

	if ( (uint32_t)sendbuffer < 0x24000000 )
	{
		lcd_send_stringposDIR(0,0,"LCD buf in wrong MEM                    ");
		lcd_send_stringposDIR(2,0,"Fix .LD & compile!                      ");

#ifdef LDFILEINC
	// Check .LD Linker file for the following:

	.dma_buffer : /* Space before ':' is critical */
	{
	  *(.dma_buffer)
	} >RAM_D1


#endif


		while ( 1 ) { } // .LD file defined wrong! fix.
	}

#endif

	 HAL_I2C_DeInit(lcdi2c);

	 MX_I2C3_Init();

	  if ( HAL_I2C_IsDeviceReady(lcdi2c, (uint16_t)(SLAVE_ADDRESS_LCD<<1), 2, 1) != HAL_OK) // HAL_ERROR or HAL_BUSY or HAL_TIMEOUT
	  {

	          return 1; // No ACK received at that address

	  }

	  HAL_Delay(1);

	sendbufferpos = 0; // reset send buffer for init.
	readytosend = true;

	if ( lcd_send_cmd(0x2A) ){	 //function set (extended command set)
		return 1;
	}

	lcd_send_cmd(0x71); //function selection A
	lcd_send_data(0x00); // disable internal VDD regulator (2.8V I/O). data(0x5C) = enable regulator (5V I/O)
	lcd_send_cmd(0x28); //function set (fundamental command set)
	lcd_send_cmd(0x08); //display off, cursor off, blink off
	lcd_send_cmd(0x2A); //function set (extended command set)
	lcd_send_cmd(0x79); //OLED command set enabled
	lcd_send_cmd(0xD5); //set display clock divide ratio/oscillator frequency
	lcd_send_cmd(0x70); //set display clock divide ratio/oscillator frequency
	lcd_send_cmd(0x78); //OLED command set disabled
	lcd_send_cmd(0x09); //extended function set (4-lines)
	lcd_send_cmd(0x06); //COM SEG direction
	lcd_send_cmd(0x72); //function selection B
	lcd_send_data(0x00); //ROM CGRAM selection
	lcd_send_cmd(0x2A); //function set (extended command set)
	lcd_send_cmd(0x79); //OLED command set enabled
	lcd_send_cmd(0xDA); //set SEG pins hardware configuration
	lcd_send_cmd(0x10); //set SEG pins hardware configuration
	lcd_send_cmd(0xDC); //function selection C
	lcd_send_cmd(0x00); //function selection C
	lcd_send_cmd(0x81); //set contrast control
	lcd_send_cmd(0x7F); //set contrast control
	lcd_send_cmd(0xD9); //set phase length
	lcd_send_cmd(0xF1); //set phase length
	lcd_send_cmd(0xDB); //set VCOMH deselect level
	lcd_send_cmd(0x40); //set VCOMH deselect level
	lcd_send_cmd(0x78); //OLED command set disabled
	lcd_send_cmd(0x28); //function set (fundamental command set)
	lcd_send_cmd(0x01); //clear display
	lcd_send_cmd(0x80); //set DDRAM address to 0x00
	lcd_send_cmd(0x0C); //display ON


	// wait for ready? 	if ( lcdi2c->State == HAL_I2C_STATE_READY  )

	if ( lcd_updatedisplay() != 0 ) // don't wait for timer update.
	{
		DeviceState.LCD = OFFLINE;
		return 0;
	}

	DeviceState.LCD = OPERATIONAL;

	return 1;


#else
	// 4 bit initialisation
	HAL_Delay(50);  // wait for >40ms
	lcd_send_cmd (0x30);
	HAL_Delay(5);  // wait for >4.1ms
	lcd_send_cmd (0x30);
	HAL_Delay(1);  // wait for >100us
	lcd_send_cmd (0x30);
	HAL_Delay(10);
	lcd_send_cmd (0x20);  // 4bit mode
	HAL_Delay(10);

  // dislay initialisation
	lcd_send_cmd (0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	HAL_Delay(1);
	lcd_send_cmd (0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	HAL_Delay(1);
	lcd_send_cmd (0x01);  // clear display
	HAL_Delay(1);
	HAL_Delay(1);
	lcd_send_cmd (0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	HAL_Delay(1);
	lcd_send_cmd (0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
#endif
	return 0;
}


int lcd_send_stringline( int row, char *str, uint8_t priority )
{
	if ( priority <= LinePriority[row]) // only allow update if not priority overridden
	{
		char line[21] = "                    ";

		int copylen = strlen(str);

		if ( copylen > 20 )
			copylen = 20; // ensure don't print off screen.

		memcpy(line, str, copylen); // copy string into

		lcd_send_stringpos( row, 0, line );
		LinePriorityTime[row] = gettimer()+2000; // show for at least 200ms
		LinePriority[row]=priority;
		return 0;
	} else return 1; // no update allowed.
}


int lcd_send_stringpos( int row, int col, char *str )
{
	if ( !inerror ) {

		int copylen = strlen(str);

		if ( col + copylen > 20 )
			copylen = 20-copylen; // ensure don't print off screen.

		memcpy(&LCDBuffer[col+row*20], str, copylen); // copy sring into

#ifdef DIRECT
		uint8_t data_t[40];
		int row_offsets[] = { 0x00, 0x20, 0x40, 0x60 };//THIS WAS THE CHANGE FOR 4 LINES

		/*
		10 = 0x80  command byte next, continue processing control
		11 = 0xC0 data byte next, continue processing control
		01 = 0x40  continous data next, command bit ignored.
		00 = 0x00 continous data next.
		*/

		/*
		int i = 0;

		data_t[i] = 0x80; // 0x80 continuation bit with cmd
		i++;
		data_t[i] = (0x80 | (col + row_offsets[row])); // command acted.
		i++;

		data_t[i] = 0xC0; // 0xC0 continuation bit with data // command interpreted as data
		i++;
		data_t[i] = 'T';// (0x80 | (0 + row_offsets[0]));
		i++;

		data_t[i] = 0x80; // 0xC0 continuation bit with data // command interpreted as data
		i++;

		data_t[i] = (0x80 | (0 + row_offsets[2]));
		i++;

		data_t[i] = 0x40; // 0xC0 continuation bit with data // command interpreted as data
		i++;

		data_t[i] = 'E'; // skipped, treated as command?
		i++;

		data_t[i] = 'S';
		i++;

		data_t[i] = 'T';
		i++;
		/*/


		data_t[0] = 0x80; // 0x80 continuation bit with cmd
		data_t[1] = (0x80 | (col + row_offsets[row])); // command acted.
		data_t[2] = 0x40; // 0x40, rest of send is data. data bit only set 0b01000000 // @ sign.
		int len = 3;
		while (*str){
			data_t[len] = *str++;
			len++;
		} // loop till 0
		data_t[len] = 0;
		readytosend = false;

		if ( HAL_I2C_Master_Transmit(lcdi2c, SLAVE_ADDRESS_LCD<<1,(uint8_t *) data_t, len,10) != HAL_OK ){
			return 1;
		}
		readytosend = true;
#endif
	}

	return 0;
}


int lcd_send_stringposDIR( int row, int col, char *str )
{
	uint8_t data_t[40];

	if ( !inerror ) {

		 // copy string into buffer in addition to directly sending it, so that updates don't wipe.
		memcpy(&LCDBuffer[col+row*20], str, strlen(str));


		// create send command.
		int row_offsets[] = { 0x00, 0x20, 0x40, 0x60 };//THIS WAS THE CHANGE FOR 4 LINES

		data_t[0] = 0x80; // 0x80 continuation bit with cmd
		data_t[1] = (0x80 | (col + row_offsets[row])); // command acted.
		data_t[2] = 0x40; // 0x40, rest of send is data. data bit only set 0b01000000 // @ sign.
		int len = 3;
		while (*str){
			data_t[len] = *str++;
			len++;
		} // loop till 0
		data_t[len] = 0;

		// output in blocking fashion to ensure gets sent even if interrupts not yet enabled.
		if ( HAL_I2C_Master_Transmit(lcdi2c, SLAVE_ADDRESS_LCD<<1,(uint8_t *) data_t, len,10) != HAL_OK ){ // blocking write to make sure it gets sent.
			return 1;
		}
	}

	return 0;
}


void lcd_setscrolltitle( char * str )
{

	for ( int i=0;i<20;i++)
		ScrollTitle[i] = 32;
	ScrollTitle[20] = 0;

	int copylen = strlen(str);
	if ( copylen > 20 ) copylen = 20;

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
		for ( int j=0;j<20;j++){
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
	for ( int i=0;i<20*3;i++){ 	// clear bottom of display buffer.
		LCDBuffer[20+i] = 32;
	}

	for ( int i=0;i<3;i++){
		if ( LinePriority[i] == 255 ) // only allow update if not priority overridden
		{
			if ( i+ScrollLinesTop <= ScrollLinesPos )
				lcd_send_stringpos( 1+i, 0, &ScrollLines[i+ScrollLinesTop][0] );
		}
	}

	if ( ScrollLinesTop > 0 ) LCDBuffer[20+19] = 18;

	if ( ScrollLinesTop < ScrollLinesPos-3 ) LCDBuffer[20*3+19] = 19;

	// TODO print arrrows to indicate more lines.
	return 1;
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
			memcpy(&ScrollLines[ScrollLinesPos][0],str,20); // copy string into lines
			ScrollLinesPos++;
			if ( ScrollLinesTop < ScrollLinesPos-3)
				scroll = 1;

		} else
		{
			memmove(&ScrollLines[0][0],&ScrollLines[1][0],21*(ScrollLinesMax-1)); // TODO shunt lines back. Should reimplement as ring buffer?
			memcpy(&ScrollLines[ScrollLinesPos-1][0],str,20); // copy string into lines
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
		lcderrortime = gettimer();
	}

	lcd_update();

}

int lcd_clearerror( void )
{
	inerror = false;
	return 0;
}

int initLCD( void )
{
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
