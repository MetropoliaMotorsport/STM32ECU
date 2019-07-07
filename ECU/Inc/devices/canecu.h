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

#define FLSpeed_COBID			0x71 // 112 // 0x70 orig
#define FLSpeed_BUS				hfdcan1
#define FRSpeed_COBID			0x70 // 113  // 0x71 orig
#define FRSpeed_BUS				hfdcan1
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


#define IVT_BUS					hfdcan2
#define BMS_BUS					hfdcan1
#define PDM_BUS

#define InverterL_COBID			0x7E // 126
#define InverterR_COBID			0x7F // 127
#define Inverter_BUS			hfdcan2

#define ECU_CAN_ID				0x20 // send +1

// brake temp sensors 0x1a - first two bytes.

//#define IVT_ID					0x411
//#define BMS_ID

// can open guard/states.

#define PREOPERATION			0x7F
#define OPERATIONAL				0x05
#define STOPPED					0x04
#define BOOTUP					0x00
#define OFFLINE					0xFE
#define ERROR					0xFF

#define ENABLED					1
#define DISABLED				0


int cancount;

struct CanData {
	uint8_t dlcsize;
	uint8_t data[8];
	uint32_t time;
	uint32_t lastseen;
	uint32_t count;
	uint8_t newdata;
	uint8_t processed;
};

// CANBus
FDCAN_TxHeaderTypeDef TxHeaderTime, TxHeader1, TxHeader2;

volatile struct CanState {
	volatile uint8_t receiving;

	volatile struct CanData InverterLERR;
	volatile struct CanData InverterRERR;

	volatile struct CanData InverterLPDO1;
	volatile struct CanData InverterLPDO2;
	volatile struct CanData InverterLPDO3;
	volatile struct CanData InverterLPDO4;

	volatile struct CanData InverterRPDO1;
	volatile struct CanData InverterRPDO2;
	volatile struct CanData InverterRPDO3;
	volatile struct CanData InverterRPDO4;

	volatile struct CanData InverterNMT;

	volatile struct CanData FLeftSpeedERR;
	volatile struct CanData FRightSpeedERR;

	volatile struct CanData FLeftSpeedPDO1;
	volatile struct CanData FRightSpeedPDO1;

	volatile struct CanData FLeftSpeedNMT;
	volatile struct CanData FRightSpeedNMT;

	volatile struct CanData PDM;
	volatile struct CanData PDMVolts;

	volatile struct CanData BMSVolt;
	volatile struct CanData BMSError;
	volatile struct CanData BMSOpMode;

	volatile struct CanData ECU;
	volatile struct CanData ECUConfig;

	volatile struct CanData IVT[8];

	volatile struct CanData IVTMsg;
	volatile struct CanData IVTI;
	volatile struct CanData IVTU1;
	volatile struct CanData IVTU2;
	volatile struct CanData IVTU3;
	volatile struct CanData IVTT;
	volatile struct CanData IVTW;
	volatile struct CanData IVTAs;
	volatile struct CanData IVTWh;
} CanState;

//canbus

int getNMTstate(volatile struct CanData *data );

void resetCanTx(volatile uint8_t CANTxData[8]);
int ResetCanReceived( void );
int ResetOperationCanReceived( void );
char CAN1Send( FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData );
char CAN2Send( FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData );
char CAN_NMT( uint8_t, uint8_t );
char CAN_ConfigRequest( uint8_t command, uint8_t success );
char CANKeepAlive( void );
uint8_t CANSendPDM( uint8_t highvoltage, uint8_t buzz );

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

void ResetCanData(volatile struct CanData *data );


char CAN_SendErrors( void );
char reTransmitError(uint32_t canid, uint8_t *CANRxData, uint32_t DataLength );
char reTransmitOnCan1(uint32_t canid, uint8_t *CANRxData, uint32_t DataLength );

char CAN_SendTimeBase( void );

void processCANData( void );

char CAN_SendADCminmax( void );
char CAN_SendADC( volatile uint32_t *ADC_Data, uint8_t error );
//char CAN_SendADCVals( void );

// initialisation
void FDCAN1_start(void);
void FDCAN2_start(void);

#endif /* CANECU_H_ */
