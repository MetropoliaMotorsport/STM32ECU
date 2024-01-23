/*
 * power.c
 *
 *  Created on: Jul 17, 2020
 *      Author: Visa
 */

#include "ecumain.h"
#include "limits.h"
#include "task.h"
#include "power.h"
#include "powerloss.h"
#include "powernode.h"
#include "errors.h"
#include "adcecu.h"
#include "inverter.h"
#include "taskpriorities.h"
#include "debug.h"
#include "timerecu.h"
#include "semphr.h"
#include "output.h"
#include "input.h"

TaskHandle_t PowerTaskHandle = NULL;

#define POWERSTACK_SIZE 128*6
#define POWERTASKNAME  "PowerTask"
StaticTask_t xPOWERTaskBuffer;
StackType_t xPOWERStack[POWERSTACK_SIZE];

#define PowerQUEUE_LENGTH    20
#define PowerErrorQUEUE_LENGTH    20
#define PowerITEMSIZE		sizeof( Power_msg )
#define PowerErrorITEMSIZE		sizeof( Power_Error_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t PowerStaticQueue;
static StaticQueue_t PowerErrorStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
 uxQueueLength * uxItemSize bytes. */
uint8_t PowerQueueStorageArea[PowerQUEUE_LENGTH * PowerITEMSIZE];
uint8_t PowerErrorQueueStorageArea[PowerErrorQUEUE_LENGTH * PowerErrorITEMSIZE];

QueueHandle_t PowerQueue, PowerErrorQueue;

ShutdownState Shutdown;

bool HVLost;

// task shall take power handling request, and forward them to nodes.
// ensure contact is kept with brake light board as brake light is SCS.

// how to ensure power always enabled?

static SemaphoreHandle_t waitStr = NULL;

char PNodeWaitStr[20] = "";
uint32_t curpowernodesOnline = 0;

char* getPNodeWait(void) {
	static char PowerWaitStrRet[20] = "";
	xSemaphoreTake(waitStr, portMAX_DELAY);
	strcpy(PowerWaitStrRet, PNodeWaitStr);
	xSemaphoreGive(waitStr);
	return PowerWaitStrRet;
	// TODO add a mutex
}

void SetHVLost(void) {
	if (!HVLost) {
		HVLost = true;
		DebugMsg("Logging HV Lost");
	}
}

void ClearHVLost(void) {
	if (HVLost) {
		HVLost = false;
		DebugMsg("Clearing HV Lost");
	}
}

bool CheckHVLost(void) {
	// if ( Shutdown.AIRm == 0 || Shutdown.AIRp == 0 || Shutdown.TS_OFF || HVLost ||
	if (CarState.VoltageINV < 60) {
		return true;
	}
	return false;
}

