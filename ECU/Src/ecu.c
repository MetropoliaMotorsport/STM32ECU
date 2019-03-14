/*
 * ecu.c
 *
 *  Created on: 30 Dec 2018
 *      Author: Visa
 */

#include "fdcan.h"
#include "tim.h"
#include "adc.h"
#include "ecu.h"
#include "vhd44780.h"

//variables that need to be accessible in ISR's

// variables for button debounce interrupts, need to be global to be seen in timer interrupt
volatile static char InButtonpress;
static uint16_t ButtonpressPin;

// ADC conversion buffer/
ALIGN_32BYTES (static uint32_t aADCxConvertedData[ADC_CONVERTED_DATA_BUFFER_SIZE]);



/** deal with endianness for canbus
 * function from https://stackoverflow.com/questions/39622332/reading-big-endian-files-in-little-endian-system
 * not actually being currently used
 */
void swapByteOrder_int16(double *current, const int16_t *rawsignal, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        int16_t x = rawsignal[2*i];
        x = (x*1u << 8) | (x*1u >> 8);
        current[i] = x;
    }
}

/**
 * function to perform a linear interpolation using given input/output value arrays and raw data.
 */
int16_t linearInteropolate(uint16_t Input[], int16_t Output[], uint16_t count, uint16_t RawADCInput)
{
    int i;

    if(RawADCInput < Input[0])
    {  // if input less than first table value return first.
        return Output[0];
    }

    if(RawADCInput > Input[count-1])
    { // if input larger than last table value return last.
        return Output[count-1];
    }

    // loop through input values table till we find space where requested fits.
    for (i = 0; i < count-1; i++)
    {
        if (Input[i+1] > RawADCInput)
        {
            break;
        }
    }

    int dx,dy;

    /* interpolate */
    dx = Input[i+1] - Input[i];
    dy = Output[i+1] - Output[i];
    return Output[i] + ((RawADCInput - Input[i]) * dy / dx);
}

/**
 * convert raw steering ADC input into a steering percentage left/right.
 * -- is there a deadzone?
 */
int8_t getSteeringAngle(uint16_t RawADCInput)
{
	// calibrated input range for steering, from left lock to center to right lock.
	// check if this can be simplified?
    static uint16_t SteeringInput[] = { 1210,1270,1320,1360,1400,1450,1500,1540,1570,1630,1680,1720,1770,2280,2700,3150,3600,4100,4700,5000,5500 };
    // output steering range. +-100%
    static int16_t SteeringOutput[] = { -100,-90,-80,-70,-60,-50,-40,-30,-20,-10,0,10,20,30,40,50,60,70,80,90,100 };
    static int count = sizeof(SteeringInput)/sizeof(SteeringInput[0]);

    return linearInteropolate(SteeringInput, SteeringOutput, count, RawADCInput);
}

/**
 * convert raw front brake reading into calibrated brake position
 */
uint8_t getBrakeF(uint16_t RawADCInput)
{
    static uint16_t BrakeFInput[] = { 385, 1940 }; // calibrated input range
    static int16_t BrakeFOutput[] = { 0, 250 }; // output range
    static int count = sizeof(BrakeFInput)/sizeof(BrakeFInput[0]);

    return linearInteropolate(BrakeFInput, BrakeFOutput, count, RawADCInput);
}

/**
 * convert raw rear brake reading into calibrated brake position
 */
uint8_t getBrakeR(uint16_t RawADCInput)
{
    static uint16_t BrakeRInput[] = { 380, 1915 }; // calibrated input range
    static int16_t BrakeROutput[] = { 0, 250 }; // output range
    static int count = sizeof(BrakeRInput)/sizeof(BrakeRInput[0]);

    return linearInteropolate(BrakeRInput, BrakeROutput, count, RawADCInput);
}

/**
 * convert front/rear brake inputs into brake balance percentage.
 */

uint32_t getBrakeBalance(uint16_t RawADCInputF, uint16_t RawADCInputR)
{
	// should this be with raw values?
	return (RawADCInputF * 100) / ( RawADCInputF + RawADCInputR );
}


void setTorqueReqPerc(uint16_t RawADCInputL, uint16_t RawADCInputR)
{
    static uint16_t TorqueReqLInput[] = { 140, 660 }; // calibration values for left input
    static int16_t TorqueReqLOutput[] = { 0, 100 };
    static int countL = sizeof(TorqueReqLInput)/sizeof(TorqueReqLInput[0]);

    static uint16_t TorqueReqRInput[] = { 250, 710 }; // calibration values for right input
    static int16_t TorqueReqROutput[] = { 0, 100 };
    static int countR = sizeof(TorqueReqRInput)/sizeof(TorqueReqRInput[0]);

    if( RawADCInputL >= TorqueReqLInput[0] && RawADCInputR > 0 ) // RawADCInputR >= TorqueReqRInput[0] check this, looks wrong to me in simulink
    {
    	ADCState.Torque_Req_L_Percent = linearInteropolate(TorqueReqLInput, TorqueReqLOutput, countL, RawADCInputL);
    	ADCState.Torque_Req_R_Percent = linearInteropolate(TorqueReqRInput, TorqueReqROutput, countR, RawADCInputR);
    	ADCState.Torque_Req_L = ADCState.Torque_Req_L_Percent * CarState.Torque_Req_Max * 0.01;
    	ADCState.Torque_Req_R = ADCState.Torque_Req_R_Percent * CarState.Torque_Req_Max * 0.01;
    } else
    {
    	ADCState.Torque_Req_L_Percent = 0;
    	ADCState.Torque_Req_R_Percent = 0;
    	ADCState.Torque_Req_L = 0;
    	ADCState.Torque_Req_R = 0;
    }

}

/**
 * process driving mode selector input, adjusts max possible torque request. Equivalent to gears.
 */

uint8_t getDrivingMode(uint16_t RawADCInput)
{ // torq_req_max, call once a second

	// why is there not 1600,65 to keep hard steps

    static const uint16_t DrivingModeInput[] = { 100,300,470,690,900,1200,1600,1800 };
    static const uint8_t DrivingModeOutput[] = { 5,10,15,20,25,30,45,65 };

    static const int count = sizeof(DrivingModeInput)/sizeof(DrivingModeInput[0]);

    int i;

    for(i=0;(DrivingModeInput[i] < RawADCInput) && (i < count-1);i++) { };

    //return this position in output table, no interpolation needed

    return DrivingModeOutput[i];

}

uint8_t getCoolantTemp(uint16_t RawADCInput)
{
    static uint16_t CoolantInput[] = { 53,67,85,109,140,182,239,313,413,537,698,882,1095,1313 };
    static int16_t CoolantOutput[] = { 130,120,110,100,90,80,70,60,50,40,30,20,10,0 };
    static int count = sizeof(CoolantInput)/sizeof(CoolantInput[0]);

    if ( RawADCInput < CoolantInput[0] ) { return 0; }

    if ( RawADCInput > CoolantInput[count-1] ) { return 0; }

    return linearInteropolate(CoolantInput, CoolantOutput, count, RawADCInput);
}

/**
 * returns gpio port for given output number.
 */

