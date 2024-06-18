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


typedef struct nodepowerreqstruct {
	uint8_t bus;
	uint8_t nodeid; //
	uint8_t output; // enable
	uint8_t state; // request state
	uint32_t error[6];
	uint32_t timesent;
} nodepowerreq;

typedef struct nodepowerpwmstruct {
	uint8_t bus;
	uint8_t nodeid; //
	uint8_t output;
	uint8_t dutycycle; // enable
	uint32_t timesent;
} nodepowerpwmreq;

typedef struct devicepowerreqstruct {
	DevicePower device; //
	uint8_t nodeid;
	uint8_t output; // which bit of enable request is this device on
	bool pwm;
	bool expectedstate; // what state are we requesting.
	bool waiting;
	bool actualstate;
} devicepowerreq;

static uint32_t devicecount = 0;

// TODO this list should be sanity checked for duplicates at tune time.
devicepowerreq DevicePowerList[] = {

		{ Buzzer, PNode2_ID, OUT0_2 },
		{ Inverters, PNode2_ID, OUT2_1 },
		{Brake, PNode2_ID, OUT3_1},
		{ None } };


nodepowerreq PowerRequests[] = { //TODO How does it work?


		{ 1, 34, 0, 0, { 0 } }, { 1, 36, 0, 0, { 0 } }, { 1, 37, 0, 0, { 0 } },

		{ 0 } };

nodepowerpwmreq nodefanpwmreqs[2] = { 0 };
bool queuedfanpwmLeft = false;
bool queuedfanpwmRight = false;


bool processPNode1Data(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle);
bool processPNode2Data(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle);

uint32_t getOldestPNodeData(void) {

	uint32_t time1 = gettimer(); //TODO fix it back
#if 0
	if (PowerNode1.time < time)
		time = PowerNode1.time;

	if (PowerNode2.time < time)
		time = PowerNode2.time;
#endif
	return time1;
}

bool processPNode1Data(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle) {	
	
	return true;
}

bool processPNode2Data(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle) // Rear
{

	return true;
}

CANData PowerNode1;
CANData PowerNode2;

bool setActualDevicePower(uint8_t nodeid, uint8_t channel, bool state) {
	// find the device
	for (int i = 0; DevicePowerList != None; i++) {
		if (DevicePowerList[i].nodeid == nodeid
				&& DevicePowerList[i].output == channel) {
			DevicePowerList[i].waiting = false;
			DevicePowerList[i].actualstate = state;
			return true;
		}
	}
	return false;
}

#define MAXPNODEERRORS		40

struct PowerNodeError {
	uint8_t nodeid;
	uint32_t error;
} PowerNodeErrors[MAXPNODEERRORS];

uint8_t PowerNodeErrorCount = 0;


bool getNodeDevicePower(DevicePower device) {
	for (int i = 0; DevicePowerList[i].device != None; i++) {
		if (DevicePowerList[i].device == device)
			return DevicePowerList[i].actualstate;
	}

	// device not found
	return false;
}

// Queue up power node requests to be sent.
bool setNodeDevicePower(DevicePower device, bool state, bool reset) {
//	for ( int i=0;DevicePowerList[i].device != None;i++)
//	{

	int i = getPowerDeviceIndex(device);

	if (DevicePowerList[i].device == device) { // found the device in list, try to set request.
		DevicePowerList[i].expectedstate = state;

		if (reset) {
			DevicePowerList[i].actualstate = state; // resetting internal state, so current expected actual state is false?
			DevicePowerList[i].waiting = false;
		} else {
			// check if we're already in the expected state.
			if (DevicePowerList[i].expectedstate
					!= DevicePowerList[i].actualstate)
				DevicePowerList[i].waiting = true;
			else
				DevicePowerList[i].waiting = false;
		}

		for (int j = 0; PowerRequests[j].nodeid != 0; j++) // search though the list of power nodes
				{
			if (PowerRequests[j].nodeid == DevicePowerList[i].nodeid) // check if node on list matches the wanted id.
					{
				// check there isn't a current error on the request.
				if (PowerRequests[j].error[DevicePowerList[i].output] == 0
						|| reset) {
					if (reset) // we had a pending request, cancel it.
					{
						PowerRequests[j].error[DevicePowerList[i].output] = 0;
						PowerRequests[j].output &= ~(0x1
								<< DevicePowerList[i].output); // set enable output
						PowerRequests[j].state &= ~(0x1
								<< DevicePowerList[i].output); // set enable output
						return true;
					}

					// no error, can proceed with request.
					// check existing request queued for node..
					bool enabled = PowerRequests[j].output
							& (0x1 << DevicePowerList[i].output);
					if (!enabled || // no request yet made
							(enabled
									&& (PowerRequests[j].state
											& (0x1 << DevicePowerList[i].output))
											!= state) // request different to previously not processed request
							) {
						PowerRequests[j].output |= (0x1
								<< DevicePowerList[i].output); // set enable output
						if (state == true)
							PowerRequests[j].state |= (0x1
									<< DevicePowerList[i].output); // set bit
						else
							PowerRequests[j].state &= ~(0x1
									<< DevicePowerList[i].output); // unset bit

						return true;

					} else {
						return false; // request already set
					}
				}
				return false; // error on channel, couldn't set
			}
		}
	}
//	}
	return false; // return if device was found and request set.
}

int getPowerDeviceIndex(DevicePower device) {
	int i = 0;
	for (; DevicePowerList[i].device != None; i++) {
		if (DevicePowerList[i].device == device) {
			return i;
		}
	}
	return i;
}

void resetPowerNodes(void) {
	Shutdown.BOTS = false;
	Shutdown.InertiaSwitch = false;
	Shutdown.BSPDAfter = false;
	Shutdown.BSPDBefore = false;
	Shutdown.CockpitButton = false;
	Shutdown.LeftButton = false;
	Shutdown.RightButton = false;
	Shutdown.BMSReason = false;
	Shutdown.IMD = false;
	Shutdown.AIRm = false;
	Shutdown.AIRp = false;
	Shutdown.PRE = false;
	// reset errors

	PowerNodeErrorCount = 0;

}

int initPowerNodes(void) {

	RegisterResetCommand(resetPowerNodes);

	resetPowerNodes();

	RegisterCan1Message(&PowerNode1);

	RegisterCan1Message(&PowerNode2);

	return 0;
}
