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

#ifdef SIEMENS
	#include "siemensinverter.h"
#endif
#ifdef LENZE
	#include "lenzeinverter.h"
#endif

DeviceStatus GetInverterState( void );
int8_t InverterStateMachineResponse( volatile InverterState *Inverter);


#define INVSTACK_SIZE 128*2
#define INVTASKNAME  "DebugTask"
#define INVTASKPRIORITY 1
StaticTask_t xINVTaskBuffer;
StackType_t xINVStack[ INVSTACK_SIZE ];

TaskHandle_t InvTaskHandle;

#define InvQUEUE_LENGTH    20
#define InvITEMSIZE		   sizeof( Inv_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t InvStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t InvQueueStorageArea[ InvQUEUE_LENGTH * InvITEMSIZE ];

QueueHandle_t InvQueue;

SemaphoreHandle_t InvUpdating;

struct invState invState;

InverterState getInvState(uint8_t inv )
{

	if ( inv >=0 && inv < MOTORCOUNT )
		return invState.Inverter[inv];
	else
	{
		InverterState invalidinv = {0};
		return invalidinv;
	}
}

void InverterAllowTorque( bool allow )
{
	invState.AllowTorque = allow;
};

void InverterSetTorque( vectoradjust *adj, int16_t MaxSpeed )
{
	xSemaphoreTake(InvUpdating, portMAX_DELAY);
	invState.Inverter[RearLeftInverter].Torque_Req = adj->RL;
	invState.Inverter[FrontLeftInverter].Torque_Req = adj->FL;
	invState.Inverter[RearRightInverter].Torque_Req = adj->RR;
	invState.Inverter[FrontRightInverter].Torque_Req = adj->FR;
	invState.maxSpeed = MaxSpeed; // convert to right value as needed.
	xSemaphoreGive(InvUpdating);
}

int InverterGetSpeed( void )
{
	// TODO implement properly.
	int32_t speed = 0;

	// find slowest wheel to define speed.
	for ( int i = 0; i<MOTORCOUNT;i++)
	{
		if (invState.Inverter[i].Speed < speed )
			speed = invState.Inverter[i].Speed;
	}

	return speed;
}


// task shall take power handling request, and forward them to nodes.
// ensure contact is kept with brake light board as brake light is SCS.

// how to ensure power always enabled?

volatile bool invertersinerror = false;

//DeviceStatus RequestedState[MOTORCOUNT];
uint16_t command;


bool checkStatusCode( uint8_t status )
{
	switch ( status )
	{
		case 49 : // ready to switch on.
		case 51 : // on
		case 55 : // operation
		case 64 : // startup
		case 96 : //
		case 104 : // error
		case 200 : // very error // c0   c8     192-200 errors.
			return true;
			break;
		default :
			return false;
	}
}


void StopMotors( void )
{


}

DeviceStatus Inverter;
DeviceStatus InverterStates[MOTORCOUNT];

