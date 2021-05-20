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
int8_t InverterStateMachineResponse( InverterState_t *Inverter);


#define INVSTACK_SIZE 128*2
#define INVTASKNAME  "InvTask"
#define INVTASKPRIORITY 3
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

InverterState_t InverterState[MOTORCOUNT];

InverterState_t getInvState(uint8_t inv )
{

	if ( inv >=0 && inv < MOTORCOUNT )
		return InverterState[inv];
	else
	{
		InverterState_t invalidinv = {0};
		return invalidinv;
	}
}

void InverterAllowTorque(uint8_t inv, bool allow )
{
	if ( inv >=0 && inv < MOTORCOUNT )
		InverterState[inv].AllowTorque = allow;
};

void InverterAllowTorqueAll( bool allow )
{
	for ( int i = 0; i<MOTORCOUNT;i++)
	{
		InverterState[i].AllowTorque = allow;
	}
}

void InverterSetTorque( vectoradjust *adj, int16_t MaxSpeed )
{
	xSemaphoreTake(InvUpdating, portMAX_DELAY);
	InverterState[RearLeftInverter].Torque_Req = adj->RL;
	InverterState[FrontLeftInverter].Torque_Req = adj->FL;
	InverterState[RearRightInverter].Torque_Req = adj->RR;
	InverterState[FrontRightInverter].Torque_Req = adj->FR;
	//InverterState[i].MaxSpeed = MaxSpeed; // convert to right value as needed.
	xSemaphoreGive(InvUpdating);
}

void InverterSetTorqueInd( uint8_t inv, int16_t req, int16_t speed )
{
	xSemaphoreTake(InvUpdating, portMAX_DELAY);
	InverterState[inv].Torque_Req = req;
	InverterState[inv].MaxSpeed = speed; // convert to right value as needed.
	xSemaphoreGive(InvUpdating);
}

