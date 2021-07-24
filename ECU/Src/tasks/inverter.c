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
#include "debug.h"
#include "power.h"
#include "taskpriorities.h"
#include "timerecu.h"

#ifdef SIEMENS
	#include "siemensinverter.h"
#endif
#ifdef LENZE
	#include "lenzeinverter.h"
#endif

DeviceStatus GetInverterState( void );
int8_t getInverterControlWord( const InverterState_t *Inverter );
void InvInternalResetRDO( void );

bool InvStartupState( volatile InverterState_t *Inverter, const uint8_t CANRxData[8], bool resend );

#define INVSTACK_SIZE 128*2
#define INVTASKNAME  "InvTask"
StaticTask_t xINVTaskBuffer;
StackType_t xINVStack[ INVSTACK_SIZE ];

TaskHandle_t InvTaskHandle;

#define InvQUEUE_LENGTH    20
#define InvITEMSIZE		   sizeof( Inv_msg )

#define InvCfgQUEUE_LENGTH    10   // should only have one pending send per inverter.
#define InvCfgITEMSIZE		   sizeof( InvCfg_msg )

/* The variable used to hold the queue's data structure. */
static StaticQueue_t InvStaticQueue, InvCfgStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t InvQueueStorageArea[ InvQUEUE_LENGTH * InvITEMSIZE ];
uint8_t InvCfgQueueStorageArea[ InvCfgQUEUE_LENGTH * InvCfgITEMSIZE ];

QueueHandle_t InvQueue, InvCfgQueue;

SemaphoreHandle_t InvUpdating;

InverterState_t InverterState[MOTORCOUNT];
InverterState_t invalidinv = {0};

