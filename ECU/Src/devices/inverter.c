/*
 * inverter.c
 *
 *  Created on: 23 Mar 2021
 *      Author: Visa
 */

// 1041 ( 411h )  52 1 0 0 -< turn on

// 1041 ( 411h )  49 0 175 0<- trigger message

#include "ecumain.h"

#ifdef SIEMENS
	#include "siemensinverter.h"
#endif
#ifdef LENZE
	#include "lenzeinverter.h"
#endif


#ifdef RTOS
DeviceStatus GetInverterState( uint16_t Status );
int8_t InverterStateMachine( volatile InverterState *Inverter);
#endif


osThreadId_t InvTaskHandle;
const osThreadAttr_t InvTask_attributes = {
  .name = "InvTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128*2
};

#define InvQUEUE_LENGTH    20
#define InvITEMSIZE		   sizeof( Inv_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t InvStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t InvQueueStorageArea[ InvQUEUE_LENGTH * InvITEMSIZE ];

QueueHandle_t InvQueue;


// task shall take power hangling request, and forward them to nodes.
// ensure contact is kept with brake light board as brake light is SCS.

// how to ensure power always enabled?

volatile bool invertersinerror = false;

DeviceStatus RequestedState=BOOTUP;
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
	for ( int i=0;i<MOTORCOUNT;i++){
  //      CANSendInverter( 0b00000110, 0, i );  // request inverters go to non operational state before cutting power.
	}
}


// task to manage inverter state.
void InvTask(void *argument)
{
	Inv_msg msg;

	TickType_t xLastWakeTime;

	DeviceState.Inverter = OFFLINE;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	while( 1 )
	{
		if ( xQueueReceive(InvQueue,&msg,0) ) // queue to receive requested operational state.
		{
			// check if allowable state.
			RequestedState = msg.state;
		}

		// check for data received.
		for ( int i=0;i<MOTORCOUNT;i++)
		{
			receiveINVStatus(&CarState.Inverters[i]);
			receiveINVSpeed(&CarState.Inverters[i]);
			receiveINVTorque(&CarState.Inverters[i]);
		}

		DeviceStatus lowest=OPERATIONAL;
		DeviceStatus highest=OFFLINE;

		if ( !invertersinerror )
		for ( int i=0;i<MOTORCOUNT;i++) // speed is received
		{
			// only process inverter state if inverters have been seen and not in error state.
			if ( CarState.Inverters[i].InvState != 0xFF && DeviceState.Inverters[i] != OFFLINE )
			{
			// run the state machine and get command to match current situation.
				command = InverterStateMachine( &CarState.Inverters[i] );

				// maybe store highest too so that operation can continue with only some operating motors if necessary?
				if ( GetInverterState( CarState.Inverters[i].InvState ) < lowest ) lowest = GetInverterState( CarState.Inverters[i].InvState );
				if ( GetInverterState( CarState.Inverters[i].InvState ) > highest ) highest = GetInverterState( CarState.Inverters[i].InvState );

				// only send command if we're not in wanted state to try and transition towards it.
				if ( GetInverterState( CarState.Inverters[i].InvState ) != RequestedState )
				{
#ifdef IVTEnable // Only allow transitions to states requesting HV if it's available, and allowed?.
					if ( ( RequestedState > STOPPED && CarState.VoltageINV > 480 && CarState.Inverters[i].HighVoltageAllowed) || RequestedState <= STOPPED )
#endif
					{
	//					CANSendInverter( command, 0, i );
					}
				}
			} else if ( lowest != INERROR ) lowest = OFFLINE;
			DeviceState.Inverter = lowest;

		} else
		{
			DeviceState.Inverter = INERROR;
		}

		// store lowest known inverter state as global state.


		vTaskDelayUntil( &xLastWakeTime, CYCLETIME ); // only allow one command per cycle
	}

	osThreadTerminate(NULL);
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

DeviceStatus GetInverterState ( uint16_t Status ) // status 104, failed to turn on HV 200, failure of encoders/temp
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


bool invertersStateCheck( DeviceStatus state )
{
#ifdef RTOS
	if ( DeviceState.Inverter ==  state ) return true;
	else return false;
#else
	bool requestedstate = true;
	for ( int i=0;i<MOTORCOUNT;i++){
		 if ( GetInverterState( CarState.Inverters[i].InvState ) != state ) requestedstate = false;
	}
	return requestedstate;
#endif
}


int8_t InverterStateMachine( volatile InverterState *Inverter ) // returns response to send inverter based on current state.
{
	uint16_t State, TXState;

	bool HighVoltageAllowed;//, ReadyToDriveAllowed; //, TsLED, RtdmLED;

	State = Inverter->InvState;
	HighVoltageAllowed = Inverter->HighVoltageAllowed;

	// first check for fault status, and issue reset.

	TXState = 0; // default  do nothing state.
	// process regular state machine sequence
	switch ( GetInverterState(State) )
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
				TXState = 0b00000111; // request Switch on message, State 3..
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


long getInvSpeedValue( uint8_t *data)
{
	//		 Speed_Right_Inverter.data.longint * (1/4194304) * 60; - convert to rpm.
	return getLEint32(&data[2]) * ( 1.0/4194304 ) * 60;
}


void resetInv( void )
{
	for ( int i=0;i<MOTORCOUNT; i++)
	{
		CarState.Inverters[i].InvState = 0xFF;
#ifdef SIEMENS
		CarState.Inverters[i].InvStateCheck = 0xFF;
		CarState.Inverters[i].InvStateCheck3 = 0xFF;
#endif
		CarState.Inverters[i].InvBadStatus = 1;
		CarState.Inverters[i].Torque_Req = 0;
		CarState.Inverters[i].Speed = 0;
		CarState.Inverters[i].HighVoltageAllowed = false;
		CarState.Inverters[i].InverterNum = i;
		CarState.Inverters[i].MCChannel = false;
	}

	CarState.Inverters[0].COBID = InverterRL_COBID;
	CarState.Inverters[1].COBID = InverterRR_COBID;

//	CarState.Inverters[0].MCChannel = InverterRL_Channel;
//	CarState.Inverters[1].MCChannel = InverterRR_Channel;

#if MOTORCOUNT > 2
	CarState.Inverters[2].COBID = InverterFL_COBID;
	CarState.Inverters[3].COBID = InverterFR_COBID;

//	CarState.Inverters[2].MCChannel = InverterFL_Channel;
//	CarState.Inverters[3].MCChannel = InverterFR_Channel;
#endif

	for ( int i=0;i<MOTORCOUNT;i++)
	{
		Errors.InvAllowReset[i] = 1;
	}

	for ( int i=0;i<MOTORCOUNT;i++){
		DeviceState.Inverters[i] = OFFLINE;
	}

	Errors.InverterError = 0; // reset logged errors.
}


int initInv( void )
{
	resetInv();

	RegisterResetCommand(resetInv);


	registerInverterCAN();

	InvQueue = xQueueCreateStatic( InvQUEUE_LENGTH,
							  InvITEMSIZE,
							  InvQueueStorageArea,
							  &InvStaticQueue );

	vQueueAddToRegistry(InvQueue, "InverterQueue" );

	InvTaskHandle = osThreadNew(InvTask, NULL, &InvTask_attributes);

  return 0;
}

uint8_t invRequestState( DeviceStatus state )
{
	if ( DeviceState.Inverter != state )
	{
		Inv_msg msg;
		msg.state  = state;
		xQueueSend(InvQueue, &msg, 0);
		return 0;
	} else return 1; // this is operating with cansync, no extra needed.
}

