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
	#define NO_INPUTS 12 // 12 digital inputs, one being used for PWM
#endif

// Input DI6 being used for PWM steering input. Mapped to Input 4.

#ifdef HPF19 // TODO check if inputs still work on HPF19
	#define UserBtn				(0) // ok
	#define Config_Input		(1) // ok
	#define Center_Input		(1)
	#define RTDM_Input			(2) // ok
	#define Left_Input			(3) // ok
	#define TS_Input			(4) // ok
	#define Right_Input			(5) // ?
	#define StartStop_Input		(6) //
	#define Up_Input			(7) //
	#define Down_Input			(8) //
#endif

#ifdef HPF20

#define DI2		(0)
#define DI3		(1)
#define DI4		(2)
#define DI5		(3)
#define DI6		(4)
#define DI7		(5)
#define DI8		(6)
#define DI10	(7)
#define DI11	(8)
#define DI13	(9)
#define DI14	(10)
#define DI15	(11)

	#define TS_Input DI6 // input 4.
	#define RTDM_Input DI4 // input 2
	#define StartStop_Input DI7 //
	#define Config_Input DI3 // same as center button
	#define Center_Input DI3
	#define Left_Input DI11
	#define Right_Input DI14
	#define Up_Input DI15
	#define Down_Input DI2
#endif

struct ButtonData {
	GPIO_TypeDef * port;
	uint16_t pin;
	uint32_t lastpressed;
	uint32_t count;
	bool pressed;
	// define the hardware button for passing button data including reading it
	uint8_t logic; // 0 for press on low, 1 for press on high.
};


extern CANData CANButtonInput;
volatile struct ButtonData Input[NO_INPUTS];

volatile static char InButtonpress;

int initPWM( void );
bool receivePWM( void );
int getPWMDuty( void ); // returns duty cycle as %*10


int checkReset( void );
int CheckActivationRequest( void );
int CheckLimpActivationRequest( void );
int CheckTSActivationRequest( void );
int CheckRTDMActivationRequest( void );

int CheckButtonPressed( uint8_t In );
int GetUpDownPressed( void );
int GetLeftRightPressed( void );

void clearButtons(void);
void debouncebutton( volatile struct ButtonData *button );

void InputTimerCallback( void );

void receiveInput(uint8_t * CANRxData);

int initInput( void );

#endif /* INPUT_H_ */
