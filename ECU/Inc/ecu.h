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

//#define WATCHDOG

//#define TWOWHEELS

//#define BACKUPCAN

//#define SETTORQUEMODE

#define TSALP

//#define BENCH // benchtest compile, don't complain about missing BMS or IMD messages. Doesn't check canbuserrors.

#define TESTBOARD

#define RTTDEBUG

#define LCDBUFFER

//#define NOBRAKELIGHTCONTROL

//#define ADCDEBUGINFO

//#define TORQUEERRORCHECK

#define IGNOREI2CERR

#define MAXERROROUTPUT       (40)

#define TORQUE_DIFFERENCE (30)
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

//#define HPF19

// HPF 20 doesn't use any local ADC, all via analogue nodes, and PWM.
#ifdef HPF20
    #define RTOS
	#define EEPROMSTORAGE
	#define SCREEN
	#define POWERNODES
	#define ANALOGNODES
//	#define MATLAB
	#define USEIMU
	#define PWMSTEERING
	#define LENZE
#ifdef TWOWHEELS
	#define MOTORCOUNT		(2)
#else
	#define MOTORCOUNT		(4)
#endif
	#define retransmitIMU
#endif

#ifdef HPF19
	// Use onboard ADC, else expect ADC values by CAN.
	#define STMADC
// Retransmit IVT messages for BMS
// #define retransmitIVT
#endif

// Calibration settings for pedals.

#define ACCELERATORLZERO 4200
#define ACCELERATORLMAX  53000

#define ACCELERATORRZERO 4800
#define ACCELERATORRMAX  53000

#define BRAKEZERO 14100 // 0 bar?
#define BRAKEMAX  62914 // 240 bar settings.

// Brake pressure values

#ifdef FSG23
//changed new BPsensors from 240bar 1v-5v to 140bar 0,5v-4,5v
#define APPSBrakeLight			42
#define APPSBrakeHard			58 //
#define APPSBrakeRelease		42 //
#define RTDMBRAKEPRESSURE		6 // set a CAN trigger to allow this easier without reprogramming for wheels up testing.

#else
#ifdef HPF2023
#define APPSBrakeLight			3
#define APPSBrakeHard			30 // 70
#define APPSBrakeRelease		2 // 30
#define RTDMBRAKEPRESSURE		7 // set a CAN trigger to allow this easier without reprogramming for wheels up testing.

#else
#define APPSBrakeLight			65
#define APPSBrakeHard			90 // 70
#define APPSBrakeRelease		65 // 30
//#define RTDMBRAKEPRESSURE		30
#define RTDMBRAKEPRESSURE		10 // set a CAN trigger to allow this easier without reprogramming for wheels up testing.
#endif
#endif

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

#define PUMPMINIMUM_I ( 6 ) // 600mA?

// Trigger percentage for FAN on APPS
#define TORQUEFANLATCHPERCENTAGE 30

// Use second PDM message for compatibility for fan control
#define PDMSECONDMESSAGE

// Check shutdown switches as part of error state.
//#define SHUTDOWNSWITCHCHECK

// Coolant overtemp limits.
#define COOLANTLIMPTEMP 75
#define COOLANTLIMPTEMPHALF 70
#define COOLANTLIMPEXITTEMP 65
#define COOLANTMAXTEMP	80

// Debug aids.

#define debugrun
//#define debug
// Show extra error LED statuses, non rules compliant.
//#define errorLED

// Send status message every cycle
#define everyloop

// send log messages for AIM / remote reading of status
//#define LOGGINGON

// Use IVT
#define IVTEnable				// if not defined, IVT ignored and assumed present, giving a nominal voltage.

// HV will still be allowed without TSAL connected
#define NOTSAL

// Use BMS Messages.
#define BMSEnable				// if not defined, BMS ignored and assumed present.


// Define whether front speed encoders are expected.
//#define FRONTSPEED				// enable front speed encoder reading.

// do not go to error state for non crucial can devices going offbus/timing out.
//#define NOTIMEOUT

// do not go to error state if IVT goes off bus.
// #define NOIVTTIMEOUT

// Allow auto reset with shutdown buttons / DC Undervoltage on inverters.
// #define AUTORESET

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
//#define MINSPEEDFORCONTROL   100

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

#define CYCLETIME				(20)

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
//#define PROCESSLOOPTIME 		10   // should be 100 for 10ms in normal operation, bigger number for slower main loop in testing. - 50?
#define BMSTIMEOUT				450 // was 5 seconds as bodge
#define IVTTIMEOUT				450  // < 500ms for rules compliance on Power reading.
#define IVTTIMEOUTWATTS			450
#define IMUTIMEOUT				30 // needs to be uptodate to be useful.
#define INVERTERTIMEOUT			100 // 10 cycles, 100ms.
#define SICKTIMEOUT             20 // 2 cycles, then set speeds to zero.
#endif
#ifdef MEMORATOR
#define MEMORATORTIMEOUT		2000 // 2 seconds, should be sending one message a second.
#endif

