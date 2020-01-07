/*
 * ecu.h
 *
 *  Created on: 29 Dec 2018
 *      Author: Visa
 *      Edited by Markus Jahn for HPF017
 */

#ifndef ECU_H_
#define ECU_H_

// Calibration settings for pedals.

#define ACCELERATORLZERO 2700
#define ACCELERATORLMAX  49000

#define ACCELERATORRZERO 2000
#define ACCELERATORRMAX  25000

#define BRAKEZERO 500 // 0 bar?
#define BRAKEMAX  65000 // 240 bar settings.

// Brake pressure values

#define APPSBrakeHard			30 // 70
#define APPSBrakeRelease		10 // 30
//#define RTDMBRAKEPRESSURE		30
#define RTDMBRAKEPRESSURE		10 // set a CAN trigger to allow this easier without reprogramming for wheels up testing.

#define LIMPNM					10 // limp mode torque

// Minimum acceptable voltage on TS for startup.
#define MINHV					500 // minimum voltage to allow TS enable.

// Enable workaround to use second ADC for one of the APPS pedals due to shield issue.
#define USEADC3

// Use only one canbus for all functions, for bench testing. Not fully working.
//#define ONECAN

// Both CAN's are connected to one bus for bench testing, not entirely working.
//#define sharedCAN

// Use watchdog to reset if 10ms loop fails. -- timings currently not properly set.
//#define WATCHDOG

// Use onboard ADC, else expect ADC values by CAN.
#define STMADC

// Use basic torquevectoring
#define TORQUEVECTOR
#define TORQUEVECTORSTARTANGLE 20  //40
#define TORQUEVECTORSTOPANGLE  70  //90
#define TORQUEVECTORMAXNM	   8   // Can have x2 difference between L&R

// Use green dash LED to indicate shutdownswitch status.
#define SHUTDOWNSWITCHSTATUS

// Error state due to Coolant overtemp - no seperate indicator currently
//#define COOLANTSHUTDOWN

// Control cooling fan via APPS trigger.
#define FANCONTROL

// Trigger percentage for FAN on APPS
#define TORQUEFANLATCHPERCENTAGE 30

// Use second PDM message for compatibility for fan control
#define PDMSECONDMESSAGE

// Check shutdown switches as part of error state.
#define SHUTDOWNSWITCHCHECK

// Coolant overtemp limits.
#define COOLANTLIMPTEMP 75
#define COOLANTLIMPEXITTEMP 70
#define COOLANTMAXTEMP	80

// Debug aids.

#define debugrun
//#define debug
// Show extra error LED statuses, non rules compliant.
//#define errorLED

// Send status message every cycle
#define everyloop

// send log messages for AIM / remote reading of status
#define LOGGINGON

// Use IVT

// #define IVTEnable				// hpf017 no ivt. if not defined, IVT ignored and assumed present, giving a nominal voltage.

// HV will still be allowed without TSAL connected
#define NOTSAL

// Use BMS Messages.
//#define BMSEnable				// hpf017 if not defined, BMS ignored and assumed present.

// Retransmit IVT messages for BMS
//#define retransmitIVT          // hpf017

// Define whether front speed encoders are expected.
//#define FRONTSPEED				// enable front speed encoder reading.

// do not go to error state for non crucial can devices going offbus/timing out.
// #define NOTIMEOUT

// do not go to error state if IVT goes off bus.
#define NOIVTTIMEOUT

// Allow auto reset with shutdown buttons / DC Undervoltage on inverters.
#define AUTORESET

// Send an error on incorrect data received
#define SENDBADDATAERROR

// Retransmit any received data deemed incorrect or implausible
//#define RETRANSMITBADDATA

// Transmit error messages and status on 2nd CANBUS also.
#define CAN2ERRORSTATUS

// Allow a 450ms window of brake + apps before throttle is cut.
#define APPSALLOWBRAKE

#define APPSBRAKETIME	1000 //300ms brake allowance for apps before trigger cut power.

// Allow limp mode to be exited on request.
#define ALLOWLIMPCANCEL

// Try to restart CANBUS if ECU goes offbus.
//#define RECOVERCAN

// Do not send any torque request to inverters, for bench testing safely.
//#define NOTORQUEREQUEST

// Very simple attempt at some control code.
//#define CONTROLTEST
#define MINSPEEDFORCONTROL   100


// continue to set DriveMode when not in TS active.
#define SETDRIVEMODEINIDLE

// How frequently to send status messages in loops
#define STATUSLOOPCOUNT 		10 // how many loops between regular status updates.

// Log message frequency
#define LOGLOOPCOUNTFAST	 	1
#define LOGLOOPCOUNTSLOW	 	10

// Timeout Values, for bench testing with App if SIM defined and real car if not.

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
#define PDMTIMEOUT				4500 // 450ms to be rules compliant
#define PROCESSLOOPTIME 		100   // should be 100 for 10ms in normal operation, bigger number for slower main loop in testing. - 50?
#define BMSTIMEOUT				50000 // 5 seconds
#define IVTTIMEOUT				4500  // < 500ms for rules compliance on Power reading.
#define IVTTIMEOUTWATTS			4500
#define INVERTERTIMEOUT			1000 // 10 cycles, 100ms.
#define SICKTIMEOUT             200 // 2 cycles, then set speeds to zero.
#define STMADC
#endif

#define DEBUGMCU 0x5C001000

#define REVV 					0x2003
#define REVY 					0x1003

#define MaxRunningErrorCount    50
#define ReduceErrorCountRate	5

// Do not process APPS position ADC
//#define NOAPPS

// Do not read steering angle ADC
#define NOSTEERING

// Do not process Brake pressure ADC
//#define NOBRAKES

// Do not process Coolant temperature ADC
#define NOTEMPERATURE

// Do not process Drive mode selector ADC
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
	uint8_t TestHV;

	char BMS_relay_status;
	char IMD_relay_status;
	char BSPD_relay_status;
	char AIROpen;
	char ShutdownSwitchesClosed;

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
#ifdef TORQUEVECTOR
	uint8_t  TorqueVectoring;
#endif

	uint8_t Torque_Req_Max;
    uint8_t Torque_Req_CurrentMax;
    uint32_t PowerLimit;
    uint8_t DrivingMode;
    
    uint8_t FanPowered;

	uint8_t APPSstatus;
    
    uint8_t LimpRequest;
    uint8_t LimpActive;
    uint8_t LimpDisable;

	int32_t Current;
	int32_t VoltageINV;
	int32_t VoltageBMS;
	int32_t VoltageIVTAccu;
	int32_t VoltageLV;
	int32_t CurrentLV;
	int32_t VoltageAIRPDM;
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

	uint16_t IVTU2Receive;

	uint16_t IVTWReceive;

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
