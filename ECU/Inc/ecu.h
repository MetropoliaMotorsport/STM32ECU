/*
 * ecu.h
 *
 *  Created on: 29 Dec 2018
 *      Author: Visa
 */

#ifndef ECU_H_
#define ECU_H_


// Calibration settings

#define ACCELERATORLZERO 11000
//#define ACCELERATORLMAX  51000
#define ACCELERATORLMAX  50000

#define ACCELERATORRZERO 11500
//#define ACCELERATORRMAX  53000
#define ACCELERATORRMAX  50000

#define BRAKEZERO 14100 // 0 bar?
#define BRAKEMAX  62914 // 240 bar settings.



#define USEADC3
//#include "ecumain.h"

//#define sharedCAN // bench testing.
//#define WATCHDOG
#define STMADC
#define debugrun
//#define debug
//#define errorLED
#define ALLOWLIMPCANCEL

#define everyloop

// device enable/disable etc

#define LOGGINGON
#define IVTEnable				// if not defined, IVT ignored and assumed present, giving a nominal voltage.
#define BMSEnable				// if not defined, BMS ignored and assumed present.
#define retransmitIVT
//#define FRONTSPEED				// enable front speed encoder reading.
//#define NOTIMEOUT
#define NOIVTTIMEOUT
//#define SENDBADDATAERROR
//#define RETRANSMITBADDATA
#define CAN2ERRORSTATUS
//#define RECOVERCAN

//#define NOTORQUEREQUEST

#define STATUSLOOPCOUNT 		10 // how many loops between regular status updates.
#define LOGLOOPCOUNTFAST	 	1  // x many loops to send fast log data
#define LOGLOOPCOUNTSLOW	 	10


#define MINHV					500 // minimum voltage to allow TS enable.
//#define ONECAN // only use CAN1 to ease testing.

//50ms


//#define SIM
#ifdef SIM
#define PDMTIMEOUT				5000
#define BMSTIMEOUT				5000
#define IVTTIMEOUT				5000
#define IVTTIMEOUTLONG			5000
#define IVTTIMEOUTWATTS			5000
#define PROCESSLOOPTIME 		2000
#define INVERTERTIMEOUT			1000
#else
#define PDMTIMEOUT				4500 //
#define PROCESSLOOPTIME 		100 // should be 100 for 10ms in normal operation, bigger number for slower main loop in testing. - 50?
#define BMSTIMEOUT				50000 // 5 seconds
#define IVTTIMEOUT				20000
//#define IVTTIMEOUTLONG			800
#define IVTTIMEOUTWATTS			50000
#define INVERTERTIMEOUT			1000 // 10 cycles, 100ms.
#define SICKTIMEOUT             200 // 2 cycles, then set speeds to zero.
#define STMADC
#endif


#define MaxRunningErrorCount    10
#define ReduceErrorCountRate	10

//#define NOAPPS
//#define NOSTEERING
//#define NOBRAKES
//#define NOTEMPERATURE
//#define NODRIVINGMODE


#define StartupState			0
#define PreOperationalState		1
#define OperationalReadyState	2
#define IdleState				3
#define TSActiveState			4
#define RunningState			5
#define ReceivingConfig			30
#define TestingState			10
#define LimpState				20
#define OperationalErrorState   50
#define FatalErrorState			99

// APPS

#define APPSBrakeHard			30 // 70
#define APPSBrakeRelease		10 // 30
#define RTDMBRAKEPRESSURE		30

uint16_t ErrorCode; // global error code.

// status codes to send for errors.

#define OperationalStateOverrun		0xF
#define PowerOnRequestBeforeReady 	0x10
#define PowerOnRequestTimeout   	0x11


#define BrakeFErrorBit			0
#define BrakeRErrorBit			1
#define CoolantLErrorBit		2
#define CoolantRErrorBit		3
#define SteeringAngleErrorBit	4
#define AccelLErrorBit			5
#define AccelRErrorBit			6
#define DrivingModeErrorBit	    7


#define InverterLErrorBit		9
#define InverterRErrorBit		10

#define BMSVoltageErrorBit		11

#define PDMReceived				0
#define BMSReceived				1
#define InverterReceived		2
#define InverterLReceived		2
#define FLeftSpeedReceived		3
#define FRightSpeedReceived		4
#define PedalADCReceived		5
#define IVTReceived				6
#define InverterRReceived		7
//#define YAWReceived			8


#define LeftInverter			0
#define RightInverter			1