GPIO_TypeDef* getGpioPort(int output)
{
	switch ( output ) { // set gpio values for requested port
		case 1 :
			return Output1_GPIO_Port;
		case 2 :
			return Output2_GPIO_Port;
		case 3 :
			return Output3_GPIO_Port;
		case 4 :
			return Output4_GPIO_Port;
		case 5 :
			return Output5_GPIO_Port;
		case 6 :
			return Output6_GPIO_Port;
		case 7 :
			return Output7_GPIO_Port;
		case 8 :
			return Output8_GPIO_Port;
		case 13 :
			return LD1_GPIO_Port;
		case 14 :
			return LD2_GPIO_Port;
		case 15 :
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
		case 1 :
			return Output1_Pin;
		case 2 :
			return Output2_Pin;
		case 3 :
			return Output3_Pin;
		case 4 :
			return Output4_Pin;
		case 5 :
			return Output5_Pin;
		case 6 :
			return Output6_Pin;
		case 7 :
			return Output7_Pin;
		case 8 :
			return Output8_Pin;
		case 13 :
			return LD1_Pin;
		case 14 :
			return LD2_Pin;
		case 15 :
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
	if(getGpioPin(output) != 0)
	{
		HAL_GPIO_WritePin(getGpioPort(output), getGpioPin(output), state);
	}
}

/**
 * @brief Toggles the state of specified GPIO output using programs defined input numbering
 */
void toggleOutput(int output)
{
	if(getGpioPin(output) != 0)
	{
		HAL_GPIO_TogglePin(getGpioPort(output), getGpioPin(output));
	}
}

/**
 * @brief setup start state of LED's to off.
 */
void setupLEDs( void )
{
	LED1.blinking = 0;
	LED1.pin = LD1_Pin;

	LED2.blinking = 0;
	LED2.pin = LD2_Pin;

	LED3.blinking = 0;
	LED3.pin = LD3_Pin;

	TS_LED.blinking = 0;
	TS_LED.pin = TSALLED_Output;

	RTDM_LED.blinking = 0;
	RTDM_LED.pin = RTDMLED_Output;

	STOP_LED.blinking = 0;
	STOP_LED.pin = STOPLED_Output;

	BMS_LED.blinking = 0;
	BMS_LED.pin = BMSLED_Output;

	IMD_LED.blinking = 0;
	IMD_LED.pin = IMDLED_Output;

	BSPD_LED.blinking = 0;
	BSPD_LED.pin = BSPDLED_Output;
}

/**
 * @brief set the current state of LED's as defined by carstate variables.
 */
void setLEDs( void )
{

	// Check. 10 second delay for IMD led in simulink. IMD Light power off delay. missed earlier, significance?

	setOutput(BMS_LED.pin, CarState.BMS_relay_status);
	setOutput(IMD_LED.pin, CarState.IMD_relay_status);
	setOutput(BSPD_LED.pin, CarState.BSPD_relay_status);

	if ( CarState.TSALLeftInvLED == 1 && CarState.TSALRightInvLED == 1 )
	{
		TS_LED.blinking = 1; // cockpit led
	} else if ( CarState.TSALLeftInvLED >= 2 && CarState.TSALRightInvLED >= 2)
	{
		setOutput(TS_LED.pin, 1);
		TS_LED.blinking = 0;

	} else
	{
		setOutput(TS_LED.pin, 0);
		TS_LED.blinking = 0;
	}

	if ( CarState.RtdmLeftInvLED == 1 && CarState.RtdmRightInvLED == 1 )
	{
			RTDM_LED.blinking = 1;
	} else if ( CarState.RtdmLeftInvLED >= 2 && CarState.RtdmRightInvLED >= 2)
	{
		setOutput(RTDM_LED.pin, 1);
		RTDM_LED.blinking  = 0;

	} else
	{
		setOutput(RTDM_LED.pin, 0);
		RTDM_LED.blinking  = 0;
	}

	setOutput(STOP_LED.pin, CarState.StopLED);
}

/**
 * @brief set status of a button to not activated
 */
void resetButton( struct ButtonData button )
{
	button.lastpressed = 0;
	button.count = 0;
	button.pressed = 0;
}

/**
 * returns total 0.1ms since turnon to use as comparison timestamp
 */
volatile uint32_t gettimer(void)
{
	 return (secondson*10000) + __HAL_TIM_GetCounter(&htim3);
}

/**
 * @brief only process button input if input registered
 */
void debouncebutton( volatile struct ButtonData *button )
{
		if( !button -> pressed )
		{ // only process new button press if last not read
				if(HAL_GPIO_ReadPin(button -> port, button -> pin ) )
				{ // only process as input if button down
					button -> pressed = 1;
					button -> lastpressed=gettimer();
					button -> count++;
			}
		}
}

/**
 * startup routine to ensure all buttons are initialised to unset state.
 */

void setupButtons(void)
{
	UserBtn.port = USER_Btn_GPIO_Port;
	UserBtn.pin = USER_Btn_Pin;
	Input1.port = Input1_GPIO_Port;
	Input1.pin = Input1_Pin;
	Input2.port = Input2_GPIO_Port;
	Input2.pin = Input2_Pin;
	Input3.port = Input3_GPIO_Port;
	Input3.pin = Input3_Pin;
	resetButton(UserBtn);
	resetButton(Input1);
	resetButton(Input2);
	resetButton(Input3);
	resetButton(Input4);
	resetButton(Input5);
	resetButton(Input6);
}

/**
 * @brief set filter states and start CAN module and it's interrupt for CANBUS1
 */
void FDCAN1_start(void)
{
  FDCAN_FilterTypeDef	sFilterConfig1;

  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    // Initialization Error
    Error_Handler();
  }

  if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK) // if can2 not initialised before filters set they seem to be lost
  {
    // Initialization Error
    Error_Handler();
  }

  HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, DISABLE, DISABLE);

  // Configure Rx filter to only accept expected ID's into receive interrupt
  sFilterConfig1.IdType = FDCAN_STANDARD_ID; // standard, not extended FD frame filter.
  sFilterConfig1.FilterType = FDCAN_FILTER_RANGE; // filter all the id's between id1 and id2 in filter definition.
  sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // FDCAN_FILTER_TO_RXFIFO0; // set can1 to receive via fifo0

  sFilterConfig1.FilterIndex = 1;
  sFilterConfig1.FilterID1 = 0xF; // 0xf 0x0 for all
  sFilterConfig1.FilterID2 = 0xF; // 07ff  0x0 for all


  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  sFilterConfig1.FilterIndex++; // filter for
  sFilterConfig1.FilterID1 = 0x520;
  sFilterConfig1.FilterID2 = 0x523;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  sFilterConfig1.FilterIndex++; // filter for fake canbus ADC id's
  sFilterConfig1.FilterID1 = 0x600;
  sFilterConfig1.FilterID2 = 0x605;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  sFilterConfig1.FilterIndex++; // filter for fake button presses id's + induce hang.
  sFilterConfig1.FilterID1 = 0x610;
  sFilterConfig1.FilterID2 = 0x614;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  /* Start the FDCAN module */
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
  {
    // Start Error
    Error_Handler();
  }

  // start can receive interrupt for CAN1
  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
  {
    // Notification Error
    Error_Handler();
  }

  // Prepare Tx Headers

  // header for sending time base
  TxHeaderTime.Identifier = 0x100;
  TxHeaderTime.IdType = FDCAN_STANDARD_ID;
  TxHeaderTime.TxFrameType = FDCAN_DATA_FRAME;
  TxHeaderTime.DataLength = FDCAN_DLC_BYTES_3;
  TxHeaderTime.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeaderTime.BitRateSwitch = FDCAN_BRS_OFF;
  TxHeaderTime.FDFormat = FDCAN_CLASSIC_CAN;
  TxHeaderTime.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  TxHeaderTime.MessageMarker = 0;

  // general purpose debug tx header
  TxHeader1.Identifier = 0x101;
  TxHeader1.IdType = FDCAN_STANDARD_ID;
  TxHeader1.TxFrameType = FDCAN_DATA_FRAME;
  TxHeader1.DataLength = FDCAN_DLC_BYTES_8;
  TxHeader1.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeader1.BitRateSwitch = FDCAN_BRS_OFF;
  TxHeader1.FDFormat = FDCAN_CLASSIC_CAN;
  TxHeader1.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  TxHeader1.MessageMarker = 0;

  TxHeader2.Identifier = 0x102;
  TxHeader2.IdType = FDCAN_STANDARD_ID;
  TxHeader2.TxFrameType = FDCAN_DATA_FRAME;
  TxHeader2.DataLength = FDCAN_DLC_BYTES_8;
  TxHeader2.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeader2.BitRateSwitch = FDCAN_BRS_OFF;
  TxHeader2.FDFormat = FDCAN_CLASSIC_CAN;
  TxHeader2.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  TxHeader2.MessageMarker = 0;

}

