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

		SetErrorLogging( false);
		preoperationstate = 0xFFFF; // should be 0 at point of driveability, so set to opposite in initial state.
		InverterAllowTorqueAll(false);

		resetOutput(STARTLED, Off);
		resetOutput(TSLED, On);
		resetOutput(RTDMLED, Off);

		ReadyToStart = 0xFFFF;
		// set startup powerstates to bring devices up.

		initVectoring();

		//   	NMTReset(); //send NMT reset when first enter state to catch any missed boot messages, see if needed or not.
		// send to individual devices rather than reset everything if needed.
	}

	{
		CAN_SendStatus(1, PreOperationalState, preoperationstate);

		// do power request

		// test power error checking.
		if (DeviceState.IVTEnabled && DeviceState.IVT == OFFLINE) {
			if (!powerErrorOccurred(IVT))
				setDevicePower(IVT, true);
			else {
				Errors.ErrorPlace = 0xAA;
				Errors.ErrorReason = 0; //TODO error code for lost power.
				return OperationalErrorState;
			}
		}

		// generate device waiting string.

		if (preoperationstate != 0 || ReadyToStart != 0) {
			// TODO add a checker that if all devices on canbus missing, show this instead of individual.

			strcpy(str, "Wait:");

			char *nodewait = "";

#ifdef POWERNODES
			nodewait = getPNodeWait();
			if (nodewait[0] != 0) {
				sprintf(&str[strlen(str)], "P%s ", nodewait);
			}
#endif

#ifdef ANALOGNODES
			if (DeviceState.Sensors != OPERATIONAL) {
				nodewait = getNodeWait();
				if (nodewait[0] != 0) {
					sprintf(&str[strlen(str)], "A%s ", nodewait);
				}
			}
#endif

			if (preoperationstate & (0x1 << InverterReceived)) {
				if (getEEPROMBlock(0)->InvEnabled)
					strcat(str, "INV ");
				else
					strcat(str, "INVDIS ");
			}

			if (preoperationstate & (0x1 << BMSReceived)) {
				strcat(str, "BMS ");
			}
			if (preoperationstate & (0x1 << IVTReceived)) {
				strcat(str, "IVT ");
			}
			if (str[strlen(str)] == 32) {
				str[strlen(str)] = 0;
			}

			if (ReadyToStart != 0) {
				strcpy(str, "Err:");


				if (ReadyToStart & (0x1 << READYTSALBIT)) {
					strcat(str, "TSAL ");
				}

				if (ReadyToStart & (0x1 << READYCONFIGBIT)) {
					strcat(str, "CFG ");
				}

				if (ReadyToStart & (0x1 << READYDEVBIT)) {
					strcat(str, "DEV ");
				}

				if (ReadyToStart & (0x1 << READYSENSBIT)) {
					strcat(str, "SNS ");
				}

				if (ReadyToStart & (0x1 << READYPOWERBIT)) {
					strcat(str, "PWR ");
				}

				if (ReadyToStart & (0x1 << READYINVBIT)) {

					if (getEEPROMBlock(0)->InvEnabled)
						strcat(str, "INV ");
					else
						strcat(str, "INVDIS ");
				}

			}

		} else {
			// TODO print any non critical warning still.
			ClearCriticalError(); // clear any pending critical errors from startup procedure.
		}

	}

	ReadyToStart = 0;

	if (!inConfig()) {
		if (CheckButtonPressed(Config_Input)) {
			if (!testmotors) {
				DebugPrintf("Enter Config\r\n");
				ConfigInput(0xFFFF);
				ReadyToStart |= (1 << READYCONFIGBIT); // we're probably entering config, don't allow startup this cycle.
			}
		}

		static bool showbrakebal = false;

		static bool showadc = false;

		static bool showcurrent = false;

		switch (GetLeftRightPressed()) {
		case -1:
			if (!showadc && !showcurrent)
				showbrakebal = !showbrakebal;
			break;
		case 1:
			if (!showbrakebal && !showcurrent)
				showadc = !showadc;
			break;
		}

		switch (GetUpDownPressed()) {
		case -1:
			if (showcurrent) {
				clearRunningData();
			}
			break;
		case 1:
			if (!showadc && !showbrakebal)
				showcurrent = !showcurrent;
			break;
		}

		if (showcurrent) {
			sprintf(str, "IVT %3dA  0%3dA", runtimedata_p->maxIVTI,
					runtimedata_p->maxMotorI[0]);
			sprintf(str, "1%3dA 2%3dA 3%3dA ", runtimedata_p->maxMotorI[1],
					runtimedata_p->maxMotorI[2], runtimedata_p->maxMotorI[3]);
		}

		if (showbrakebal)

				//Print brake balance

		if (showadc) {
			sprintf(str, "L%3d%% R%3d%% B%3d%% ",
					APPS1.data / 10,
					APPS2.data / 10,
					BPPS.data / 10);
			sprintf(str, "Ang %d", SteeringAngle.data);

		}
	} else {
		ReadyToStart |= (1 << READYCONFIGBIT); // being in config means not ready to start.
		// process config input.

		if (CheckButtonPressed(Config_Input)) {
			DebugPrintf("Enter\r\n");
			ConfigInput( KEY_ENTER);
		}

		switch (GetLeftRightPressed()) {
		case -1:
			DebugPrintf("Left\r\n");
			ConfigInput( KEY_LEFT);
			break;
		case 1:
			DebugPrintf("Right\r\n");
			ConfigInput( KEY_RIGHT);
			break;
		}

		switch (GetUpDownPressed()) {
		case -1:
			DebugPrintf("Up\r\n");
			ConfigInput( KEY_UP);
			break;
		case 1:
			DebugPrintf("Down\r\n");
			ConfigInput( KEY_DOWN);
			break;
		}

	}

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

