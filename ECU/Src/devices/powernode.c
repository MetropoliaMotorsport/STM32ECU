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

#ifdef HPF2023

// TODO this list should be sanity checked for duplicates at tune time.
devicepowerreq DevicePowerList[] = {

//		{ Telemetry, 33, 4 },
//		{ Front1, 33, 5 },

		{ ECU, 34, 4, true, 0, true }, // ECU has to have power or we aren't booted.. so just assume it.
		{ Front2, 34, 5 },

//		{ LeftFans, 35, 0 },
//		{ RightFans, 35, 1 },
//		{ LeftPump, 35, 4 },
//		{ RightPump, 35, 5 },

		{ IVT, 36, 4 }, { Accu, 36, 2 }, // TODO make sure this channel cannot be reset for latching rules.
		{ AccuFan, 36, 5 },

		{ Brake, 37, 4 }, { Buzzer, 37, 1 }, { Inverters, 37, 2 },
		//{ TSAL, 37, 3, true, 0, true }, // essential to be powered, else not compliant.
		//{ Back1, 37, 4 },
		{ TSALG, 37, 5, true, 0, true }, // TSAL getting constant power from this FSG2023
		{ None } };

#else

devicepowerreq DevicePowerList[] = { //TODO update power distribution
				{ Front1, 1, 5 },

				{ Inverters, 2, 3 },
				{ ECU, 1, 4, true, 0, true }, // ECU has to have power or we aren't booted.. so just assume it.
				{ Front2, 1, 5 },

				{ LeftFans, 2, 2 },
				{ RightFans, 2, 3 },
				{ LeftPump, 2, 4 },
				{ RightPump, 2, 5 },

				{ Current, 2, 1 },
				{ Buzzer, 2, 2 },
				{ Brake, 2, 3 },
				{ Back1, 2, 4 },
				{ TSAL, 2, 5, true, 0, true }, // essential to be powered, else not compliant.

				{ None }
};
#endif
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



void PNode2Timeout(uint16_t id) {


	SetCriticalError(CRITERBL); // brakelight is a critical signal.
}


bool processPNode1Data(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle) {
	
	
	static bool first = false;
	if (!first) {
		DebugMsg("PNode 1 First msg.");
		first = true;
	}
	// 0x1f 0x0001 10000
	if (DataLength >> 16 == PowerNode1.dlcsize
//		&& CANRxData[0] & ~(0b00011100) != 0 // check mask fit
			&& CANRxData[1] < 255 && (CANRxData[2] >= 0 && CANRxData[2] <= 255)
			&& CANRxData[3] < 255) {
		xTaskNotify(PowerTaskHandle, ( 0x1 << PNode1Bit ), eSetBits);

		bool newstate = CANRxData[0] & (0x1 << 2);
		if (Shutdown.CockpitButton != newstate) {
			DebugPrintf(
					"Cockpit Shutdown Button sense state changed to %s at (%ul)",
					newstate ? "Closed" : "Open", gettimer());
			Shutdown.CockpitButton = newstate;
		}

		newstate = CANRxData[0] & (0x1 << 3);
		if (Shutdown.LeftButton != newstate) {
			DebugPrintf(
					"Left Shutdown Button sense state changed to %s at (%ul)",
					newstate ? "Closed" : "Open", gettimer());
			Shutdown.LeftButton = newstate;
		}

		newstate = CANRxData[0] & (0x1 << 4);
		if (Shutdown.RightButton != newstate) {
			DebugPrintf(
					"Right Shutdown Button sense state changed to %s at (%ul)",
					newstate ? "Closed" : "Open", gettimer());
			Shutdown.RightButton = newstate;
		}

//		CarState.I_Inverters =  CANRxData[1];
//		CarState.I_ECU =  CANRxData[2];
//		CarState.I_Front2 =  CANRxData[3];
		return true;

	} else // bad data.
	{
		return false;
	}
}