void FDCAN2_start(void)
{
  FDCAN_FilterTypeDef sFilterConfig2;

#ifdef ONECAN
  // hfdcan2 = hfdcan1;
#endif

 /* if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK) // initted already in fdcan1_start
  {
    // Initialization Error
    Error_Handler();
  } */


  HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_REJECT, FDCAN_REJECT, DISABLE, DISABLE);

  // Configure Rx filter for can2
  sFilterConfig2.IdType = FDCAN_STANDARD_ID;
  sFilterConfig2.FilterIndex = 64;
  sFilterConfig2.FilterType = FDCAN_FILTER_RANGE;
  sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO1; // set can2 to receive into fifo1
  sFilterConfig2.FilterID1 = 0x1;
  sFilterConfig2.FilterID2 = 0x1;

//  HAL_FDCAN_ConfigInterruptLines

  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }


  sFilterConfig2.FilterIndex++;
  sFilterConfig2.FilterID1 = 0x1FE;
  sFilterConfig2.FilterID2 = 0x1FF;

  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  sFilterConfig2.FilterIndex++;
  sFilterConfig2.FilterID1 = 0x2FE;
  sFilterConfig2.FilterID2 = 0x2FF;

  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  sFilterConfig2.FilterIndex++;
  sFilterConfig2.FilterID1 = 0x77E;
  sFilterConfig2.FilterID2 = 0x77F;

  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

  sFilterConfig2.FilterIndex++;
  sFilterConfig2.FilterID1 = 0xF;
  sFilterConfig2.FilterID2 = 0xF;

  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK)
  {
    // Filter configuration Error
    Error_Handler();
  }

#ifndef ONECAN
  // Start the FDCAN module
  if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK)
  {
    //  Start Error
    Error_Handler();
  }
#endif

  // start can receive interrupt for second can's messages
  if (HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO1_NEW_MESSAGE , 0) != HAL_OK)
  {
    // Notification Error
    Error_Handler();
  }

}

void resetCanTx(uint8_t CANTxData[8])
{
	for(int i = 0;i < 8;i++){
		CANTxData[i]=0;
	}
}

char CAN1Send( FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData )
{
	// loop till free slot if none, rather than give up as currently set. Let watchdog catch if gets into loop unable to send?
	// perhaps have two send routines, critical and non critical.

	while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) < 1 )
	{
// wait for can buffer slot.
	}
	if (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) > 0)
	{

		if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, pTxHeader, pTxData) != HAL_OK)
		{
		          // Transmission request Error
  			 //     HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin, 1);
			return 1;
  //			      HAL_Delay(1);
		    Error_Handler();
		}
	}
	return 0;
}

char CAN2Send( FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData )
{
	while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2) < 1 )
	{
// wait for can slot.
	}
	if (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2) > 0)
	{
		if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, pTxHeader, pTxData) != HAL_OK)
		{
		          /* Transmission request Error */
			//HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin, 1);
			return 1;

			Error_Handler();
		}
	}
	return 0;
}


char CANKeepAlive( void )
{
	// send can id 0x80 to can 0 value 1. Marked Info to PDM in Simulink. Call once per second.
	// datalength?
	FDCAN_TxHeaderTypeDef TxHeaderPDM;

	TxHeaderPDM.Identifier = 0x80;
	TxHeaderPDM.IdType = FDCAN_STANDARD_ID;
	TxHeaderPDM.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderPDM.DataLength = FDCAN_DLC_BYTES_1; // only one byte defined, check this
	TxHeaderPDM.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderPDM.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderPDM.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderPDM.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderPDM.MessageMarker = 0;

	uint8_t CANTxData[1] = { 1 };
	return CAN1Send( &TxHeaderPDM, CANTxData );
}

char CANSendState( char buzz, char highvoltage )
{
	FDCAN_TxHeaderTypeDef TxHeaderHV;

	TxHeaderHV.Identifier = 0x118;
	TxHeaderHV.IdType = FDCAN_STANDARD_ID;
	TxHeaderHV.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderHV.DataLength = FDCAN_DLC_BYTES_2; // only two bytes defined in send protocol, check this
	TxHeaderHV.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderHV.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderHV.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderHV.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderHV.MessageMarker = 0;

	uint8_t CANTxData[8] = { highvoltage, buzz, 0, 0, 0, 0, 0, 0 };
	return CAN1Send( &TxHeaderHV, CANTxData );
}

char CAN_NMT( void )
{
	FDCAN_TxHeaderTypeDef TxHeaderNMT;

	TxHeaderNMT.Identifier = 0x0;
	TxHeaderNMT.IdType = FDCAN_STANDARD_ID;
	TxHeaderNMT.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderNMT.DataLength = FDCAN_DLC_BYTES_2; // only two bytes defined in send protocol, check this
	TxHeaderNMT.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderNMT.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderNMT.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderNMT.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderNMT.MessageMarker = 0;

	uint8_t CANTxData[2] = { 1,0 };
	return CAN2Send( &TxHeaderNMT, CANTxData );
}

char CANSendInverter( uint16_t response, uint16_t request, uint8_t inverter )
{
	FDCAN_TxHeaderTypeDef TxHeaderInverter;

	if(inverter==0)
	{
		TxHeaderInverter.Identifier = 0x47E;
	} else
	{
		TxHeaderInverter.Identifier = 0x47F;
	}

	TxHeaderInverter.IdType = FDCAN_STANDARD_ID;
	TxHeaderInverter.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderInverter.DataLength = FDCAN_DLC_BYTES_4; // only two bytes defined in send protocol, check this // four seen in logs
	TxHeaderInverter.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderInverter.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeaderInverter.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderInverter.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderInverter.MessageMarker = 0;

	uint8_t CANTxData[8];

	resetCanTx(CANTxData);

	storeLEint16(response,&CANTxData[0]);
	storeLEint16(request,&CANTxData[2]);

	return CAN2Send( &TxHeaderInverter, CANTxData );
}

