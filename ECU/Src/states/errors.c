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
#include "lcd.h"
#include "semphr.h"
#include "queue.h"

#define MAXERRORMSGLENGTH  MAXERROROUTPUT

static uint8_t criticalerrorset = 0;

struct error_msg {
  char msg[MAXERRORMSGLENGTH+1];
  time_t time;
};

#define ERRORQUEUE_LENGTH    40
#define ERRORITEMSIZE		sizeof( struct lcd_msg )
static StaticQueue_t ERRORStaticQueue;
uint8_t ERRORQueueStorageArea[ ERRORQUEUE_LENGTH * ERRORITEMSIZE ];

QueueHandle_t ERRORQueue;

volatile ErrorsType Errors;

uint16_t ErrorCode;

uint8_t critcalerrorcode;

bool logerrors = false;

void LogError( char *message )
{
	if ( logerrors )
	{
		struct error_msg error;
		error.time = getTime();
		strncpy( error.msg, message, MAXERRORMSGLENGTH);
		xQueueSendToBack(ERRORQueue,&error,0); // send it to error state handler queue for display to user.

		lcd_send_stringline( 3, message, 2);
	}
	DebugMsg( message ); // also send it to UART output immediately.
}

void SetErrorLogging( bool log )
{
	logerrors = log;
}

int OperationalErrorHandler( uint32_t OperationLoops )
{
	// determine whether occurred error is totally or partially recoverable, and what to do. Try to reset device in question.
	// If still not co-operating either disable, suggest limp mode, or if absolutely critical, total failure.

	// essentially if eg front speed sensors, or yaw sensor, or steering angle, or a single brake sensor
	// then allow near normal operation without those sensors operating.

	// if one inverter

	static uint32_t errorstatetime = 0;

#ifndef everyloop
	if ( ( OperationLoops % LOGLOOPCOUNTSLOW ) == 0 ) // only send status message every 5'th loop to not flood, but keep update on where executing
#endif
	{
		// TODO get a better way to indicate error state.
		  CAN_SendStatus(1, OperationalErrorState, Errors.OperationalReceiveError+(0<<16));
	}

	if ( OperationLoops == 0) // reset state on entering/rentering.
	{
		DebugMsg("Entering Error State");
		char str[LCDCOLUMNS+1];
		lcd_startscroll();
		lcd_setscrolltitle("ERROR State");

		sprintf(str,"Loc:%.2X Code:%.4X", Errors.ErrorPlace, Errors.ErrorReason);
		DebugMsg(str);
		lcd_send_stringscroll(str);

		InverterAllowTorqueAll( false );  // immedietly stop allowing torque request.

		ShutdownCircuitSet( false );

		ClearHVLost();

		ClearCriticalError();

#ifdef PDM
        sendPDM( 0 ); //disable high voltage on error state;
#endif

        CAN_SendTimeBase();

		sprintf(str,"Errorstate: %.4X", 0);
		lcd_send_stringscroll(str);
		// send cause of error state.

		ConfigReset();

		lcd_send_stringscroll(str);

		if ( !CheckShutdown() ) // indicate shutdown switch status with blinking rate.
		{
			lcd_send_stringscroll("Shutdown switches");
		}

//		CAN_NMT( 2, 0x0 ); // send stop command to all nodes.  /// verify that this stops inverters.
		blinkOutput(TSLED,LEDBLINK_FOUR,LEDBLINKNONSTOP);
		blinkOutput(RTDMLED,LEDBLINK_FOUR,LEDBLINKNONSTOP);
		errorstatetime = gettimer();
	}

	struct error_msg error;

	while ( uxQueueMessagesWaiting( ERRORQueue ) )
	{
	//	char str[LCDCOLUMNS+1];
		xQueueReceive(ERRORQueue,&error,0);

		lcd_send_stringscroll(error.msg);
	}

	if ( Errors.InverterError ){
		CAN_SENDINVERTERERRORS();
		lcd_send_stringscroll("Inverter error");
	}

	if ( Errors.ErrorPlace ){
		CAN_SendErrors();
	}

	if (  Shutdown.BMSReason != 0 )
	{
		char statusstr[32];
		sprintf(statusstr, "ERROR State BMS %d", Shutdown.BMSReason );
		lcd_setscrolltitle(statusstr);
	}

#ifdef PDM
	receivePDM();
#endif

	if ( !CheckShutdown() ) // indicate shutdown switch status with blinking rate.
	{
		blinkOutput(TSLED,LEDBLINK_ONE,LEDBLINKNONSTOP);
		blinkOutput(RTDMLED,LEDBLINK_ONE,LEDBLINKNONSTOP);
	} else
	{
		blinkOutput(TSLED,LEDBLINK_FOUR,LEDBLINKNONSTOP);
		blinkOutput(RTDMLED,LEDBLINK_FOUR,LEDBLINKNONSTOP);
	}

#ifdef AUTORESET
	bool allowautoreset = true;
	bool invertererror = false;

	if ( GetInverterState() == INERROR ) invertererror = true;

	for ( int i=0;i<MOTORCOUNT;i++)
	{
		if ( !Errors.InvAllowReset[i] ) allowautoreset = false;
	}
#endif

	int allowreset = 0; // allow reset if this is still 0 after checks.

	char str[80] = "ERROR: ";

	if ( errorstatetime + MS1000*2 > gettimer() ) // ensure error state is seen for at least 2 seconds.
	{
		allowreset += 1;
		strcat(str, "" );
	}

	if ( 0 ) // getPowerErrors() ) // inverter error checked in next step.
	{
//		allowreset += 2;
		strcat(str, "PDM " );
	}

	if ( DeviceState.timeout )
	{
		strcat(str, "NTO " );	
	}

	if ( ! ( DeviceState.ADCSanity == 0 ))
	{
		allowreset +=4;
		strcat(str, "PDL " );

	}

#ifdef SHUTDOWNSWITCHCHECK
    if ( !CheckShutdown() )
    {
    	allowreset +=8;  // only allow exiting error state if shutdown switches closed. - maybe move to only for auto
    	strcat(str, "SDC " );
    }
#endif

    strpad(str,20, true);

    if ( allowreset == 0 )
    {
    	lcd_setscrolltitle("ERROR (Reset OK) ");
    } else
    {
    	lcd_setscrolltitle(str);
    }

	// wait for restart request if allowed by error state.
	if ( allowreset == 0 && ( checkReset() == 1 // manual reset
#ifdef AUTORESET
        		|| ( invertererror  && allowautoreset ) // or automatic reset if allowed inverter error.
#endif
           )
		)
	{
//		setupButtons();
//		setupLEDs();
		//loopcount = 0;
		lcd_endscroll();
		return StartupState;
		// try to perform a full reset back to startup state here on user request.
	}

		// check for restart request. -> pre operation.
  return OperationalErrorState;

  //return FatalErrorState; // only go to fatal error if error deemed unrecoverable.
}


void SetCriticalError( uint8_t err )
{
	criticalerrorset |= ( 1<<err);
	DebugPrintf("Logging critical error %lu", err);
}

uint8_t CheckCriticalError(void)
{
	return criticalerrorset;
}

void ClearCriticalError(void)
{
	criticalerrorset = 0;
}


int initERRORState( void )
{
	ERRORQueue = xQueueCreateStatic( ERRORQUEUE_LENGTH,
							  ERRORITEMSIZE,
							  ERRORQueueStorageArea,
							  &ERRORStaticQueue );

	vQueueAddToRegistry(ERRORQueue, "ERRORQueue" );

	SetErrorLogging( true );

	return 0;
}