bool processPNode2Data(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle) // Rear
{

	static bool first = false;
	if (!first) {
		DebugMsg("PNode 2 First msg.");
		first = true;
	}

	if (DataLength >> 16 == PowerNode2.dlcsize
			&& (CANRxData[0] >= 0 && CANRxData[0] <= 10)
			&& (CANRxData[1] >= 0 && CANRxData[1] <= 10)
			&& (CANRxData[2] >= 0 && CANRxData[2] <= 10)
			&& (CANRxData[3] >= 0 && CANRxData[3] <= 10)
			&& (CANRxData[4] >= 0 && CANRxData[4] <= 10)
			&& (CANRxData[5] >= 0 && CANRxData[5] <= 255)
			&& (CANRxData[6] >= 0 && CANRxData[6] <= 100)) {
		xTaskNotify(PowerTaskHandle, ( 0x1 << PNode2Bit ), eSetBits);

//		CarState.I_BrakeLight = CANRxData[0];
//		CarState.I_Buzzers = CANRxData[1];
		CarState.I_IVT = CANRxData[2];
		CarState.I_AccuPCBs = CANRxData[3];
		CarState.I_AccuFans = CANRxData[4];
		CarState.Freq_IMD = CANRxData[5]; // IMD Shutdown.

#if 0
		// normal operational status, else assume error.
		if ( CarState.Freq_IMD >= 5 && CarState.Freq_IMD < 15 )
			Shutdown.IMD = true;
		else
			Shutdown.IMD = false;
#endif
		DeviceState.BrakeLight = OPERATIONAL;

//		should be 10 Hz in normal situation (I think duty cycle was based on measured resistance or something)
//		and then 50 Hz for fault state (50% duty cycle),
//		40 Hz is internal error,
//		20 Hz we should never see, then not sure what 30 Hz is
//		it also wasn't 10 Hz on power up,
//		I dont' remember what it was on power up though, might have been 30 Hz

		CarState.DC_IMD = CANRxData[6];

		return true;
	} else // bad data.
	{
		return false;
	}
}

CANData PowerNode1;
CANData PowerNode2;




void setAllPowerActualOff(void) {
	for (int i = 0; DevicePowerList[i].device != None; i++) {
		if (DevicePowerList[i].device != ECU) {
			if (DevicePowerList[i].actualstate == true)
				;
			//		DevicePowerList[i].waiting = false;
			DevicePowerList[i].actualstate = false;
		}
	}
}

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



// ERROR list from powernode main.h

#define ERR_CAN_BUFFER_FULL			1
#define ERR_CAN_FIFO_FULL			2
#define ERR_MESSAGE_DISABLED		3
#define ERR_DLC_0					4
#define ERR_DLC_LONG				5
#define ERR_SEND_FAILED				6
#define ERR_RECIEVE_FAILED			7
#define ERR_INVALID_COMMAND			8
#define ERR_COMMAND_SHORT			9
#define ERR_RECIEVED_INVALID_ID 	10
#define ERR_CANBUSOFFLINE			11

#define ERR_MODIFY_INVALID_MESSAGE	33
#define ERR_MODIFY_INVALID_THING	34
#define ERR_CLEAR_INVALID_ERROR		35

#define ERR_MESS_INVALID_BYTES		97
#define ERR_MESS_UNDEFINED			98

#define WARN_UNDERVOLT_U5				193
#define WARN_OVERVOLT_U5				194
#define WARN_UNDERTEMP_U5				195
#define WARN_OVERTEMP_U5				196
#define WARN_UNDERCURR_U5I0				197
#define WARN_OVERCURR_U5I0				198
#define WARN_UNDERCURR_U5I1				199
#define WARN_OVERCURR_U5I1				200
#define ERROR_OVERCURR_TRIP_U5_0		201 //PO0
#define ERROR_OVERCURR_TRIP_U5_1		202 //PO1
#define WARN_UNDERVOLT_U6				203
#define WARN_OVERVOLT_U6				204
#define WARN_UNDERTEMP_U6				205
#define WARN_OVERTEMP_U6				206
#define WARN_UNDERCURR_U6I0				207
#define WARN_OVERCURR_U6I0				208
#define WARN_UNDERCURR_U6I1				209
#define WARN_OVERCURR_U6I1				210
#define ERROR_OVERCURR_TRIP_U6_0		211 //PO2
#define ERROR_OVERCURR_TRIP_U6_1		212 //PO3
#define WARN_UNDERVOLT_U7				213
#define WARN_OVERVOLT_U7				214
#define WARN_UNDERTEMP_U7				215
#define WARN_OVERTEMP_U7				216
#define WARN_UNDERCURR_U7I0				217
#define WARN_OVERCURR_U7I0				218
#define WARN_UNDERCURR_U7I1				219
#define WARN_OVERCURR_U7I1				220
#define ERROR_OVERCURR_TRIP_U7_0		221 //PO4
#define ERROR_OVERCURR_TRIP_U7_1		222 //PO5

