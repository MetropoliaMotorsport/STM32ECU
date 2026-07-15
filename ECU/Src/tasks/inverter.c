/*
 * inverter.c
 *
 *  Created on: 23 Mar 2021
 *      Author: Visa
 */

// 1041 ( 411h )  52 1 0 0 -< turn on
// 1041 ( 411h )  49 0 175 0<- trigger message
#include "ecumain.h"
#include "limits.h"
#include "eeprom.h"
#include "errors.h"
#include "inverter.h"
#include "semphr.h"
#include "torquecontrol.h"
#include "watchdog.h"
#include "power.h"
#include "power.h"
#include "taskpriorities.h"
#include "timerecu.h"
#include "can_ids.h"
#include "lenzeinverter.h"
#include "node_device.h"

DeviceStatus GetInverterState(void);
int8_t getInverterControlWord(const InverterState_t *Inverter);
void InvInternalResetRDO(void);

bool InvStartupState(volatile InverterState_t *Inverter,
		const uint8_t CANRxData[8], bool resend);

#define INVSTACK_SIZE 128*3
#define INVTASKNAME  "InvTask"
StaticTask_t xINVTaskBuffer;
StackType_t xINVStack[INVSTACK_SIZE];

TaskHandle_t InvTaskHandle;

#define InvQUEUE_LENGTH    20
#define InvITEMSIZE		   sizeof( Inv_msg )

#define InvCfgQUEUE_LENGTH    20   // should only have one pending send per inverter.
#define InvCfgITEMSIZE		   sizeof( InvCfg_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t InvStaticQueue, InvCfgStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
 uxQueueLength * uxItemSize bytes. */
uint8_t InvQueueStorageArea[InvQUEUE_LENGTH * InvITEMSIZE];
uint8_t InvCfgQueueStorageArea[InvCfgQUEUE_LENGTH * InvCfgITEMSIZE];

QueueHandle_t InvQueue, InvCfgQueue;

SemaphoreHandle_t InvUpdating;

InverterState_t InverterState[MAX_MOTORCOUNT];
InverterState_t invalidinv = { 0 };

InverterState_t* getInvState(uint8_t inv) {
	if (inv >= 0 && inv < MAX_MOTORCOUNT)
		return &InverterState[inv];
	else {
		return &invalidinv;
	}
}


void InverterAllowTorqueAll( bool allow) {
	for (int i = 0; i < MAX_MOTORCOUNT; i++) {
		InverterState[i].AllowTorque = allow;
	}
}


// task shall take power handling request, and forward them to nodes.
// ensure contact is kept with brake light board as brake light is SCS.

// how to ensure power always enabled?

volatile bool invertersinerror = false;

DeviceStatus InverterStates[MAX_MOTORCOUNT];

bool InvSendSDO(uint16_t id, uint16_t idx, uint8_t sub, uint32_t data) {
	InvCfg_msg msg;
	msg.id = id;
	msg.idx = idx;
	msg.sub = sub;
	msg.data = data;
	if (xQueueSend(InvCfgQueue, &msg, 0))
		return true;
	else {
		//DebugPrintf("Failed to add SDO %d %4X to queue at (%lu)", id, idx,
				//gettimer());
		return false;
	}
}


