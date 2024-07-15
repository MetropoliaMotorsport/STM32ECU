/*
 * powernode.c
 *
 *  Created on: 15 Jun 2020
 *      Author: Visa
 */

#include "ecumain.h"
#include "powernode.h"
#include "power.h"
#include "debug.h"
#include "errors.h"
#include "timerecu.h"
#include <stdarg.h>
#include "bms.h"
#include "can_ids.h"

#include <stdio.h>
#include <time.h>

#define MAXECUCURRENT 		20
#define MAXFANCURRENT		20
#define MAXPUMPCURRENT		20

// TODO this list should be sanity checked for duplicates at tune time.
//Need to be added into araraay accorfing to the device number. Kinda sucks, so needs to be fixed.
devicepowerreq DevicePowerList[] = {
		{None},
		{Buzzer, PNode2_ID, OUT0_2, },
		{Inverters, PNode2_ID, OUT2_1 },
		{Brake, PNode2_ID, OUT3_1},
		{SideFans, PNode2_ID, OUT1_2, 2},//////////////TODO put right output value
		{None},
		{LeftPump, PNode2_ID, OUT2_2, 1},
		{RightPump, PNode2_ID, OUT1_2, 1},
		{None} };

bool processPNodeHeartBeat(const uint8_t CANRxData[8], const uint32_t DataLength,
		 CANData *datahandle) {

			int8_t Pnode = datahandle->id == PNode1_ID ? 29 : 30; 
	
			for(int i = 0; i < DEVICE_COUNT; i++) {
				if (&DevicePowerList[i] != NULL && DevicePowerList[i].nodeid == Pnode){
						
					DevicePowerList[i].actualstate = CANRxData[0] & (1 << DevicePowerList[i].output);
				}
			}				

	return true;
}

CANData PowerNode1HeartBeat = {&DeviceState.PowerNode1, Pnode1_Hearbeat_ID, 1, processPNodeHeartBeat};
CANData PowerNode2HeartBeat = {&DeviceState.PowerNode2, Pnode2_Hearbeat_ID, 1, processPNodeHeartBeat};



uint8_t PowerNodeErrorCount = 0;

// Queue up power node requests to be sent.
bool setNodeDevicePower(DevicePower device, bool state, bool reset) {

	DevicePowerList[device].expectedstate = state;
	uint8_t data[3] = {2, DevicePowerList[device].output, state};
	CAN2Send(DevicePowerList[device].nodeid, 3, data);
	
	return false; // return if device was found and request set.
}

bool setNodeDevicePWM(DevicePower device, uint8_t dutycycle) {

	DevicePowerList[device].dutycycle = dutycycle;
	uint8_t data[3] = {1, DevicePowerList[device].output, dutycycle};
	CAN2Send(DevicePowerList[device].nodeid, 3, data);
	
	return false; // return if device was found and request set.
}

int initPowerNodes(void) {

	RegisterCan2Message(&PowerNode1HeartBeat);
	RegisterCan2Message(&PowerNode2HeartBeat);

	return 0;
}