void PowerTask(void *argument) {
	xEventGroupSync(xStartupSync, 0, 1, portMAX_DELAY);

	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT(PowerQueue);

	Power_msg msg;
	Power_Error_msg errormsg;

	char str[MAXERROROUTPUT];

	resetPowerLost();
	xQueueReset(PowerErrorQueue);

	uint32_t powernodesOnline = 0;
	uint32_t lastpowernodesOnline = 0;
	uint32_t powernodesOnlineSince = 0;
	uint32_t count = 0;
	uint32_t lastseenHV = 0;
	uint32_t lastseenpumpR = 0;
	uint32_t lastseenpumpL = 0;
	uint32_t restartpumpR = 0;
	uint32_t restartpumpL = 0;
	bool HVactive = false;

	uint32_t lastseenall = 0;

	uint32_t oldestcritical = 0;

	bool fanssent = false;

	bool LVdown = false;
	uint32_t LVdowntime = 0;

	bool IMDset = false;
	bool TSOFFset = true;
	bool BMSset = false;

	//setOutputNOW(BMSLED, Off);

	while (1) {
		lastpowernodesOnline = powernodesOnline;
		// read the values from last cycle.
		xTaskNotifyWait( pdFALSE, ULONG_MAX, &powernodesOnline, 0);

		// block and wait for main cycle.
		xEventGroupSync(xCycleSync, 0, 1, portMAX_DELAY);

		oldestcritical = getOldestPNodeData();

		uint32_t curtime = gettimer();

		if (DeviceState.PowerNodes == OPERATIONAL) {
			if (curtime - CYCLETIME * 2 - 1 > oldestcritical) {
				DebugPrintf("Oldest PNode data %d old at (%lu)",
						curtime - oldestcritical, curtime);
			}
		}

		// find oldest data.
		count++;
		while (xQueueReceive(PowerErrorQueue, &errormsg, 0)) {
			if (errormsg.error == 0xFFFF) {
				snprintf(str, MAXERROROUTPUT, "Power err: LV Down at (%lu)",
						gettimer());

				LogError(str);
				LVdown = true;
				LVdowntime = gettimer();

				setAllPowerActualOff();
				CAN_SendErrorStatus(7, 0, 0);
			} else {

				if (PNodeGetErrType(errormsg.error)) // only show full errors for now.
						{
					snprintf(str, MAXERROROUTPUT,
							"Power err: %d %lu %s at (%lu)", errormsg.nodeid,
							errormsg.error, PNodeGetErrStr(errormsg.error),
							gettimer());
					LogError(str);
				} else {
					snprintf(str, MAXERROROUTPUT,
							"Power warn: %d %lu %s at (%lu)", errormsg.nodeid,
							errormsg.error, PNodeGetErrStr(errormsg.error),
							gettimer());
					DebugMsg(str);
				}

			}
		}

		if (LVdown && gettimer() - LVdowntime > 500) {
			// check if we have voltage back.
			if (checkPowerState()) {
				LVdown = false;
				resetPowerLost();
				DebugPrintf("LV Power restored at (%lu)", curtime);
			}
		}

		// clear command queue
		while (xQueueReceive(PowerQueue, &msg, 0)) {
#ifdef POWERNODES
			switch (msg.cmd) {
			case PowerErrorReset:
				setNodeDevicePower(msg.power, msg.enabled, true);
				break;

			case PowerError:
				break;

			case DirectPowerCmd:
				if (msg.power == LeftPump) {
					if (msg.enabled)
						lastseenpumpL = curtime;
					else
						lastseenpumpL = 0;
				} else if (msg.power == RightPump) {
					if (msg.enabled)
						lastseenpumpR = curtime;
					else
						lastseenpumpR = 0;
				}

				setNodeDevicePower(msg.power, msg.enabled, false);
				break;
			case FanPowerCmd:
				sendFanPWM(msg.PWMLeft, msg.PWMRight);
				break;
			default:
				break;
			}
#else
 // on PDM only fans and HV are controlled.

#endif
		}

		// check if powernodes received.

		uint32_t looptime = gettimer();

		xSemaphoreTake(waitStr, portMAX_DELAY);

		powernodesOnlineSince |= powernodesOnline; // cumulatively add

		if ((powernodesOnline & PNodeAllBit) == PNodeAllBit) // all expected power nodes reported in. // TODO automate
		{
			if (DeviceState.PowerNodes != OPERATIONAL) {
				DebugMsg("Power nodes all online");
			}
			DeviceState.PowerNodes = OPERATIONAL;
			PNodeWaitStr[0] = 0;
			powernodesOnlineSince = 0;
			curpowernodesOnline = powernodesOnline;
			lastseenall = looptime;
		} else if (looptime - lastseenall > PDMTIMEOUT) // only update status to rest of code every timeout val.
		{
			lastseenall = looptime;
			setPowerNodeStr(powernodesOnlineSince);
			strcpy(PNodeWaitStr, getPNodeStr());

			// update the currently available nodes
			curpowernodesOnline = powernodesOnlineSince;

			if (powernodesOnlineSince == 0) {
				if (DeviceState.PowerNodes != OFFLINE) {
					DebugMsg("Power node timeout");
				}
				DeviceState.PowerNodes = OFFLINE; // can't see any nodes, so offline.
			} else {
				if (DeviceState.PowerNodes != INERROR) {
					DebugMsg("Power nodes partially online");
				}
				DeviceState.PowerNodes = INERROR; // haven't seen all needed, so in error.
			}

			powernodesOnlineSince = 0;
		}

		if (CarState.VoltageINV > TSACTIVEV) {
			lastseenHV = looptime;
		}

#if 0
		if ( looptime - lastseenHV > PDMTIMEOUT )
		{
			if ( HVactive )
			{
				HVactive = false;
				SetHVLost();
			}
		}
#endif

		if ((curpowernodesOnline & PNODECRITICALBITS) == PNODECRITICALBITS) {
			DeviceState.CriticalPower = OPERATIONAL;
			// we've received all the SCS data
		} else {
			DeviceState.CriticalPower = INERROR;
		}

		if (lastpowernodesOnline != powernodesOnline) {
			char str[40];
			snprintf(str, 40, "Powernodes diff %lu", count);
			//		DebugMsg(str);
		}

		xSemaphoreGive(waitStr);
#if 1
		if (!CheckBMS()) {
#ifndef BENCH

#endif
			if (!BMSset) {
				SetCriticalError(CRITERBMS); // keep flagging BMS error whilst on.
				setOutputNOW(BMSLED, On);
				BMSset = true;
			}
		}
#endif
#if 1 // allow AMS light to go out when normal operation is resumed. Light merely shows state of timeout of error.
		else {
			if (BMSset) {
				//setOutputNOW(BMSLED, Off);
				BMSset = false;
			}
		}
#endif
		if (CheckTSOff()) {
			if (!TSOFFset) {
				DebugMsg("TSOff on");
				setOutput(TSOFFLED, On);
				TSOFFset = true;
			}
		} else {
			if (TSOFFset) {
				DebugMsg("TSOff off");
				setOutput(TSOFFLED, Off);
				TSOFFset = false;
			}
		}

#ifdef TSALP
		static uint32_t nexttsal = 0;
		static bool tsalset;
		static bool tsalgset = true;

		if (CarState.VoltageINV > 60) {
			if (tsalgset) {
				tsalgset = false;
				setDevicePower(TSALG, false);
			}
		} else {
			if (!tsalgset) {
				tsalgset = true;
				setDevicePower(TSALG, true);
			}
		}

		if (CarState.VoltageINV > 50) {
			if (curtime > nexttsal) {
				tsalset = !tsalset;
				setDevicePower(TSAL, tsalset);
				nexttsal = curtime + 200;
			}
		} else {
			if (tsalset)
				setDevicePower(TSAL, false);
			tsalset = false;
			nexttsal = 0;
		}
#endif

		if (CheckIMD()) {
			if (!IMDset) {
#ifndef BENCH
				SetCriticalError(CRITERIMD); // keep flagging IMD error whilst on to not allow operation.
#endif
				setOutputNOW(IMDLED, On);
				setOutputNOW(LED6, On);
				IMDset = true;
			}
		} else {
			if (IMDset) {
				//setOutputNOW(IMDLED, Off);
				//setOutputNOW(LED6, Off);
				IMDset = false;
			}
		}

#ifdef PUMPMINIMUM_I
		if (lastseenpumpL) // pump should be on if this is not 0., assume that both pumps should be running if one is.
		{
			if (CarState.I_LeftPump > PUMPMINIMUM_I) {
				lastseenpumpL = curtime; // update we've seen OK value.
			} else if (curtime > lastseenpumpL + 2000) {
				// two seconds since saw an acceptable current from PUMP. reset it.
				lastseenpumpL = curtime;
				resetDevicePower(LeftPump);
				setDevicePower(LeftPump, false);
				restartpumpL = curtime + 2000;
			}

			if (restartpumpL && curtime > restartpumpL) {
				setDevicePower(LeftPump, true);
				restartpumpL = 0;
			}

			if (CarState.I_RightPump > PUMPMINIMUM_I) {
				restartpumpL = 0;
				lastseenpumpR = curtime;
			} else if (curtime > lastseenpumpR + 2000) {
				// two seconds since saw an acceptable current from PUMP. reset it.
				lastseenpumpL = curtime;
				resetDevicePower(RightPump);
				setDevicePower(RightPump, false);
				restartpumpR = curtime + 2000;
				setDevicePower(RightPump, true);
			}

			if (restartpumpR && curtime > restartpumpR) {
				setDevicePower(RightPump, true);
				restartpumpR = 0;
			}
		}
#endif

		// set the fan PWM speed when seen fan power node online.
		if (!fanssent && powernodesOnline & (1 << POWERNODE_FAN_BIT)) {
			sendFanPWM(getEEPROMBlock(0)->FanMax, getEEPROMBlock(0)->FanMax);
			fanssent = true;

			// if fans are set on in menu, put them on at startup.
			if (getEEPROMBlock(0)->Fans) {
				setDevicePower(LeftFans, true); // queue up enabling fans.
				setDevicePower(RightFans, true);
			}
		} else {
			sendPowerNodeReq(); // process pending power requests, but not if also sent fan PWM request this cycle.
		}
	}

	vTaskDelete(NULL);
}

