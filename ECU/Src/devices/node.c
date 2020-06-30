/*
 * powernode.c
 *
 *  Created on: 15 Jun 2020
 *      Author: Visa
 */

#include "ecumain.h"
#include <stdarg.h>

#include <stdio.h>

bool processNodeErrData(uint8_t CANRxData[8], uint32_t DataLength );
bool processNodeAckData(uint8_t CANRxData[8], uint32_t DataLength );

CanData NodeErr = { NULL, NodeErr_ID, 6, processNodeErrData, NULL, 0 };
CanData NodeAck = { NULL, NodeAck_ID, 3, processNodeAckData, NULL, 0 };


#define MAXPNODEERRORS		40


struct PowerNodeError
{
	uint8_t nodeid;
	uint32_t error;
} PowerNodeErrors[MAXPNODEERRORS];

uint8_t PowerNodeErrorCount = 0;



#define ERR_CAN_FIFO_FULL			1
#define ERR_SEND_FAILED				2
#define ERR_RECIEVED_INVALID_ID		3
#define ERR_RECIEVE_FAILED			4
#define ERR_INVALID_COMMAND			5
#define ERR_COMMAND_SHORT			6

#define ERR_WRONG_BYTES				33
#define ERR_INCORRECT_TF			34
#define ERR_INCORRECT_TF_VOLTAGE	35
#define ERR_INCORRECT_TF_NTC		36
#define ERR_INCORRECT_TF_I_TRANS	37

#define WARN_OVERCURR				49
#define ERR_OVERCURR_SHUTOFF		50

#define ERR_INVALID_CONFIG_ID		65


/*
volatile bool PowerNode33Ack = false;
volatile bool PowerNode34Ack = false;
volatile bool PowerNode35Ack = false;
volatile bool PowerNode36Ack = false;
volatile bool PowerNode37Ack = false;
*/

uint32_t Get_Error(uint8_t errorpage, uint8_t errorbit )
{
//	canErrors[(error/32)]  |= (1<<(error%32));
//	canErrorToTransmit |= (1<<(error/32));
	return errorpage*32+errorbit;
}


bool processNodeErrData(uint8_t data[8], uint32_t DataLength)
{

	//  0   1536    6  36   4   0   0   0 112   808.810870 R  //  0b01110000   4, 5, 6 + 4*  132, 133,   switch off.


	if ( DataLength >> 16 == NodeErr.dlcsize )
	{
		uint32_t errorcode=data[1]*32+(data[2]*16777216+data[3]*65536+data[4]*256+data[5]);

		for ( int i = 0;i<32;i++){
			if ( errorcode & (1<<(i) ) )
			{
				if ( PowerNodeErrorCount < MAXPNODEERRORS )
				{
					PowerNodeErrors[PowerNodeErrorCount].nodeid = data[0];
					PowerNodeErrors[PowerNodeErrorCount].error = Get_Error(data[1], i );
					// do something with the found error.
					processPNodeErr(PowerNodeErrors[PowerNodeErrorCount].nodeid, PowerNodeErrors[PowerNodeErrorCount].error);
					PowerNodeErrorCount++;
				}
			}
		}
	}
	return true;
}

bool processNodeAckData(uint8_t CANRxData[8], uint32_t DataLength )
{

	if ( CANRxData[0] > 30) processPNodeAckData(CANRxData, DataLength);

	return true;
}

