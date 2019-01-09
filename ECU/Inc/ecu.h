/*
 * ecu.h
 *
 *  Created on: 29 Dec 2018
 *      Author: Visa
 */

#include "main.h"

#ifndef ECU_H_
#define ECU_H_

#define debugrun

#define SteeringADC      	0
#define ThrottleLADC		1
#define ThrottleRADC		2
#define BrakeFADC			3
#define BrakeRADC			4
#define DrivingModeADC		5
#define CoolantTemp1ADC		6
#define CoolantTemp2ADC		7

#define NumADCChan 8
#define SampleSize 10 // how many samples to average
#define ADC_CONVERTED_DATA_BUFFER_SIZE   ((uint32_t) NumADCChan*SampleSize)   /* Size of array aADCxConvertedData[] */
#define Debouncetime 10000 // only allow one button press every 20ms

#define TS_Switch Input1
#define StopMotors_Switch Input2
#define RTDM_Switch Input3
#define SLIP_CTRLplus Input4
#define SLIP_CTRLminus Input5

#define BMSLED_Output 		1
#define IMDLED_Output		2
#define BSPDLED_Output		3
#define TSALLED_Output		4
#define RTDMLED_Output		5
#define STOPLED_Output		6
#define LED1_Output			13
#define LED2_Output			14
#define LED3_Output			15

#define LeftInverter		0
#define RightInverter		1


// interrupt timebase variable
volatile uint32_t secondson;

struct CanData {
	union {
	unsigned long longint;
	unsigned char bytearray[4];
	} data;
	long time;
	char newdata;
};

struct ButtonData {
	uint32_t lastpressed;
	uint32_t count;
	char pressed;
	// define the hardware button for passing button data including reading it
	GPIO_TypeDef * port;
	uint16_t pin;
};

struct OutputData {
	uint8_t pin;
	volatile char blinking;
};

// ADC

// averaged ADC data
volatile uint32_t ADC_Data[NumADCChan];

struct ADCState {
	volatile char newdata;
	int8_t SteeringAngle;
	uint8_t BrakeF;
	uint8_t BrakeR;
	uint8_t Future_Torque_Req_Max;
	uint8_t CoolantTemp1;
	uint8_t CoolantTemp2;
	uint8_t Torque_Req_L_Percent;
	uint8_t Torque_Req_R_Percent;
	uint16_t Torque_Req_L;
	uint16_t Torque_Req_R;
} ADCState;

volatile struct CarState {
	uint32_t	brake_balance;
	char ReadyToDrive_Ready;

	char HighVoltageOn_Allowed;
	char HighVoltageOn_Allowed1;

	char ReadyToDrive_Allowed;
	char ReadyToDrive_Allowed1;

	char HighVoltageOn_Ready;
	char Buzzer_Sounding;

	char BMS_relay_status;
	char IMD_relay_status;
	char BSPD_relay_status;

	char TSALLeftInvLED;
	char TSALRightInvLED;

	char RtdmLeftInvLED;
	char RtdmRightInvLED;

	uint16_t LeftInv;
	uint16_t RightInv;

	uint8_t Torque_Req_Max;

	int32_t Wheel_Speed_Left_Calculated;
	int32_t Wheel_Speed_Right_Calculated;
	int32_t Wheel_Speed_Rear_Average;

	char StopLED;

} CarState;

// CANBus
FDCAN_TxHeaderTypeDef TxHeaderTime, TxHeader1, TxHeader2; //  TxHeader0,  TxHeader3

volatile struct CanData nmt_status;
volatile struct CanData Status_Right_Inverter;
volatile struct CanData Status_Left_Inverter;
volatile struct CanData Speed_Right_Inverter;
volatile struct CanData Speed_Left_Inverter;
volatile struct CanData Actual_Torque_Right_Inverter_Raw;
volatile struct CanData Actual_Torque_Left_Inverter_Raw;
volatile struct CanData BMS_relay_status;
volatile struct CanData IMD_relay_status;
volatile struct CanData BSPD_relay_status;
volatile struct CanData power;
volatile struct CanData BatAmps;
volatile struct CanData BatVoltage;
volatile struct CanData Accu_Voltage;
volatile struct CanData Accu_Current;

// button inputs
volatile struct ButtonData UserBtn, Input1, Input2, Input3, Input4, Input5, Input6;
struct OutputData LED1, LED2, LED3, TS_LED, RTDM_LED, STOP_LED, BMS_LED, IMD_LED, BSPD_LED;

// helpers
void swapByteOrder_int16(double *current, const int16_t *rawsignal, size_t length);
void storeBEint32(uint32_t input, uint8_t Data[4]);
void storeBEint16(uint16_t input, uint8_t Data[2]);
void storeLEint32(uint32_t input, uint8_t Data[4]);
void storeLEint16(uint16_t input, uint8_t Data[2]);

int16_t linearInteropolate(uint16_t Input[], int16_t Output[], uint16_t count, uint16_t RawADCInput);
uint32_t gettimer(void);

// adc converters
int8_t getSteeringAngle(uint16_t RawADCInput);
uint8_t getBrakeF(uint16_t RawADCInput);
uint8_t getBrakeR(uint16_t RawADCInput);
uint8_t getDrivingMode(uint16_t RawADCInput);
uint8_t getCoolantTemp(uint16_t RawADCInput);
uint8_t getTorqueReqPerc(uint16_t RawADCInput);

// gpio

GPIO_TypeDef* getGpioPort(int output);
int getGpioPin(int output);
void setOutput(int output, char state);
void toggleOutput(int output);
void resetButton( struct ButtonData button );
void setLEDs( void );
void debouncebutton( volatile struct ButtonData *button );

//canbus
void resetCanTx(uint8_t CANTxData[8]);
char CAN1Send( FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData );
char CAN2Send( FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData );
char CANKeepAlive( void );
char CANSendState( char buzz, char highvoltage );
char CANSendInverter( uint16_t response, uint16_t request, uint8_t inverter );
char CANTorqueRequest( uint16_t request );
char CANLogData( void );

// car state

void processADCInput( void );
void processCANData( void );

char AllowedToDrive( void );
void RTDMCheck( void );
char InverterStateMachine( int8_t Inverter );
void RearSpeedCalculation( void );
uint16_t PedalTorqueRequest( void );

// initialisation
void setupInterrupts( void );
void setupCarState( void );
void startupLEDs(void);
void FDCAN1_start(void);
void FDCAN2_start(void);
void setupButtons(void);
void setupLEDs( void );
void startADC(void);

#endif /* ECU_H_ */