// task to manage inverter state.
void InvTask(void *argument)
{
	xEventGroupSync( xStartupSync, 0, 1, portMAX_DELAY ); // ensure that tasks don't start before all initialisation done.

	Inv_msg msg;

	bool reseterror = false;

	bool allowregen = false;

	TickType_t xLastWakeTime;

	Inverter = OFFLINE;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	uint8_t watchdogBit = registerWatchdogBit("InvTask");

	uint32_t invexpected = 0;

	// TODO move to inverter file.
	//calculate the inverter expected messages to match number of motors,
	for ( int i=0;i<MOTORCOUNT;i++)
	{
		invexpected += (0x111 << i *3); // three message flags per motor, status,
	}

	while( 1 )
	{
		// TODO change to a single element queue.
		if ( xQueueReceive(InvQueue,&msg,0) ) // queue to receive requested operational state.
		{
			for ( int i=0;i<MOTORCOUNT;i++)
			{
				invState.Inverter[i].InvRequested = msg.state;
			}

		}

		uint32_t InvReceived = 0;
		xTaskNotifyWait( pdFALSE,    /* Don't clear bits on entry. */
						   ULONG_MAX,        /* Clear all bits on exit. */
						   &InvReceived, /* Stores the notified value. */
						   0 );

		// TODO improve.

		if ( InvReceived == invexpected ) // 4 inverters
		{
			Inverter = OPERATIONAL;
		} else
		{
			Inverter = INERROR; // haven't seen all needed.
		}

		DeviceStatus lowest=OPERATIONAL;
		DeviceStatus highest=OFFLINE;

		// TODO determine when to run inverter config on first detection. - done by detecting APPC traffic in canbus response.

		if ( !invertersinerror )
		for ( int i=0;i<MOTORCOUNT;i++) // speed is received
		{
			// only process inverter state if inverters have been seen and not in error state.
			if ( invState.Inverter[i].InvStateAct != OFFLINE && InverterStates[i] != OFFLINE )
			{
			// run the state machine and get command to match current situation.
				command = InverterStateMachineResponse( &invState.Inverter[i] );

				// maybe store highest too so that operation can continue with only some operating motors if necessary?
				if ( invState.Inverter[i].InvStateAct < lowest ) lowest = invState.Inverter[i].InvStateAct;
				if ( invState.Inverter[i].InvStateAct > highest ) highest = invState.Inverter[i].InvStateAct;

				// only change command if we're not in wanted state to try and transition towards it.
				if ( invState.Inverter[i].InvStateAct != invState.Inverter[i].InvRequested )
				{
#ifdef IVTEnable // Only allow transitions to states requesting HV if it's available, and allowed?.
					if ( ( invState.Inverter[i].InvRequested > STOPPED && CarState.VoltageINV > 480 && invState.Inverter[i].HighVoltageAllowed) || invState.Inverter[i].InvRequested <= STOPPED )
#endif
					{
						//InvSend( &invState.Inverter[i], command, 0, 0 );
						invState.Inverter[i].InvCommand = command;
					}
				}
			} else if ( lowest != INERROR ) lowest = OFFLINE;

			// store lowest known inverter state as global state, or error if not in operational state.

			Inverter = lowest;

		} else
		{
			Inverter = INERROR; // deal with clearing error if possible here.

			reseterror = true; // for now just try to reset error if there is one regardless.
			// TODO add proper error checking, automatically reset on some errors like PDO timeout.

			if ( reseterror )
			{
				for ( int i=0;i<MOTORCOUNT;i++)
				{
					InvResetError(&invState.Inverter[i]);
					// TODO set inverter command for error state?
				}
				reseterror = false;
			}

		}

		// ensure the torque requests aren't modified mid send.
		xSemaphoreTake(InvUpdating, portMAX_DELAY);

		// process actual request if inverters are online.
		if ( Inverter > OFFLINE )
		{
			for ( int i=0;i<MOTORCOUNT;i++)
			{
				// but only send an actual torque request if both car and inverter state allow it.
				if ( invState.AllowTorque && Inverter == OPERATIONAL )
				{
					// only allow a negative torque if regen is allowed.
					if ( !allowregen && invState.Inverter[i].Torque_Req < 0 )
						InvSend( &invState.Inverter[i], invState.maxSpeed, 0);
					else
						InvSend( &invState.Inverter[i], invState.maxSpeed, invState.Inverter[i].Torque_Req );
				} else
				{
					InvSend( &invState.Inverter[i], 0, 0 );
				}
			}
		}
		xSemaphoreGive(InvUpdating);

		setWatchdogBit(watchdogBit);
		vTaskDelayUntil( &xLastWakeTime, CYCLETIME ); // only allow one command per cycle
	}

	vTaskDelete(NULL);
}



int8_t InitInverterData( void )
{
	for ( int i=0;i<MOTORCOUNT;i++)
	{

	}
	return 0;
}


	// fail process, inverters go from 31->33h->60h->68h  when no HV supplied and request startup.
// states 3->1 ( stop )->-99 ( error )