//	if ( testmotorslast ) InverterSetTorque(&adj, 1000);

#if 1
	if (CarState.APPSstatus)
		setOutput(RTDMLED, On);
	else
		setOutput(RTDMLED, Off);
#endif
	// Check startup requirements.

	if (!getDevicePower(Front1) || !getDevicePower(Front2)
			|| !getDevicePower(Inverters)) {
		if (!TSLEDstate) {
			setOutput(TSLED, On);
			TSLEDstate = true;
		}
	}
	if (getDevicePower(Front1) && getDevicePower(Front2)
			&& getDevicePower(Inverters)) {
		if (TSLEDstate) {
			setOutput(TSLED, Off);
			TSLEDstate = false;
		}
	}

	static bool powerset = false;

	if (CheckTSActivationRequest()) {
		if (!powerset) {
			invRequestState(BOOTUP);
			resetDevicePower(Inverters);
			setDevicePower(Inverters, true);
			setDevicePower(RightPump, true);
			setDevicePower(LeftPump, true);
			setDevicePower(RightFans, true);
			setDevicePower(LeftFans, true);
			DebugMsg("Power requested.");
			CAN_SendDebug(PWRR_ID);
			powerset = true;
		} else {
			setDevicePower(Inverters, false);
			DebugMsg("Power off to inverters requested.");
			CAN_SendDebug(PWRINV_ID);
			powerset = false;
		}

	}

	if (preoperationstate != 0) {
		ReadyToStart |= (1 << READYDEVBIT);
	}

	for (int i = 0; i < MOTORCOUNT; i++) {
		if (getInvState(i)->Device == OFFLINE) {
			ReadyToStart |= (1 << READYINVBIT);
		}
	}

	if (DeviceState.CriticalSensors != OPERATIONAL) {
		ReadyToStart |= (1 << READYSENSBIT);
	} // require critical sensor nodes online for startup.
	if (DeviceState.PowerNode1 != OPERATIONAL || DeviceState.PowerNode2 != OPERATIONAL) {
		ReadyToStart |= (1 << READYPOWERBIT);
	}
#ifdef CHECKTSALPOWER
	if ( !getDevicePower(TSAL) ) { ReadyToStart |= (1<<READYTSALBIT); } // require TSAL power to allow startup.
	#endif
	if (ReadyToStart == 0) {
		setOutput(STARTLED, On);
		// devices are ready and in pre operation state.
		// check for request to move to active state.

		if (testmotors) {
			return PreOperationalState;
		}

		if (CheckActivationRequest()) // check if driver has requested activation and if so proceed
		{
			OperationLoops = 0;
			return OperationalReadyState; // normal operational state on request
		}
	} else { // hardware not ready for active state
			 //setOutput(STARTLED, On);
		blinkOutput(STARTLED, BlinkSlow, 100);
		if (OperationLoops == 50) // 0.5 seconds, send reset nmt, try to get inverters online if not online at startup.
				{

		}

		if (CheckActivationRequest() == 1) {
			if (1) // calculate this to max time for expecting everything online
			{
				// user pressed requesting startup sequence before devices ready
				blinkOutput(TSLED, BlinkFast, 1000);
				CAN_SendErrorStatus(1, PowerOnRequestBeforeReady, 0);

			}
		}
	}

	if (CheckActivationRequest())
		DebugPrintf("Start pressedr\n");
		CAN_SendDebug(STT_ID);

	if (CheckTSActivationRequest())
		DebugPrintf("TS pressedr\n");
		CAN_SendDebug(TSP_ID);

	if (CheckRTDMActivationRequest())
		DebugPrintf("RTDM pressedr\n");

	return PreOperationalState; // nothing caused entry to a different state, continue in current state.
}

