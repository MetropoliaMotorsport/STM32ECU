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
DI6 pin 24 PE11 ok   input 7
DI7 pin 25 PF12 ok    input 8
 */

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
//	#define TS_Input DI6 // input 4.
//	#define RTDM_Input DI4 // input 2
//	#define StartStop_Input DI7 //
//	#define Config_Input DI11 // same as center button
//	#define Center_Input DI11
//	#define Left_Input DI3
//	#define Right_Input DI2
//	#define Up_Input DI15
//	#define Down_Input DI14
#endif

typedef enum input {
#ifdef HPF20
	TS_Input=DI6, // input 4.
	RTDM_Input=DI4, // input 2
	StartStop_Input=DI7, //
	Config_Input=DI11, // same as center button
	Center_Input=DI11,
	Left_Input=DI3,
	Right_Input=DI2,
	Up_Input=DI15,
	Down_Input=DI14,
#endif

#ifdef HPF19
	UserBtn=0, // ok
	Config_Input=1, // ok
	Center_Input=1,
	RTDM_Input=2, // ok
	Left_Input=3, // ok
	TS_Input=4, // ok
	Right_Input=5, // ?
	StartStop_Input=6, //
	Up_Input=7, //
	Down_Input=8, //
#endif

	Input0=0,
	Input1=1,
	Input2=2,
	Input3=3,
	Input4=4,
	Input5=5,
	Input6=6,
	Input7=7,
	Input8=8,
	Input9=9,
	Input10=10,
	Input11=11
} input;

#define REPEATRATE				(500)

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

typedef struct input_msg {
	input input;
} input_msg;

extern QueueHandle_t InputQueue;

extern CANData CANButtonInput;

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
#ifndef RTOS
void debouncebutton( volatile struct ButtonData *button );

void InputTimerCallback( void );
#endif

void receiveInput(uint8_t * CANRxData);

int initInput( void );

#endif /* INPUT_H_ */