// task to manage inverter state.
void InvTask(void *argument) {
	xEventGroupSync(xStartupSync, 0, 1, portMAX_DELAY); // ensure that tasks don't start before all initialisation done.

	resetInv();

	uint8_t watchdogBit = registerWatchdogBit("InvTask");

	Inv_msg msg;

	DeviceState.Inverter = OFFLINE;

	//DebugMsg("Inv Waiting setup");
	CAN_SendErrorStatus(8, 0, 0);

	uint32_t InvReceived = 0;

	CarState.AllowTorque = true; // hack for now, this should be controlled somewhere.

	for (int i = 0; i < MAX_MOTORCOUNT; i++)
		InverterState[i].appc_on = true;


	while (1) {

		for(int i = 0; i < MAX_MOTORCOUNT; i++)
		{			
			if(!InverterState[i].appc_on){					
					uint8_t msg[8] = { 0 };
					uint8_t msg2[8] = {0};

					CAN1Send(LENZE_RPDO5_ID +  InverterState[i].COBID, 8, msg);

					msg[0] = getInverterControlWord(&InverterState[i]);

				if(InverterState[i].InvState == OPERATIONAL && CarState.PRE_Done && CarState.InvRunning)
					{					

						int32_t vel = 20000 * SPEEDSCALING;
						int16_t torque;
						if(InverterState[i].COBID == 0xE)
							torque = CarState.pedalreq * TORQUESCALING; //* (CarState.MaxTorque / MAXInverterTorque);
						else
							torque = CarState.pedalreq * TORQUESCALING * 0.5;

						storeLEint32(vel, &msg[2]);
						storeLEint16(torque, &msg[6]);

						storeLEint16(620*16, &msg2[0]); //max DC voltage
						storeLEint16(400*16, &msg2[2]); // min DC voltage.
						storeLEint16(20*16, &msg2[4]); // max power
						storeLEint16(0, &msg2[6]); // max regeneration
											
					}

			
					CAN1Send(LENZE_RPDO3_ID + InverterState[i].COBID, 8, msg);
					CAN1Send(LENZE_RPDO1_ID + InverterState[i].COBID, 8, msg);

					CAN1Send(LENZE_RPDO4_ID + InverterState[i].COBID, 8, msg2);
					CAN1Send(LENZE_RPDO2_ID + InverterState[i].COBID, 8, msg2);
				
			}

			if((gettimer() - InverterState[i].rdo_time > 2000) && InverterState[i].appc_on && InverterState[i].rdo_ctnr != 0){
				InverterState[i].appc_on = false;
				CAN_SendDebug(inverters_received);
				CANSendSDO(bus0, InverterState[i].COBID + 31, 0x4004, 1, 1234);
			}
		}
			vTaskDelay(1);
		

		// only allow one command per cycle. Switch to syncing with main task to not go out of sync?

		xEventGroupSync(xCycleSync, 0, 1, portMAX_DELAY); // wait for main cycle.
		// after synced, send current state for next cycle. Higher priority task, so should be received first.
		xTaskNotifyWait( pdFALSE, ULONG_MAX, &InvReceived, 0);
	}

	// clear up if somehow get here.
	vTaskDelete(NULL);
}

// fail process, inverters go from 31->33h->60h->68h  when no HV supplied and request startup.
// states 3->1 ( stop )->-99 ( error )

DeviceStatus InternalInverterState(uint16_t Status) // status 104, failed to turn on HV 200, failure of encoders/temp
{
	// establish current state machine position from return status.
	if ((Status & 0b01001111) == 0b01000000) // 64
			{ // Switch on disabled
		return BOOTUP;
	} else if ((Status & 0b01101111) == 0b00100001) // 49
			{ // Ready to switch on
		return STOPPED;
	} else if ((Status & 0b01101111) == 0b00100011) // 51
			{ // Switched on. HV?
		return PREOPERATIONAL;
	} else if ((Status & 0b01101111) == 0b00100111) // 55
			{ // Operation enabled.
		return OPERATIONAL;
	} else if (((Status & 0b01101111) == 0b00000111)
			|| ((Status & 0b00011111) == 0b00010011)) { // Quick Stop Active
		return QUICKSTOP;
	} else if (((Status & 0b01001111) == 0b00001111)
			|| ((Status & 0b01001111) == 0b00001001)) { // fault reaction active, will move to fault status next
		return INERRORSTOPPING;
	} else if (((Status & 0b01001111) == 0b00001000)
			|| ((Status & 0b00001000) == 0b00001000)) { // fault status
		return INERROR;
		// send reset
	} else { // unknown state
		return 0; // state 0 will request reset to enter State 1,
		// will fall here at start of loop and if unknown status.
	}
}

DeviceStatus GetInverterState(void) {
	return DeviceState.Inverter;
}



