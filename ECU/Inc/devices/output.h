/*
 * output.h
 *
 *  Created on: 14 Apr 2019
 *      Author: drago
 */

#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "main.h"

/*
D0 - pin 10 PE13  output1 middle orange
D1 - pin 11 PF15  output2 TS LED ( CH11 )
D2 - pin 12 PG9   output3 ?
D3 - pin 13 PE8   output4 BSPD LED
D4 - pin 14 PE7   output5 MID_WHITE
D5 - pin 15 PE10  output6 IMD ( CH 9 )
D6 - pin 16 PE12  output7 RTMD LED ( CH13 )
D7 - pin 17 PE14  output8 BMS ( CH8 )
*/

#ifdef HPF19
	#define BMSLED_Output 		(7)
	#define IMDLED_Output		(5)
	#define BSPDLED_Output		(3)

	#define TSLED_Output		(1)
	#define TSOFFLED_Output		(0)
	#define RTDMLED_Output		(6)
	//#define STOPLED_Output      (0)

	#define LED1_Output			(8)
	#define LED2_Output			(9)
	#define LED3_Output			(10)

	#define OUTPUTCount		    (11)
#endif

#ifdef HPF20
	#define BMSLED_Output 		(7)
	#define IMDLED_Output		(5)
	#define BSPDLED_Output		(3)

	#define TSLED_Output		(1)
	#define TSOFFLED_Output		(0)
	#define RTDMLED_Output		(6)
	//#define STOPLED_Output      (0)

	#define LED1_Output			(12)
	#define LED2_Output			(13)
	#define LED3_Output			(14)
	#define LED4_Output			(15)
	#define LED5_Output			(16)
	#define LED6_Output			(17)
	#define LED7_Output			(18)

	#define OUTPUTCount		    (19)
#endif

// values to define blinking rate of led output's in fraction of second.

#define LEDBLINK_FOUR   	5 // four times / second
#define LEDBLINK_THREE		4 // two times  / second
#define LEDBLINK_TWO		3 // one full cycle / second
#define LEDBLINK_ONE		2 // toggles every second.

#define LEDBLINKNONSTOP     255

#define LEDON				1
#define LEDOFF				0


struct OutputData {
	volatile uint8_t state;
	volatile uint8_t blinkingrate;
	volatile uint8_t blinktime;

};

struct OutputData LEDs[11];

void setOutput(int output, char state);
void setOutputNOW(int output, char state); //  doesn't wait for timer to set output
void toggleOutput(int output);
void toggleOutputMetal(int output);
void blinkOutput(int output, int blinkingrate, int time);
void setupLEDs( void );
void setLEDs( void );
void startupLEDs(void);

int getGpioPin(int output);
GPIO_TypeDef* getGpioPort(int output);

#endif /* OUTPUT_H_ */
