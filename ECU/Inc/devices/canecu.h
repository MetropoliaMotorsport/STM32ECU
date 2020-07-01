/*
 * ecu.h
 *
 *  Created on: 27 Apr 2019
 *      Author: Visa
 */

#ifndef CANECU_H_
#define CANECU_H_

//#include "ecumain.h"

// definition of CAN ID's for nodes - Bus definitions not currently used

#define CANBUS0 				hfdcan2
#define	CANBUS1					hfdcan1


#define BMSBASE_ID				0x8
#define BMSVOLT_ID				BMSBASE_ID+3 // 0xB is voltage.
#define FLSpeed_COBID			0x71 // 112 // 0x70 orig
#define FLSpeed_BUS				CAN1
#define FRSpeed_COBID			0x70 // 113  // 0x71 orig
#define FRSpeed_BUS				CAN1
#define IVTCmd_ID		    	0x411
#define IVTMsg_ID			    0x511
#define IVTI_ID			    	0x521
#define IVTU1_ID		    	0x522
#define IVTU2_ID		    	0x523
#define IVTU3_ID				0x524
#define IVTT_ID					0x525
#define IVTW_ID			    	0x526
#define IVTAs_ID		    	0x527
#define IVTWh_ID				0x528
#define IVTBase_ID 				IVTI_ID
#define PDM_ID					0x520
#define MEMORATOR_ID			0x7B

#define NodeErr_ID         0x600
#define NodeCmd_ID			0x602
#define NodeAck_ID			0x601

#define AdcSimInput_ID			0x608

#define AnalogNode1_ID			(1664)
#define AnalogNode9_ID			(1680)
#define AnalogNode10_ID			(1682)
#define AnalogNode11_ID			(1684)
#define AnalogNode12_ID			(1686)
#define AnalogNode13_ID			(1688)
#define AnalogNode14_ID			(1690)
#define AnalogNode15_ID			(1692)
#define AnalogNode16_ID			(1694)
#define AnalogNode17_ID			(1696)
#define AnalogNode18_ID			(1698)

#define PowerNode33_ID			(1710)
#define PowerNode34_ID			(1711)
#define PowerNode35_ID			(1712)
#define PowerNode36_ID			(1713)
#define PowerNode37_ID			(1714)

#define CANB0					(2)
#define CANB1					(1)

#ifdef ONECAN

	#define IVT_BUS					CANB1
	#define BMS_BUS					CANB1
	#define PDM_BUS					CANB1

	#define INV1_BUS				CANB1
	#define INV2_BUS				CANB1
#else

	#define IVT_BUS					CANB0
	#define BMS_BUS					CANB1
	#define PDM_BUS

	#define INV1_BUS				CANB0
	#define INV2_BUS				CANB0
#endif

#define InverterRL_COBID			0x7E // 126 // swap
#define InverterRR_COBID			0x7F // 127 // swap

#ifdef HPF20
#define InverterFL_COBID			0x7C // 124 // swap
#define InverterFR_COBID			0x7D // 125 // swap
#endif

#define Inverter_BUS			hfdcan2

#define ECU_CAN_ID				0x20 // send +1

// brake temp sensors 0x1a - first two bytes.

//#define IVT_ID					0x411
//#define BMS_ID

// can open guard/states.

/*
#define PREOPERATION			0x7F
#define OPERATIONAL				0x05
#define STOPPED					0x04
#define BOOTUP					0x00
#define OFFLINE					0xFE
#define ERROR					0xFF
*/



#define ENABLED					(true)
#define DISABLED				(false)


int cancount;


typedef bool (*DataHandler)(uint8_t CANRxData[8], uint32_t DataLength );
typedef void (*TimeoutHandler)( uint16_t id );


typedef volatile struct CanDataType {
	volatile uint8_t *devicestate;
	uint16_t id;
	uint8_t dlcsize;
	DataHandler getData;
	TimeoutHandler doTimeout;
	uint32_t timeout;
	uint32_t time;
	bool	 seen;
	uint16_t error;
	uint16_t receiveerr;
	bool	 errorsent;
} CanData;

// CANBus

volatile struct CanState {
	volatile CanData InverterERR[INVERTERCOUNT];

	volatile CanData InverterPDO1[INVERTERCOUNT];
	volatile CanData InverterPDO2[INVERTERCOUNT];
	volatile CanData InverterPDO3[INVERTERCOUNT];
	volatile CanData InverterPDO4[INVERTERCOUNT];

	volatile CanData InverterNMT;

#ifndef HPF20
	volatile struct CanData FLeftSpeedERR;
	volatile struct CanData FRightSpeedERR;

	volatile struct CanData FLeftSpeedPDO1;
	volatile struct CanData FRightSpeedPDO1;

	volatile struct CanData FLeftSpeedNMT;
	volatile struct CanData FRightSpeedNMT;
#endif

	volatile CanData Memorator;

//	volatile CanData BMSVolt;
//	volatile CanData BMSError;
//	volatile CanData BMSOpMode;

	volatile CanData ECU;
//	volatile CanData ECUConfig;

	/*
	volatile CanData  AnalogNode15; // tyre temps FL
	volatile CanData  AnalogNode16; // tyre temps FR
	volatile CanData  AnalogNode17; // tyre temps RL
	volatile CanData  AnalogNode18; // tyre temps RR
	 */
} CanState;

//canbus

int getNMTstate(volatile CanData *data );

void resetCanTx(volatile uint8_t CANTxData[8]);
int ResetCanReceived( void );
int ResetOperationCanReceived( void );
char CAN1Send( uint16_t id, uint8_t dlc, uint8_t *pTxData );
char CAN2Send( uint16_t id, uint8_t dlc, uint8_t *pTxData );
char CAN_NMT( uint8_t, uint8_t );
char CAN_ConfigRequest( uint8_t command, uint8_t success );
char CANKeepAlive( void );
uint8_t CANSendPDM( uint8_t highvoltage, uint8_t buzz );
#ifdef PDMSECONDMESSAGE
uint8_t CANSendPDMFAN( void );
#endif

char CAN_SendErrorStatus( char state, char substate, uint32_t errorcode );
char CAN_SendStatus( char state, char substate, uint32_t errorcode );
char CANSendInverter( uint16_t response, uint16_t request, uint8_t inverter );
char CANTorqueRequest( uint16_t requestl, uint16_t requestr );
char CANLogDataFast( void );
char CANLogDataSlow( void );
char CAN_NMTSyncRequest( void );

char CAN_SendLED( void );

char CAN_SENDINVERTERERRORS( void );

char CAN_SendIVTTurnon( void );
char CAN_SendIVTTrigger( void );

void ResetCanData(volatile CanData *data );


char CAN_SendErrors( void );
char reTransmitError(uint32_t canid, uint8_t *CANRxData, uint32_t DataLength );
char reTransmitOnCan1(uint32_t canid, uint8_t *CANRxData, uint32_t DataLength );

char CAN_SendTimeBase( void );

char CAN_SendADCminmax( void );
char CAN_SendADC( volatile uint32_t *ADC_Data, uint8_t error );
//char CAN_SendADCVals( void );


void processCANData(CanData * datahandle, uint8_t CANRxData[8], uint32_t DataLength );
int receivedCANData( CanData * datahandle );

// initialisation
void FDCAN1_start(void);
void FDCAN2_start(void);

#endif /* CANECU_H_ */