#define ERROR_READ_TEMP					224
#define WARN_TEMP_MEASURE_OVERFLOW		225
#define WARN_VOLT_MEASURE_OVERFLOW		226

#define WARN_PWM_INVALID_CHANNEL		257
#define WARN_PWM_CHANNEL_UNINITIALIZED	258
#define WARN_UNDEFINED_GPIO				259
#define WARN_PWM_NOT_ENABLED			260

//
// command 12 is used to clear an error from switch shutoff
// until the error is cleared the node will regularly send a warning
// about which channel has been shutoff, byte 2 should be set to the
// number of which channel to clear (0 to 5)

// for example: [1][12][3] will clear an error on channel 3 from node 1

#define U5I0_SWITCH_OFF				129 //PO0
#define U5I1_SWITCH_OFF				130 //PO1
#define U6I0_SWITCH_OFF				131 //PO2
#define U6I1_SWITCH_OFF				132 //PO3
#define U7I0_SWITCH_OFF				133 //PO4
#define U7I1_SWITCH_OFF				134 //PO5

#define MAXPNODEERRORS		40

struct PowerNodeError {
	uint8_t nodeid;
	uint32_t error;
} PowerNodeErrors[MAXPNODEERRORS];

uint8_t PowerNodeErrorCount = 0;