char CANTorqueRequest( uint16_t request )
{
  if( AllowedToDrive()){
	  CANSendInverter(  CarState.LeftInvResponse, request, LeftInverter );
	  CANSendInverter(  CarState.RightInvResponse, request, RightInverter );
  } else
  {
	  CANSendInverter(  CarState.LeftInvResponse, 0, LeftInverter );
	  CANSendInverter(  CarState.RightInvResponse, 0, RightInverter );
  }
  return 0;
}

void RTDMCheck( void )
{
	// EV4.11.6 RTDM Check
	// Closing the shutdown circuit by any part defined in EV 6.1.2 must not (re-)activate the TS.
	// Additional action must be required.
	if (RTDM_Switch.pressed   // removed !, switch was being read as active when not latched.
	    	&& ADCState.BrakeR >= 40
			&& CarState.ReadyToDrive_AllowedR
			&& CarState.ReadyToDrive_AllowedL)
	{
		CarState.Buzzer_Sounding = 1;
		CarState.ReadyToDrive_Ready = 1;
		RTDM_Switch.pressed = 0;
	} else
	{
		RTDM_Switch.pressed = 0; // reset switch if we didn't meet allowed enable state.
	}

// this is bouncing between states for some reason, debug.
	if( TS_Switch.pressed      // debounce needs to be changed to show button held or released? // removed !
			&& CarState.HighVoltageOn_AllowedR
		    && CarState.HighVoltageOn_AllowedL
		    && !CarState.IMD_relay_status
		    && !CarState.BMS_relay_status
		    && !CarState.BSPD_relay_status)
		{
			CarState.HighVoltageOn_Ready = 10;
			TS_Switch.pressed =  0;
		} else
		{
			TS_Switch.pressed =  0; // reset switch if we didn't meet allowed turn on state.

		}

	if ( CarState.IMD_relay_status
	    || CarState.BMS_relay_status
	    || CarState.BSPD_relay_status)
		{
			CarState.HighVoltageOn_Ready = 0;
			CarState.ReadyToDrive_Ready = 0;
		}

	if ( StopMotors_Switch.pressed ) // request TS off
	{
		CarState.HighVoltageOn_Ready = 0;
		CarState.ReadyToDrive_Ready = 0;
		CarState.Buzzer_Sounding = 0;
		CANTorqueRequest(0);
		StopMotors_Switch.pressed = 0;
		TS_Switch.pressed =  0;
		RTDM_Switch.pressed =  0;
	}

	if ( CarState.ReadyToDrive_Ready && (( Speed_Right_Inverter.time + 2000  < gettimer() ) || ( Speed_Left_Inverter.time + 2000  < gettimer() ) ) )
	{
		CarState.ReadyToDrive_Ready = 0; // not heard from inverters in 0.2 seconds, halt request
		CarState.HighVoltageOn_Ready = 0;
		// request full stop.
	}

		// output 10 to can0 0x118 offset 0 && HighVoltageOn_Ready state for PDM

		CANSendState(CarState.Buzzer_Sounding, CarState.HighVoltageOn_Ready);
}

/*
 * APPS Check
 *
 * Direct translation of Simulink code, look to rewrite?
 */
uint16_t PedalTorqueRequest( void )
{

	//T 11.8.8:  If an implausibility occurs between the values of the APPSs and persists for more than 100 ms

	//[EV ONLY] The power to the motor(s) must be immediately shut down completely.
	//It is not necessary to completely deactivate the tractive system, the motor controller(s)
	// shutting down the power to the motor(s) is sufficient.

	// current code immediately orders no torque from motors if check fails.

	static char No_Torque_Until_Pedal_Released = 0; // persistent local variable
	uint16_t Torque_drivers_request = 0;

	//The absolute value of the difference between the APPS (Accelerator Pedal Position Sensors)

	int difference = abs(ADCState.Torque_Req_L_Percent - ADCState.Torque_Req_R_Percent);

	//The average value of the APPS
	int AverageTorqueRequestPercent = (ADCState.Torque_Req_L_Percent + ADCState.Torque_Req_R_Percent) /2;

	char status=0;

	//   -Implausibility Test Failure : In case of more than 10 percent implausbility between the APPS,torque request is 0
	//   -Implausbility allowed : more than 10 percent
	//   -Brake Pressure allowed : less than 65
	//   -Torque-Brake Violation : Free
	if( difference>10
	    || (ADCState.Torque_Req_R_Percent==0 && ADCState.Torque_Req_L_Percent>0)
		|| (ADCState.Torque_Req_L_Percent==0 && ADCState.Torque_Req_R_Percent >0) )
	{
	    Torque_drivers_request = 0;
	    status=1;
	}

	//   -Normal Driving Conditions
	//   -Implausbility allowed : less or equal to 10 percent
	//   -Brake Pressure allowed : less than 65
	//   -Torque-Brake Violation : Free

	else if( difference<=10
			&& ADCState.BrakeR < 30
			&& No_Torque_Until_Pedal_Released==0 )
	{
		Torque_drivers_request = ADCState.Torque_Req_L*1000/65;
	    status=2;
	}
	//   -Torque-Brake Violation : Accelerator Pedal and Brake Pedal pressed at the same time
	//   -Accelerator Pedal Travel : More than 25 percent
	//   -Brake Pressure allowed : more than 140
	//   -Torque-Brake Violation : Occured and marked

	else if( difference<=10
			 && ADCState.BrakeR > 70
			 && AverageTorqueRequestPercent>=25 )
	{
		Torque_drivers_request=0;
		No_Torque_Until_Pedal_Released=1;
	    status=3;
	}

	//   -After torque-brake violation :  Even if brake pedal is released, and the APPS are more than 5 percent, no torque is allowed
	//   -Accelerator Pedal Travel : More than 5 percent
	//   -Brake Pressure allowed : less than 65
	//   -Torque-Brake Violation : Still exists and won't be freed
	else if( difference<=10
			 && ADCState.BrakeR  < 30
			 && No_Torque_Until_Pedal_Released==1
			 && AverageTorqueRequestPercent >=5 )
	{
		Torque_drivers_request=0;
	    status=4;
	}

	//   -After torque-brake violation and the release of the brake pedal, torque is allowed when APPS are less than 5 percent
	//   -Accelerator Pedal Travel : Less than 5 percent
	//   -Brake Pressure allowed : less than 65
	//   -Torque-Brake Violation : Occured and will be freed

	else if ( difference<=10
			  && ADCState.BrakeR  < 30
			  && No_Torque_Until_Pedal_Released==1
			  && AverageTorqueRequestPercent < 5 )
	{
		No_Torque_Until_Pedal_Released=0;
	    Torque_drivers_request = ADCState.Torque_Req_L*1000/65;
	    status=5;
	}

	//  -Any other undefined condition, should never end up here

	else
	{
	    Torque_drivers_request=0;
	    status=6;
	}

	// can send status

	return Torque_drivers_request;

//	if(ADCState.)
}

char AllowedToDrive( void )
{

	if(CarState.LeftInvResponse == 15 && CarState.RightInvResponse == 15)
	{
		return 1;
	} else
	{
		return 0;
	}
}


