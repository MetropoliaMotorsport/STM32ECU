/*
 * power.c
 *
 *  Created on: Jul 17, 2020
 *      Author: Visa
 */

#include "ecumain.h"

osThreadId_t PowerTaskHandle;
const osThreadAttr_t PowerTask_attributes = {
  .name = "PowerTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128*2
};

#define PowerQUEUE_LENGTH    20
#define PowerITEMSIZE		sizeof( Power_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t PowerStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t PowerQueueStorageArea[ PowerQUEUE_LENGTH * PowerITEMSIZE ];

QueueHandle_t PowerQueue;


// task shall take power hangling request, and forward them to nodes.
// ensure contact is kept with brake light board as brake light is SCS.

// how to ensure power always enabled?

char PNodeWaitStr[20] = "";

char * getPNodeWait( void)
{
	if ( PNodeWaitStr[0] == 0 ) return NULL;
	else return PNodeWaitStr;
}

void PowerTask(void *argument)
{
	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */
	configASSERT( PowerQueue );

	Power_msg msg;

	TickType_t xLastWakeTime;

    const TickType_t xFrequency = 10;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	while( 1 )
	{
		if ( xQueueReceive(PowerQueue,&msg,0) )
		{
#ifdef POWERNODES
		//	int result =
					setNodeDevicePower( msg.power, msg.state );
#else
 // on PDM only fans and HV are controlled.
			if ( device == hv )
			{

			}

#endif
		}


		uint8_t powernodeson = receivePowerNodes();

		if ( powernodeson > 0 )
		{
			DeviceState.PowerNodes = INERROR; // haven't seen all needed.
			strcpy(PNodeWaitStr, getPNodeStr());
		} else
		{
			DeviceState.PowerNodes = OPERATIONAL;
			PNodeWaitStr[0] = 0;
		}

		sendPowerNodeReq(); // process pending power requests.

		vTaskDelayUntil( &xLastWakeTime, xFrequency );

	}

	osThreadTerminate(NULL);
}

int setHV( bool HV, bool buzzer )
{
#ifdef PDM
	return sendPDM( int buzzer );
#else
	bool HVR = true;

#ifdef RTOS
	if ( DeviceState.Inverter < STOPPED ) HVR = false;
	// TODO dobule check this is good enough to
#else
	for ( int i = 0;i<MOTORCOUNT;i++)
	{
		if ( ! CarState.Inverters[i].HighVoltageAllowed) HVR = false;
	} // HVR will be false if any of the inverters are not in true state.
#endif

	if ( ( HVR && HV ) || CarState.TestHV )
	{
// request HV on.
		ShutdownCircuitSet( true );
		setDevicePower( Buzzer, buzzer );
		return 1;
//		return CANSendPDM(10,buzzer); // send PDM message anyway as it's being monitored for HV state in SIM even though has no effect
	} else
	{
		ShutdownCircuitSet( false );
		return 0;
//	    return CANSendPDM(0,buzzer);
	}

	if ( ( ADCState.BrakeF > 5 ) ||      // ensure brake light power turned on if any indication brakes are being pressed.
		 ( ADCState.BrakeR > 5 ) || 		// TODO find minimum values to trigger on.
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
	if ( !CarState.Shutdown.BOTS ) return false;
	if ( !CarState.Shutdown.BSPDAfter ) return false;
	if ( !CarState.Shutdown.BSPDBefore ) return false;
	if ( !CarState.Shutdown.CockpitButton ) return false;
	if ( !CarState.Shutdown.InertiaSwitch ) return false;
	if ( !CarState.Shutdown.LeftButton ) return false;
	if ( !CarState.Shutdown.RightButton ) return false;
	if ( !CarState.Shutdown.BMS ) return false;
	if ( !CarState.Shutdown.IMD ) return false;
#endif
	return true;

}

char * ShutDownOpenStr( void )
{
	static char str[255] = "";

	sprintf(str, "%s%s%s%s%s%s%s%s%s",
		(!CarState.Shutdown.CockpitButton)?"DRV,":"",
		(!CarState.Shutdown.LeftButton)?"LFT,":"",
		(!CarState.Shutdown.RightButton)?"RGT,":"",
		(!CarState.Shutdown.InertiaSwitch)?"INRT,":"",

		(!CarState.Shutdown.BMS)?"BMS,":"",
		(!CarState.Shutdown.IMD)?"IMD,":"",

		(!CarState.Shutdown.BOTS)?"BOTS,":"",
		(!CarState.Shutdown.BSPDAfter)?"BSPDA,":"",
		(!CarState.Shutdown.BSPDBefore)?"BSPDB,":""

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
	return ADC_Data[2] * 1.22; // ~780 count ~= 0.95A ~820=1A   1.22 multiplication factor for approx mA calibrated.
}

int ShutdownCircuitState( void )
{
	return HAL_GPIO_ReadPin(Shutdown_GPIO_Port, Shutdown_Pin);
}


int setDevicePower( DevicePower device, bool state )
{
#ifdef ROS
	Power_msg msg;

	msg.power = device;
	msg.state = state;
	xQueueSend(PowerQueue, msg, 0);
	return 0;
#else
	return setNodeDevicePower( device, state );
#endif
}

int errorPower( void )
{
#ifdef HPF19
	return errorPDM()
#endif

  return 0;
}

void resetPower( void )
{
	CarState.HighVoltageReady = 0;
	setHV( false, false ); // send high voltage off request to PDM.
}


int initPower( void )
{
	RegisterResetCommand(resetPower);

	resetPower();

#ifdef RTOS
	PowerQueue = xQueueCreateStatic( PowerQUEUE_LENGTH,
							  PowerITEMSIZE,
							  PowerQueueStorageArea,
							  &PowerStaticQueue );

	vQueueAddToRegistry(PowerQueue, "PowerQueue" );

	PowerTaskHandle = osThreadNew(PowerTask, NULL, &PowerTask_attributes);
#endif

	return 0;
}

