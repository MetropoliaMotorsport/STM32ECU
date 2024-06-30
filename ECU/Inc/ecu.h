/*
 * ecu.h
 *
 *  Created on: 29 Dec 2018
 *      Author: Visa
 */

#ifndef ECU_H_
#define ECU_H_

#include <stdbool.h>
#include "freertos.h"

#define DEBUG

#define TSALP

#define TIMEINVSTATECHANGE 1000

#define MAXERROROUTPUT       (40)

#define TORQUE_DIFFERENCE	 (30)
#define TORQUE_LEFT_PRIMARY

#define HPF20
#define HPF2023
#define FSG23 //to enable quick hacks for FSG2023

//#define APPSFIXL // uncomment if APPSL not readng to temporarily copy APPSR, do not use longterm.
//#define APPSFIXR // uncomment if APPSR not readng to temporarily copy APPSL, do not use longterm.

//#define CHECKTSALPOWER

#if defined(APPSFIXL) && defined(APPSFIXR)
#error Only one apps fix.
#endif

// HPF 20 doesn't use any local ADC, all via analogue nodes, and PWM.
#ifdef HPF20
#define RTOS
#define POWERNODES
#define ANALOGNODES
#define MATLAB
#define LENZE
//#define TWOWHEELS
#ifdef TWOWHEELS
	#define MOTORCOUNT		(2)
#else
#define MOTORCOUNT		(4)
#endif
#define retransmitIMU
#endif

// Brake pressure values

//changed new BPsensors from 240bar 1v-5v to 140bar 0,5v-4,5v
#define APPSBrakeLight			42
#define APPSBrakeHard			58 //
#define APPSBrakeRelease		42 //
#define RTDMBRAKEPRESSURE		6 // set a CAN trigger to allow this easier without reprogramming for wheels up testing.


#define LIMPNM					10 // limp mode torque

// Minimum acceptable voltage on TS for startup.
#define MINHV					500 // minimum voltage to allow TS enable.

// Use only one canbus for all functions, for bench testing.
//#define ONECAN

// Both CAN's are connected to one bus for bench testing, not entirely working.
//#define sharedCAN

// Use watchdog to reset if 10ms loop fails. -- timings currently not properly set.
//#define WATCHDOG


// Error state due to Coolant overtemp - no seperate indicator currently
//#define COOLANTSHUTDOWN

// Control cooling fan via APPS trigger.
#define FANCONTROL

#define PUMPMINIMUM_I ( 6 ) // 600mA?

// Debug aids.

// Use IVT
#define IVTEnable				// if not defined, IVT ignored and assumed present, giving a nominal voltage.

// Use BMS Messages.
#define BMSEnable				// if not defined, BMS ignored and assumed present.

// Send an error on incorrect data received
#define SENDBADDATAERROR

// Retransmit any received data deemed incorrect or implausible
//#define RETRANSMITBADDATA

// Transmit error messages and status on 2nd CANBUS also.

// Allow a 450ms window of brake + apps before throttle is cut.
#define APPSALLOWBRAKE

#define APPSBRAKETIME	300 //300ms brake allowance for apps before trigger cut power.

#define REGENMINIMUM 	10 // %

#define RTDMStopTime    3 // 3 seconds from entering RTDM before stop button active.

// Try to restart CANBUS if ECU goes offbus.
#define RECOVERCAN 

// continue to set DriveMode when not in TS active.
#define SETDRIVEMODEINIDLE

// Timeout Values, for bench testing with App if SIM defined and real car if not.

#define MS1000					(1000)

#define CYCLETIME				(20)

// const TickType_t CYCLETIME = 10;

//#define PROCESSLOOPTIME 		10   // should be 100 for 10ms in normal operation, bigger number for slower main loop in testing. - 50?
#define BMSTIMEOUT				450 // was 5 seconds as bodge
#define IVTTIMEOUT				450  // < 500ms for rules compliance on Power reading.
#define IMUTIMEOUT				30 // needs to be uptodate to be useful.
#define INVERTERTIMEOUT			100 // 10 cycles, 100ms.


#define StartupState			(0)
#define PreOperationalState		(1)
#define OperationalReadyState	(2)
#define IdleState				(3)
#define TSActiveState			(4)
#define RunningState			(5)
#define ReceivingData			(30)
#define InitialConfig			(31)
#define ReceiveAck				(1)
#define ReceiveErr				(99)
#define TestingState			(10)
#define LimpState				(20)
#define OperationalErrorState   (50)
#define ReceiveTimeout			(200)

#define FatalErrorState			99

// status codes to send for errors.

