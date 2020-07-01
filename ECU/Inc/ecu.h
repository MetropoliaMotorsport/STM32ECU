/*
 * ecu.h
 *
 *  Created on: 29 Dec 2018
 *      Author: Visa
 */

#ifndef ECU_H_
#define ECU_H_

#include <stdbool.h>


#define HPF20

//#define HPF2019

#ifdef HPF20
	#define EEPROMSTORAGE
	#define SCREEN
	#define POWERNODES
	#define ANALOGNODES
#endif
// Calibration settings for pedals.

#define ACCELERATORLZERO 4200
#define ACCELERATORLMAX  53000

#define ACCELERATORRZERO 4800
#define ACCELERATORRMAX  53000

#define BRAKEZERO 14100 // 0 bar?
#define BRAKEMAX  62914 // 240 bar settings.

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
#define ONECAN

// Both CAN's are connected to one bus for bench testing, not entirely working.
//#define sharedCAN

// Use watchdog to reset if 10ms loop fails. -- timings currently not properly set.
//#define WATCHDOG

// Use onboard ADC, else expect ADC values by CAN.
#define STMADC

// Use basic torquevectoring
//#define SIMPLETORQUEVECTOR
#define TORQUEVECTORSTARTANGLE 20  //40
#define TORQUEVECTORSTOPANGLE  70  //90
#define TORQUEVECTORMAXNM	   8

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


#define MEMORATOR

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

#define IVTEnable				// if not defined, IVT ignored and assumed present, giving a nominal voltage.

// HV will still be allowed without TSAL connected
#define NOTSAL

// Use BMS Messages.
#define BMSEnable				// if not defined, BMS ignored and assumed present.

// Retransmit IVT messages for BMS
#define retransmitIVT

// Define whether front speed encoders are expected.
//#define FRONTSPEED				// enable front speed encoder reading.

// do not go to error state for non crucial can devices going offbus/timing out.
//#define NOTIMEOUT

// do not go to error state if IVT goes off bus.
// #define NOIVTTIMEOUT

// Allow auto reset with shutdown buttons / DC Undervoltage on inverters.
#define AUTORESET

// Send an error on incorrect data received
#define SENDBADDATAERROR

// Retransmit any received data deemed incorrect or implausible
//#define RETRANSMITBADDATA

// Transmit error messages and status on 2nd CANBUS also.
#ifndef ONECAN
#define CAN2ERRORSTATUS
#endif

// Allow a 450ms window of brake + apps before throttle is cut.
#define APPSALLOWBRAKE

#define APPSBRAKETIME	3000 //300ms brake allowance for apps before trigger cut power.

// Allow limp mode to be exited on request.
#define ALLOWLIMPCANCEL


#define RTDMStopTime    3 // 3 seconds from entering RTDM before stop button active.

// Try to restart CANBUS if ECU goes offbus.
#define RECOVERCAN

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
#ifdef MEMORATOR
#define MEMORATORTIMEOUT		20000 // 2 seconds, should be sending one message a second.
#endif

#ifdef HPF20
#define NODETIMEOUT				1000
#endif

#define DEBUGMCU 0x5C001000

#define REVV 					0x2003
#define REVY 					0x1003

#define MaxRunningErrorCount    10
#define ReduceErrorCountRate	10

// Do not process APPS position ADC
//#define NOAPPS

// Do not read steering angle ADC
//#define NOSTEERING

// Do not process Brake pressure ADC
//#define NOBRAKES

// Do not process Coolant temperature ADC
//#define NOTEMPERATURE

// Do not process Drive mode selector ADC
//#define NODRIVINGMODE

#define StartupState			0
#define PreOperationalState		1
#define OperationalReadyState	2
#define IdleState				3
#define TSActiveState			4
#define RunningState			5
#define ReceivingData			30
#define ReceiveAck				1
#define ReceiveErr				99
#define TestingState			10
#define LimpState				20
#define OperationalErrorState   50
#define ReceiveTimeout			200

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


#define InverterRLErrorBit		9
#define InverterRRErrorBit		10

#define BMSVoltageErrorBit		11
#ifdef HPF20
#define InverterFLErrorBit		9
#define InverterFRErrorBit		10
#endif



#define Inverter1Received		0 //
#define Inverter2Received		2

#define InverterReceived		0
//#define InverterRLReceived		1
#ifdef HPF20
//#define InverterFReceived		2
//#define InverterFLReceived		3
#else
  #define FLeftSpeedReceived		2 // TODO fix conflict?
  #define FRightSpeedReceived		3
#endif
#define PedalADCReceived		4
#define BMSReceived				5
#define PDMReceived				6
#define MEMORATORReceived		7


#define IVTReceived				8

//#define YAWReceived			8

#define INVERTERRECEIVED		20

#define POWERNODERECEIVED


#define RearLeftInverter		0
#define RearRightInverter		1

#ifdef HPF20
#define FrontLeftInverter		2
#define FrontRightInverter		3
#endif


#define INVERTERDISABLED		1
#define INVERTERREADY			2 // preoperation, ready for TS.
#define INVERTERON				3 // operating but not on, TS enabled.
#define INVERTEROPERATING		4 // output enabled ( RTDM only )
#define INVERTERERROR			-99


#ifdef HPF20
#define INVERTERCOUNT			(2)
#define Inverter1				(0)
#define Inverter2				(2)
#else
#define INVERTERCOUNT			(2)
#endif
extern volatile uint32_t ADCloops;

