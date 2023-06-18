// Heavily modified i2c lcd library.

/** Put this in the src folder **/

#include "ecumain.h"
#include "lcd.h"
#include "i2c.h"
#include "i2c-lcd.h"
#include "timerecu.h"
#include "input.h"
#include "debug.h"
#include "semphr.h"

#include <stdbool.h>

I2C_HandleTypeDef * lcdi2c;  // change your handler here accordingly
extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef hspi4;

SemaphoreHandle_t I2Crcvdone = NULL;
StaticSemaphore_t I2CrcvdoneBuffer;

#define US2066

extern uint8_t LCDBuffer[LCDBUFSIZE];

#if defined( __ICCARM__ )
  #define DMA_BUFFER \
      _Pragma("location=\".dma_buffer\"")
#else
  #define DMA_BUFFER \
      __attribute__((section(".dma_buffer")))
#endif

#ifdef US2066
   #define SLAVE_ADDRESS_LCD 0x3C // 3D if change address pin pulled high
#else
  #define SLAVE_ADDRESS_LCD 0x4E //0x4E // change this according to your setup NHD=3C or 3D, standard lcd = 4E
#endif

#define SLAVE_ADDRESS_WHEEL 0x20

DMA_BUFFER ALIGN_32BYTES (static uint8_t sendbuffer[LCDBUFSIZE*2]); // allow for command codes etc.
static int     sendbufferpos = 0;