bool CheckShutdown(void) // returns true if shutdown circuit other than ECU is closed
{
#ifdef HPF2023
	return true;
#endif
#ifdef HPF20
//	if ( !Shutdown.BSPDBefore ) return false;
//	if ( !Shutdown.BSPDAfter ) return false;
//	if ( !Shutdown.BOTS ) return false;
//	if ( !Shutdown.InertiaSwitch ) return false;
// ECU is here.
//	if ( !Shutdown.CockpitButton ) return false;
//	if ( !Shutdown.RightButton ) return false;
//	if ( !Shutdown.LeftButton ) return false;
#
	if (!Shutdown.BMS)
		return false;
//	if ( !Shutdown.IMD ) return false;
#endif
	return true;
}

bool CheckBMS(void) // returns true if shutdown circuit other than ECU is closed
{
#ifdef HPF2023
	if (HAL_GPIO_ReadPin(BMS_Input_Port, BMS_Input_Pin)) {
		DebugMsg("BMS input PIN");
	}
	if (DeviceState.BMS != OPERATIONAL) {
		DebugMsg("BMS NOT operational");
	}
	return (!(HAL_GPIO_ReadPin(BMS_Input_Port, BMS_Input_Pin)
			|| DeviceState.BMS != OPERATIONAL));
#else
	return Shutdown.BMS;
#endif
}