#ifdef HPF20
#define NODETIMEOUT				200
#define NODECRITICALTIMEOUT		50
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

// status codes to send for errors.

#define ReceivedCriticalError		0xFF
#define OperationalStateOverrun		0xF
#define PowerOnRequestBeforeReady 	0x10
#define PowerOnRequestTimeout   	0x11
#define HVlostError					0x12

#ifdef HPF20
#define BrakeFReceivedBit			ANode11Bit
#define BrakeRReceivedBit			ANode11Bit
#define CoolantLReceivedBit			2
#define CoolantRReceivedBit			3
#define SteeringAngleReceivedBit	31
#define AccelLReceivedBit			ANode1Bit
#define AccelRReceivedBit			ANode11Bit
#endif

#ifdef HPF19
#define BrakeFReceivedBit			0
#define BrakeRReceivedBit			1
#define CoolantLReceivedBit			2
#define CoolantRReceivedBit			3
#define SteeringAngleReceivedBit	31
#define AccelLReceivedBit			ANode1Bit
#define AccelRReceivedBit			ANode11Bit
#define DrivingModeReceivedBit	    7
#endif

#define Inverter1ErrorBit		9
#define Inverter2ErrorBit		10

#define BMSVoltageErrorBit		11

#define InverterReceived		0
//#define FLeftSpeedReceived		2
//#define FRightSpeedReceived		3
#define PedalADCReceived		4
#define BMSReceived				5
#define PDMReceived				6
#define MEMORATORReceived		7

#define IVTReceived				8

#define PNodeReceived			9

#define INVERTERRECEIVED		20

//#define POWERNODERECEIVED

#if 0
#define RearLeftInverter		0
#define RearRightInverter		1

#ifdef HPF20
#define FrontLeftInverter		2
#define FrontRightInverter		3
#endif
#endif

extern volatile uint32_t ADCloops;

typedef struct {
	uint8_t brake_balance;

	uint8_t TorqueVectoring;

	float Torque_Req;
	uint8_t Torque_Req_Max;
    uint8_t Torque_Req_CurrentMax;
    uint32_t PowerLimit;
    bool AllowTorque;
    bool AllowRegen;
    uint8_t DrivingMode;
    uint8_t PedalProfile;
    bool RegenLight;

    uint8_t FanPowered;

	uint8_t APPSstatus;

    uint8_t LimpRequest;
    uint8_t LimpActive;
	uint8_t LimpNM;
    bool LimpDisable;

	int32_t Current;
	int32_t VoltageINV;
	int32_t VoltageBMS;
	uint16_t LowestCellV;
	uint16_t HighestCellV;
	int32_t VoltageIVTAccu;
	int32_t VoltageLV;
	int32_t CurrentLV;
	int32_t VoltageAIRPDM;
	int32_t Power;
	int32_t Wh;
    uint8_t BMSSOC;
	float   SOC;

	uint8_t I_BrakeLight;
	uint8_t I_Buzzers;
	uint8_t I_IVT;
	uint8_t I_AccuPCBs;
	uint8_t I_AccuFans;
	uint8_t Freq_IMD;
	uint8_t DC_IMD;

	uint8_t I_LeftFans;
	uint8_t I_RightFans;
	uint8_t I_LeftPump;
	uint8_t I_RightPump;

	bool allowtsactivation;

} CarStateType;

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
	bool LoggingEnabled;
	DeviceStatus ADC;
	uint16_t ADCSanity;
	DeviceStatus Sensors;
	DeviceStatus CriticalSensors;
	DeviceStatus CriticalPower;
	DeviceStatus PWM;
	DeviceStatus Inverter;
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

	bool timeout;


} DeviceStateType;

// helpers
void storeBEint32(const uint32_t input, uint8_t Data[4]);
void storeBEint16(const uint16_t input, uint8_t Data[2]);
void storeLEint32(const uint32_t input, uint8_t Data[4]);
void storeLEint16(const uint16_t input, uint8_t Data[2]);

uint32_t getLEint32( const uint8_t data[4] );
uint16_t getLEint16( const uint8_t data[2] );
uint32_t getBEint32( const uint8_t data[4] );
uint16_t getBEint16( const uint8_t data[2] );

uint8_t getByte(const uint32_t input, const int8_t returnbyte);

char * getDeviceStatusStr( const DeviceStatus status );

int initECU( void );

#endif /* ECU_H_ */
