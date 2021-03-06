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

TaskHandle_t PowerTaskHandle = NULL;

#define POWERSTACK_SIZE 128*6
#define POWERTASKNAME  "PowerTask"
StaticTask_t xPOWERTaskBuffer;
StackType_t xPOWERStack[ POWERSTACK_SIZE ];


#define PowerQUEUE_LENGTH    20
#define PowerErrorQUEUE_LENGTH    20
#define PowerITEMSIZE		sizeof( Power_msg )
#define PowerErrorITEMSIZE		sizeof( Power_Error_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t PowerStaticQueue;
static StaticQueue_t PowerErrorStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t PowerQueueStorageArea[ PowerQUEUE_LENGTH * PowerITEMSIZE ];
uint8_t PowerErrorQueueStorageArea[ PowerErrorQUEUE_LENGTH * PowerErrorITEMSIZE ];

QueueHandle_t PowerQueue, PowerErrorQueue;

ShutdownState Shutdown;

// task shall take power handling request, and forward them to nodes.
// ensure contact is kept with brake light board as brake light is SCS.

// how to ensure power always enabled?

char PNodeWaitStr[20] = "";

char * getPNodeWait( void)
{
	return PNodeWaitStr;
	// TODO add a mutex
}

void PowerTask(void *argument)
{
	xEventGroupSync( xStartupSync, 0, 1, portMAX_DELAY );

	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT( PowerQueue );

	Power_msg msg;
	Power_Error_msg errormsg;

	char str[MAXERROROUTPUT];

	bool resetLV = false;

	resetPowerLost();
	xQueueReset(PowerErrorQueue);

	uint32_t powernodesOnline = 0;
	uint32_t lastpowernodesOnline = 0;
	uint32_t count = 0;

	uint32_t lastseenall = 0;

	while( 1 )
	{
		lastpowernodesOnline = powernodesOnline;
		// read the values from last cycle.
		xTaskNotifyWait( pdFALSE, ULONG_MAX, &powernodesOnline, 0 );

		// block and wait for main cycle.
		xEventGroupSync( xCycleSync, 0, 1, portMAX_DELAY );

		count++;
		while ( xQueueReceive(PowerErrorQueue,&errormsg,0) )
		{
			if ( errormsg.error == 0xFFFF )
			{
				LogError("Power err: LV Down");
				resetLV = true;

				setAllPowerActualOff();
			} else
			{
				snprintf(str, MAXERROROUTPUT, "Power err: %d %lu %s", errormsg.nodeid, errormsg.error, PNodeGetErrStr( errormsg.error ));
				LogError( str );
			}
		}

		if ( resetLV )
			resetPowerLost();

		// clear command queue
		while ( xQueueReceive(PowerQueue,&msg,0) )
		{
#ifdef POWERNODES
			switch ( msg.cmd )
			{
			case PowerErrorReset :
				setNodeDevicePower( msg.power, msg.enabled, true );
				break;

			case PowerError : break;

			case DirectPowerCmd :
				setNodeDevicePower( msg.power, msg.enabled, false );
				break;
			case FanPowerCmd :
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

		if ( powernodesOnline == PNodeAllBit ) // all expected power nodes reported in. // TODO automate
		{
			DeviceState.PowerNodes = OPERATIONAL;
			PNodeWaitStr[0] = 0;
			lastseenall = gettimer();
		} else
		{
			// allow some timeout before declaring error, but only after we've actually already got online.

			if ( gettimer() - lastseenall > PDMTIMEOUT && DeviceState.PowerNodes > OFFLINE)
			{
				DeviceState.PowerNodes = INERROR; // haven't seen all needed.
				setPowerNodeStr( powernodesOnline );
				strcpy(PNodeWaitStr, getPNodeStr());
				DebugMsg("Power node timeout");
			}

			// brake powernode should be a priority as priority signal.
		}

		if ( lastpowernodesOnline != powernodesOnline )
		{
			char str[40];
			snprintf(str, 40, "Powernodes diff %lu", count);
	//		DebugMsg(str);
		}

		sendPowerNodeReq(); // process pending power requests.
	}

	vTaskDelete(NULL);
}

int setRunningPower( bool HV, bool buzzer )
{
#ifdef PDM
	return sendPDM( int buzzer );
#else
	bool HVR = true;

	if ( GetInverterState() < STOPPED ) HVR = false;

	if ( ( HVR && HV ) )
	{
// request HV on.
	//	ShutdownCircuitSet( true );
		setDevicePower( Buzzer, buzzer );
		return 1;
//		return CANSendPDM(10,buzzer); // send PDM message anyway as it's being monitored for HV state in SIM even though has no effect
	} else
	{
	//	ShutdownCircuitSet( false );
		return 0;
//	    return CANSendPDM(0,buzzer);
	}

	if ( ( ADCState.BrakeF > 5 ) ||      // ensure brake light power turned on if any indication brakes are being pressed.
		 ( ADCState.BrakeR > 5 ) || 		// TODO find suitable minimum values to trigger on.
		 ( ADCState.Regen_Percent > 5) )
	{
		setDevicePower( Brake,  true);
	} else
	{
		setDevicePower( Brake,  false);
	};
#endif
}

bool CheckShutdown( void ) // returns true if shutdown circuit other than ECU is closed
{

#ifdef HPF20
	if ( !Shutdown.BOTS ) return false;
	if ( !Shutdown.BSPDAfter ) return false;
	if ( !Shutdown.BSPDBefore ) return false;
	if ( !Shutdown.CockpitButton ) return false;
	if ( !Shutdown.InertiaSwitch ) return false;
	if ( !Shutdown.LeftButton ) return false;
	if ( !Shutdown.RightButton ) return false;
	if ( !Shutdown.BMS ) return false;
	if ( !Shutdown.IMD ) return false;
#endif
	return true;

}

char * ShutDownOpenStr( void )
{
	static char str[255] = "";

	// TODO move these into actual order of SDC.

	sprintf(str, "%s%s%s%s%s%s%s%s%s",
		(!Shutdown.CockpitButton)?"DRV,":"",
		(!Shutdown.LeftButton)?"LFT,":"",
		(!Shutdown.RightButton)?"RGT,":"",
		(!Shutdown.InertiaSwitch)?"INRT,":"",

		(!Shutdown.BMS)?"BMS,":"",
		(!Shutdown.IMD)?"IMD,":"",

		(!Shutdown.BOTS)?"BOTS,":"",
		(!Shutdown.BSPDAfter)?"BSPDA,":"",
		(!Shutdown.BSPDBefore)?"BSPDB,":""

	);

	int len=strlen(str);

	if ( len > 0)
		str[len-1] = 0;

	return str;
}


void ShutdownCircuitSet( bool state )
{
	HAL_GPIO_WritePin( Shutdown_GPIO_Port, Shutdown_Pin, state);
}

int ShutdownCircuitCurrent( void )
{
	// check if ADC ok
#ifdef STMADC
	return ADC_Data[2] * 1.22; // ~780 count ~= 0.95A ~820=1A   1.22 multiplication factor for approx mA calibrated.
#else
	return 0;
#endif
}

int ShutdownCircuitState( void )
{
	return HAL_GPIO_ReadPin(Shutdown_GPIO_Port, Shutdown_Pin);
}

// request a power state for a device.
bool setDevicePower( DevicePower device, bool state )
{
	Power_msg msg;

	msg.cmd = DirectPowerCmd;
	msg.power = device;
	msg.enabled = state;
	return ( xQueueSend(PowerQueue, &msg, 0) );

}

bool getDevicePower( DevicePower device)
{
	return getNodeDevicePower( device );
}

// reset a device's power channel
bool resetDevicePower( DevicePower device )
{
	Power_msg msg;

	msg.cmd = PowerErrorReset;
	msg.power = device;
	return ( xQueueSend(PowerQueue, &msg, 0) );

}

// directly set the current power state of device.
bool setPowerState( DevicePowerState devicestate, bool enabled )
{
	Power_msg msg;

	if ( devicestate != DirectPowerCmd && devicestate != PowerError)
	{
		msg.cmd = devicestate;
		msg.enabled = enabled;
		return xQueueSend(PowerQueue, &msg, 0);
	}
	return false;
}

void resetPower( void )
{
	setRunningPower( false, false ); // send high voltage off request to PDM.
}

void FanPWMControl( uint8_t leftduty, uint8_t rightduty )
{
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



char * getDevicePowerNameLong( DevicePower device )
{
	switch ( device )
	{
	case None : return "None";
	case Buzzer: return "Buzzer";
	case Telemetry: return "Telemetry";
	case Front1: return "Front1";
	case Inverters: return "Inverters";
	case ECU: return "ECU";
	case Front2: return "Front2";
	case LeftFans: return "LeftFans";
	case RightFans: return "RightFans";
	case LeftPump: return "LeftPump";
	case RightPump: return "RightPump";
	case IVT: return "IVT";
	case Current: return "Current";
	case TSAL: return "TSAL";
	case Brake: return "Brake";
	case Accu: return "Accu";
	case AccuFan: return "AccuFan";
	case Back1: return "Back1";
	}
	return "Error";
}


bool getPowerHVReady( void )
{
	return true; // TODO implement rather than dummy.
}

bool PowerLogError( uint8_t nodeid, uint32_t errorcode)
{
	Power_Error_msg msg;

	msg.nodeid = nodeid;
	msg.error = errorcode;

	if ( xPortIsInsideInterrupt() )
		return xQueueSendFromISR(PowerErrorQueue, &msg, NULL );
	else
		return ( xQueueSend(PowerErrorQueue, &msg, 0) );
}

int initPower( void )
{
	RegisterResetCommand(resetPower);

	resetPower();

	PowerQueue = xQueueCreateStatic( PowerQUEUE_LENGTH,
							  PowerITEMSIZE,
							  PowerQueueStorageArea,
							  &PowerStaticQueue );

	vQueueAddToRegistry(PowerQueue, "PowerQueue" );

	PowerErrorQueue = xQueueCreateStatic( PowerErrorQUEUE_LENGTH,
							  PowerErrorITEMSIZE,
							  PowerErrorQueueStorageArea,
							  &PowerErrorStaticQueue );

	vQueueAddToRegistry(PowerErrorQueue, "PowerErrorQueue" );


	PowerTaskHandle = xTaskCreateStatic(
						  PowerTask,
						  POWERTASKNAME,
						  POWERSTACK_SIZE,
						  ( void * ) 1,
						  POWERTASKPRIORITY,
						  xPOWERStack,
						  &xPOWERTaskBuffer );

	initPowerLossHandling();

	return 0;
}