DeviceStatus InternalInverterState ( uint16_t Status ) // status 104, failed to turn on HV 200, failure of encoders/temp
{
	// establish current state machine position from return status.
	if ( ( Status & 0b01001111 ) == 0b01000000) // 64
	{ // Switch on disabled
		return BOOTUP; //1;
	}
	else if ( ( Status & 0b01101111 ) == 0b00100001 ) // 49
	{ // Ready to switch on
		return STOPPED;//2;
	}
	else if ( ( Status & 0b01101111 ) == 0b00100011 ) // 51
	{ // Switched on. HV?
		return PREOPERATIONAL;//3;
	}
	else if ( ( Status & 0b01101111 ) == 0b00100111 ) // 55
	{ // Operation enabled.
		return OPERATIONAL;//4;
	}
	else if ( ( ( Status & 0b01101111 ) == 0b00000111 )
			 || ( ( Status & 0b00011111 ) == 0b00010011 ) )
	{ // Quick Stop Active
		return INERROR; // -1;
	}
	else if  ( ( ( Status & 0b01001111 ) == 0b00001111 )
			 || ( ( Status & 0b01001111 ) == 0b00001001 ) )
	{ // fault reaction active, will move to fault status next
		return INERROR; // -2;
	}
	else if  ( ( ( Status & 0b01001111 ) == 0b00001000 )
			 || ( ( Status & 0b00001000 ) == 0b00001000 ) )
	{ // fault status
		return INERROR;//-99;
		// send reset
	} else
	{ // unknown state
		return 0; // state 0 will request reset to enter State 1,
		// will fall here at start of loop and if unknown status.
	}
}


DeviceStatus GetInverterState( void )
{
	return Inverter;
}

bool invertersStateCheck( DeviceStatus state )
{
	if ( Inverter == state ) return true;
	else return false;
}


int8_t InverterStateMachineResponse( volatile InverterState *Inverter ) // returns response to send inverter based on current state.
{
	uint16_t TXState;

	DeviceStatus State;

	bool HighVoltageAllowed;//, ReadyToDriveAllowed; //, TsLED, RtdmLED;

	State = Inverter->InvStateAct;
	HighVoltageAllowed = Inverter->HighVoltageAllowed;

	// first check for fault status, and issue reset.

	TXState = 0; // default  do nothing state.
	// process regular state machine sequence
	switch ( State )
	{
		case OFFLINE : // state 0: Not ready to switch on, no can message. Internal state only at startup.
			HighVoltageAllowed = false;  // High Voltage is not allowed
			TXState=0b10000000; // send bit 128 reset message to enter state 1 in case in fault. - fault reset.
			break;

		case BOOTUP : // State 1: Switch on Disabled.
			HighVoltageAllowed = false;
			TXState = 0b00000110; // send 0110 shutdown message to request move to State 2.
			break;

		case STOPPED : // State 2: Ready to switch on
			 // We are ready to turn on, so allow high voltage.
			// we are in state 2, process.
			// process shutdown request here, to move to move to state 1.
			if ( CarState.HighVoltageReady )
			{  // TS enable button pressed and both inverters are marked HV ready proceed to state 3.
				HighVoltageAllowed = true;
#ifdef LENZE
				TXState = 0b00001111; // Lenze doesn't want to go to pre operational from stopped, have to skip straight to operational.
#else
				TXState = 0b00000111; // request Switch on message, State 3..
#endif
			} else
			{
				HighVoltageAllowed = false;
				TXState = 0b00000110; // no change, continue to request State 2.
			}
			break;

		case PREOPERATIONAL : // State 3: Switched on   <---- check this case.
			  // we are powered on, so allow high voltage.
			if ( CarState.HighVoltageReady )// IdleState ) <-
			{  // TS enable button has been pressed, proceed to request power on if all inverters on.
				HighVoltageAllowed = true;
				TXState = 0b00001111; // Request Enable operation, State 4.
			}
			else if ( !CarState.HighVoltageReady )
			{ // return to switched on state.
				HighVoltageAllowed = false;
				TXState = 0b00000110; // 0b00000000; // request Disable Voltage, drop to ready state., alternately Quick Stop 0b00000010
			}
			else
			{  // no change, continue to request State 3.
				TXState = 0b00000111;
			}
			break;

		case OPERATIONAL : // State 4: Operation Enable
			 // we are powered on, so allow high voltage.
	/*		if ( CarState.HighVoltageReady ) //  && OperationalState == TSActiveState)
			{ // no longer in RTDM mode, but still got HV, so drop to idle.
				HighVoltageAllowed = 1;
				TXState = 0b00000111; // request state 3: Switched on.
			}
			else */
			if ( !CarState.HighVoltageReady )
			{ // full motor stop has been requested
				HighVoltageAllowed = false; // drop back to ready to switch on.
				TXState = 0b00000110;//0b00000000; // request Disable Voltage., alternately Quick Stop 0b00000010 - test to see if any difference in behaviour.
			}
			else
			{ // no change, continue to request operation.
				TXState = 0b00001111;
				HighVoltageAllowed = true;
			}
			break;

	//	case -1 : //5 Quick Stop Active - Fall through to default to reset state.

	//	case -2 : //98 Fault Reason Active

	//	case -99 : //99 Fault

		case INERROR:
		default : // unknown identifier encountered, ignore. Shouldn't be possible to get here due to filters.
			HighVoltageAllowed = false;
			TXState = 0b10000000; // 128
			//TXState = 0b00000000; // 0
			break;
		}

	//  offset 0 length 32: power

	Inverter->HighVoltageAllowed = HighVoltageAllowed;
	return TXState;
}


