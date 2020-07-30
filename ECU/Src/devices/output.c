/*
 * output.c
 *
 *  Created on: 14 Apr 2019
 *      Author: drago
 */

#include "ecumain.h"

/**
 * returns gpio port for given output number.
 */


#ifdef HPF19
GPIO_TypeDef* getGpioPort(int output)
{
	switch ( output ) { // set gpio values for requested port
		case 0 :
			return Output1_GPIO_Port;
		case 1 :
			return Output2_GPIO_Port;
		case 2 :
			return Output3_GPIO_Port;
		case 3 :
			return Output4_GPIO_Port;
		case 4 :
			return Output5_GPIO_Port;
		case 5 :
			return Output6_GPIO_Port;
		case 6 :
			return Output7_GPIO_Port;
		case 7 :
			return Output8_GPIO_Port;
		case 8 :
			return LD1_GPIO_Port;
		case 9 :
			return LD2_GPIO_Port;
		case 10 :
			return LD3_GPIO_Port;
		default :
			return 0;
	}
}

/**
 * returns GPIO pin for given output number.
 */

int getGpioPin(int output)
{
	switch ( output )
	{ // set gpio values for requested port
		case 0 :
			return Output1_Pin;
		case 1 :
			return Output2_Pin;
		case 2 :
			return Output3_Pin;
		case 3 :
			return Output4_Pin;
		case 4 :
			return Output5_Pin;
		case 5 :
			return Output6_Pin;
		case 6 :
			return Output7_Pin;
		case 7 :
			return Output8_Pin;
		case 8 :
			return LD1_Pin;
		case 9 :
			return LD2_Pin;
		case 10 :
			return LD3_Pin;
		default :
			return 0;
	}
}
#endif

#ifdef HPF20
GPIO_TypeDef* getGpioPort(int output)
{

	switch ( output ) { // set gpio values for requested port
		case 0 :
			return DO1_GPIO_Port;
		case 1 :
			return DO2_GPIO_Port;
		case 2 :
			return DO3_GPIO_Port;
		case 3 :
			return DO4_GPIO_Port;
		case 4 :
			return DO5_GPIO_Port;
		case 5 :
			return DO6_GPIO_Port;
		case 6 :
			return DO7_GPIO_Port;
		case 7 :
			return DO11_GPIO_Port;
		case 8 :
			return DO12_GPIO_Port;
		case 9 :
			return DO13_GPIO_Port;
		case 10 :
			return DO14_GPIO_Port;
		case 11 :
			return DO15_GPIO_Port;

		case 12 :
			return LD7_GPIO_Port;
		case 13 :
			return LD8_GPIO_Port;
		case 14 :
			return LD9_GPIO_Port;
		case 15 :
			return LD0_GPIO_Port;
		case 16 :
			return LD1_GPIO_Port;
		case 17 :
			return LD3_GPIO_Port;
		case 18 :
			return LD4_GPIO_Port;

		default :
			return 0;
	}
}

/**
 * returns GPIO pin for given output number.
 */

int getGpioPin(int output)
{
	switch ( output ) { // set gpio values for requested port
		case 0 :
			return DO1_Pin;
		case 1 :
			return DO2_Pin;
		case 2 :
			return DO3_Pin;
		case 3 :
			return DO4_Pin;
		case 4 :
			return DO5_Pin;
		case 5 :
			return DO6_Pin;
		case 6 :
			return DO7_Pin;
		case 7 :
			return DO11_Pin;
		case 8 :
			return DO12_Pin;
		case 9 :
			return DO13_Pin;
		case 10 :
			return DO14_Pin;
		case 11 :
			return DO15_Pin;
		case 12 :
			return LD7_Pin;
		case 13 :
			return LD8_Pin;
		case 14 :
			return LD9_Pin;
		case 15 :
			return LD0_Pin;
		case 16 :
			return LD1_Pin;
		case 17 :
			return LD3_Pin;
		case 18 :
			return LD4_Pin;
		default :
			return 0;
	}
}
#endif


/**
 * @brief sets specific output state of the state of specified GPIO output using programs defined input numbering
 */
void setOutput(int output, char state)
{
	if ( output < OUTPUTCount ){
		if ( state == 0 )
		  LEDs[output].state = 0;
		else
		  LEDs[output].state = 9;
	}
//	LEDs[output].blinktime = 0;
//	LEDs[output].blinkingrate = 0;
}

void setOutputNOW(int output, char state)
{
	if ( output < OUTPUTCount ){
		if ( state == 0 )
		{
			HAL_GPIO_WritePin(getGpioPort(output), getGpioPin(output), 0);
			LEDs[output].state = 0;
		} else
		{
			HAL_GPIO_WritePin(getGpioPort(output), getGpioPin(output), 1);
			LEDs[output].state = 9;
		}
	}
}

/**
 * @brief Toggles the state of specified GPIO output using programs defined input numbering
 */
