/*
 * ecu.h
 *
 *  Created on: 29 Dec 2018
 *      Author: Visa
 */

#ifndef ADCECU_H_
#define ADCECU_H_

// redefine channels to match shield pins to make easier.

/*
ADC_Data[0]	working as ai1   ~4k to 65k. op amp              1.74k/3.3k     5v -> 3.275v   pa3
ADC_Data[1]	working as ai7   ~5k-43k - 13-65k now.               150k/268k      4.5v -> 3.275v  pc0
ADC_Data[2]	working as ai3    ~12.5k to 65k op amp. ( 84.7m, 2.89v max reading )   1.74k/3.3k   5v -> 3.275v   pa5
ADC_Data[3]	working as ai4,   ~270k resistor.  3.3v max.         no divider           3.3v     pa6
ADC_Data[4]	working as ai5,   ~270k resistor, 3.3v max.          no divider           3.3v     pa7
ADC_Data[5]	working as ai2 ~8.5k to 65k op amp.                  1.74k/3.3k    5v -> 3.275v   pa4
---ADC_Data[6] volatile uint32_t   old ai6, very small input range. ~31k-42k 0-5v.  1.2-1.8v output for 0-5v input, climbing over time.
ADC_Data[7]	volatile uint32_t	working ai0 on connector. ~4k to 65k op amp.         1.74k/3.3k    5v -> 3.275v   pa0
ADC_data[8]	new ai6  | --                                 385k/150k   10v->3.125      pf4
*/

#define SteeringADC			5 // ai2
#define ThrottleLADC		1 // ai7
#define ThrottleRADC		8 // ai6
//#define ThrottleRADC		1 // ai6olld
#define BrakeFADC			0 // ai1  pa3 PIN CN9-1
#define BrakeRADC			7 // ai0  pa0 PIN CN10-29
#define DrivingModeADC		2 // ai3
#define CoolantTempLADC     3 // ai5 tmp_l // L is not connected.
#define CoolantTempRADC     3 // ai4 tmp_r should be 3, but disconnected.

#define NumADCChan		   	8
#define NumADCChanADC3 		2
#define SampleSize			4 // how many samples to average
#define ADC_CONVERTED_DATA_BUFFER_SIZE   ((uint32_t) NumADCChan*SampleSize)   /* Size of array aADCxConvertedData[] */
#define ADC_CONVERTED_DATA_BUFFER_SIZE_ADC3   ((uint32_t) NumADCChanADC3*SampleSize)

// using local or remote ADC.
volatile char usecanADC;

volatile char minmaxADC;
// ADC

// averaged ADC data
volatile uint32_t ADC_Data[NumADCChan+NumADCChanADC3];
volatile uint32_t ADC_DataError[NumADCChan+NumADCChanADC3];
volatile uint32_t ADC_DataMin[NumADCChan+NumADCChanADC3];
volatile uint32_t ADC_DataMax[NumADCChan+NumADCChanADC3];

struct  {
	volatile char newdata;
	int8_t SteeringAngle;
	uint8_t BrakeF;
	uint8_t BrakeR;
	uint8_t CoolantTempL;
	uint8_t CoolantTempR;
	uint16_t CoolantTempRRaw;
	uint8_t Torque_Req_L_Percent;
	uint8_t Torque_Req_R_Percent;
	uint8_t DrivingMode;
} ADCState;

struct  {
	volatile char newdata;
	int8_t SteeringAngle;
	uint8_t BrakeF;
	uint8_t BrakeR;
	uint8_t CoolantTempL;
	uint8_t CoolantTempR;
	uint8_t Torque_Req_L_Percent;
	uint8_t Torque_Req_R_Percent;
	uint8_t DrivingMode;
} CANADC;


struct ADCTable {
	uint16_t *Input;
	int16_t *Output;
	uint16_t Elements;
};

struct ADCInterpolationTables { // pointers to array data for linear interpolation values, use elements field to know how large arrays are.
		struct ADCTable Steering;
		struct ADCTable SteeringAngle;

		struct ADCTable BrakeF;
		struct ADCTable BrakeR;

		struct ADCTable AccelL;
		struct ADCTable AccelR;

		struct ADCTable TorqueCurve;

		struct ADCTable CoolantL;
		struct ADCTable CoolantR;

		struct ADCTable ModeSelector;
} ADCInterpolationTables;

int16_t linearInterpolate(uint16_t Input[], int16_t Output[], uint16_t count, uint16_t RawADCInput);

// adc converters
int getSteeringAngle(uint16_t RawADCInput);
int getBrakeF(uint16_t RawADCInput);
int getBrakeR(uint16_t RawADCInput);
int getBrakeBalance(uint16_t RawADCInputF, uint16_t RawADCInputR);
int getDrivingMode(uint16_t RawADCInput);
int getCoolantTemp1(uint16_t RawADCInput);
int getCoolantTemp2(uint16_t RawADCInput);
int getTorqueReqPercL(uint16_t RawADCInputL);
int getTorqueReqPercR(uint16_t RawADCInputR);
int getTorqueCurve( uint16_t ADCInput );

// car state

void SetupADCInterpolationTables( void );

HAL_StatusTypeDef startADC(void);
HAL_StatusTypeDef stopADC( void );

void minmaxADCReset(void);

uint16_t CheckADCSanity( void );

#endif /* ADCECU_H_ */