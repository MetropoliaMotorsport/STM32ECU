/*
 * adcecu.h
 *
 *  Created on: 29 Dec 2018
 *      Author: Visa
 */

#ifndef ADCECU_H_
#define ADCECU_H_
#include "eeprom.h"
#include "ecu.h"
#include "semphr.h"

#ifdef HPF19
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
#endif


#ifdef HPF20
	#define ThrottleLADC		(0) //
	#define ThrottleRADC		(1) //
	#define BrakeFADC			(3) //
	#define BrakeRADC			(4) //
	#define ShutdownADC			(2)
	#define TemperatureADC	    (6)

	#define NumADCChan		   	(3)
	#define NumADCChanADC3 		(3)
	#define SampleSize			(4)
#ifdef STMADC
	#define ADC_CONVERTED_DATA_BUFFER_SIZE   ((uint32_t) NumADCChan*SampleSize*2)
	/* Size of array aADCxConvertedData[] */
	#define ADC_CONVERTED_DATA_BUFFER_SIZE_ADC3   ((uint32_t) NumADCChanADC3*SampleSize*2)
#endif
#endif

#ifdef HPF19
// using local or remote ADC.
volatile char usecanADC;
#endif

//extern volatile char minmaxADC;
// ADC

#ifdef STMADC
typedef enum ADC_cmd{ adc1, adc3 } ADC_cmd;

typedef struct ADC_msg {
	uint32_t cmd;
} ADC_msg;


// averaged ADC data
volatile uint32_t ADC_Data[NumADCChan+NumADCChanADC3];
volatile uint32_t ADC_DataError[NumADCChan+NumADCChanADC3];
volatile uint32_t ADC_DataMin[NumADCChan+NumADCChanADC3];
volatile uint32_t ADC_DataMax[NumADCChan+NumADCChanADC3];
#endif


extern SemaphoreHandle_t ADCUpdate;

typedef struct {
	uint32_t Oldest;
	volatile char newdata;
	int16_t SteeringAngle;
	uint16_t SteeringAngleAct;
	uint16_t SteeringDuty;
	uint16_t SteeringFreq;
	uint16_t BrakeF;
	uint16_t BrakeR;
	uint8_t CoolantTempL;
	uint8_t CoolantTempR;
	uint16_t CoolantTempRRaw;
	uint16_t Torque_Req_L_Percent;
	uint16_t Torque_Req_R_Percent;
	uint16_t Regen_Percent;
	uint16_t APPSL;
	uint16_t APPSR;
	uint16_t Regen;
	uint8_t DrivingMode;
} ADCState_t;

extern volatile ADCState_t ADCState;
extern volatile ADCState_t ADCStateNew;

typedef struct {
// node sensor data.
	int OilTemp1;
	int OilTemp2;
	int OilTemp3;
	int OilTemp4;

	int Susp1;
	int susp2;
	int Susp3;
	int susp4;

	int WaterTemp1;
	int WaterTemp2;
	int WaterTemp3;
	int WaterTemp4;
	int WaterTemp5;
	int WaterTemp6;

	uint16_t BrakeTemp1;
	uint16_t BrakeTemp2;
	uint16_t BrakeTemp3;
	uint16_t BrakeTemp4;

	int TireTemp1;
	int TireTemp2;
	int TireTemp3;

	int TireTemp4;
	int TireTemp5;
	int TireTemp6;

	int TireTemp7;
	int TireTemp8;
	int TireTemp9;

	int TireTemp10;
	int TireTemp11;
	int TireTemp12;
} ADCStateSensors_t;

extern volatile ADCStateSensors_t ADCStateSensors;

#ifdef HPF19
struct  {
	volatile char newdata;
	int16_t SteeringAngle;
	uint8_t BrakeF;
	uint8_t BrakeR;
	uint8_t CoolantTempL;
	uint8_t CoolantTempR;
	uint8_t Torque_Req_L_Percent;
	uint8_t Torque_Req_R_Percent;
	uint8_t DrivingMode;
} CANADC;
#endif

struct ADCTable {
	uint16_t *Input;
	int16_t *Output;
	uint16_t Elements;
};

typedef struct ADCInterpolationTables { // pointers to array data for linear interpolation values, use elements field to know how large arrays are.
		struct ADCTable Steering;
		struct ADCTable SteeringAngle;

		struct ADCTable BrakeF;
		struct ADCTable BrakeR;

		struct ADCTable Regen;

		struct ADCTable AccelL;
		struct ADCTable AccelR;

		struct ADCTable TorqueCurve;

		struct ADCTable Coolant;

		struct ADCTable ModeSelector;
#ifdef TORQUEVECTOR
		struct ADCTable TorqueVector;
#endif
} ADCInterpolationTables_t;

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
int getBrakeTravelPerc(uint16_t RawADCInput);

int getTorqueReqCurve( uint16_t ADCInput );

#ifdef TORQUEVECTOR
int getTorqueVector(uint16_t RawADCInput);
#endif

// car state

bool SetupADCInterpolationTables( eepromdata * data );
#ifdef HPF19
void SetupNormalTorque( void );
void SetupLargeLowRangeTorque( void );
void SetupLowTravelTorque( void );
#endif
void SetupTorque( int request );

#ifdef STMADC
HAL_StatusTypeDef startADC(void);
#endif


void minmaxADCReset(void);

#ifdef HPF19
void receiveCANInput( uint8_t * CANRxData );

int initCANADC( void );
#endif

uint32_t CheckADCSanity( void );

char * getADCWait( void);
uint32_t getAnalogueNodesOnline( void );

int initADC( void );

extern TaskHandle_t ADCTaskHandle;

#endif /* ADCECU_H_ */