char* PNodeGetErrStr(uint32_t error) {
	switch (error) {
	case U5I0_SWITCH_OFF:
		return "Ch0 Off";
	case U5I1_SWITCH_OFF:
		return "Ch1 Off";
	case U6I0_SWITCH_OFF:
		return "Ch2 Off";
	case U6I1_SWITCH_OFF:
		return "Ch3 Off";
	case U7I0_SWITCH_OFF:
		return "Ch4 Off";
	case U7I1_SWITCH_OFF:
		return "Ch5 Off";

	case ERR_CAN_BUFFER_FULL:
		return "CAN_BUFFER_FULL";
	case ERR_CAN_FIFO_FULL:
		return "CAN_FIFO_FULL";
	case ERR_MESSAGE_DISABLED:
		return "MESSAGE_DISABLED";
	case ERR_DLC_0:
		return "DLC_0";
	case ERR_DLC_LONG:
		return "DLC_LONG";
	case ERR_SEND_FAILED:
		return "SEND_FAILED";
	case ERR_RECIEVE_FAILED:
		return "RECIEVE_FAILED";
	case ERR_INVALID_COMMAND:
		return "INVALID_COMMAND";
	case ERR_COMMAND_SHORT:
		return "COMMAND_SHORT";
	case ERR_RECIEVED_INVALID_ID:
		return "RECIEVED_INVALID_ID";
	case ERR_CANBUSOFFLINE:
		return "CANBUSOFFLINE";

	case ERR_MODIFY_INVALID_MESSAGE:
		return "MODIFY_INVALID_MESSAGE";
	case ERR_MODIFY_INVALID_THING:
		return "MODIFY_INVALID_THING";
	case ERR_CLEAR_INVALID_ERROR:
		return "CLEAR_INVALID_ERROR";

	case ERR_MESS_INVALID_BYTES:
		return "MESS_INVALID_BYTES";
	case ERR_MESS_UNDEFINED:
		return "MESS_UNDEFINED";

	case WARN_UNDERVOLT_U5:
		return "UNDERVOLT_CH0-1";
	case WARN_OVERVOLT_U5:
		return "OVERVOLT_CH0-1";
	case WARN_UNDERTEMP_U5:
		return "UNDERTEMP_CH0-1";
	case WARN_OVERTEMP_U5:
		return "OVERTEMP_CH0-1";
	case WARN_UNDERCURR_U5I0:
		return "UNDERCURR_CH0";
	case WARN_OVERCURR_U5I0:
		return "OVERCURR_CH0";
	case WARN_UNDERCURR_U5I1:
		return "UNDERCURR_CH1";
	case WARN_OVERCURR_U5I1:
		return "OVERCURR_CH1";
	case ERROR_OVERCURR_TRIP_U5_0:
		return "OVERCURR_CH0";
	case ERROR_OVERCURR_TRIP_U5_1:
		return "OVERCURR_CH1";
	case WARN_UNDERVOLT_U6:
		return "UNDERVOLT_CH2-3";
	case WARN_OVERVOLT_U6:
		return "OVERVOLT_CH2-3";
	case WARN_UNDERTEMP_U6:
		return "UNDERTEMP_CH2-3";
	case WARN_OVERTEMP_U6:
		return "OVERTEMP_CH2-3";
	case WARN_UNDERCURR_U6I0:
		return "UNDERCURR_CH2";
	case WARN_OVERCURR_U6I0:
		return "OVERCURR_CH2";
	case WARN_UNDERCURR_U6I1:
		return "UNDERCURR_CH3";
	case WARN_OVERCURR_U6I1:
		return "OVERCURR_CH3";
	case ERROR_OVERCURR_TRIP_U6_0:
		return "OVERCURR_CH2";
	case ERROR_OVERCURR_TRIP_U6_1:
		return "OVERCURR_CH3";
	case WARN_UNDERVOLT_U7:
		return "UNDERVOLT_CH4-5";
	case WARN_OVERVOLT_U7:
		return "OVERVOLT_CH4-5";
	case WARN_UNDERTEMP_U7:
		return "UNDERTEMP_CH4-5";
	case WARN_OVERTEMP_U7:
		return "OVERTEMP_CH4-5";
	case WARN_UNDERCURR_U7I0:
		return "UNDERCURR_CH4";
	case WARN_OVERCURR_U7I0:
		return "OVERCURR_CH4";
	case WARN_UNDERCURR_U7I1:
		return "UNDERCURR_CH5";
	case WARN_OVERCURR_U7I1:
		return "OVERCURR_CH5";
	case ERROR_OVERCURR_TRIP_U7_0:
		return "OVERCURR_CH4";
	case ERROR_OVERCURR_TRIP_U7_1:
		return "OVERCURR_CH5";

	case ERROR_READ_TEMP:
		return "READ_TEMP";
	case WARN_TEMP_MEASURE_OVERFLOW:
		return "TEMP_MEASURE_OVERFLOW";
	case WARN_VOLT_MEASURE_OVERFLOW:
		return "VOLT_MEASURE_OVERFLOW";

	case WARN_PWM_INVALID_CHANNEL:
		return "PWM_INVALID_CHANNEL";
	case WARN_PWM_CHANNEL_UNINITIALIZED:
		return "PWM_CHANNEL_UNINITIALIZED";
	case WARN_UNDEFINED_GPIO:
		return "UNDEFINED_GPIO";
	case WARN_PWM_NOT_ENABLED:
		return "PWM_NOT_ENABLED";
	default:
		return "Unknown";
	}
}