bool CheckTSOff(void) // returns true if shutdown circuit other than ECU is closed
{
	return Shutdown.TS_OFF;
}

bool CheckIMD(void) // returns true if shutdown circuit other than ECU is closed
{
#ifdef HPF2023
	if (HAL_GPIO_ReadPin(IMD_Input_Port, IMD_Input_Pin)) {
		DebugMsg("IMD input PIN");
	}
	if (DeviceState.BMS != OPERATIONAL) {
		DebugMsg("BMS NOT operational in IMD check");
	}
	return (HAL_GPIO_ReadPin(IMD_Input_Port, IMD_Input_Pin)
			|| DeviceState.BMS != OPERATIONAL);
#else
	return Shutdown.IMD;
#endif
}

#define MAXSHUTDOWNSTR	40

char* ShutDownOpenStr(void) {
	static char str[255] = "";

	// TODO move these into actual order of SDC.

	snprintf(str, 40, "%s%s%s%s%s%s%s%s%s",
			"", //		(!Shutdown.BSPDBefore)?"BSPDB,":""
			"", //		(!Shutdown.BSPDAfter)?"BSPDA,":"",
			"", //		(!Shutdown.BOTS)?"BOTS,":"",
			(!Shutdown.InertiaSwitch) ? "INRT," : "",
			(!Shutdown.CockpitButton) ? "DRV," : "",
			(!Shutdown.RightButton) ? "RGT," : "",
			(!Shutdown.LeftButton) ? "LFT," : "", (!Shutdown.BMS) ? "BMS," : "",
			"" //		(!Shutdown.IMD)?"IMD,":"",
			);

//	int len=strlen(str);

//	if ( len > 0)
//		str[len-1] = 0;

	return str;
}

void ShutdownCircuitSet( bool state) {
	HAL_GPIO_WritePin( DO15_GPIO_Port, DO15_Pin, state);
	HAL_GPIO_WritePin( Shutdown_GPIO_Port, Shutdown_Pin, state);
}

int ShutdownCircuitState(void) {
	return HAL_GPIO_ReadPin(Shutdown_GPIO_Port, Shutdown_Pin);
}

// request a power state for a device.
bool setDevicePower(DevicePower device, bool state) {
	Power_msg msg;

	msg.cmd = DirectPowerCmd;
	msg.power = device;
	msg.enabled = state;
	return (xQueueSend(PowerQueue, &msg, 0));

}

bool getDevicePower(DevicePower device) {
	return getNodeDevicePower(device);
}

// reset a device's power channel
bool resetDevicePower(DevicePower device) {
	Power_msg msg;

	msg.cmd = PowerErrorReset;
	msg.power = device;
	msg.enabled = false;
	return (xQueueSend(PowerQueue, &msg, 0));

}