typedef struct { // new structure for inverter related data, so that it can be used as general pointer.
	uint8_t InverterNum;
	bool HighVoltageAllowed;
	uint16_t InvState;
	uint16_t InvBadStatus;
	uint16_t InvStateCheck;
	uint16_t InvStateCheck3;
	uint16_t InvCommand;

	uint16_t Torque_Req;
	uint16_t InvTorque;

	int32_t Speed;
	uint16_t COBID;
} InverterState;  // define external into realmain?

volatile struct CarState {
	uint8_t brake_balance;

	char HighVoltageReady;
	uint8_t TestHV;

	char BMS_relay_status;
	char IMD_relay_status;
	char BSPD_relay_status;
	char AIROpen;
	char ShutdownSwitchesClosed;

	InverterState Inverters[INVERTERCOUNT];
	uint16_t COBID;

#ifdef TORQUEVECTOR
	uint8_t  TorqueVectoring;
#endif

	uint8_t Torque_Req_Max;
    uint8_t Torque_Req_CurrentMax;
    uint32_t PowerLimit;
    uint8_t DrivingMode;
    uint8_t PedalProfile;
    
    uint8_t FanPowered;

	uint8_t APPSstatus;
    
    uint8_t LimpRequest;
    bool LimpActive;
    bool LimpDisable;

	int32_t Current;
	int32_t VoltageINV;
	int32_t VoltageBMS;
	int32_t VoltageIVTAccu;
	int32_t VoltageLV;
	int32_t CurrentLV;
	int32_t VoltageAIRPDM;
	int32_t Power;

	int32_t Speed[4];

//	int32_t Wheel_Speed_Rear_Average;
//	int32_t Wheel_Speed_Average;

//	uint8_t StopLED;

	uint8_t I_BrakeLight;
	uint8_t I_Buzzers;
	uint8_t I_IVT;
	uint8_t I_AccuPCBs;
	uint8_t I_AccuFans;
	uint8_t Freq_IMD;
	uint8_t DC_IMD;

} CarState, LastCarState, ErrorCarState;  // define external into realmain?


/*
struct Device {
uint8_t	State;
uint8_t id_count;
 *CANid;
		id_count
		[can ids]
			id
			Timeout period Used to timeout operational state, if zero, don't timeout on this data.
			id data handler func. ( return bool, data good or bad )
			Errors.FLSpeedReceive++; ( data error )
			LastReceived
			timeouterrors.


	};

};
*/

typedef enum DeviceStatustype {
	BOOTUP,
	STOPPED,
	PREOPERATIONAL,
	OPERATIONAL,
	OFFLINE,
	INERROR
} DeviceStatus;

struct DeviceState {
	DeviceStatus CAN1;
	DeviceStatus CAN2;
	bool FrontSpeedSensors;
	bool IVTEnabled;
	bool BMSEnabled;
	bool LoggingEnabled;
	DeviceStatus ADC;
	DeviceStatus Inverters[INVERTERCOUNT];
	DeviceStatus BMS;
	DeviceStatus PDM;
	DeviceStatus FLSpeed;
	DeviceStatus FRSpeed;
	DeviceStatus IVT;
	DeviceStatus LCD;
	DeviceStatus EEPROM;
	DeviceStatus Memorator;


	DeviceStatus AnalogNode1;
	DeviceStatus AnalogNode9;
	DeviceStatus AnalogNode10;
	DeviceStatus AnalogNode11;
	DeviceStatus AnalogNode12;
	DeviceStatus AnalogNode13;
	DeviceStatus AnalogNode14;

	DeviceStatus AnalogNode15; // tyre temps FL
	DeviceStatus AnalogNode16; // tyre temps FR
	DeviceStatus AnalogNode17; // tyre temps RL
	DeviceStatus AnalogNode18; // tyre temps RR


	DeviceStatus PowerNode33; // [BOTS, inertia switch, BSPD.], Telemetry, front power
	DeviceStatus PowerNode34; // [shutdown switches.], inverters, ECU, Front,
	DeviceStatus PowerNode35; // Cooling ( fans, pumps )
	DeviceStatus PowerNode36; // BRL, buzz, IVT, ACCUPCB, ACCUFAN, imdfreq, dc_imd?
	DeviceStatus PowerNode37; // [?], Current, TSAL.
//	char ;
} volatile DeviceState;


struct ErrorCount {
	uint16_t OperationalReceiveError;
	uint16_t State;
	uint8_t  InvAllowReset[INVERTERCOUNT];
//	uint8_t  LeftInvAllowReset;
//    uint8_t  RightInvAllowReset;
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

 // specific devices
	bool	 ADCSent;
	uint32_t eepromerror;

	uint32_t ADCError;
	uint16_t ADCTimeout;
	uint16_t ADCErrorState;

	uint16_t INVReceiveStatus[INVERTERCOUNT];
	uint16_t INVReceiveSpd[INVERTERCOUNT];
	uint16_t INVReceiveTorque[INVERTERCOUNT];

	uint16_t CANSendError1;
	uint16_t CANSendError2;
} volatile Errors;

// helpers
void swapByteOrder_int16(double *current, const int16_t *rawsignal, size_t length);
void storeBEint32(uint32_t input, uint8_t Data[4]);
void storeBEint16(uint16_t input, uint8_t Data[2]);
void storeLEint32(uint32_t input, uint8_t Data[4]);
void storeLEint16(uint16_t input, uint8_t Data[2]);

uint8_t getByte(uint32_t input, int8_t returnbyte);


#endif /* ECU_H_ */
