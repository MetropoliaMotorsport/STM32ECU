/*
 * input.c
 *
 *  Created on: 14 Apr 2019
 *      Author: drago
 */

#include "ecumain.h"

// variables for button debounce interrupts, need to be global to be seen in timer interrupt
static uint16_t ButtonpressPin;


int checkReset( void )
{
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 1;
	}

	if ( CanState.ECU.newdata )
	{ // check ECU can data for reset message.
		CanState.ECU.newdata = 0; // we've seen message in error state loop.
		if ( ( CanState.ECU.data[0] == 0x99 ) && ( CanState.ECU.data[1] == 0x99 ))
		{
			return 1;
		}
	}

	if(Input[6].pressed != 0){ //StartStop_Switch
			Input[6].pressed = 0;
	//		blinkOutput(TSOFFLED_Output,LEDBLINK_FOUR,1);
			return 1;
	}

	return 0;
}



int CheckActivationRequest( void )
{
#ifdef ONBOARDBUTTON
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 0;
	}
#endif

	if(Input[6].pressed != 0){ //StartStop_Switch
			Input[StartStop_Switch].pressed = 0;
	//		blinkOutput(TSOFFLED_Output,LEDBLINK_FOUR,1);
			return 1;
	}

	return 0;
}


int CheckLimpActivationRequest( void )
{
#ifdef ONBOARDBUTTON
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 0;
	}
#endif

	// could be a can message or other source of activation request also.

	// eg, in future for driverless.

	return 0;
}


int CheckTSActivationRequest( void )
{
#ifdef ONBOARDBUTTON
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 0;
	}
#endif

	if(Input[4].pressed){
				Input[4].pressed = 0;
		//		blinkOutput(TSLED_Output,LEDBLINK_FOUR,1);
				return 1;
	}


	// could be a can message or other source of activation request also.

	// eg, in future for driverless.

	else return 0;
}


int CheckRTDMActivationRequest( void )
{
#ifdef ONBOARDBUTTON
	if(Input[UserBtn].pressed){
			Input[UserBtn].pressed = 0;
			return 0;
	}
#endif
//	if(Input[RTDM_Switch].pressed){
//			Input[RTDM_Switch].pressed = 0;
	if(Input[2].pressed){ // RTDM_Switch
				Input[2].pressed = 0;

	//		blinkOutput(RTDMLED_Output,LEDBLINK_FOUR,1);
			return 1;
	}


	// could be a can message or other source of activation request also.

	// eg, in future for driverless.

	else return 0;
}


/**
 * @brief only process button input if input registered
 */
void debouncebutton( volatile struct ButtonData *button )
{
		if( !button -> pressed )
		{ // only process new button press if last not read
				if(!HAL_GPIO_ReadPin(button -> port, button -> pin ) ) // switch to inverted for pull down buttons.
				{ // only process as input if button down
					button -> pressed = 1;
					button -> lastpressed=gettimer();
					button -> count++;
				} else
				{
					if (button->pin == Input[0].pin) // user button pressed, exception, this presses on high.
					{
						button -> pressed = 1;
						button -> lastpressed=gettimer();
						button -> count++;
					}
				}

		}
}

/**
 * @brief set status of a button to not activated
 */
void resetButton( uint8_t i )
{
	Input[i].lastpressed = 0;
	Input[i].count = 0;
	Input[i].pressed = 0;
}

/**
 * startup routine to ensure all buttons are initialised to unset state.
 */

void setupButtons(void)
{
	Input[UserBtn].port = USER_Btn_GPIO_Port;
	Input[UserBtn].pin = USER_Btn_Pin;

/*	Input[Input1].port = Input1_GPIO_Port;
	Input[Input1].pin = Input1_Pin;

	Input[Input2].port = Input2_GPIO_Port; // red button.
	Input[Input2].pin = Input2_Pin;

	Input[Input3].port = Input3_GPIO_Port;
	Input[Input3].pin = Input3_Pin;

	Input[Input4].port = Input4_GPIO_Port; ???? Input 4 = Green button.
	Input[Input4].pin = Input4_Pin; */

	Input[Input5].port = Input5_GPIO_Port;
	Input[Input5].pin = Input5_Pin;

	Input[Input6].port = Input6_GPIO_Port; // yellow button.
	Input[Input6].pin = Input6_Pin;

/*	Input[Input7].port = Input7_GPIO_Port;
	Input[Input7].pin = Input7_Pin;
*/
	Input[Input8].port = Input8_GPIO_Port;
	Input[Input8].pin = Input8_Pin;

	for ( int i=0; i < 9; i++)
	{
		resetButton(i);
	}
}


void clearButtons(void)
{
	for ( int i=0; i < 9; i++)
	{
		Input[i].pressed = 0;
	}
}

/**
 * @brief Interrupt handler fired when a button input line is triggered
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (!InButtonpress ) { // do nothing if already in middle of processing a previous button input
		// should ideally be able to manage multiple at once, with seperate timer channels, or timers.
		InButtonpress = 1; // stop processing further button presses till debounce time given
		ButtonpressPin = GPIO_Pin; // assign the button input being handled so it can be referenced in from timer interrupt

		if ( HAL_TIM_Base_Start_IT(&htim7) != HAL_OK){
			Error_Handler();
		}

	}
}


void InputTimerCallback( void )
{
	  // timer7, being used for button input debouncing
		InButtonpress = 0;  // reset status of button processing after timer elapsed,
							// to indicate not processing input to allow triggering new timer.
		switch ( ButtonpressPin )
		{  // process the button that was pressed to start the debounce timer.

			case USER_Btn_Pin :
				debouncebutton(&Input[UserBtn]);
				break;
			case Input1_Pin:
				debouncebutton(&Input[Input1]);
				break;
			case Input2_Pin:
				debouncebutton(&Input[Input2]);
				break;
			case Input3_Pin:
				debouncebutton(&Input[Input3]);
				break;
			case Input4_Pin:
				debouncebutton(&Input[Input4]);
				break;
			case Input5_Pin:
				debouncebutton(&Input[Input5]);
				break;
			case Input6_Pin:
				debouncebutton(&Input[Input6]);
				break;
			case Input7_Pin:
				debouncebutton(&Input[Input7]);
				break;
			case Input8_Pin:
				debouncebutton(&Input[Input8]);
				break;
			default : // shouldn't get here, but catch and ignore any other input
				break;
		}
}
