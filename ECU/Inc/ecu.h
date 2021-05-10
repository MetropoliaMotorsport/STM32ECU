/*
 * ecu.h
 *
 *  Created on: 29 Dec 2018
 *      Author: Visa
 */

#ifndef ECU_H_
#define ECU_H_

#include <stdbool.h>

#define LCDBUFFER

#define HPF20

//#define HPF19

#ifdef HPF20
    #define RTOS
	#define EEPROMSTORAGE
	#define SCREEN
	#define POWERNODES
	#define ANALOGNODES
	//#define MATLAB
	#define USEIMU
	#define PWMSTEERING
	#define LENZE
	#define MOTORCOUNT		(4)
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

// Use only one canbus for all functions, for bench testing.
//#define ONECAN

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
// #define retransmitIVT

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

#define APPSBRAKETIME	300 //300ms brake allowance for apps before trigger cut power.

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


#define MS1000					(1000)
#define MS1						(1)

#define CYCLETIME				(10)

// const TickType_t CYCLETIME = 10;

//#define SIM
#ifdef SIM
#define PDMTIMEOUT				500
#define BMSTIMEOUT				500
#define IVTTIMEOUT				500
#define IVTTIMEOUTLONG			500
#define IVTTIMEOUTWATTS			500
#define PROCESSLOOPTIME 		200
#define INVERTERTIMEOUT			500
#else
#define PDMTIMEOUT				450 // 450ms to be rules compliant
#define PROCESSLOOPTIME 		10   // should be 100 for 10ms in normal operation, bigger number for slower main loop in testing. - 50?
#define BMSTIMEOUT				450 // was 5 seconds as bodge
#define IVTTIMEOUT				450  // < 500ms for rules compliance on Power reading.
#define IVTTIMEOUTWATTS			450
#define IMUTIMEOUT				30 // needs to be uptodate to be useful.
#define INVERTERTIMEOUT			100 // 10 cycles, 100ms.
#define SICKTIMEOUT             20 // 2 cycles, then set speeds to zero.
#define STMADC
#endif
#ifdef MEMORATOR
#define MEMORATORTIMEOUT		2000 // 2 seconds, should be sending one message a second.
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


#define Inverter1ErrorBit		9
#define Inverter2ErrorBit		10

#define BMSVoltageErrorBit		11

#define InverterReceived		0
#define FLeftSpeedReceived		2 // TODO fix conflict?
#define FRightSpeedReceived		3
#define PedalADCReceived		4
#define BMSReceived				5
#define PDMReceived				6
#define MEMORATORReceived		7

#define IVTReceived				8

#define INVERTERRECEIVED		20

#define POWERNODERECEIVED


#define RearLeftInverter		0
#define RearRightInverter		1

#ifdef HPF20
#define FrontLeftInverter		2
#define FrontRightInverter		3
#endif

/*
#define INVERTERDISABLED		BOOTUP//1
#define INVERTERREADY			STOPPED//2 // preoperation, ready for TS.
#define INVERTERON				PREOPERATIONAL//3 // operating but not on, TS enabled.
#define INVERTEROPERATING		OPERATIONAL//4 // output enabled ( RTDM only )
#define INVERTERERROR			INERROR//-99
*/

extern volatile uint32_t ADCloops;

typedef struct {
	bool BOTS;
	bool InertiaSwitch;
	bool BSPDAfter;
	bool BSPDBefore;
	bool CockpitButton;
	bool LeftButton;
	bool RightButton;

	bool BMS;
	uint8_t BMSReason;
	bool IMD;
	bool AIROpen;

} ShutdownState;

volatile struct CarState {
	uint8_t brake_balance;

	char HighVoltageReady;
	bool TestHV;

	uint16_t COBID;

	uint8_t  TorqueVectoring;

	int32_t Torque_Req;
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

	int32_t ActualSpeed;

//	int32_t Wheel_Speed_Rear_Average;
//	int32_t Wheel_Speed_Average;

	uint8_t I_BrakeLight;
	uint8_t I_Buzzers;
	uint8_t I_IVT;
	uint8_t I_AccuPCBs;
	uint8_t I_AccuFans;
	uint8_t Freq_IMD;
	uint8_t DC_IMD;

	ShutdownState Shutdown;

} CarState;  // define external into realmain?


typedef enum DeviceStatustype {
	INERROR,
	OFFLINE,
	BOOTUP,
	STOPPED,
	PREOPERATIONAL,
	OPERATIONAL,
} DeviceStatus;

struct DeviceState {
	DeviceStatus CAN1;
	DeviceStatus CAN2;
	bool FrontSpeedSensors;
	bool IVTEnabled;
	bool BMSEnabled;
	bool LoggingEnabled;
	DeviceStatus ADC;
	uint16_t ADCSanity;
	DeviceStatus PWM;
	DeviceStatus Inverter;
	DeviceStatus Inverters[MOTORCOUNT];
	DeviceStatus BMS;
	DeviceStatus IMU;
	DeviceStatus PDM;
	DeviceStatus FLSpeed;
	DeviceStatus FRSpeed;
	DeviceStatus IVT;
	DeviceStatus LCD;
	DeviceStatus EEPROM;
	DeviceStatus Memorator;
	DeviceStatus BrakeLight;


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

	DeviceStatus PowerNodes;
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
	uint8_t  InvAllowReset[MOTORCOUNT];
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

	uint16_t INVReceiveStatus[MOTORCOUNT];
	uint16_t INVReceiveSpd[MOTORCOUNT];
	uint16_t INVReceiveTorque[MOTORCOUNT];

	uint32_t CANSendError1;
	uint32_t CANSendError2;
} volatile Errors;

// helpers
void swapByteOrder_int16(double *current, const int16_t *rawsignal, size_t length);
void storeBEint32(uint32_t input, uint8_t Data[4]);
void storeBEint16(uint16_t input, uint8_t Data[2]);
void storeLEint32(uint32_t input, uint8_t Data[4]);
void storeLEint16(uint16_t input, uint8_t Data[2]);

uint32_t getLEint32( uint8_t data[4] );
uint16_t getLEint16( uint8_t data[2] );
uint32_t getBEint32( uint8_t data[4] );
uint16_t getBEint16( uint8_t data[2] );

uint8_t getByte(uint32_t input, int8_t returnbyte);

int initECU( void );

#endif /* ECU_H_ */
