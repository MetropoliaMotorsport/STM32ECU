/*
 * output.h
 *
 *  Created on: 14 Apr 2019
 *      Author: drago
 */

#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "ecumain.h"

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

typedef enum output_state {
	Off,
	On,
	BlinkVerySlow,
	BlinkSlow,
	BlinkMed,
	BlinkFast,
	BlinkVeryFast,
	Toggle,
	Timed,
	Nochange,
	Stopdebug
} output_state;

typedef struct output_msg {
	uint32_t output;
	enum output_state state;
	uint32_t time;
	bool debug;
} output_msg;

extern QueueHandle_t OutputQueue;

#ifdef HPF20

typedef enum output {
	BMSLED = 8, IMDLED = 6, BSPDLED = 0, TSLED = 2, STARTLED = 4, TSOFFLED = 7, // 0
	RTDMLED = 3,
#ifdef HPF2023
	ERRORLED = 17,
	LED1 = 11,
	LED2 = 12,
	LED3 = 13,
	LED4 = 14,
	LED5 = 15,
	LED6 = 16,
	LED7 = 17,
#else
	ERRORLED=18,
	LED1=12,
	LED2=13,
	LED3=14,
	LED4=15,
	LED5=16,
	LED6=17,
	LED7=18,
#endif
	Output0 = 0,
	Output1 = 1,
	Output2 = 2,
	Output3 = 3,
	Output4 = 4,
	Output5 = 5,
	Output6 = 6,
	Output7 = 7,
	Output8 = 8,
	Output9 = 9,
	Output10 = 10,
	Output11 = 11,
	Output12 = 12,
	Output13 = 13,
	Output14 = 14,
	Output15 = 15,
	Output16 = 16,
	Output17 = 17,
	Output18 = 18
} output;

#define Once					(1)
#define OUTPUTCount		    (19)
#endif

// values to define blinking rate of led output's in fraction of second.

#define LEDBLINK_FOUR   	BlinkFast //5 // four times / second
#define LEDBLINK_THREE		BlinkMed //4 // two times  / second
#define LEDBLINK_TWO		BlinkSlow //3 // one full cycle / second
#define LEDBLINK_ONE		BlinkVerySlow //2 // toggles every second.

#define LEDBLINKNONSTOP     0xFFFF

void setOutput(output output, output_state state);
void setOutputDebug(output output, output_state state);
void setOutputNOW(output output, output_state state); //  doesn't wait for timer to set output
void toggleOutput(output output);
void toggleOutputMetal(output output);
void blinkOutput(output output, output_state blinkingrate, uint32_t time);
void blinkOutputDebug(output output, output_state blinkingrate, uint32_t time);
void timeOutput(output output, uint32_t time);
void stopBlinkOutput(output output);
void resetOutput(output output, output_state state);

void setLEDs(void);
void startupLEDs(void);

int getGpioPin(output output);
GPIO_TypeDef* getGpioPort(output output);

int initOutput(void);

#endif /* OUTPUT_H_ */
