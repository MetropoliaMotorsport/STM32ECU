/*
 * input.h
 *
 *  Created on: 14 Apr 2019
 *      Author: drago
 */

#ifndef INPUT_H_
#define INPUT_H_

#include "ecu.h"

//#define Debouncetime 10000 // only allow one button press every 20ms? No longer being used, Tim7 used instead.

/*
DI0 pin 18 PB8 ok   input 1 ok
DI1 pin 19 PB9 ok   input 2
DI2 pin 20 PD14 ok  input 3
DI3 pin 21 PD15 ok  input 4
DI4 pin 22 PE3 ok   input 5
DI5 pin 23 PE2 ok ? <- pin 6, input6.
DI6 pin 24 PE11  ok   input 7
DI7 pin 25 PF12 ok    input 8
 */

#ifdef HPF19
	#define NO_INPUTS 9
#endif

#ifdef HPF20
	#define NO_INPUTS 12
#endif

#define UserBtn		(0) // ok
#define Input1		(1) // ok
#define Input2		(2) // ok
#define Input3		(3) // ok
#define Input4		(4) // ok
#define Input5		(5) // ?
#define Input6		(6) // -> input 5
#define Input7		(7) // -> input 6
#define Input8		(8) // -> input 7

#ifdef HPF19
	#define TS_Input (4) // input 4.
	#define RTDM_Input (2) // input 2
	#define StartStop_Input (6)
#endif

#ifdef HPF20
	#define TS_Input (4) // input 4.
	#define RTDM_Input (2) // input 2
	#define StartStop_Input (6)
	#define Config_Input (7)
	#define Center_Input (8)
	#define Left_Input (9)
	#define Right_Input (10)
	#define Up_Input (11)
	#define Down_Input (12)


#endif

struct ButtonData {
	uint32_t lastpressed;
	uint32_t count;
	char pressed;
	// define the hardware button for passing button data including reading it
	GPIO_TypeDef * port;
	uint16_t pin;
	uint8_t logic; // 0 for press on low, 1 for press on high.
};


volatile struct ButtonData Input[NO_INPUTS];


volatile static char InButtonpress;

int checkReset( void );
int CheckActivationRequest( void );
int CheckLimpActivationRequest( void );
int CheckTSActivationRequest( void );
int CheckRTDMActivationRequest( void );

void setupButtons(void);
void clearButtons(void);
void debouncebutton( volatile struct ButtonData *button );

void InputTimerCallback( void );


#endif /* INPUT_H_ */