int InverterGetSpeed( void )
{
	// TODO implement properly.
	int32_t speed = 0;

	// find slowest wheel to define speed.
	for ( int i = 0; i<MOTORCOUNT;i++)
	{
		if (InverterState[i].Speed < speed )
			speed =InverterState[i].Speed;
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
		uint32_t invexpectedbits = (0b111 << (i * 3));
		invexpected += invexpectedbits; // three message flags per motor, status, vals1, vals2
	}

	uint32_t InvReceived = 0;
	xTaskNotifyWait( pdFALSE,    /* Don't clear bits on entry. */
					   ULONG_MAX,        /* Clear all bits on exit. */
					   &InvReceived, /* Stores the notified value. */
					   0 );

	vTaskDelay(100); // allow a bit of time at startup to listen for bus activity responding to sync.

	volatile int count = 0;

	while ( InvReceived == invexpected || count > 10 )
	{
		xTaskNotifyWait( pdFALSE,    /* Don't clear bits on entry. */
						   ULONG_MAX,        /* Clear all bits on exit. */
						   &InvReceived, /* Stores the notified value. */
						   0 );
		count++;
	}

//	if ( InvReceived == invexpected ) // 4 inverters
	{
	#ifdef LENZE
		for ( int i=0;i<MOTORCOUNT;i++)
		{
			InvStartupCfg( &InverterState[i] );
		}
	#endif
	}

	while( 1 )
	{
		// TODO change to a single element queue.
		if ( xQueueReceive(InvQueue,&msg,0) ) // queue to receive requested operational state.
		{
			for ( int i=0;i<MOTORCOUNT;i++)
			{
				InverterState[i].InvRequested = msg.state;
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
		DeviceStatus highest=INERROR;

		// TODO determine when to run inverter config on first detection. - done by detecting APPC traffic in canbus response.

		// quick bodge to allow operation on test bench for now..
		CarState.VoltageINV = 130;

//		if ( !invertersinerror )
		for ( int i=0;i<MOTORCOUNT;i++) // speed is received
		{
			// only process inverter state if inverters have been seen and not in error state.
			if (InverterState[i].InvStateAct != OFFLINE && InverterStates[i] != OFFLINE )
			{
			// run the state machine and get command to match current situation.
				command = InverterStateMachineResponse( &InverterState[i] );

				// maybe store highest too so that operation can continue with only some operating motors if necessary?
				if (InverterState[i].InvStateAct < lowest ) lowest = InverterState[i].InvStateAct;
				if (InverterState[i].InvStateAct > highest ) highest = InverterState[i].InvStateAct;

				// only change command if we're not in wanted state to try and transition towards it.
				if (InverterState[i].InvStateAct != InverterState[i].InvRequested && InverterState[i].InvStateAct > INERROR )
				{
					// check if we've got voltage available for moving up states, otherwise stay up.
			//		if ( InverterState[i-InverterState[i].MCChannel].HighVoltageAvailable > 40 )
					{
						InverterState[i].InvCommand = command;
					}
				}
			}
			// store lowest known inverter state as global state, or error if not in operational state.

			Inverter = lowest;
		}

		// ensure requests aren't modified mid send.
		xSemaphoreTake(InvUpdating, portMAX_DELAY);

		bool allowReset = true;

		// process actual request if inverters are online.
		for ( int i=0;i<MOTORCOUNT;i++)
		{
			// initial testing, use maximum possible error reset period.
			if ( InverterState[i].InvStateAct == INERROR )
			{
				if ( allowReset && xTaskGetTickCount() - InverterState[i].errortime > ERRORTYPE1RESET )
				{
					char str[40];
					snprintf(str, 40, "Inverter Reset sent to Inv[%d]", i);
					InvResetError(&InverterState[i]);
					InverterState[i].errortime = xTaskGetTickCount();
					InverterState[i].InvRequested = BOOTUP;
					InverterState[i].InvStateAct = OFFLINE;
					InverterState[i].InvCommand =
					DebugMsg(str);
				} else
				{
					// send do nothing;
					InvSend( &InverterState[i], 0, 0);
				}
			} else
				// but only send an actual torque request if both car and inverter state allow it.
			if ( InverterState[i].AllowTorque && InverterState[i].InvStateAct == OPERATIONAL )
			{
				// only allow a negative torque if Regen is allowed.
				if ( !allowregen && InverterState[i].Torque_Req < 0 )
					InvSend( &InverterState[i], InverterState[i].MaxSpeed, 0);
				else
					InvSend( &InverterState[i], InverterState[i].MaxSpeed,InverterState[i].Torque_Req );
			} else // otherwise just send state request.
			{
				InvSend( &InverterState[i], 0, 0 );
			}
			vTaskDelay(1); // give a bit of time to clear buffer.

		}
		xSemaphoreGive(InvUpdating);

		setWatchdogBit(watchdogBit);
		// only allow one command per cycle. Switch to syncing with main task to not go out of sync?
		vTaskDelayUntil( &xLastWakeTime, CYCLETIME );
	}

	// clear up if somehow get here.
	vTaskDelete(NULL);
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


int8_t InverterStateMachineResponse( InverterState_t *Inverter ) // returns response to send inverter based on current state.
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
			//TXState = 0b10000000; // 128
			TXState = 0b00000000; // 0 don't transmit any command for error, deal with it seperately.
			break;
		}

	//  offset 0 length 32: power

	Inverter->HighVoltageAllowed = true;// HighVoltageAllowed;
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
		InverterState[i].InvStateAct = OFFLINE;
		InverterState[i].InvCommand = 0x80;
#ifdef SIEMENS
		InverterState[i].InvStateCheck = 0xFF;
		InverterState[i].InvStateCheck3 = 0xFF;
		InverterState[i].InvBadStatus = 1;
#endif
		InverterState[i].Torque_Req = 0;
		InverterState[i].Speed = 0;
		InverterState[i].HighVoltageAllowed = false;
		InverterState[i].InverterNum = i;
		InverterState[i].MCChannel = false;
		InverterState[i].InvRequested = BOOTUP;

//		InverterStates[i] = OFFLINE;

		Errors.InvAllowReset[i] = 1;
	}

	InverterState[0].COBID = InverterRL_COBID;
	InverterState[1].COBID = InverterRR_COBID;

//	InverterState[0].MCChannel = InverterRL_Channel;
	InverterState[1].MCChannel = true;;

#if MOTORCOUNT > 2
	InverterState[2].COBID = InverterFL_COBID;
	InverterState[3].COBID = InverterFR_COBID;

//	InverterState[2].MCChannel = InverterFL_Channel;
//	InverterState[3].MCChannel = InverterFR_Channel;
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
