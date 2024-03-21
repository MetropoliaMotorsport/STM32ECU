/*
 * canecu.h
 *
 *  Created on: 27 Apr 2019
 *      Author: Visa
 */

#ifndef CANECU_H_
#define CANECU_H_

#include "ecu.h"
#include "queue.h"
#include "can_ids.h"


// definition of CAN ID's for nodes - Bus definitions not currently used

#define CANBUS0 				hfdcan2
#define	CANBUS1					hfdcan1

#define FLSpeed_BUS				CAN1
#define FRSpeed_BUS				CAN1


#define CANB0					(2)
#define CANB1					(1)


#define IVT_BUS					CANB0
#define BMS_BUS					CANB1
#define PDM_BUS

#define INV1_BUS				CANB0
#define INV2_BUS				CANB0

#define Inverter_BUS			CANB0



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

enum canbus { bus0, bus1, bus2 = 0 };

typedef struct devices_online{
	bool BMS;
	bool IVT;
	bool Inverter;
	bool Pedal;
	bool PWR_Node;
	bool Node;
} devices_online;


typedef struct can_msg {
	enum canbus bus;
	uint16_t id;
	uint32_t dlc;
	uint8_t data[8];
} can_msg;

extern QueueHandle_t CanTxQueue;

extern int cancount;

typedef volatile struct CanDataType CANData;

typedef bool (*DataHandler)(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
typedef void (*TimeoutHandler)( uint16_t id );


typedef volatile struct CanDataType {
	volatile uint8_t *devicestate;
	uint16_t id;
	uint8_t dlcsize;
	DataHandler getData;
	TimeoutHandler doTimeout;
	uint32_t timeout;
	uint8_t  index;
	uint32_t time;
	uint16_t error;
	uint16_t receiveerr;
	bool	 errorsent;
} CANData;


int getNMTstate(volatile CANData *data );

void resetCanTx(volatile uint8_t CANTxData[8]);
int ResetCanReceived( void );
int ResetOperationCanReceived( void );
//uint8_t CAN1Send( uint16_t id, uint8_t dlc, const uint8_t *pTxData );
uint8_t CAN2Send( uint16_t id, uint8_t dlc, const uint8_t *pTxData );
uint8_t CANSendSDO( enum canbus bus, uint16_t id, uint16_t idx, uint8_t sub, uint32_t data);
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
char CANLogDataFast( void );
char CANLogDataSlow( void );
char CAN_NMTSyncRequest( void );

char CAN_Send4vals( uint16_t id, uint16_t val1, uint16_t val2, uint16_t val3, uint16_t val4 );

char CAN_SendLED( void );

char CAN_SENDINVERTERERRORS( void );

void ResetCanData(volatile CANData *data );


char CAN_SendErrors( void );
char CAN_SendDebug(uint16_t id);

char reTransmitError(uint32_t canid, const uint8_t *CANRxData, uint32_t DataLength );
char reTransmitOnCan1(uint32_t canid, const uint8_t *CANRxData, uint32_t DataLength );

char CAN_SendTimeBase( void );

char CAN_SendADCminmax( void );
char BMSBASE_ID( volatile uint32_t *ADC_Data, uint8_t error );
//char CAN_SendADCVals( void );

int CheckCanError( void );

void processCANData(CANData * datahandle, uint8_t * CANRxData, uint32_t DataLength );
int receivedCANData( CANData * datahandle );


int RegisterCan1Message(CANData * CanMessage);
int RegisterCan2Message(CANData * CanMessage);

// initialisation

int initCAN( void );

#endif /* CANECU_H_ */