uint8_t GetInverterState( uint16_t Status )
{
	// establish current state machine position from return status.
	if ( ( Status & 0b01001111 ) == 0b01000000 )
	{ // Switch on disabled
		return 1;
	}
	else if ( ( Status & 0b01101111 ) == 0b00100001 )
	{ // Ready to switch on
		return 2;
	}
	else if ( ( Status & 0b01101111 ) == 0b00100011 )
	{ // Switched on
		return 3;
	}
	else if ( ( Status & 0b01101111 ) == 0b00100111 )
	{ // Operation enabled. - perhaps move this to start of if/else chain, as it's primary state for loop.
		return 4;
	}
	else if ( ( ( Status & 0b01101111 ) == 0b00000111 )
			 || ( ( Status & 0b00011111 ) == 0b00010011 ) )
	{ // Quick Stop Active
		return 5;
	}
	else if  ( ( ( Status & 0b01001111 ) == 0b00001111 )
			 || ( ( Status & 0b01001111 ) == 0b00001001 ) )
	{ // fault reaction active, will move to fault status next
		return 98;
	}
	else if  ( ( ( Status & 0b01001111 ) == 0b00001000 )
			 || ( ( Status & 0b00001000 ) == 0b0001000 ) )
	{ // fault status
		return 99;
		// send reset
	} else
	{ // unknown state
		return 0; // state 0 will request reset to enter State 1,
		// will fall here at start of loop and if unknown status.
	}
}

/*
 * Direct translation of Simulink code, look to rewrite.
 */
char InverterStateMachine( int8_t Inverter )
{
	uint16_t Status, TXStatus;
	uint8_t State;

	char HighVoltageOnAllowed, ReadyToDriveAllowed, TsLED, RtdmLED;

	if( Inverter == 0 ) // left inverter
	{
		Status = Status_Left_Inverter.data.longint;
	}
	else if ( Inverter == 1 ) // right inverter
	{
		Status = Status_Right_Inverter.data.longint;
	}
	else return 0; // invalid inverter

	// first check for fault status, and issue reset.

	TXStatus = 0; // default  do nothing state.
	// process regular state machine sequence
	switch ( GetInverterState(Status) )
	{
		case 0 : // state 0: Not ready to switch on, no can message. Internal state only at startup.
			HighVoltageOnAllowed = 0;  // High Voltage is not allowed
			ReadyToDriveAllowed = 0;  // Ready to drive is not allowed
			TsLED = 0;
			RtdmLED = 0; // No LED's for state 0
			TXStatus=0b10000000; // send bit 128 reset message to enter state 1 in case in fault.
			break;

		case 1 : // State 1: Switch on Disabled.
			HighVoltageOnAllowed = 0;
			ReadyToDriveAllowed = 0;
			TsLED = 0;
			RtdmLED = 0;
			TXStatus = 0b00000110; // send 0110 shutdown message to request move to State 2.
			break;

		case 2 : // State 2: Ready to switch on
			HighVoltageOnAllowed = 1; // We are ready to turn on, so allow high voltage.
			ReadyToDriveAllowed = 0; // RTDM not allowed still inverter is switched on.
			TsLED = 1; // start blinking TS led to indicate it can be enabled.
			RtdmLED = 0;

			// we are in state 2, process.
			// process shutdown request here, to move to move to state 1.
			if ( CarState.HighVoltageOn_Ready )
			{  // TS enable button pressed and both inverters are marked HV ready proceed to state 3.
				TXStatus = 0b00000111; // request Switch on message, State 3..
			} else
			{
				TXStatus = 0b00000110; // no change, continue to request State 2.
			}
			break;

		case 3 : // State 3: Switched on
			HighVoltageOnAllowed = 1;  // we are powered on, so allow high voltage.
			ReadyToDriveAllowed = 1; // powered on, but not enabled, so not RTDM.
			TsLED = 2; // steady TS led to indicate TS powered on.
			RtdmLED = 1; // start blinking RTDM led to indicate it can be enabled.
			if ( CarState.HighVoltageOn_Ready && CarState.ReadyToDrive_Ready)
			{  // TS enable button has been pressed, proceed to request power on if both inverters on.
				TXStatus = 0b00001111; // Request Enable operation, State 4.
			}
			else if ( !CarState.HighVoltageOn_Ready )
			{ // return to switched on state.
				TXStatus = 0b00000000; // request Disable Voltage., alternately Quick Stop 0b00000010
			}
			else
			{  // no change, continue to request State 3.
				TsLED = 2;
				TXStatus = 0b00000111;
			}
			break;

		case 4 : // State 4: Operation Enable
			HighVoltageOnAllowed = 1; // we are powered on, so allow high voltage.
			ReadyToDriveAllowed = 1; // inverters are enabled so we have to be in RTDM, and all LED's on.
			TsLED = 2;
			RtdmLED = 2;
			if ( CarState.HighVoltageOn_Ready &&  !CarState.ReadyToDrive_Ready)
			{ // taken out of ready to drive, move state down.
				TXStatus = 0b00000111; // request state 3: Switched on.
			}
			else if ( !CarState.HighVoltageOn_Ready )
			{ // full motor stop has been rewuested
				TXStatus = 0b00000000; // request Disable Voltage., alternately Quick Stop 0b00000010
			}
			else
			{ // no change, continue to request operation.
				TXStatus = 0b00001111;
			}
			break;

		case 5 : // Quick Stop Active - Fall through to default to reset state.

		case 98 : // Fault Reason Active

		case 99 : // Fault

		default : // unknown identifier encountered, ignore. Shouldn't be possible to get here due to filters.
			HighVoltageOnAllowed = 0;
			ReadyToDriveAllowed = 0;
			TsLED = 0;
			RtdmLED = 0;
			TXStatus = 0b10000000; // 128
			break;
		}

	//  offset 0 length 32: power

	if( Inverter == 0 ) // left inverter
	{
		CarState.HighVoltageOn_AllowedL=HighVoltageOnAllowed;
		CarState.ReadyToDrive_AllowedL=ReadyToDriveAllowed;
		CarState.RtdmLeftInvLED = RtdmLED;
		CarState.TSALLeftInvLED = TsLED;
		CarState.LeftInvResponse = TXStatus;

	} else if ( Inverter == 1 ) // right inverter   // disabled so both inverter statuses mirrored for testing.
	{
		CarState.HighVoltageOn_AllowedR=HighVoltageOnAllowed;
		CarState.ReadyToDrive_AllowedR=ReadyToDriveAllowed;
		CarState.RtdmRightInvLED = RtdmLED;
		CarState.TSALRightInvLED = TsLED;
		CarState.RightInvResponse = TXStatus;
	}

	return 1;
}

void processCANData( void )
{
// just move this to can interrupt? redundant procedure.
	if(IMD_relay_status.newdata){
		IMD_relay_status.newdata = 0;
		CarState.IMD_relay_status = IMD_relay_status.data.longint;
	}

	if(BMS_relay_status.newdata){
		BMS_relay_status.newdata = 0;
		CarState.BMS_relay_status = BMS_relay_status.data.longint;
	}

	if(BSPD_relay_status.newdata){
		BSPD_relay_status.newdata = 0;
		CarState.BSPD_relay_status = BSPD_relay_status.data.longint;
	}
}