int8_t getInverterControlWord(const InverterState_t *Inverter) // returns response to send inverter based on current state.
{
	uint16_t TXState;

	DeviceStatus State;

	State = Inverter->InvState;

	TXState = 0; // default  do nothing state.
	// process regular state machine sequence
	switch (State) {
	case OFFLINE: // state 0: Not ready to switch on, no can message. Internal state only at startup.
		TXState = 0b10000000; // send bit 128 reset message to enter state 1 in case in fault. - fault reset.
		break;

	case BOOTUP: // State 1: Switch on Disabled.
		TXState = 0b00000110; // send 0110 shutdown message to request move to State 2.
		break;

	case STOPPED: // State 2: Ready to switch on
		// We are ready to turn on, so allow high voltage.
		// we are in state 2, process.
		// process shutdown request here, to move to move to state 1.
		if (CarState.PRE_Done) { // TS enable button pressed and both inverters are marked HV ready proceed to state 3.

			TXState = 0b00001111; // Lenze doesn't want to go to pre operational from stopped, have to skip straight to operational.

		} else {
			TXState = 0b00000110; // no change, continue to request State 2.
		}
		break;

	case PREOPERATIONAL: // State 3: Switched on   <---- check this case.
		// we are powered on, so allow high voltage if available
		if (CarState.PRE_Done)			  // IdleState ) <-
		{ // TS enable button has been pressed, proceed to request power on if all inverters on.
			TXState = 0b00001111; // Request Enable operation, State 4.
		} else if (!CarState.PRE_Done) { // return to switched on state.
			TXState = 0b00000000; // 0b00000000; // request Disable Voltage, drop to ready state.
		} else {  // no change, continue to request State 3.
			TXState = 0b00000000;
		}
		break;

	case OPERATIONAL: // State 4: Operation Enable
		// we are powered on, so allow high voltage.
		if(CarState.PRE_Done){
			TXState = 0b00001111;
		}
		else{
			TXState = 0b00000000;
		}
		
		break;
	case QUICKSTOP:

			TXState = 0b00000010;

		break;
		//	case -1 : //5 Quick Stop Active - Fall through to default to reset state.

		//	case -2 : //98 Fault Reason Active

		//	case -99 : //99 Fault

	case INERROR:
		TXState = 0b10000000; // 128
	default: // unknown identifier encountered, ignore. Shouldn't be possible to get here due to filters.

		TXState = 0b00000000; // 0 don't transmit any command for error, deal with it seperately.
		break;
	}

	return TXState;
}

void resetInv(void) {
//	InvInternalResetRDO();

	DeviceState.Inverter = OFFLINE;

	for (int i = 0; i < MAX_MOTORCOUNT; i++) {
		InverterState[i].InvState = OFFLINE;
		InverterState[i].Device = OFFLINE;
		InverterState[i].InvCommand = 0x0;
		InverterState[i].Torque_Req = 0;
		InverterState[i].Speed = 0;
		InverterState[i].Motor = i;
		InverterState[i].MCChannel = false;
		InverterState[i].InvRequested = BOOTUP;

		InverterState[i].MotorTemp = 0;
		InverterState[i].InvTemp = 0;
		InverterState[i].InvCurrent = 0;

		//InverterState[i].AllowRegen = getEEPROMBlock(0)->Regen>0?true:false;
		InverterState[i].AllowTorque = false;

		Errors.InvAllowReset[i] = 1;

		InverterState[i].rdo_ctnr = 0;
	}

	InverterState[0].COBID = Inverter1_NodeID;
	InverterState[1].COBID = Inverter1_NodeID;

	InverterState[0].MCChannel = 0;
	InverterState[1].MCChannel = 1;

#if MAX_MOTORCOUNT > 2
	InverterState[2].COBID = Inverter2_NodeID;
	InverterState[3].COBID = Inverter2_NodeID;

	InverterState[2].MCChannel = 0;
	InverterState[3].MCChannel = 1;
#endif

	Errors.InverterError = 0; // reset logged errors.
}

int initInv(void) {
	resetInv(); // sets up InverterState, id's, etc, so that CAN functions will not be called till setup.

	RegisterResetCommand(resetInv);

	registerInverterCAN();

	InvUpdating = xSemaphoreCreateMutex();
	

	InvQueue = xQueueCreateStatic(InvQUEUE_LENGTH, InvITEMSIZE,
			InvQueueStorageArea, &InvStaticQueue);

	vQueueAddToRegistry(InvQueue, "InverterQueue");

	InvCfgQueue = xQueueCreateStatic(InvCfgQUEUE_LENGTH, InvCfgITEMSIZE,
			InvCfgQueueStorageArea, &InvCfgStaticQueue);

	vQueueAddToRegistry(InvQueue, "InverterCfgQueue");

	//if (getEEPROMBlock(0)->InvEnabled)
	InvTaskHandle = xTaskCreateStatic(InvTask,
	INVTASKNAME,
	INVSTACK_SIZE, (void*) 1,
	INVTASKPRIORITY, xINVStack, &xINVTaskBuffer);

	return 0;
}