long getInvSpeedValue( uint8_t *data )
{
	//		 Speed_Right_Inverter.data.longint * (1/4194304) * 60; - convert to rpm.
	return getLEint32(&data[2]) * ( 1.0/4194304 ) * 60;
}


uint8_t invRequestState( DeviceStatus state )
{
	if ( Inverter != state )
	{
		Inv_msg msg;
		msg.state  = state;
		xQueueSend(InvQueue, &msg, 0);
		return 0;
	} else return 1; // this is operating with cansync, no extra needed.
}



void resetInv( void )
{
	for ( int i=0;i<MOTORCOUNT; i++)
	{
//		invState.Inverter[i].InvStateVal = 0xFF;
		invState.Inverter[i].InvStateAct = OFFLINE;
#ifdef SIEMENS
		invState.Inverter[i].InvStateCheck = 0xFF;
		invState.Inverter[i].InvStateCheck3 = 0xFF;
		invState.Inverter[i].InvBadStatus = 1;
#endif
		invState.Inverter[i].Torque_Req = 0;
		invState.Inverter[i].Speed = 0;
		invState.Inverter[i].HighVoltageAllowed = false;
		invState.Inverter[i].InverterNum = i;
		invState.Inverter[i].MCChannel = false;
		invState.Inverter[i].InvRequested = BOOTUP;

//		InverterStates[i] = OFFLINE;

		Errors.InvAllowReset[i] = 1;
	}

	invState.Inverter[0].COBID = InverterRL_COBID;
	invState.Inverter[1].COBID = InverterRR_COBID;

//	invState.Inverter[0].MCChannel = InverterRL_Channel;
//	invState.Inverter[1].MCChannel = InverterRR_Channel;

#if MOTORCOUNT > 2
	invState.Inverter[2].COBID = InverterFL_COBID;
	invState.Inverter[3].COBID = InverterFR_COBID;

//	invState.Inverter[2].MCChannel = InverterFL_Channel;
//	invState.Inverter[3].MCChannel = InverterFR_Channel;
#endif

	Errors.InverterError = 0; // reset logged errors.
}


int initInv( void )
{
	resetInv();

	RegisterResetCommand(resetInv);


	registerInverterCAN();

	InvUpdating = xSemaphoreCreateMutex();

	InvQueue = xQueueCreateStatic( InvQUEUE_LENGTH,
							  InvITEMSIZE,
							  InvQueueStorageArea,
							  &InvStaticQueue );

	vQueueAddToRegistry(InvQueue, "InverterQueue" );

	InvTaskHandle = xTaskCreateStatic(
						  InvTask,
						  INVTASKNAME,
						  INVSTACK_SIZE,
						  ( void * ) 1,
						  INVTASKPRIORITY,
						  xINVStack,
						  &xINVTaskBuffer );

  return 0;
}