#define INVERTERDISABLED		1
#define INVERTERREADY			2 // preoperation, ready for TS.
#define INVERTERON				3 // operating but not on, TS enabled.
#define INVERTEROPERATING		4 // output enabled ( RTDM only )
#define INVERTERERROR			-99

extern volatile uint32_t ADCloops;

volatile struct CarState {
	uint8_t brake_balance;

	char HighVoltageAllowedR;
	char HighVoltageAllowedL;
	char HighVoltageReady;

	char BMS_relay_status;
	char IMD_relay_status;
	char BSPD_relay_status;
	char AIROpen;

	uint16_t LeftInvState;
	uint16_t LeftInvBadStatus;
	uint16_t LeftInvStateCheck;
	uint16_t LeftInvStateCheck3;
	uint16_t LeftInvCommand;

	uint16_t RightInvState;
	uint16_t RightInvBadStatus;
	uint16_t RightInvStateCheck;
	uint16_t RightInvStateCheck3;
	uint16_t RightInvCommand;

	uint16_t Torque_Req_L;
	uint16_t Torque_Req_R;
	uint16_t LeftInvTorque;
	uint16_t RightInvTorque;

	uint8_t Torque_Req_Max;
    uint8_t Torque_Req_CurrentMax;
    
	uint8_t APPSstatus;
    
    uint8_t LimpRequest;
    uint8_t LimpActive;
    uint8_t LimpDisable;

	int32_t Current;
	int32_t VoltageINV;
	int32_t VoltageBMS;
	int32_t VoltageIVTAccu;
	int32_t Power;

	int32_t SpeedRL;
	int32_t SpeedRR;
	int32_t SpeedFL;
	int32_t SpeedFR;

//	int32_t Wheel_Speed_Rear_Average;
//	int32_t Wheel_Speed_Average;

//	uint8_t StopLED;
} volatile CarState, LastCarState, ErrorCarState;


struct DeviceState {
	char CAN1;
	char CAN2;
	char FrontSpeedSensors;
	char IVTEnabled;
	char BMSEnabled;
	char LoggingEnabled;
	char ADC;
	char InverterL;
	char InverterR;
	char BMS;
	char PDM;
	char FLSpeed;
	char FRSpeed;
	char IVT;
} volatile DeviceState;

struct ErrorCount {
	uint16_t OperationalReceiveError;
	uint16_t State;
	uint8_t  LeftInvAllowReset;
    uint8_t  RightInvAllowReset;
	uint16_t ErrorReason;
	uint16_t ErrorPlace;

	uint8_t  InverterError;

	uint8_t InverterErrorHistory[8][8];
	uint8_t InverterErrorHistoryPosition;
	uint8_t InverterErrorHistoryID[8];

	uint32_t CANError;
	uint32_t CANCount1;
	uint32_t CANCount2;
	uint32_t CANTimeout;

	uint16_t BMSError;
	uint16_t BMSTimeout;
	uint16_t BMSReceive;
	uint16_t BMSReceiveMax;

	uint32_t ADCError;
	uint16_t ADCTimeout;
	uint16_t ADCErrorState;

	uint16_t IVTIReceive;
	uint16_t IVTTimeout;

	uint16_t IVTU1Receive;
//	uint16_t IVTU1Timeout;

	uint16_t IVTU2Receive;
//	uint16_t IVTU2Timeout;

	uint16_t IVTWReceive;
//	uint16_t IVTWTimeout;

	uint16_t INVLReceiveStatus;
	uint16_t INVLReceiveSpd;
	uint16_t INVLReceiveTorque;

	uint16_t INVRReceiveStatus;
	uint16_t INVRReceiveSpd;
	uint16_t INVRReceiveTorque;

	uint16_t PDMError;
	uint16_t PDMTimeout;
	uint16_t PDMReceive;

	uint16_t FLSpeedError;
	uint16_t FLSpeedTimeout;
	uint16_t FLSpeedReceive;

	uint16_t FRSpeedError;
	uint16_t FRSpeedTimeout;
	uint16_t FRSpeedReceive;
	uint16_t CANSendError1;
	uint16_t CANSendError2;

} volatile Errors;

// helpers
void swapByteOrder_int16(double *current, const int16_t *rawsignal, size_t length);
void storeBEint32(uint32_t input, uint8_t Data[4]);
void storeBEint16(uint16_t input, uint8_t Data[2]);
void storeLEint32(uint32_t input, uint8_t Data[4]);
void storeLEint16(uint16_t input, uint8_t Data[2]);

char getByte(uint32_t input, int8_t returnbyte);


#endif /* ECU_H_ */