/*
 * errors.c
 *
 *  Created on: 29 Apr 2021
 *      Author: Visa
 */

#include "ecumain.h"
#include "errors.h"
#include "debug.h"
#include "inverter.h"
#include "input.h"
#include "configuration.h"
#include "output.h"
#include "power.h"
#include "timerecu.h"
#include "semphr.h"
#include "queue.h"

#define MAXERRORMSGLENGTH  MAXERROROUTPUT

static uint8_t criticalerrorset = 0;

struct error_msg {
	char msg[MAXERRORMSGLENGTH + 1];
	time_t time;
};

#define ERRORQUEUE_LENGTH    40
#define ERRORITEMSIZE		sizeof( struct error_msg ) // Nam: Temporarily replacing size of lcd_msg with size of error_msg to remove lcd.h TODO: Figure out what ERRORITEMSIZE actually needs to be
static StaticQueue_t ERRORStaticQueue;
uint8_t ERRORQueueStorageArea[ERRORQUEUE_LENGTH * ERRORITEMSIZE];

QueueHandle_t ERRORQueue;

volatile ErrorsType Errors;

uint16_t ErrorCode;

uint8_t critcalerrorcode;

bool logerrors = false;

void LogError(char *message) {
	if (logerrors) {
		struct error_msg error;
		error.time = getTime();
		strncpy(error.msg, message, MAXERRORMSGLENGTH);
		xQueueSendToBack(ERRORQueue, &error, 0); // send it to error state handler queue for display to user.
	}
	DebugMsg(message); // also send it to UART output immediately.
}

void SetErrorLogging( bool log) {
	logerrors = log;
}

int OperationalErrorHandler(uint32_t OperationLoops) {
	// determine whether occurred error is totally or partially recoverable, and what to do. Try to reset device in question.
	// If still not co-operating either disable, suggest limp mode, or if absolutely critical, total failure.

	// essentially if eg front speed sensors, or yaw sensor, or steering angle, or a single brake sensor
	// then allow near normal operation without those sensors operating.

	// if one inverter

	static uint32_t errorstatetime = 0;

	{
		// TODO get a better way to indicate error state.
		CAN_SendStatus(1, OperationalErrorState,
				Errors.OperationalReceiveError + (0 << 16));
	}

	if (OperationLoops == 0) // reset state on entering/rentering.
			{
		CAN_SendDebug(EES_ID);
		char str[MAXERRORMSGLENGTH + 1]; // Nam: Replacing LCDCOLUMNS with MAXERRORMSGLENGTH to remove lcd.h TODO: Figure out a proper value to use

		sprintf(str, "Loc:%.2X Code:%.4X", Errors.ErrorPlace,
				Errors.ErrorReason);
		DebugMsg(str);

		InverterAllowTorqueAll( false); // immedietly stop allowing torque request.

		ShutdownCircuitSet( false);

		ClearHVLost();

		ClearCriticalError();

		CAN_SendTimeBase();

		sprintf(str, "Errorstate: %.4X", 0);
		// send cause of error state.

		ConfigReset();

//		CAN_NMT( 2, 0x0 ); // send stop command to all nodes.  /// verify that this stops inverters.
		blinkOutput(TSLED, LEDBLINK_FOUR, LEDBLINKNONSTOP);
		blinkOutput(RTDMLED, LEDBLINK_FOUR, LEDBLINKNONSTOP);
		errorstatetime = gettimer();
	}

	struct error_msg error;

	while (uxQueueMessagesWaiting(ERRORQueue)) {
		xQueueReceive(ERRORQueue, &error, 0);

	}

	if (Errors.ErrorPlace) {
		CAN_SendErrors();
	}

	if (Shutdown.BMSReason != 0) {
		char statusstr[32];
		sprintf(statusstr, "ERROR State BMS %d", Shutdown.BMSReason);
	}



	if (!CheckShutdown()) // indicate shutdown switch status with blinking rate.
	{
		blinkOutput(TSLED, LEDBLINK_ONE, LEDBLINKNONSTOP);
		blinkOutput(RTDMLED, LEDBLINK_ONE, LEDBLINKNONSTOP);
	} else {
		blinkOutput(TSLED, LEDBLINK_FOUR, LEDBLINKNONSTOP);
		blinkOutput(RTDMLED, LEDBLINK_FOUR, LEDBLINKNONSTOP);
	}

	int allowreset = 0; // allow reset if this is still 0 after checks.

	char str[80] = "ERROR: ";

	if (errorstatetime + MS1000 * 2 > gettimer()) // ensure error state is seen for at least 2 seconds.
			{
		allowreset += 1;
		strcat(str, "");
	}

	if (DeviceState.timeout) {
		strcat(str, "NTO ");
	}

	// wait for restart request if allowed by error state.
	if (allowreset == 0 && (checkReset() == 1 // manual reset
#ifdef AUTORESET
        		|| ( invertererror  && allowautoreset ) // or automatic reset if allowed inverter error.
#endif
	)) {
		//loopcount = 0;
		return StartupState;
		// try to perform a full reset back to startup state here on user request.
	}

	// check for restart request. -> pre operation.
	return OperationalErrorState;

	//return FatalErrorState; // only go to fatal error if error deemed unrecoverable.
}

void SetCriticalError(uint8_t err) {
	criticalerrorset |= (1 << err);
	DebugPrintf("Logging critical error %lu", err);
}

uint8_t CheckCriticalError(void) {
	return criticalerrorset;
}

void ClearCriticalError(void) {
	criticalerrorset = 0;
}

int initERRORState(void) {
	ERRORQueue = xQueueCreateStatic(ERRORQUEUE_LENGTH, ERRORITEMSIZE,
			ERRORQueueStorageArea, &ERRORStaticQueue);

	vQueueAddToRegistry(ERRORQueue, "ERRORQueue");

	SetErrorLogging( true);

	return 0;
}