void RearSpeedCalculation( void )
{
	CarState.Wheel_Speed_Right_Calculated = Speed_Right_Inverter.data.longint * (1/4194304) * 60;
	CarState.Wheel_Speed_Left_Calculated = Speed_Left_Inverter.data.longint * (1/4194304) * 60;
	CarState.Wheel_Speed_Rear_Average = (CarState.Wheel_Speed_Right_Calculated  + CarState.Wheel_Speed_Left_Calculated)/2;
}

static char getByte(uint32_t input, int8_t returnbyte)
{
	union {
		uint32_t integer;
		unsigned char bytearray[4];
	} data;
	data.integer = input;
	return data.bytearray[returnbyte];
}

void storeBEint32(uint32_t input, uint8_t Data[4])
{
	Data[0] = getByte(input,3);
	Data[1] = getByte(input,2);
	Data[2] = getByte(input,1);
	Data[3] = getByte(input,0);
}

void storeBEint16(uint16_t input, uint8_t Data[2])
{
	Data[0] = getByte(input,1);
	Data[1] = getByte(input,0);
}

void storeLEint32(uint32_t input, uint8_t Data[4])
{
	Data[0] = getByte(input,0);
	Data[1] = getByte(input,1);
	Data[2] = getByte(input,2);
	Data[3] = getByte(input,3);
}

void storeLEint16(uint16_t input, uint8_t Data[2])
{
	Data[0] = getByte(input,0);
	Data[1] = getByte(input,1);
}


char CANLogData( void )
{
	// build data logging blocks
	FDCAN_TxHeaderTypeDef TxHeaderLog;
	uint8_t CANTxData[8];

	TxHeaderLog.IdType = FDCAN_STANDARD_ID;
	TxHeaderLog.TxFrameType = FDCAN_DATA_FRAME;
	TxHeaderLog.DataLength = FDCAN_DLC_BYTES_8; // only two bytes defined in send protocol, check this
	TxHeaderLog.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeaderLog.BitRateSwitch = FDCAN_BRS_OFF; // irrelevant to classic can
	TxHeaderLog.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeaderLog.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeaderLog.MessageMarker = 0;
    HAL_Delay(1); // for some reason without small delay here the first log ID 0x7C6 only sometimes sent
    // investigate to find out why, perhaps fifo buffer not acting as expect?

	resetCanTx(CANTxData);
	TxHeaderLog.Identifier = 0x7C6;
	storeBEint16(ADCState.Torque_Req_L, &CANTxData[0]); 	//torq_req_l can0 0x7C6 0,16be
	storeBEint16(ADCState.Torque_Req_R, &CANTxData[2]); 	//torq_req_r can0 0x7C6 16,16be

	storeBEint16(ADCState.BrakeF, &CANTxData[4]); 	//brk_press_f can0 0x7C6 32,16bee
	storeBEint16(ADCState.BrakeF, &CANTxData[6]); 	//brk_press_r can0 0x7C6 48,16be

	CAN1Send( &TxHeaderLog, CANTxData ); //lagging in sending

	resetCanTx(CANTxData);
	TxHeaderLog.Identifier = 0x7C7;
	storeBEint32(CarState.Wheel_Speed_Right_Calculated, &CANTxData[0]); //wheel_speed_right_calculated can0 0x7c7 0,32BE
	storeBEint32(CarState.Wheel_Speed_Left_Calculated, &CANTxData[4]); //wheel_speed_left_calculated can0 0x7c7 32,32BE
	CAN1Send( &TxHeaderLog, CANTxData );

	resetCanTx(CANTxData);
	TxHeaderLog.Identifier = 0x7C8;
	CANTxData[0] = ADCState.CoolantTemp1; //temp_sensor1 can0 0x7c8 0,8
	CANTxData[1] = CarState.Torque_Req_Max; //torq_req_max can0 0x7c8 8,8
	CANTxData[2] = ADCState.CoolantTemp2; 	//temp_sensor_2 can0 0x7c8 16,8
	CANTxData[3] = ADCState.Future_Torque_Req_Max; //future_torq_req_max can0 0x7c8 24,8
	storeBEint16(ADCState.Torque_Req_L_Percent, &CANTxData[4]); //torq_req_l_perc can0 0x7c8 32,16be
	storeBEint16(ADCState.Torque_Req_R_Percent, &CANTxData[6]); //torq_req_r_perc can0 0x7c8 48,16be
	CAN1Send( &TxHeaderLog, CANTxData );

	resetCanTx(CANTxData);
	TxHeaderLog.Identifier = 0x7C9;
	CANTxData[6] = HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1);
    CANTxData[7] = HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2);

	CAN1Send( &TxHeaderLog, CANTxData );
	storeBEint16(Actual_Torque_Left_Inverter_Raw.data.longint, &CANTxData[0]); //actual_torque_left_inverter_raw can0 0x7c9 0,16be
	storeBEint16(Actual_Torque_Right_Inverter_Raw.data.longint, &CANTxData[2]); //actual_torque_right_inverter_raw can0 0x7c9 16,16be

	resetCanTx(CANTxData);
	TxHeaderLog.Identifier = 0x7CA; // not being sent in current simulink, but is set?
	CANTxData[6] = HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1);
    CANTxData[7] = HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2);
	// storeBEint32(CarState.brake_balance,&CANTxData[0]); //brake_balance can0 0x7CA 0,32be
	CANTxData[0] = ADCState.SteeringAngle;
	CAN1Send( &TxHeaderLog, CANTxData );

	return 0;
}

