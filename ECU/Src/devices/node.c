/*
 * node.c
 *
 *  Created on: 15 Jun 2020
 *      Author: Visa
 */

#include "ecumain.h"
#include "analognode.h"
#include "powernode.h"
#include "node.h"
#include <stdarg.h>
#include <stdio.h>

bool processNodeErrData(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle);
bool processNodeAckData(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle);

CANData NodeErr = { NULL, NodeErr_ID, 6, processNodeErrData, NULL, 0 };
CANData NodeAck = { NULL, NodeAck_ID, 8, processNodeAckData, NULL, 0 };

// Analogue node errors.

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

uint32_t Get_Error(uint8_t errorpage, uint8_t errorbit) {
	return errorpage * 32 + errorbit;
}

// received a node error, process it.
bool processNodeErrData(const uint8_t data[8], const uint32_t DataLength,
		const CANData *datahandle) {
	//  0   1536    6  36   4   0   0   0 112   808.810870 R  //  0b01110000   4, 5, 6 + 4*  132, 133,   switch off.

	if (DataLength >> 16 == NodeErr.dlcsize) {
		// reverse the actual error code for lookup from the can data.
		uint32_t errorcode = data[1] * 32
				+ (data[2] * 16777216 + data[3] * 65536 + data[4] * 256
						+ data[5]);

		// check which error bits set
		for (int i = 0; i < 32; i++) {
			if (errorcode & (1 << (i))) {
				if (data[0] > 32) {
					processPNodeErr(data[0], Get_Error(data[1], i), datahandle);
				} else {
					processANodeErr(data[0], Get_Error(data[1], i), datahandle);
					// analogue node error.
				}
			}
		}
	}
	return true;
}

bool processNodeAckData(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle) {
	// [node ID], [command], [d1], [d2], [d3], [d4], [ack_counter], [command]
	if (CANRxData[0] > 30)
		processPNodeAckData(CANRxData, DataLength, datahandle);

	return true;
}

int initNodes(void) {
	RegisterCan1Message(&NodeErr);
	RegisterCan1Message(&NodeAck);

	RegisterCan2Message(&NodeErr); // nodes on both CAN's
	RegisterCan2Message(&NodeAck);

	return 0;
}