void toggleOutput(int output)
{
	if ( output < OUTPUTCount ){
		if ( LEDs[output].state == 0 ) LEDs[output].state = 9; else LEDs[output].state = 0;
	}
}

void toggleOutputMetal(int output)
{
	if ( output < OUTPUTCount ){
		if(getGpioPin(output) != 0)
		{
			HAL_GPIO_TogglePin(getGpioPort(output), getGpioPin(output));
		}
	}
}

void blinkOutput(int output, int blinkingrate, int time) // max 30 seconds if timed.
{
	if ( output < OUTPUTCount ){
		switch ( blinkingrate )
		{
			case LEDOFF : LEDs[output].blinkingrate = 0; break;
			case LEDON : LEDs[output].blinkingrate = 9; break;
			case LEDBLINK_ONE : LEDs[output].blinkingrate = 8; break;
			case LEDBLINK_TWO : LEDs[output].blinkingrate = 4; break;
			case LEDBLINK_THREE : LEDs[output].blinkingrate = 2; break;
			case LEDBLINK_FOUR : LEDs[output].blinkingrate = 1; break;
			default :
				LEDs[output].blinkingrate = 9;
				break;
		}

		if ( time == 255 ){
			LEDs[output].blinktime = LEDBLINKNONSTOP;
		} else
		{
			LEDs[output].blinktime = time*8;
		}
	}
}

void timeOutput(int output, int time) // max 30 seconds if timed.
{
	if ( output < OUTPUTCount ){
		LEDs[output].blinkingrate = 0;

		if ( time == 255 ){
			LEDs[output].blinktime = LEDBLINKNONSTOP;
		} else
		{
			LEDs[output].blinktime = time*8;
		}
	}
}

/**
 * @brief setup start state of LED's to off.
 */
int initOutput( void )
{
	for ( int i = 0; i < OUTPUTCount; i++)
	{
		LEDs[i].blinkingrate = 0;
		LEDs[i].state = 0;
		LEDs[i].blinktime = 0; // set default state of LED's to off, no blinking
	}

	blinkOutput(LED1_Output, LEDBLINK_ONE, LEDBLINKNONSTOP); // startup board activity blinker/power light.
	return 0;
}

/**
 * @brief set the current state of LED's as defined by carstate variables.
 */
void __setLEDs( void )
{
	// Check. 10 second delay for IMD led in simulink. IMD Light power off delay. missed earlier, significance?

	setOutput(BMSLED_Output, CarState.Shutdown.BMS);
	setOutput(IMDLED_Output, CarState.Shutdown.IMD);
	setOutput(BSPDLED_Output, CarState.Shutdown.BSPDBefore);

//	if ( CarState.TSALLeftInvLED == 1 && CarState.TSALRightInvLED == 1 )
	{
		LEDs[TSLED_Output].blinkingrate = 2; // cockpit led

   } //else if ( CarState.TSALLeftInvLED >= 2 && CarState.TSALRightInvLED >= 2)
	{
		LEDs[TSLED_Output].blinkingrate = 0;

	}// else
	{
		LEDs[TSLED_Output].blinkingrate = 0;
	}

//	if ( CarState.RtdmLeftInvLED == 1 && CarState.RtdmRightInvLED == 1 )
	{
		LEDs[RTDMLED_Output].blinkingrate = 2;
	}// else if ( CarState.RtdmLeftInvLED >= 2 && CarState.RtdmRightInvLED >= 2)
	{
		LEDs[RTDMLED_Output].blinkingrate = 0;

	} //else
	{
		LEDs[RTDMLED_Output].blinkingrate = 0;
	}

//	LEDs[TSOFFLED_Output].blinkingrate = CarState.StopLED;
}


void startupLEDs(void)
{
	 //small onboard led display to indicate board startup
	  setOutputNOW(LED1_Output,LEDON);
	  setOutputNOW(LED2_Output,LEDON);
	  setOutputNOW(LED3_Output,LEDON);
	  HAL_Delay(300);
	  setOutputNOW(LED1_Output,LEDOFF);
	  HAL_Delay(300);
	  setOutputNOW(LED2_Output,LEDOFF);
	  HAL_Delay(300);
	  setOutputNOW(LED3_Output,LEDOFF);

#ifdef OLDPOWEROn
	  // display status LED's for two seconds to indicate power on.
	  setOutput(1,LEDON);
	  setOutput(2,LEDON);
	  setOutput(3,LEDON);
	  // HAL_Delay(2000);
	  HAL_Delay(500);
#endif
	  setOutput(BMSLED_Output,LEDON);
	  setOutput(IMDLED_Output,LEDON);
	  setOutput(BSPDLED_Output,LEDON);
	  // HAL_Delay(2000);
	  HAL_Delay(500);


	  for(int i=0;i<=OUTPUTCount;i++){ // turn off all LED's
		  setOutput(i, LEDOFF);
	  }
}