void processADCInput( void )
{
	if(ADCState.newdata){
		ADCState.newdata = 0;
		ADCState.SteeringAngle = getSteeringAngle(ADC_Data[SteeringADC]);

		ADCState.BrakeF = getBrakeF(ADC_Data[BrakeFADC]);
		ADCState.BrakeR = getBrakeR(ADC_Data[BrakeRADC]);
		CarState.brake_balance = ( ADCState.BrakeF * 100 ) / (ADCState.BrakeF + ADCState.BrakeR);
		ADCState.CoolantTemp1 = getCoolantTemp(ADC_Data[CoolantTemp1ADC]);
		ADCState.CoolantTemp2 = getCoolantTemp(ADC_Data[CoolantTemp2ADC]);

		ADCState.Future_Torque_Req_Max = getDrivingMode(ADC_Data[DrivingModeADC]);
		setTorqueReqPerc(ADC_Data[ThrottleLADC],ADC_Data[ThrottleRADC]);
	//	ADCState.Torque_Req_L_Percent = getTorqueReqLPerc(ADC_Data[ThrottleLADC]);
	//	ADCState.Torque_Req_R_Percent = getTorqueReqRPerc(ADC_Data[ThrottleRADC]);
	}
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

void setupInterrupts( void )
{
	InButtonpress = 1;

	// enable and start timer interrupt
	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	// enable button interrupts
	HAL_NVIC_SetPriority(USER_Btn_EXTI_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USER_Btn_EXTI_IRQn);

	HAL_NVIC_SetPriority(Input1_EXTI_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(Input1_EXTI_IRQn);

	HAL_NVIC_SetPriority(Input2_EXTI_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(Input2_EXTI_IRQn);

	HAL_NVIC_SetPriority(Input3_EXTI_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(Input3_EXTI_IRQn);


	if ( HAL_TIM_Base_Start_IT(&htim3) != HAL_OK){
		  Error_Handler();
	}
	InButtonpress = 0;
	// start LCD update Timer.
	if ( HAL_TIM_Base_Start_IT(&htim6) != HAL_OK){
		Error_Handler();
	}

	  HAL_NVIC_SetPriority(TIM17_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(TIM17_IRQn);

	if ( HAL_TIM_Base_Start_IT(&htim17) != HAL_OK){
		Error_Handler();
	}
}

void startADC(void)
{
	// start ADC conversion
	  if (HAL_ADC_Start_DMA(&hadc1,(uint32_t *)aADCxConvertedData,ADC_CONVERTED_DATA_BUFFER_SIZE) != HAL_OK)
	  {
	      Error_Handler();
	  }
}

void stopADC( void )
{
	  if (HAL_ADC_Stop_DMA(&hadc1) != HAL_OK)
	  {
	      Error_Handler();
	  }
}


void setupCarState( void )
{
	CarState.brake_balance=0;

	CarState.ReadyToDrive_Ready = 0;

	CarState.HighVoltageOn_AllowedR = 0;
	CarState.HighVoltageOn_AllowedL = 0;

	CarState.ReadyToDrive_AllowedR = 0;
	CarState.ReadyToDrive_AllowedL = 0;

	CarState.HighVoltageOn_Ready = 0;

	CarState.Buzzer_Sounding = 0;

	CarState.LeftInvState = 0;
	CarState.RightInvState = 0;

	usecanADC = 0;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(!usecanADC){ // don't process ADC values if ECU has been setup to use dummy values
		for (int i = 0; i<NumADCChan;i++)
		{
			int sum = 0;

			for( int j = 0; j < SampleSize; j++)
			{
				sum =  sum + (aADCxConvertedData[i+NumADCChan*j]); // calculate sum of sample data  for one ADC channel
			}
			ADC_Data[i] = sum/SampleSize; // store the value in ADC_Data from buffer averaged over 10 samples for better accuracy.
		}
		ADCState.newdata = 1;
	}
//    ADCcount++;
}

/**
 * interrupt rx callback for canbus0
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t CANRxData[8];
	if(hfdcan->Instance == FDCAN1){
		setOutput(15,1);
	} else if(hfdcan->Instance == FDCAN2) {
//		setOutput(14,1);
	}

	if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
    // Retreive Rx messages from RX FIFO0
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, CANRxData) != HAL_OK)
		{
			// Reception Error
			Error_Handler();
		}

		// process incoming packet
		switch ( RxHeader.Identifier ){

			case 	0xF :  // BMS can0 id 0xF
	//			offset 0 length 32: power
    //			offset 32 length 16 big endian: BatAmps
	//			offset 48 length 16: BatVoltage
				power.time = gettimer();
				power.newdata = 1;
				power.data.longint = CANRxData[0]*16777216+CANRxData[1]*65536+CANRxData[2]*256+CANRxData[3];

				BatAmps.time = gettimer();
				BatAmps.newdata = 1;
				BatAmps.data.longint = CANRxData[4]*256+CANRxData[5];

				BatVoltage.time = gettimer();
				BatVoltage.newdata = 1;
				BatVoltage.data.longint = 65536+CANRxData[6]*256+CANRxData[7];
				break;

			case	0x520 : // PDM can0
				//	0x520,0,8 -> BMS_relay_status
				//	0x520,8,8 -> IMD_relay_status
				//	0x520,16,8 -> BSPD_relay_status
				BMS_relay_status.time = gettimer();
				BMS_relay_status.newdata = 1;
				BMS_relay_status.data.longint = CANRxData[0];

				IMD_relay_status.time = gettimer();
				IMD_relay_status.newdata = 1;
				IMD_relay_status.data.longint = CANRxData[1];

				BSPD_relay_status.time = gettimer();
				BSPD_relay_status.newdata = 1;
				BSPD_relay_status.data.longint = CANRxData[2];
				break;
			case	0x521 : // IVT Can0 0x521,24,24BE * 0.001 -> Accu_Voltage
				Accu_Voltage.time = gettimer();
				Accu_Voltage.newdata = 1;
				Accu_Voltage.data.longint = CANRxData[3]*16777216+CANRxData[4]*65536+CANRxData[5]*256+CANRxData[6];
				break;
			case	0x523 : // IVT Can0 0x523,24,24BE * 0.001 -> Accu_Current
				Accu_Current.time = gettimer();
				Accu_Current.newdata = 1;
				Accu_Current.data.longint = CANRxData[3]*16777216+CANRxData[4]*65536+CANRxData[5]*256+CANRxData[6];
				break;
			case 	0x600 : // debug ID to send arbitraty 'ADC' values for testing.
				if( CANRxData[0] ) // if received value in ID is not 0 assume true and switch to fakeADC over CAN.
				{
			//		stopADC(); //  disable ADC DMA interrupt to stop processing ADC input.
					// crashing if breakpoint ADC interrupt after this, just check variable in interrupt handler for now.
					usecanADC = 1; // set global state to use fake canbus ADC
					for(int i = 0; i < NumADCChan;i++)
					{
						ADC_Data[i] = 0;   // blank out the currently random ADC data when disable ADC reading.
					}
				} else // value of 0 received, switch back to real ADC.
				{
					usecanADC = 0;
	//				startADC(); // restart ADC DMA interrupt.
				}
				break;
			case	0x601 : // debug ID for steering data.
				if( usecanADC )  // check if we're operating on fake canbus ADC
				{
					ADC_Data[SteeringADC] = CANRxData[0]*256+CANRxData[1]; // set ADC_Data for steering
					ADCState.newdata = 1;
				}
				break;
			case	0x602 : // debug ID for accelerator data.
				if( usecanADC )
				{
					ADC_Data[ThrottleLADC] = CANRxData[0]*256+CANRxData[1]; // set ADC_data for Left Throttle
					ADC_Data[ThrottleRADC] = CANRxData[2]*256+CANRxData[3]; // set ADC_data for Right Throttle
					ADCState.newdata = 1;
				}
				break;
			case 	0x603 : // debug ID for brake data.
				if( usecanADC )
				{
					ADC_Data[BrakeFADC] = CANRxData[0]*256+CANRxData[1]; // set ADC_data for Front Brake
					ADC_Data[BrakeRADC] = CANRxData[2]*256+CANRxData[3]; // set ADC_data for Rear Brake
					ADCState.newdata = 1;
				}
				break;
			case	0x604 : // debug ID for driving mode data.
				if( usecanADC )
				{
					ADC_Data[DrivingModeADC] = CANRxData[0]*256+CANRxData[1]; // set ADC_Data for driving mode
					ADCState.newdata = 1;
				}
				break;
			case 	0x605 : // debug ID for temperature data.
				if( usecanADC )
				{
					ADC_Data[CoolantTemp1ADC] = CANRxData[0]*256+CANRxData[1]; // set ADC_data for First Coolant Temp
					ADC_Data[CoolantTemp2ADC] = CANRxData[2]*256+CANRxData[3]; // set ADC_data for Second Coolant Temp
					ADCState.newdata = 1;
				}
				break;
			case	0x610 : // debug id for fake can TS button
				if(CANRxData[0]){
					TS_Switch.pressed = 1;
					TS_Switch.lastpressed = gettimer();
				}
				break;
			case	0x611 : // debug id for fake can RTDM button
				if(CANRxData[0]){
					RTDM_Switch.pressed = 1;
					RTDM_Switch.lastpressed = gettimer();
				}
				break;
			case	0x612 : // debug id for fake stop motors button
				if(CANRxData[0]){
					StopMotors_Switch.pressed = 1;
					StopMotors_Switch.lastpressed = gettimer();
				}
				break;
//REMOVE FROM LIVE CODE.
			case	0x613 : // debug id to induce a hang state, for testing watchdog.
				while ( 1 ){
					// do nothing.
				}
				break;


			default : // unknown identifier encountered, ignore. Shouldn't be possible to get here due to filters.

				break;
		}

		RxHeader.Identifier = 0; // workaround: rx header does not seem to get reset properly?
								 // receiving e.g. 15 after 1314 seems to result in 1315

		if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
		{
      // Notification Error
			Error_Handler();
		}
	}
}

/**
 * interrupt rx callback for canbus1
 */
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t CANRxData[8];
	setOutput(14,1); // turn on internal LED to indicate can receive activity.

	if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET) // if there is a message waiting process it
	{
		//timercount = __HAL_TIM_GetCounter(&htim3);
		/* Retrieve Rx message from RX FIFO1 */
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &RxHeader, CANRxData) != HAL_OK)
		{
			/* Reception Error */
			Error_Handler();
		}

		// check rest of header data? Can2 is inverter information
		switch ( RxHeader.Identifier ){ // identify which data packet we are processing.

			case 	0x0 :  // id 0x0,0,8 -> nmt_status
				nmt_status.time = gettimer();
				nmt_status.data.longint = CANRxData[0];
				nmt_status.newdata = 1;
				break;
			case	0x1FE : // 0x1FE,0,16LE -> Status_Right_Inverter
				Status_Right_Inverter.time = gettimer();
				Status_Right_Inverter.newdata = 1;
				Status_Right_Inverter.data.longint = CANRxData[1]*256+CANRxData[0];
				break;
			case	0x1FF :  // 0x1FF,0,16LE -> Status_Left_Inverter
				Status_Left_Inverter.time = gettimer();
				Status_Left_Inverter.newdata = 1;
				Status_Left_Inverter.data.longint = CANRxData[1]*256+CANRxData[0];
				break;
			case	0x2FE :  // 0x2FE,16,32LE -> Speed_Right_Inverter
				// looking at logs, receiveing lots of data on this ID that seems to be varying
				// investigate commissioning setup?
				Speed_Right_Inverter.time = gettimer();
				Speed_Right_Inverter.newdata = 1;
				Speed_Right_Inverter.data.longint = CANRxData[5]*16777216+CANRxData[4]*65536+CANRxData[3]*256+CANRxData[2];
				break;
			case	0x2FF :  // 0x2FF,16,32LE -> Speed_Left_Inverter
				Speed_Left_Inverter.time = gettimer();
				Speed_Left_Inverter.newdata = 1;
				Speed_Left_Inverter.data.longint = CANRxData[5]*16777216+CANRxData[4]*65536+CANRxData[3]*256+CANRxData[2];
				break;
				// 0x3fe/f and 0x4fe/f also sent by inverters, ignored in elektrobit.

			case	0x77E : // 0x77E,8,16LE -> Actual_Torque_Right_Inverter_Raw
				// only seen once at start in can log.
				Actual_Torque_Right_Inverter_Raw.time = gettimer();
				Actual_Torque_Right_Inverter_Raw.newdata = 1;
				Actual_Torque_Right_Inverter_Raw.data.longint = CANRxData[2]*256+CANRxData[1];
				break;

			case	0x77F :	//0x77F,8,16LE -> Actual_Torque_Left_Inverter_Raw
				// not seen in can log
				Actual_Torque_Left_Inverter_Raw.time = gettimer();
				Actual_Torque_Left_Inverter_Raw.newdata = 1;
				Actual_Torque_Left_Inverter_Raw.data.longint = CANRxData[2]*256+CANRxData[1];
				break;
			default : // any other received packets
				break;
		}

	/*	if ((RxHeader2.Identifier == 0x2) && (RxHeader2.IdType == FDCAN_STANDARD_ID) && (RxHeader2.DataLength == FDCAN_DLC_BYTES_1))
		{
		//	ubKeyNumber = CANRxData[0];
		} */

		RxHeader.Identifier = 0; // workaround: rx header does not seem to get reset properly?
								 // receiving e.g. 15 after 1314 seems to result in 1315

		// send notification that can message has been read
		if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) != HAL_OK)
		{
			/* Notification Error */
			Error_Handler();
		}
	}
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *canp) {

// Re-enable receive interrupts!

// (The error handling code in HAL_CAN_IRQHandler() disables this for some reason!)
 setOutput(15,1);
 setOutput(14,1);
 setOutput(13,1);
 while ( 1 ){

 }
//__HAL_CAN_ENABLE_IT(canp, FDCAN_I FDCAN_IT_FMP1);
// (or only re-enable the one you are using)

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

/**
 * @brief IRQ handler for timer3 used to keep timebase.
 */
void TIM3_IRQHandler()
{
    HAL_TIM_IRQHandler(&htim3);
}

void TIM6_IRQHandler()
{
    HAL_TIM_IRQHandler(&htim6);
}

/**
 * @brief timer interrupt to keep a timebase and process button debouncing.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if ( htim->Instance == TIM3 ){
		setOutput(15,0);
		setOutput(14,0);
		secondson = secondson + 1; // increment global time counter
		HAL_GPIO_TogglePin(LD1_GPIO_Port,LD1_Pin); // toggle led to indicate board active

		if ( TS_LED.blinking ) { toggleOutput(TS_LED.pin); } // process the blinking of car state LED's
		// could possibly be changed to PWM to take out of interrupt processing?
		if ( RTDM_LED.blinking ) { toggleOutput(RTDM_LED.pin); }

	} else if ( htim->Instance == TIM7 ){
	  // timer7, being used for button input debouncings
		InButtonpress = 0;  // reset status of button processing after timer elapsed,
							// to indicate not processing input to allow triggering new timer.
		switch ( ButtonpressPin ) {  // process the button that was pressed to start the debounce timer.

		case USER_Btn_Pin :
			debouncebutton(&UserBtn);
			break;
		case Input1_Pin:
			debouncebutton(&Input1);
			break;
		case Input2_Pin:
			debouncebutton(&Input2);
			break;
		case Input3_Pin:
			debouncebutton(&Input3);
			break;
		case Input4_Pin:
			debouncebutton(&Input4);
			break;
		case Input5_Pin:
			debouncebutton(&Input5);
			break;
		case Input6_Pin:
			debouncebutton(&Input6);
			break;
		default : // shouldn't get here, but catch and ignore any other input
			break;
		}
	} else if ( htim->Instance == TIM6 )
	{
//		hd44780_irq();
	} else if ( htim->Instance == TIM17 )
	{
			hd44780_Isr(); // call interrupt handler to run next clock tick for display.
//	 		HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
	}

}