bool PNodeGetErrType(uint32_t error) {
	switch (error) {
	case U5I0_SWITCH_OFF:
	case U5I1_SWITCH_OFF:
	case U6I0_SWITCH_OFF:
	case U6I1_SWITCH_OFF:
	case U7I0_SWITCH_OFF:
	case U7I1_SWITCH_OFF:

	case ERR_CAN_BUFFER_FULL:
	case ERR_CAN_FIFO_FULL:
	case ERR_MESSAGE_DISABLED:
	case ERR_DLC_0:
	case ERR_DLC_LONG:
	case ERR_SEND_FAILED:
	case ERR_RECIEVE_FAILED:
	case ERR_INVALID_COMMAND:
	case ERR_COMMAND_SHORT:
	case ERR_RECIEVED_INVALID_ID:
	case ERR_CANBUSOFFLINE:

	case ERR_MODIFY_INVALID_MESSAGE:
	case ERR_MODIFY_INVALID_THING:
	case ERR_CLEAR_INVALID_ERROR:

	case ERR_MESS_INVALID_BYTES:
	case ERR_MESS_UNDEFINED:
		return true;

	case WARN_UNDERVOLT_U5:
	case WARN_OVERVOLT_U5:
	case WARN_UNDERTEMP_U5:
	case WARN_OVERTEMP_U5:
	case WARN_UNDERCURR_U5I0:
	case WARN_OVERCURR_U5I0:
	case WARN_UNDERCURR_U5I1:
	case WARN_OVERCURR_U5I1:

	case WARN_UNDERVOLT_U6:
	case WARN_OVERVOLT_U6:
	case WARN_UNDERTEMP_U6:
	case WARN_OVERTEMP_U6:
	case WARN_UNDERCURR_U6I0:
	case WARN_OVERCURR_U6I0:
	case WARN_UNDERCURR_U6I1:
	case WARN_OVERCURR_U6I1:

	case WARN_UNDERVOLT_U7:
	case WARN_OVERVOLT_U7:
	case WARN_UNDERTEMP_U7:
	case WARN_OVERTEMP_U7:
	case WARN_UNDERCURR_U7I0:
	case WARN_OVERCURR_U7I0:
	case WARN_UNDERCURR_U7I1:
	case WARN_OVERCURR_U7I1:
		return false;

	case ERROR_OVERCURR_TRIP_U5_0:
	case ERROR_OVERCURR_TRIP_U5_1:
	case ERROR_OVERCURR_TRIP_U6_0:
	case ERROR_OVERCURR_TRIP_U6_1:
	case ERROR_OVERCURR_TRIP_U7_0:
	case ERROR_OVERCURR_TRIP_U7_1:

	case ERROR_READ_TEMP:
		return true;
	case WARN_TEMP_MEASURE_OVERFLOW:
	case WARN_VOLT_MEASURE_OVERFLOW:

	case WARN_PWM_INVALID_CHANNEL:
	case WARN_PWM_CHANNEL_UNINITIALIZED:
	case WARN_UNDEFINED_GPIO:
	case WARN_PWM_NOT_ENABLED:
		return false;
	default:
		return true;
	}
}


char PNodeStr[10] = "";

char* getPNodeStr(void) {
	if (PNodeStr[0] == 0)
		return "";
	else
		return PNodeStr;
}

bool getNodeDevicePower(DevicePower device) {
	for (int i = 0; DevicePowerList[i].device != None; i++) {
		if (DevicePowerList[i].device == device)
			return DevicePowerList[i].actualstate;
	}

	// device not found
	return false;
}