InverterState_t * getInvState(uint8_t inv )
{
	if ( inv >=0 && inv < MOTORCOUNT )
		return &InverterState[inv];
	else
	{
		return &invalidinv;
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
	// TODO implement conversion for gear ratio..
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

DeviceStatus InverterStates[MOTORCOUNT];


void HandleInverter( InverterState_t * Inverter )
{
	char str[80];
	// only process inverter state if inverters have been seen and not in error state.
	if (Inverter->InvState != OFFLINE ) // && InverterStates[Inverter->Motor] != OFFLINE )
	{
	// run the state machine response and get command to match current situation.
		command = getInverterControlWord( Inverter );
		Inverter->InvReqCommand = command;

		// only change command if we're not in wanted state to try and transition towards it.
		if (Inverter->InvState != Inverter->InvRequested && Inverter->InvState > INERROR )
		{
//			if ( Inverter->Motor == 1 )
			// check if we've got voltage available for moving up states, otherwise stay up.
			if ( Inverter->HighVoltageAvailable )
			{
				Inverter->InvCommand = command;
			}
		}
	}
#define INVDEBUG
	// initial testing, use maximum possible error reset period regardless of error.
	if ( Inverter->InvState == INERROR )
	{
		// only reset errors when we've got HV otherwise will always have motor conn error.
		if ( gettimer() - Inverter->errortime > ERRORTYPE1RESETTIME && Inverter->HighVoltageAvailable  )
		{

			InvResetError(Inverter);
			Inverter->errortime = gettimer();
			Inverter->InvRequested = BOOTUP;
#ifdef INVDEBUG
			snprintf(str, 80, "Inverter Reset sent to Inv[%d] at (%lu)", Inverter->Motor, gettimer());
			DebugMsg(str);
#endif
		} else
		{
			InvSend( Inverter, false ); // continue sending PDO.
		}
	}
	else
	{
//			xSemaphoreTake(InvUpdating, portMAX_DELAY);
		InvSend( Inverter, false );
//			xSemaphoreGive(InvUpdating);
	}
}


bool InvSendSDO( uint16_t id, uint16_t idx, uint8_t sub, uint32_t data)
{
	InvCfg_msg msg;
	msg.id = id;
	msg.idx = idx;
	msg.sub = sub;
	msg.data = data;
	if ( xQueueSend(InvCfgQueue, &msg, 0) )
		return true;
	else
	{
		DebugPrintf("Failed to add SDO %d %4X to queue at (%lu)", id, idx, gettimer());
		return false;
	}
}

char * getMotorsEnabledStr( void )
{
	static char enabledstr[MOTORCOUNT+1];
	uint8_t motorsenabled = getEEPROMBlock(0)->EnabledMotors;
	snprintf(enabledstr,MOTORCOUNT+1, "%s%s%s%s",
		(1 << 0 ) & motorsenabled?"0":"", 	(1 << 1 ) & motorsenabled?"1":"",
		(1 << 2 ) & motorsenabled?"2":"",   (1 << 3 ) & motorsenabled?"3":"");
	return enabledstr;
}


// task to manage inverter state.
void InvTask(void *argument)
{
	xEventGroupSync( xStartupSync, 0, 1, portMAX_DELAY ); // ensure that tasks don't start before all initialisation done.

	uint8_t watchdogBit = registerWatchdogBit("InvTask");

	Inv_msg msg;

	DeviceState.Inverter = OFFLINE;

	uint32_t invexpected[MOTORCOUNT];

	for ( int i=0; i<MOTORCOUNT; i++)
	{
		invexpected[i] = getInvExpected(i);

	}

	TickType_t lastseen[MOTORCOUNT];

#if 0
	// inverters should be shutdown at startup.
	if ( getDevicePower(Inverters) )
	{
		if ( !setDevicePower( Inverters, false ) )
			DebugMsg("Error requesting power off for inverters to reset.");
	}

	while ( !getDevicePower(Inverters) )
	{
		vTaskDelay(CYCLETIME);
		setWatchdogBit(watchdogBit);
	}


	for ( int i=0;i<10;i++ )
	{
		vTaskDelay(CYCLETIME); // wait a few cycles for power to be off.
		setWatchdogBit(watchdogBit);
	}


	if (!setDevicePower( Inverters, true ) )
	{
			DebugMsg("Error requesting power on for inverters.");
	}
#endif

	DebugMsg("Inv Waiting setup");
	CAN_SendStatus(8, 0, 0);

	uint32_t InvReceived = 0;

	bool firstactive[4] = {false};
	bool firstreceive[4] = {false};
	bool firstbad[4] = {false};

	char str[80];

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

		InvCfg_msg cfgmsg;
		if ( xQueueReceive(InvCfgQueue,&cfgmsg,0) ) // queue of pending inverter cfg commands, to send them at a controlled pace.
		{
			CANSendSDO(bus0, cfgmsg.id, cfgmsg.idx, cfgmsg.sub, cfgmsg.data);
		}

		vTaskDelay(3); // wait a bit so not right at sync point.

		int online = 0;
		DeviceStatus lowest = OPERATIONAL;

		for ( int i=0; i<MOTORCOUNT; i++) // speed is received
		{
			if( InverterState[i].SetupState == 0xFF )
			{
				if ( !firstactive[i] )
				{
					firstactive[i] = true;
					snprintf(str, 60, "Inv[%d] first active cycle at (%lu)", i, gettimer());
					DebugMsg(str);
					CAN_SendStatus(9, i, 0);
				}

				if ( ( InvReceived & invexpected[i] ) == invexpected[i] ) // everything received for inverter i
				{
					if ( i == 0 && !firstreceive[i])
					{
						firstreceive[i] = true;
						snprintf(str, 60, "Inv[%d] first received ok at (%lu)", i, gettimer());
						DebugMsg(str);
					}
					lastseen[i] = gettimer();
					InverterState[i].Device = OPERATIONAL;
					HandleInverter(&InverterState[i]);
				}
				else
				{
					if ( i == 0 && !firstbad[i] )
					{
						firstbad[i] = true;
						snprintf(str, 60, "Inv[%d] not received, got %lu, expected %lu at (%lu)", i, InvReceived & invexpected[i], invexpected[i], gettimer());
						DebugMsg(str);

						CAN_SendStatus(9, i, 1);

					}

					// should always be sending PDO if we're configured to stop timeouts.
					HandleInverter(&InverterState[i]);

					if ( gettimer()-lastseen[i] > INVERTERTIMEOUT && InverterState[i].Device != OFFLINE ) //
					{
						firstbad[i] = false;
						if ( InverterState[i].Device != OFFLINE )
						{
							snprintf(str, 40, "Inverter %d Timeout (%lu)", i, gettimer());
							DebugMsg(str);
							InverterState[i].Device = OFFLINE;
							// prevent automatically putting back online for now.
							InverterState[i].SetupState = 0;
							CAN_SendStatus(9, i, 2);

							firstbad[i] = false;
							firstactive[i] = false;
							firstreceive[i] = false;
						}
					}
				}

				if ( InverterState[i].Device != OFFLINE )
				{
					online++;
					if ( InverterState[i].Device < lowest )
						lowest = InverterState[i].Device;
				}

			} else
			{
				// state 1 should be triggered automatically by inverter startup right now, not attempting to force it.
				if ( !InverterState[i].MCChannel ) // if we've not configured inverters, only deal with APPC channel.
				{
					static uint8_t dummyCAN[8] = {0,0,0,0,0,0,0,0};

					// don't do anything if setup state 0, or 0xFF. Or if less then 1s since SetupLastSeenTime
					if ( InverterState[i].SetupState == 1
						&& gettimer() - InverterState[i].SetupLastSeenTime > 1000)
					{
						snprintf(str, 80, "Starting Inverter %d private CFG after APPC setup (last %lu) (%lu)", i, InverterState[i].SetupLastSeenTime, gettimer());
						DebugMsg(str);
						// no messages for a second, APPC has finished setting up MC's, carry on with state machine.
						InverterState[i].SetupState = 2; // start the setup state machine
						CAN_SendStatus(9, i, 3);

						InvStartupState( &InverterState[i], dummyCAN, false );
						snprintf(str, 80, "Starting Inverter %d StartupState called. (last %lu) (%lu)", i, InverterState[i].SetupLastSeenTime, gettimer());
						DebugMsg(str);
					} else if ( InverterState[i].SetupState < 0xFE && InverterState[i].SetupState > 1
							&& gettimer() - InverterState[i].SetupLastSeenTime > 100 )
					{
						// check if inverters are drawing any current, if so, keep trying.

						if ( InverterState[i].SetupTries < 10 )
						{
							snprintf(str, 80, "\nTimeout during Inverter %d private CFG setup at state %d, resending (%lu)\n", i, InverterState[i].SetupState, gettimer());
							DebugMsg(str);
							CAN_SendStatus(9, i, 4);
							InverterState[i].SetupTries++;
							InvStartupState( &InverterState[i], dummyCAN, true );
						} else
						{
							if ( InverterState[i].SetupState != 0xFE )
							{
								snprintf(str, 80, "Giving up on Inverter %d private CFG setup in state %d (%lu)\n", i, InverterState[i].SetupState, gettimer());
								DebugMsg(str);
							}
							InverterState[i].SetupState = 0xFE;
						}
					} else
					{
						// not seen inverters yet.
					}
				}
			}

			if ( online == 4 )
				DeviceState.Inverter = lowest; // set current lowest state as operational state
			else
				DeviceState.Inverter = OFFLINE;

			vTaskDelay(1);
		}

		setWatchdogBit(watchdogBit);
		// only allow one command per cycle. Switch to syncing with main task to not go out of sync?

		xEventGroupSync( xCycleSync, 0, 1, portMAX_DELAY ); // wait for main cycle.
		// after synced, send current state for next cycle. Higher priority task, so should be received first.
		xTaskNotifyWait( pdFALSE, ULONG_MAX,  &InvReceived, 0 );
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
		return BOOTUP;
	}
	else if ( ( Status & 0b01101111 ) == 0b00100001 ) // 49
	{ // Ready to switch on
		return STOPPED;
	}
	else if ( ( Status & 0b01101111 ) == 0b00100011 ) // 51
	{ // Switched on. HV?
		return PREOPERATIONAL;
	}
	else if ( ( Status & 0b01101111 ) == 0b00100111 ) // 55
	{ // Operation enabled.
		return OPERATIONAL;
	}
	else if ( ( ( Status & 0b01101111 ) == 0b00000111 )
			 || ( ( Status & 0b00011111 ) == 0b00010011 ) )
	{ // Quick Stop Active
		return INERRORSTOPPING;
	}
	else if  ( ( ( Status & 0b01001111 ) == 0b00001111 )
			 || ( ( Status & 0b01001111 ) == 0b00001001 ) )
	{ // fault reaction active, will move to fault status next
		return INERRORSTOPPING;
	}
	else if  ( ( ( Status & 0b01001111 ) == 0b00001000 )
			 || ( ( Status & 0b00001000 ) == 0b00001000 ) )
	{ // fault status
		return INERROR;
		// send reset
	} else
	{ // unknown state
		return 0; // state 0 will request reset to enter State 1,
		// will fall here at start of loop and if unknown status.
	}
}


DeviceStatus GetInverterState( void )
{
	return DeviceState.Inverter;
}

bool invertersStateCheck( const DeviceStatus state )
{
	if ( DeviceState.Inverter == state ) return true;
	else return false;
}


int8_t getInverterControlWord( const InverterState_t *Inverter ) // returns response to send inverter based on current state.
{
	uint16_t TXState;

	DeviceStatus State;

	State = Inverter->InvState;

	TXState = 0; // default  do nothing state.
	// process regular state machine sequence
	switch ( State )
	{
		case OFFLINE : // state 0: Not ready to switch on, no can message. Internal state only at startup.
			TXState=0b10000000; // send bit 128 reset message to enter state 1 in case in fault. - fault reset.
			break;

		case BOOTUP : // State 1: Switch on Disabled.
			TXState = 0b00000110; // send 0110 shutdown message to request move to State 2.
			break;

		case STOPPED : // State 2: Ready to switch on
			 // We are ready to turn on, so allow high voltage.
			// we are in state 2, process.
			// process shutdown request here, to move to move to state 1.
			if ( getPowerHVReady() )
			{  // TS enable button pressed and both inverters are marked HV ready proceed to state 3.
#ifdef LENZE
				TXState = 0b00001111; // Lenze doesn't want to go to pre operational from stopped, have to skip straight to operational.
#else
				TXState = 0b00000111; // request Switch on message, State 3..
#endif
			} else
			{
				TXState = 0b00000110; // no change, continue to request State 2.
			}
			break;

		case PREOPERATIONAL : // State 3: Switched on   <---- check this case.
			  // we are powered on, so allow high voltage if available
			if ( getPowerHVReady() )// IdleState ) <-
			{  // TS enable button has been pressed, proceed to request power on if all inverters on.
				TXState = 0b00001111; // Request Enable operation, State 4.
			}
			else if ( !getPowerHVReady() )
			{ // return to switched on state.
				TXState = 0b00000110; // 0b00000000; // request Disable Voltage, drop to ready state.
			}
			else
			{  // no change, continue to request State 3.
				TXState = 0b00000111;
			}
			break;

		case OPERATIONAL : // State 4: Operation Enable
			 // we are powered on, so allow high voltage.
			if ( getPowerHVReady() && !Inverter->AllowTorque )
			{ // no longer in RTDM mode, but still got HV, so drop to idle.
				TXState = 0b00000111; // request state 3: Switched on.
			}
			else
			if ( !getPowerHVReady() )
			{   // full motor stop has been requested
				// drop back to ready to switch on.
				TXState = 0b00000110;//0b00000000; // request Disable Voltage., alternately Quick Stop 0b00000010 - test to see if any difference in behaviour.
			}
			else
			{ // no change, continue to request operation.
				TXState = 0b00001111;
			}
			break;

	//	case -1 : //5 Quick Stop Active - Fall through to default to reset state.

	//	case -2 : //98 Fault Reason Active

	//	case -99 : //99 Fault

		case INERROR:
			TXState = 0b10000000; // 128
		default : // unknown identifier encountered, ignore. Shouldn't be possible to get here due to filters.

			TXState = 0b00000000; // 0 don't transmit any command for error, deal with it seperately.
			break;
		}

	return TXState;
}


long getInvSpeedValue( uint8_t *data )
{
	//		 Speed_Right_Inverter.data.longint * (1/4194304) * 60; - convert to rpm.
	return getLEint32(&data[2]) * ( 1.0/4194304 ) * 60;
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



void resetInv( void )
{
//	InvInternalResetRDO();

	DeviceState.Inverter = OFFLINE;

	for ( int i=0;i<MOTORCOUNT; i++)
	{
		InverterState[i].InvState = OFFLINE;
		InverterState[i].Device = OFFLINE;
		InverterState[i].InvCommand = 0x0;
		InverterState[i].Torque_Req = 0;
		InverterState[i].Speed = 0;
		InverterState[i].Motor = i;
		InverterState[i].MCChannel = false;
		InverterState[i].InvRequested = BOOTUP;

		InverterState[i].AllowRegen = false;
		InverterState[i].AllowTorque = false;

		Errors.InvAllowReset[i] = 1;
	}

	InverterState[0].COBID = InverterRL_COBID;
	InverterState[1].COBID = InverterRR_COBID;

	InverterState[0].MCChannel = InverterRL_Channel;
	InverterState[1].MCChannel = InverterRR_Channel;

#if MOTORCOUNT > 2
	InverterState[2].COBID = InverterFL_COBID;
	InverterState[3].COBID = InverterFR_COBID;

	InverterState[2].MCChannel = InverterFL_Channel;
	InverterState[3].MCChannel = InverterFR_Channel;
#endif

	Errors.InverterError = 0; // reset logged errors.
}


int initNoInv( void )
{
	resetInv();
	return 0;
}

int initInv( void )
{
	resetInv(); // sets up InverterState, id's, etc, so that CAN functions will not be called till setup.

	RegisterResetCommand(resetInv);

	registerInverterCAN();

	InvUpdating = xSemaphoreCreateMutex();

	InvQueue = xQueueCreateStatic( InvQUEUE_LENGTH,
							  InvITEMSIZE,
							  InvQueueStorageArea,
							  &InvStaticQueue );

	vQueueAddToRegistry(InvQueue, "InverterQueue" );

	InvCfgQueue = xQueueCreateStatic( InvCfgQUEUE_LENGTH,
							  InvCfgITEMSIZE,
							  InvCfgQueueStorageArea,
							  &InvCfgStaticQueue );

	vQueueAddToRegistry(InvQueue, "InverterCfgQueue" );

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