volatile static bool inerror = false;
volatile static bool readytosend = true;
volatile static uint32_t lcderrorcount = 0;
static bool rcvwait = false;

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if ( hi2c->Instance == I2C3 || hi2c->Instance == I2C4 ) // LCD could be on either I2C3 or I2C4
	{
		readytosend = true;
		sendbufferpos = 0;
		inerror = false;
	}
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if ( hi2c->Instance == I2C3 || hi2c->Instance == I2C4 ) // LCD could be on either I2C3 or I2C4
	{
		xSemaphoreGiveFromISR(I2Crcvdone, &xHigherPriorityTaskWoken);
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int lcd_errorcount( void ){
	return lcderrorcount;
}

void LCD_I2CError( void )
{
	if ( rcvwait )
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		rcvwait = false;
		xSemaphoreGiveFromISR(I2Crcvdone, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	} else
	{
	//	volatile uint32_t err = HAL_I2C_ERROR_NONE;
		// check error in i2c handle HAL_I2C_ERROR_AF
		lcderrorcount++;
		readytosend = false;
		inerror = true;
	}

}

int lcd_getstate( void )
{
	HAL_I2C_StateTypeDef state = HAL_I2C_GetState(lcdi2c);

	if ( state != HAL_I2C_STATE_READY)
	{
		return 1;
	}
	return 0;
}

void lcd_resetbuffer()
{
	sendbufferpos=0;
}

int lcd_send( void )
{
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

	data_t[0] = 0b11111000; //en=1, rs=0
	data_t[1] = cmd & 0b11110000;;  //en=0, rs=0
	data_t[2] = cmd << 4;


	if ( HAL_SPI_Transmit(&hspi4, data_t, 3, 50)  != HAL_OK ){
		return 1;
	}
#endif

#ifdef US2066
	sendbuffer[sendbufferpos] = 0x80;
	sendbuffer[sendbufferpos+1] = cmd;
	sendbufferpos +=2;
#else
	  char data_u, data_l;
		uint8_t data_t[4];
		data_u = (cmd&0xf0);
		data_l = ((cmd<<4)&0xf0);
		data_t[0] = data_u|0x0C;  //en=1, rs=0
		data_t[1] = data_u|0x08;  //en=0, rs=0
		data_t[2] = data_l|0x0C;  //en=1, rs=0
		data_t[3] = data_l|0x08;  //en=0, rs=0

		HAL_I2C_Master_Transmit(lcdi2c, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);

#endif

	return 0;
}


int lcd_send_data (char data)
{

#ifdef SPI
	uint8_t data_t[2];
	data = reverse_byte(data);

	data_t[0] = 0b11111010;  //en=1, rs=0
	data_t[1] = data & 0b11110000;;  //en=0, rs=0
	data_t[2] = data << 4;

	if ( HAL_SPI_Transmit(&hspi4, data_t, 3, 50)  != HAL_OK ){
		return 1;
	}
#endif

#ifdef US2066
	sendbuffer[sendbufferpos] = 0xC0;
	sendbuffer[sendbufferpos+1] = data;
	sendbufferpos +=2;
#else
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=0
	data_t[1] = data_u|0x09;  //en=0, rs=0
	data_t[2] = data_l|0x0D;  //en=1, rs=0
	data_t[3] = data_l|0x09;  //en=0, rs=0
	HAL_I2C_Master_Transmit (lcdi2c, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
#endif
	return 0;
}


void lcd_clear_internal(void)
{
	lcd_send_cmd (0x80);
	for (int i=0; i<70; i++)
	{
		lcd_send_data (' ');
	}
}


void lcd_put_cur(int row, int col)
{
	int row_offsets[] = { 0x00, 0x20, 0x40, 0x60 };//THIS WAS THE CHANGE FOR 4 LINES
	col = (0x80 | (col + row_offsets[row])); //was 0x80
    lcd_send_cmd (col);
}


int lcd_init (I2C_HandleTypeDef *i2chandle)
{
	lcdi2c = i2chandle;

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

	HAL_I2C_DeInit(lcdi2c);

	MX_I2C3_Init();
	MX_I2C4_Init();

	if ( HAL_I2C_IsDeviceReady(lcdi2c, (uint16_t)(SLAVE_ADDRESS_LCD<<1), 2, 1) != HAL_OK) // HAL_ERROR or HAL_BUSY or HAL_TIMEOUT
	{
		return 1; // No ACK received at that address
	}

	vTaskDelay(1);

	sendbufferpos = 0; // reset send buffer for init.
	readytosend = true;

#ifdef US2066

	lcd_send_cmd(0x2A); //function set (extended command set)
	lcd_send_cmd(0x71); //function selection A
	lcd_send_data(0x00); // disable internal VDD regulator (2.8V I/O). data(0x5C) = enable regulator (5V I/O)
	lcd_send_cmd(0x28); //function set (fundamental command set)
	// not turning display off means it doesn't blink on re-init, so is silent error.
//	lcd_send_cmd(0x08); //display off, cursor off, blink off
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
	// don't clear display, w're writing the whole thing anyway
	lcd_send_cmd(0x01); //clear display
	lcd_send_cmd(0x80); //set DDRAM address to 0x00
	lcd_send_cmd(0x0C); //display ON

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

	DeviceState.LCD = OPERATIONAL;

	return 1;

}

static bool interrupthigh( void )
{
#ifdef TESTBOARD
	return HAL_GPIO_ReadPin(WHLINT_GPIO_Port, WHLINT_Pin );
#else
	return HAL_GPIO_ReadPin(DI7_GPIO_Port, DI7_Pin );
	//return true; // currently causes to check every cycle.
#endif
}

int callcount = 0;

void wheel_read_input(void)
{
	if ( !I2Crcvdone )
	{
		I2Crcvdone = xSemaphoreCreateBinaryStatic( &I2CrcvdoneBuffer );
	}
	// check if interrupt signal is high, if so read in a loop till it's not high, upto x reads to prevent display blocking.

	uint8_t rcvbuffer[2] = { 0 };

	HAL_StatusTypeDef transmitstatus = HAL_OK;

	int count = 0;
	if ( !interrupthigh() ) // no interrupt flagged
		return;

	do
	{
		//DebugPrintf("Got wheel interrupt");
		callcount++;
		count++;
		rcvwait = true;
		transmitstatus = HAL_I2C_Master_Receive_IT(lcdi2c, SLAVE_ADDRESS_WHEEL<<1,(uint8_t *) rcvbuffer, 2);
		if ( transmitstatus != HAL_OK ) {
			//wheelinerror = true;
			DebugPrintf("I2C read failed 1\n");

			return;
		}

	    xSemaphoreTake(I2Crcvdone, portMAX_DELAY);
		rcvwait = false;

		// wait for receive to complete, or fail.


		switch (rcvbuffer[0])
		{
		case 1:
			switch (rcvbuffer[1] )
			{
			case 1: DebugPrintf("Wheel: JST: U"); setInput(Up_Input); break;
			case 2: DebugPrintf("Wheel: JST: R"); setInput(Right_Input); break;
			case 3: DebugPrintf("Wheel: JST: D"); setInput(Down_Input); break;
			case 4: DebugPrintf("Wheel: JST: L"); setInput(Left_Input); break;
			case 5: DebugPrintf("Wheel: JST: C"); setInput(Center_Input); break;
			default:
				break;
			}
		case 2:
			switch (rcvbuffer[1] )
			{
			case 1: DebugPrintf("Wheel: BTN: UL"); break;
			case 2: DebugPrintf("Wheel: BTN: LL"); break;
			case 3: DebugPrintf("Wheel: BTN: UR"); break;
			case 4: DebugPrintf("Wheel: BTN: LR"); break;
			default:
				DebugPrintf("Wheel: BTN: Bad"); break;
				break;
			}
		case 3:
			DebugPrintf("Wheel: Enc L: %d", rcvbuffer[1]); break;
		case 4:
			DebugPrintf("Wheel: Enc R: %d", rcvbuffer[1]); break;
		case 0:
		default:
			DebugPrintf("Wheel: rcv: %02x %02x - %lu", rcvbuffer[0], rcvbuffer[1], callcount);
			break;
		}

	} while ( count < 10 && interrupthigh() ); // ensures we can't get stuck in an infinite loop waiting
}

// last resort direct lcd update command bypassing send task., not relying on interrupt, or RTOS.
int lcd_send_stringposDIR( int row, int col, char *str )
{
	uint8_t data_t[40];

	int copylenmax = LCDCOLUMNS - col;

	// ensure we don't write past last column of LCD.
	if ( strlen(str) < copylenmax )
	{
		copylenmax = strlen(str);
	}

	 // copy string into buffer in addition to directly sending it, so that updates don't wipe.
//	memcpy(&LCDBuffer[col+row*20], str, strlen(str));

	if ( LCDQueue ) // if queue created, then add the string to queue too to properly display in lcd update.
	{
		lcd_send_stringpos( row, col, str, 0 );
	}

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
		return 0;
	}

	return 1;
}


int lcd_dosend( void )
{
	static uint32_t lastreset = 0;
	// rtctime

	if ( inerror ) // || gettimer() - lastreset > 1000 )
	{
		vTaskDelay(50); // allow time for whatever caused the error to settle.
		lcd_init(lcdi2c); // queus up buffer to send.
		vTaskDelay(5);
		readytosend = true;
		lastreset = gettimer();
	}

#ifdef HPF2023
	wheel_read_input();
#endif

	// used for initialisation, and any other special commands. send blocking to ensure works.
	if ( sendbufferpos != 0 && readytosend )
	{
		if ( lcdi2c->State == HAL_I2C_STATE_READY  )
		{
			HAL_StatusTypeDef transmitstatus = HAL_I2C_Master_Transmit_IT(lcdi2c, SLAVE_ADDRESS_LCD<<1,(uint8_t *) sendbuffer, sendbufferpos);
			if ( transmitstatus != HAL_OK ){
				sendbufferpos=0;
				inerror = true;
				readytosend = false;
				return 0;
			}
			inerror = false;
			readytosend = false;
			sendbufferpos=0;
			return 1;
		}
		return 0;
	} else if ( readytosend ) // && ( lcdi2c->State == HAL_I2C_STATE_READY  ) )
	{
		readytosend = false;
		sendbufferpos = 0;

		// set send buffer command to start updating start of screen.

		sendbuffer[0] = 0x80;
		sendbuffer[1] = 0x80; // position.
		sendbuffer[2] = 0x40;

		int len = 3;

		memcpy(&sendbuffer[len], LCDBuffer, LCDBUFSIZE);

		// Send whole screen buffer rather than try to update piece by piece for simplicity.

		volatile HAL_StatusTypeDef transmitstatus =  HAL_I2C_Master_Transmit_IT(lcdi2c, SLAVE_ADDRESS_LCD<<1,(uint8_t *) sendbuffer, LCDBUFSIZE+3);

		if ( transmitstatus != HAL_OK )
		{
			// send failed to be initialised, assume screen has fallen off bus
			// set error state to stop trying till screen is reinitialised.
			sendbufferpos=0;
			inerror = true;
			return 0;
		} else
		{
			return 1;
		}

		lcd_resetbuffer();
	} else // not in error, but weren't ready to send.
	{
		vTaskDelay(10); // allow some time to finish a potential send in progress, then allow trying again.
		readytosend = true;
		return 0;
	}
	return 1;
}