bool getNodeDeviceExpectedPower(DevicePower device) {
	for (int i = 0; DevicePowerList[i].device != None; i++) {
		if (DevicePowerList[i].device == device)
			return DevicePowerList[i].expectedstate;
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

bool sendFanPWM(uint8_t leftduty, uint8_t rightduty) {
	// minimum useful duty is 18/255

	// ensure we don't end up in a stall state
	if (leftduty < 20 && leftduty > 0)
		leftduty = 20;
	if (rightduty < 20 && rightduty > 0)
		rightduty = 20;

	if (nodefanpwmreqs[0].nodeid == 0) // nodeid not yet set, find and assign it.
			{
		uint8_t index = getPowerDeviceIndex(LeftFans);

		for (int j = 0; PowerRequests[j].nodeid != 0; j++) {
			if (PowerRequests[j].nodeid == nodefanpwmreqs[0].nodeid) {
				nodefanpwmreqs[0].bus = PowerRequests[j].bus;
			}
		}

		nodefanpwmreqs[0].nodeid = DevicePowerList[index].nodeid;
		nodefanpwmreqs[0].output = DevicePowerList[index].output;

		index = getPowerDeviceIndex(RightFans);

		for (int j = 0; PowerRequests[j].nodeid != 0; j++) {
			if (PowerRequests[j].nodeid == nodefanpwmreqs[0].nodeid) {
				nodefanpwmreqs[1].bus = PowerRequests[j].bus;
			}
		}

		nodefanpwmreqs[1].nodeid = DevicePowerList[index].nodeid;
		nodefanpwmreqs[1].output = DevicePowerList[index].output;
	}
	nodefanpwmreqs[0].dutycycle = leftduty;

	nodefanpwmreqs[1].dutycycle = rightduty;

	queuedfanpwmLeft = true;
	queuedfanpwmRight = true;
	return true;
}

int sendPowerNodeErrReset(uint8_t id, uint8_t channel) {
	uint8_t candata[8] = { id, 12, channel };
	// look up bus, coul
	for (int j = 0; PowerRequests[j].nodeid != 0; j++) {
		if (PowerRequests[j].nodeid == id) {
			if (PowerRequests[j].bus == 2)
				CAN2Send(NodeCmd_ID, 3, candata);
			else
				CAN1Send(NodeCmd_ID, 3, candata);
		}
	}
	return 0;
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

// check and return if an power error has occured on a device.
uint32_t powerErrorOccurred(DevicePower device) {
//	for ( int i=0;DevicePowerList[i].device != None; i++)
//	{
	int i = getPowerDeviceIndex(device);

	if (DevicePowerList[i].device == device) { // found the device in list, try to set request.

		for (int j = 0; PowerRequests[j].nodeid != 0; j++) {
			if (PowerRequests[j].nodeid == DevicePowerList[i].nodeid) {
				return PowerRequests[j].error[DevicePowerList[i].output];
				//		if ( PowerRequests[j].error[DevicePowerList[i].output] )
				//			return true;
				//		else
				//			return false;
			}
		}
	}
//	}
	return 0;
}

int sendPowerNodeReq(void) {
	uint8_t candata[8] = { 0, 1, 0, 0 };
	bool senderror = false;

	for (int i = 0; PowerRequests[i].nodeid; i++) {
		if (PowerRequests[i].output != 0) {
			candata[0] = PowerRequests[i].nodeid;
			candata[2] = PowerRequests[i].output;
			candata[3] = PowerRequests[i].state;
			if (PowerRequests[i].nodeid != 36) // no commands to node 36?
					{
				if (PowerRequests[i].bus == 2)
					CAN2Send(NodeCmd_ID, 8, candata);
				else
					CAN1Send(NodeCmd_ID, 8, candata);
			}
			// don't reset request until we've seen an ack -> use ack handler to clear request.
			// give a small delay between can power request messages, to allow a tiny bit of leeway for some inrush maybe.
			vTaskDelay(1);
		}
	};

	if (queuedfanpwmLeft || queuedfanpwmRight) {
		candata[1] = 2; // pwm set command.

		if (nodefanpwmreqs[0].nodeid == nodefanpwmreqs[1].nodeid) // both requests on same node.
				{
			candata[0] = nodefanpwmreqs[0].nodeid;
			candata[2] = (0x1 << nodefanpwmreqs[0].output)
					+ (0x1 << nodefanpwmreqs[1].output);
			candata[3] = nodefanpwmreqs[0].dutycycle;
			candata[4] = nodefanpwmreqs[1].dutycycle;

			if (nodefanpwmreqs[0].bus == 2)
				CAN2Send(NodeCmd_ID, 5, candata);
			else
				CAN1Send(NodeCmd_ID, 5, candata);

			//CAN1Send(NodeCmd_ID, 5, candata );
		} else // fans on different nodes
		{
			if (queuedfanpwmLeft) {
				candata[0] = nodefanpwmreqs[0].nodeid;
				candata[2] = (0x1 << nodefanpwmreqs[0].output);
				candata[3] = nodefanpwmreqs[0].dutycycle;

				if (nodefanpwmreqs[0].bus == 2)
					CAN2Send(NodeCmd_ID, 4, candata);
				else
					CAN1Send(NodeCmd_ID, 4, candata);

				//CAN1Send(NodeCmd_ID, 4, candata );
			}

			if (queuedfanpwmRight) {
				candata[0] = nodefanpwmreqs[1].nodeid;
				candata[2] = (0x1 << nodefanpwmreqs[1].output);
				candata[3] = nodefanpwmreqs[1].dutycycle;

				if (nodefanpwmreqs[1].bus == 2)
					CAN2Send(NodeCmd_ID, 4, candata);
				else
					CAN1Send(NodeCmd_ID, 4, candata);

				//CAN1Send(NodeCmd_ID, 4, candata );
			}
		}
	}

	return senderror;
}

uint8_t getDevicePowerListSize(void) {
	int i = 0;
	for (; DevicePowerList[i].device != None; i++)
		;
	devicecount = i;
	return i;
}

DevicePower getDevicePowerFromList(uint32_t i) {
	if (i <= devicecount)
		return DevicePowerList[i].device;
	else
		return None;
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