// directly set the current power state of device.
bool setPowerState(DevicePowerState devicestate, bool enabled) {
	Power_msg msg;

	if (devicestate != DirectPowerCmd && devicestate != PowerError) {
		msg.cmd = devicestate;
		msg.enabled = enabled;
		return xQueueSend(PowerQueue, &msg, 0);
	}
	return false;
}

void resetPower(void) {

}

void FanPWMControl(uint8_t leftduty, uint8_t rightduty) {
//	for example: [7][2][5][255][128] will set the lowest numbered output (DI3)
//to have a 100% duty cycle and the third output (DI5) to have a 50% duty cycle (if configured as PWM output)
	Power_msg msg;

	msg.cmd = FanPowerCmd;
	msg.PWMLeft = leftduty;
	msg.PWMRight = rightduty;
	xQueueSend(PowerQueue, &msg, 0);

	/*	if(ADCState.Torque_Req_R_Percent > TORQUEFANLATCHPERCENTAGE*10) // if APPS position over requested% trigger fan latched on.
	 {
	 if ( !getNodeDevicePower(LeftFans ) )
	 setDevicePower( LeftFans, true );

	 if ( !getNodeDevicePower(RightFans ) )
	 setDevicePower( RightFans, true );

	 CarState.FanPowered = 1;
	 }
	 */
}

char* getDevicePowerNameLong(DevicePower device) {
	switch (device) {
	case None:
		return "None";
	case Buzzer:
		return "Buzzer";
	case Telemetry:
		return "Telemetry";
	case Front1:
		return "Front1";
	case Inverters:
		return "Inverters";
	case ECU:
		return "ECU";
	case Front2:
		return "Front2";
	case LeftFans:
		return "LeftFans";
	case RightFans:
		return "RightFans";
	case LeftPump:
		return "LeftPump";
	case RightPump:
		return "RightPump";
	case IVT:
		return "IVT";
	case Current:
		return "Current";
	case TSAL:
		return "TSAL";
	case Brake:
		return "Brake";
	case Accu:
		return "Accu";
	case AccuFan:
		return "AccuFan";
	case Back1:
		return "Back1";
	}
	return "Error";
}

xTimerHandle timerHndlBuzzer;

bool soundBuzzer(void) {
	DebugPrintf("Sounding buzzer\n");
	resetDevicePower(Buzzer);
	setDevicePower(Buzzer, true);
	xTimerStart(timerHndlBuzzer, 0);
	return true;
}

static void stopBuzzer(xTimerHandle pxTimer) {
	DebugPrintf("Stopping buzzer\n");
	setDevicePower(Buzzer, false);
}

bool getPowerHVReady(void) {
	return true; // TODO implement rather than dummy.
}

bool PowerLogError(uint8_t nodeid, uint32_t errorcode) {
	Power_Error_msg msg;

	msg.nodeid = nodeid;
	msg.error = errorcode;

	if (xPortIsInsideInterrupt())
		return xQueueSendFromISR(PowerErrorQueue, &msg, NULL);
	else
		return (xQueueSend(PowerErrorQueue, &msg, 0));
}

int initPower(void) {
	RegisterResetCommand(resetPower);

	resetPower();

	HVLost = false;

	waitStr = xSemaphoreCreateMutex();

	PowerQueue = xQueueCreateStatic(PowerQUEUE_LENGTH, PowerITEMSIZE,
			PowerQueueStorageArea, &PowerStaticQueue);

	vQueueAddToRegistry(PowerQueue, "PowerQueue");

	PowerErrorQueue = xQueueCreateStatic(PowerErrorQUEUE_LENGTH,
			PowerErrorITEMSIZE, PowerErrorQueueStorageArea,
			&PowerErrorStaticQueue);

	vQueueAddToRegistry(PowerErrorQueue, "PowerErrorQueue");

	timerHndlBuzzer = xTimerCreate("buzzertimer", /* name */
	pdMS_TO_TICKS(1000), /* period/time */
	pdFALSE, /* auto reload */
	(void*) 0, /* timer ID */
	stopBuzzer); /* callback */

	PowerTaskHandle = xTaskCreateStatic(PowerTask,
	POWERTASKNAME,
	POWERSTACK_SIZE, (void*) 1,
	POWERTASKPRIORITY, xPOWERStack, &xPOWERTaskBuffer);

	initPowerLossHandling();

	return 0;
}

