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

typedef enum output_state { Off, On, BlinkVerySlow, BlinkSlow, BlinkMed, BlinkFast, BlinkVeryFast, Toggle, Timed, Nochange } output_state;

typedef struct output_msg {
	uint8_t output;
	enum output_state state;
	uint32_t time;
} output_msg;

extern QueueHandle_t OutputQueue;

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

typedef enum output {
	BMSLED=7,
	IMDLED=5,
	BSPDLED=3,
	TSLED=1,
	TSOFFLED=0,
	RTDMLED=6,
	LED1=12,
	LED2=13,
	LED3=14,
	LED4=15,
	LED5=16,
	LED6=17,
	LED7=18,
	Output0=0,
	Output1=1,
	Output2=2,
	Output3=3,
	Output4=4,
	Output5=5,
	Output6=6,
	Output7=7,
	Output8=8,
	Output9=9,
	Output10=10,
	Output11=11,
	Output12=12,
	Output13=13,
	Output14=14,
	Output15=15,
	Output16=16,
	Output17=17,
	Output18=18
} output;

#ifndef RTOS
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
#endif

	#define OUTPUTCount		    (19)
#endif

// values to define blinking rate of led output's in fraction of second.

#define LEDBLINK_FOUR   	BlinkFast //5 // four times / second
#define LEDBLINK_THREE		BlinkMed //4 // two times  / second
#define LEDBLINK_TWO		BlinkSlow //3 // one full cycle / second
#define LEDBLINK_ONE		BlinkVerySlow //2 // toggles every second.

#define LEDBLINKNONSTOP     0xFFFF

#ifndef RTOS
struct OutputData {
	volatile uint8_t state;
	volatile uint8_t blinkingrate;
	volatile uint16_t blinktime;

};

struct OutputData LEDs[OUTPUTCount];
#endif

void setOutput(output output, output_state state);
void setOutputNOW(output output, output_state state); //  doesn't wait for timer to set output
void toggleOutput(output output);
void toggleOutputMetal(output output);
void blinkOutput(output output, output_state blinkingrate, uint32_t time);
void stopBlinkOutput(output output);

void setLEDs( void );
void startupLEDs(void);

int getGpioPin(output output);
GPIO_TypeDef* getGpioPort(output output);

int initOutput( void );

#endif /* OUTPUT_H_ */