#define ReceivedCriticalError		0xFF
#define OperationalStateOverrun		0xF
#define PowerOnRequestBeforeReady 	0x10
#define PowerOnRequestTimeout   	0x11
#define HVlostError					0x12


#define InverterReceived		0
#define BMSReceived				1
#define IVTReceived				2
#define PowerNode1Received		3
#define PowerNode2Received		4
#define AnalogNode1Received		5
#define AnalogNode2Received		6

typedef enum {TEST, MAX, AUTOCROSS, ENDURANCE} MODE;

typedef struct {
	uint8_t brake_balance;

	uint8_t TorqueVectoring;

	float Torque_Req;
	uint32_t PowerLimit;
	bool AllowTorque;
	bool AllowRegen;
	
	uint8_t PedalProfile;
	bool RegenLight;

	uint8_t FanPowered;

	uint8_t APPSstatus;

	//////////////////////// Vehicle Mode
	MODE DrivingMode;					// Vehicle Mode Selection
	uint8_t Torque_Req_Max; 			// Maximum Torque Requested
	uint8_t Torque_Req_CurrentMax; 		// Current Maximum Torque Requested
	//////////////////////// Vehicle Dynamics Control
	bool AllowTV;						// Allow Torque Vectoring
	bool AllowTC;						// Allow Traction Control
	uint8_t PowerBalance; 				// 0-100% power balance between front and rear.
	////////////////////////

	uint8_t LimpRequest;
	uint8_t LimpActive;
	uint8_t LimpNM;
	bool LimpDisable;

	int32_t Current;
	int32_t VoltageINV;
	int32_t VoltageBMS;

	uint16_t HighestCellV;
	int32_t VoltageIVTAccu;
	int32_t Power;
	int32_t Wh;

	float SOC;

	uint8_t I_BrakeLight;
	uint8_t I_Buzzers;
	uint8_t I_IVT;
	uint8_t I_AccuPCBs;
	uint8_t I_AccuFans;
	uint8_t Freq_IMD;
	uint8_t DC_IMD;

	uint8_t I_LeftPump;
	uint8_t I_RightPump;

	bool HV_on;
	bool allowtsactivation;
	bool PRE_Done;

	float pedalreq;
	float MaxTorque;

} CarStateType;

// How frequently to send status messages in loops
#define STATUSLOOPCOUNT 		10 // how many loops between

typedef enum DeviceStatustype {
	INERROR,
	INERRORSTOPPING,
	OFFLINE,
	STOPPED,
	BOOTUP,
	PREOPERATIONAL,
	OPERATIONAL,
} DeviceStatus;

typedef struct {
	DeviceStatus NONE;
	DeviceStatus CAN1;
	DeviceStatus CAN0;
	bool FrontSpeedSensors;
	bool IVTEnabled;
	bool BMSEnabled;

	DeviceStatus Sensors;
	DeviceStatus CriticalSensors;
	DeviceStatus CriticalPower;
	DeviceStatus Inverter;
	DeviceStatus BMS;
	DeviceStatus IMU;
	DeviceStatus FLSpeed;
	DeviceStatus FRSpeed;
	DeviceStatus IVT;
	DeviceStatus EEPROM;
	DeviceStatus BrakeLight;

	DeviceStatus BPPS;
	DeviceStatus APPS1;
	DeviceStatus APPS2;
	DeviceStatus SteeringAngle;
	DeviceStatus WaterLevel;
	DeviceStatus HeavesFront;
	DeviceStatus HeavesRear;
	DeviceStatus Rolls1;
	DeviceStatus Rolls2;
	DeviceStatus BrakeFront;
	DeviceStatus BrakeRear;

	DeviceStatus AnalogNode1;
	DeviceStatus AnalogNode2;

	DeviceStatus PowerNode1;
	DeviceStatus PowerNode2;

	DeviceStatus Dash_BTNs;
	
//	char ;

	bool timeout;

} DeviceStateType;

// helpers
void storeBEint32(const uint32_t input, uint8_t Data[4]);
void storeBEint16(const uint16_t input, uint8_t Data[2]);
void storeLEint32(const uint32_t input, uint8_t Data[4]);
void storeLEint16(const uint16_t input, uint8_t Data[2]);

uint32_t getLEint32(const uint8_t data[4]);
uint16_t getLEint16(const uint8_t data[2]);
uint32_t getBEint32(const uint8_t data[4]);
uint16_t getBEint16(const uint8_t data[2]);

uint8_t getByte(const uint32_t input, const int8_t returnbyte);

char* getDeviceStatusStr(const DeviceStatus status);

int initECU(void);

#endif /* ECU_H_ */
