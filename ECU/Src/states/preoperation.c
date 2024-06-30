/**
 ******************************************************************************
 * @file           : operation.c
 * @brief          : Operational Loop and related function
 ******************************************************************************

 ******************************************************************************
 */

#include "ecumain.h"
#include "runningprocess.h"
#include "preoperation.h"
#include "configuration.h"
#include "errors.h"
#include "power.h"
#include "node_device.h"
#include "bms.h"
#include "input.h"
#include "output.h"
#include "inverter.h"
#include "powernode.h"
#include "timerecu.h"
#include "debug.h"
#include "ivt.h"
#include "eeprom.h"
#include "canecu.h"

//#define PRINTDEBUGRUNNING

static uint16_t DevicesOnline(uint16_t returnvalue) {

	//TODO update function

	returnvalue = 0;
	return returnvalue; // should be 0 when everything ready.
}

static volatile bool testmotors = false;

void setTestMotors( bool state) {
	testmotors = state;
}

#define READYCONFIGBIT		0
//#define READYSDCBIT     	1
#define READYDEVBIT			2
#define READYINVBIT			3
#define READYSENSBIT		4
#define READYPOWERBIT		5
#define READYTSALBIT		6
#define READYTESTING		7

uint8_t buz_timer = 0;
// get external hardware upto state to allow entering operational state on request.
int PreOperationState(uint32_t OperationLoops) {
//	static int OperationLoops = 0;

	static uint16_t preoperationstate;
	static uint16_t ReadyToStart;
	static uint32_t ledtimer;
	static bool TSLEDstate;

	char str[80] = "";


	if (OperationLoops == 0) {
		
		TSLEDstate = false;
		ledtimer = gettimer();

		CAN_SendDebug(EPOS_ID);

		//DebugMsg("Entering Pre Operation State");

		//SetErrorLogging( false);
		preoperationstate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state.
		//InverterAllowTorqueAll(false);


		//////////////////////////////TODO Fix LED control
		//resetOutput(STARTLED, Off);
		//resetOutput(TSLED, On);
		//resetOutput(RTDMLED, Off);
		//////////////////////////////

		ReadyToStart = 0;
		// set startup powerstates to bring devices up.

		initVectoring();

		

		//   	NMTReset(); //send NMT reset when first enter state to catch any missed boot messages, see if needed or not.
		// send to individual devices rather than reset everything if needed.
	}


	SendPwrCMD(Inverters, true);
	vTaskDelay(1);

	SendPwrCMD(RightPump, true);
	SendPwrCMD(LeftPump, true);

	{
		CAN_SendStatus(1, PreOperationalState, preoperationstate);

		// do power request

	}
/*
	ReadyToStart = 0;
	
	vTaskDelay(5);

	preoperationstate = DevicesOnline(preoperationstate);

	// set drive mode

	setCurConfig();

	// allow APPS checking before RTDM
	int16_t pedalreq;
	float Torque_Req = PedalTorqueRequest(&pedalreq);

	vectoradjust adj;
	speedadjust spd;

	doVectoring(Torque_Req, &adj, &spd, pedalreq);
*/
	ShutdownCircuitSet(true);
	
	PedalTorqueRequest(NULL);

	if(CarState.PRE_Done && buz_timer < 58)
	{		buz_timer++;
		SendPwrCMD(Buzzer, (buz_timer < 56 ? true : false));		
	}
		

	return PreOperationalState; // nothing caused entry to a different state, continue in current state.
}

