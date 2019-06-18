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
 * returns gpio pin for given output number.
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

/**
 * @brief sets specific output state of the state of specified GPIO output using programs defined input numbering
 */
void setOutput(int output, char state)
{
	if ( state == 0 )
	  LEDs[output].state = 0;
	else
	  LEDs[output].state = 9;
//	LEDs[output].blinktime = 0;
//	LEDs[output].blinkingrate = 0;
}

void setOutputNOW(int output, char state)
{
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

/**
 * @brief Toggles the state of specified GPIO output using programs defined input numbering
 */
void toggleOutput(int output)
{
	if ( LEDs[output].state == 0 ) LEDs[output].state = 9; else LEDs[output].state = 0;
}

void toggleOutputMetal(int output)
{
	if(getGpioPin(output) != 0)
	{
		HAL_GPIO_TogglePin(getGpioPort(output), getGpioPin(output));
	}
}

void blinkOutput(int output, int blinkingrate, int time) // max 30 seconds if timed.
{
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
		LEDs[output].blinktime = 255;
	} else
	{
		LEDs[output].blinktime = time*8;
	}
}

void timeOutput(int output, int time) // max 30 seconds if timed.
{
	LEDs[output].blinkingrate = 0;

	if ( time == 255 ){
		LEDs[output].blinktime = 255;
	} else
	{
		LEDs[output].blinktime = time*8;
	}
}

/**
 * @brief setup start state of LED's to off.
 */
void setupLEDs( void )
{
	for ( int i = 0; i < 12; i++)
	{
		LEDs[i].blinkingrate = 0;
		LEDs[i].state = 0;
		LEDs[i].blinktime = 0; // set default state of LED's to off, no blinking
	}

	  blinkOutput(LED1_Output, LEDBLINK_ONE, LEDBLINKNONSTOP); // startup board activity blinker/power light.
}

/**
 * @brief set the current state of LED's as defined by carstate variables.
 */
void __setLEDs( void )
{
	// Check. 10 second delay for IMD led in simulink. IMD Light power off delay. missed earlier, significance?

	setOutput(BMSLED_Output, CarState.BMS_relay_status);
	setOutput(IMDLED_Output, CarState.IMD_relay_status);
	setOutput(BSPDLED_Output, CarState.BSPD_relay_status);

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
	 //small led display to indicate board startup
	  HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, 1);
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, 1);
	  HAL_Delay(300);
	  HAL_GPIO_WritePin(LD1_GPIO_Port,LD1_Pin, 0);
	  HAL_Delay(300);
	  HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin, 0);
	  HAL_Delay(300);
	  HAL_GPIO_WritePin(LD3_GPIO_Port,LD3_Pin, 0);

	  // display status LED's for two seconds to indicate power on.
	  setOutput(1,1);
	  setOutput(2,1);
	  setOutput(3,1);

	 // HAL_Delay(2000);
	  HAL_Delay(500);
	  for(int i=1;i<=12;i++){
		  setOutput(i, 0);
	  }
}
