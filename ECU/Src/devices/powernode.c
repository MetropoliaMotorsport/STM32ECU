/*
 * powernode.c
 *
 *  Created on: 15 Jun 2020
 *      Author: Visa
 */

#include "ecumain.h"
#include "powernode.h"
#include "power.h"
#include "debug.h"
#include "errors.h"
#include "timerecu.h"
#include <stdarg.h>

#include <stdio.h>
#include <time.h>

#define MAXECUCURRENT 		20
#define MAXFANCURRENT		20
#define MAXPUMPCURRENT		20

typedef struct nodepowerreqstruct {
	uint8_t nodeid; //
	uint8_t output; // enable
	uint8_t state; // request state
	uint32_t error[6];
	uint32_t timesent;
} nodepowerreq;

typedef struct nodepowerpwmstruct {
	uint8_t nodeid; //
	uint8_t output;
	uint8_t dutycycle; // enable
	uint32_t timesent;
} nodepowerpwmreq;

typedef struct devicepowerreqstruct {
	DevicePower device; //
	uint8_t nodeid;
	uint8_t output; // which bit of enable request is this device on
	bool pwm;
	bool expectedstate; // what state are we requesting.
	bool waiting;
	bool actualstate;
} devicepowerreq;


static uint32_t devicecount = 0;

#ifdef HPF2023

// TODO this list should be sanity checked for duplicates at tune time.
devicepowerreq DevicePowerList[] =
{

//		{ Telemetry, 33, 4 },
//		{ Front1, 33, 5 },

		{ ECU, 34, 4, true, 0, true }, // ECU has to have power or we aren't booted.. so just assume it.
		{ Front2, 34, 5 },

		{ LeftFans, 35, 2 },
		{ RightFans, 35, 3 },
		{ LeftPump, 35, 4 },
		{ RightPump, 35, 5 },

		{ IVT, 36, 3 },
		{ Accu, 36, 4 }, // TODO make sure this channel cannot be reset for latching rules.
		{ AccuFan, 36, 5},

		{ Brake, 37, 0 },
		{ Buzzer, 37, 1 },
		{ Inverters, 37, 2 },
		{ TSAL, 37, 3, true, 0, true }, // essential to be powered, else not compliant.
		//{ Back1, 37, 4 },
		{ None }
};

#else

// TODO this list should be sanity checked for duplicates at tune time.
devicepowerreq DevicePowerList[] =
{
		{ Telemetry, 33, 4 },
		{ Front1, 33, 5 },

		{ Inverters, 34, 3 },
		{ ECU, 34, 4, true, 0, true }, // ECU has to have power or we aren't booted.. so just assume it.
		{ Front2, 34, 5 },

		{ LeftFans, 35, 2 },
		{ RightFans, 35, 3 },
		{ LeftPump, 35, 4 },
		{ RightPump, 35, 5 },

//		{ Brake, 36, 1 },
//		{ Buzzer, 36, 2 },
		{ IVT, 36, 3 },
		{ Accu, 36, 4 }, // TODO make sure this channel cannot be reset for latching rules.
		{ AccuFan, 36, 5},


		{ Current, 37, 1 },
		{ Buzzer, 37, 2 },
		{ Brake, 37, 3 },
		{ Back1, 37, 4 },
		{ TSAL, 37, 5, true, 0, true }, // essential to be powered, else not compliant.

		{ None }
};

#endif

nodepowerreq PowerRequests[] =
{
#ifndef HPF2023
		{ 33, 0, 0, {0} },
#endif
		{ 34, 0, 0, {0} },
		{ 35, 0, 0, {0} },
		{ 36, 0, 0, {0} },
		{ 37, 0, 0, {0} },
		{ 0 }
};

nodepowerpwmreq nodefanpwmreqs[2] = {0};
bool queuedfanpwmLeft = false;
bool queuedfanpwmRight = false;

#ifndef HPF2023
bool processPNode33Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
#endif
bool processPNode34Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processPNode35Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processPNode36Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processPNode37Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

#ifndef HPF2023
void PNode33Timeout( uint16_t id );
void PNode34Timeout( uint16_t id );
void PNode36Timeout( uint16_t id );
#endif
void PNode37Timeout( uint16_t id ); // critical due to brakelight

//bool processPNodeTimeout(uint8_t CANRxData[8], uint32_t DataLength );


bool processPNodeErr(const uint8_t nodeid, const uint32_t errorcode, const CANData * datahandle );
bool processPNodeAckData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

#ifndef HPF2023
CANData  PowerNode33 = { &DeviceState.PowerNode33, PowerNode33_ID, 3, processPNode33Data, PNode33Timeout, NODETIMEOUT }; // [BOTS, inertia switch, BSPD.], Telemetry, front power
#endif
CANData  PowerNode34 = { &DeviceState.PowerNode34, PowerNode34_ID, 4, processPNode34Data, NULL, NODETIMEOUT }; // [shutdown switches.], inverters, ECU, Front,
CANData  PowerNode35 = { &DeviceState.PowerNode35, PowerNode35_ID, 4, processPNode35Data, NULL, NODETIMEOUT }; // Cooling ( fans, pumps )
CANData  PowerNode36 = { &DeviceState.PowerNode36, PowerNode36_ID, 7, processPNode36Data, NULL, NODETIMEOUT }; // BRL, buzz, IVT, ACCUPCB, ACCUFAN, imdfreq, dc_imd?
CANData  PowerNode37 = { &DeviceState.PowerNode37, PowerNode37_ID, 4, processPNode37Data, PNode37Timeout, NODETIMEOUT }; // [?], Current, TSAL.


int sendPowerNodeErrReset( uint8_t id, uint8_t channel );


uint32_t getOldestPNodeData( void )
{
	uint32_t time = gettimer();
#ifndef HPF2023
	if ( PowerNode33.time <  time ) time = PowerNode33.time;
#endif
	if ( PowerNode34.time <  time ) time = PowerNode34.time;
#ifndef HPF2023
	if ( PowerNode35.time <  time ) time = PowerNode35.time;
//	if ( PowerNode36.time <  time ) time = PowerNode36.time; // not currently operational
#endif
	if ( PowerNode37.time <  time ) time = PowerNode37.time;
	return time;
}

#ifndef HPF2023
void PNode33Timeout( uint16_t id )
{
	Shutdown.BOTS = 0;
	Shutdown.InertiaSwitch = 0;
	Shutdown.BSPDAfter = 0;
	Shutdown.BSPDBefore = 0;
}

void PNode34Timeout( uint16_t id )
{
	Shutdown.CockpitButton = 0; // TODO set to right inputs.
	Shutdown.LeftButton = 0;
	Shutdown.RightButton = 0;
}

void PNode36Timeout( uint16_t id )
{
	DeviceState.BrakeLight = OFFLINE;
    SetCriticalError(CRITERBL);
}
#endif

void PNode37Timeout( uint16_t id )
{
#ifdef HPF2023

#else
	Shutdown.AIRm = false;
	Shutdown.AIRp = false;
	Shutdown.PRE = false;
	Shutdown.TS_OFF = false;
	Shutdown.IMD = true;
#endif
	SetCriticalError(CRITERBL); // brakelight is a critical signal.
}

#ifndef HPF2023
bool processPNode33Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("PNode 33 First msg.");
		first = true;
	}

	if ( DataLength >> 16 == PowerNode33.dlcsize
		&& CANRxData[0] <= 0b00011101 // max possible value. check for zeros in unused fields?
		&& CANRxData[1] < 255
		&& ( CANRxData[2] >= 0 && CANRxData[2] < 255 )
		)
	{
		xTaskNotify( PowerTaskHandle, ( 0x1 << PNode33Bit ), eSetBits);

		bool newstate = CANRxData[0] & (0x1 << 4);
		if ( Shutdown.BOTS != newstate )
		{
			DebugPrintf("BOTS Shutdown sense state changed to %s", newstate?"Closed":"Open");
			Shutdown.BOTS = newstate;
		}

		newstate = CANRxData[0] & (0x1 << 0);
		if ( Shutdown.InertiaSwitch != newstate )
		{
			DebugPrintf("Inertia Switch Shutdown sense state changed to %s", newstate?"Closed":"Open");
			Shutdown.InertiaSwitch = newstate;
		}

		newstate = CANRxData[0] & (0x1 << 2);
		if ( Shutdown.BSPDAfter != newstate )
		{
			DebugPrintf("BSPD After Switch Shutdown sense state changed to %s", newstate?"Closed":"Open");
			Shutdown.BSPDAfter = newstate;
		}

		newstate = CANRxData[0] & (0x1 << 3);
		if ( Shutdown.BSPDBefore != newstate )
		{
			DebugPrintf("BSPD Before Switch Shutdown sense state changed to %s", newstate?"Closed":"Open");
			Shutdown.BSPDBefore = newstate;
		}

//		CarState.I_Telementry =  CANRxData[1];
//		CarState.I_Front1 =  CANRxData[2];
		return true;

	} else // bad data.
	{
		return false;
	}
}
#endif

bool processPNode34Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	static bool first = false;
	if ( !first )
	{
		DebugMsg("PNode 34 First msg.");
		first = true;
	}
	// 0x1f 0x0001 10000
	if ( DataLength >> 16 == PowerNode34.dlcsize
//		&& CANRxData[0] & ~(0b00011100) != 0 // check mask fit
		&& CANRxData[1] < 255
		&& ( CANRxData[2] >= 0 && CANRxData[2] <= 255 )
		&& CANRxData[3] < 255
		)
	{
		xTaskNotify( PowerTaskHandle, ( 0x1 << PNode34Bit ), eSetBits);

		bool newstate = CANRxData[0] & (0x1 << 2);
		if ( Shutdown.CockpitButton != newstate )
		{
			DebugPrintf("Cockpit Shutdown Button sense state changed to %s at (%ul)", newstate?"Closed":"Open", gettimer());
			Shutdown.CockpitButton = newstate;
		}

		newstate = CANRxData[0] & (0x1 << 3);
		if ( Shutdown.LeftButton != newstate )
		{
			DebugPrintf("Left Shutdown Button sense state changed to %s at (%ul)", newstate?"Closed":"Open", gettimer());
			Shutdown.LeftButton = newstate;
		}

		newstate = CANRxData[0] & (0x1 << 4);
		if ( Shutdown.RightButton != newstate )
		{
			DebugPrintf("Right Shutdown Button sense state changed to %s at (%ul)", newstate?"Closed":"Open", gettimer());
			Shutdown.RightButton = newstate;
		}

//		CarState.I_Inverters =  CANRxData[1];
//		CarState.I_ECU =  CANRxData[2];
//		CarState.I_Front2 =  CANRxData[3];
		return true;

	} else // bad data.
	{
		return false;
	}
}

bool processPNode35Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle ) // Cooling
{

	static bool first = false;
	if ( !first )
	{
		DebugMsg("PNode 35 First msg.");
		first = true;
	}

	if ( DataLength >> 16 == PowerNode35.dlcsize
//		&& ( CANRxData[0] >= 0 && CANRxData[0] <= MAXFANCURRENT )
//		&& ( CANRxData[1] >= 0 && CANRxData[1] <= MAXFANCURRENT )
//		&& ( CANRxData[2] >= 0 && CANRxData[2] <= MAXPUMPCURRENT )
//		&& ( CANRxData[3] >= 0 && CANRxData[3] <= MAXPUMPCURRENT )
		)
	{
		xTaskNotify( PowerTaskHandle, ( 0x1 << PNode35Bit ), eSetBits);
		CarState.I_LeftFans = CANRxData[0];
		CarState.I_RightFans = CANRxData[1];
		CarState.I_LeftPump = CANRxData[2];
		CarState.I_RightPump = CANRxData[3];
		return true;

	} else // bad data.
	{
		return false;
	}
}


bool processPNode36Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle ) // Rear
{

	static bool first = false;
	if ( !first )
	{
		DebugMsg("PNode 36 First msg.");
		first = true;
	}

	if ( DataLength >> 16 == PowerNode36.dlcsize
		&& ( CANRxData[0] >= 0 && CANRxData[0] <= 10 )
		&& ( CANRxData[1] >= 0 && CANRxData[1] <= 10 )
		&& ( CANRxData[2] >= 0 && CANRxData[2] <= 10 )
		&& ( CANRxData[3] >= 0 && CANRxData[3] <= 10 )
		&& ( CANRxData[4] >= 0 && CANRxData[4] <= 10 )
		&& ( CANRxData[5] >= 0 && CANRxData[5] <= 255 )
		&& ( CANRxData[6] >= 0 && CANRxData[6] <= 100 )
		)
	{
		xTaskNotify( PowerTaskHandle, ( 0x1 << PNode36Bit ), eSetBits);

//		CarState.I_BrakeLight = CANRxData[0];
//		CarState.I_Buzzers = CANRxData[1];
		CarState.I_IVT = CANRxData[2];
		CarState.I_AccuPCBs = CANRxData[3];
		CarState.I_AccuFans = CANRxData[4];
		CarState.Freq_IMD = CANRxData[5]; // IMD Shutdown.

#if 0
		// normal operational status, else assume error.
		if ( CarState.Freq_IMD >= 5 && CarState.Freq_IMD < 15 )
			Shutdown.IMD = true;
		else
			Shutdown.IMD = false;
#endif
		DeviceState.BrakeLight = OPERATIONAL;

//		should be 10 Hz in normal situation (I think duty cycle was based on measured resistance or something)
//		and then 50 Hz for fault state (50% duty cycle),
//		40 Hz is internal error,
//		20 Hz we should never see, then not sure what 30 Hz is
//		it also wasn't 10 Hz on power up,
//		I dont' remember what it was on power up though, might have been 30 Hz

		CarState.DC_IMD = CANRxData[6];

		return true;
	} else // bad data.
	{
		return false;
	}
}

bool processPNode37Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{

	static bool first = false;
	if ( !first )
	{
		DebugMsg("PNode 37 First msg.");
		first = true;
	}

#if 1
	if ( 1 )
#else
	if ( DataLength >> 16 == PowerNode37.dlcsize
		&& CANRxData[0] <= 0b00011101 // max possible value. check for zeros in unused fields?
		&& CANRxData[1] < 100
		&& ( CANRxData[2] >= 0 && CANRxData[2] <= 100 )
		)
#endif
	{
		xTaskNotify( PowerTaskHandle, ( 0x1 << PNode37Bit ), eSetBits);

#ifdef HPF2023
// no inputs on node 37.
#else
		bool newstate = ! ( CANRxData[0] & (0x1 << 0) ); // DI3
		if ( Shutdown.PRE != newstate )
		{
			DebugPrintf("PREcharge sense state changed to %s at (%ul)", newstate?"Closed":"Open", gettimer());
			Shutdown.PRE = newstate;
		}

		newstate = ! ( CANRxData[0] & (0x1 << 2) ); // DI5
		if ( Shutdown.AIRp != newstate )
		{
			DebugPrintf("AIR Plus sense state changed to %s at (%ul)", newstate?"Closed":"Open", gettimer());
			Shutdown.AIRp = newstate;
		}
#if 0
		newstate = ! ( CANRxData[0] & (0x1 << 3) ); // DI6
		if ( Shutdown.AIRm != newstate )
		{
			DebugPrintf("AIR minus state changed to %s at (%ul)", newstate?"Closed":"Open", gettimer());
			Shutdown.AIRm = newstate;
		}
#endif
		newstate = ! ( CANRxData[0] & (0x1 << 3) ); // DI6
		if ( Shutdown.IMD != newstate )
		{
			DebugPrintf("IMD sense state changed to %s at (%ul)", newstate?"True":"False", gettimer());
			Shutdown.IMD = newstate;
		}

		newstate = ( CANRxData[0] & (0x1 << 4) ); // DI15
		if ( Shutdown.TS_OFF != newstate )
		{
			DebugPrintf("TS_OFF sense state changed to %s at (%ul)", newstate?"True":"False", gettimer());
			Shutdown.TS_OFF = newstate;
		}
#endif

		//		CarState.I_BrakeLight = CANRxData[0];
		//		CarState.I_Buzzers = CANRxData[1];

//		CarState.??? = (CANRxData[0] & (0x1 << 4) );
//		CarState.I_currentMeasurement =  CANRxData[1];
//		CarState.I_TSAL =  CANRxData[2];
		return true;
	} else // bad data.
	{
		return false;
	}
}

void setAllPowerActualOff( void )
{
	for ( int i = 0; DevicePowerList[i].device != None; i++ )
	{
		if ( DevicePowerList[i].device != ECU )
		{
			if ( DevicePowerList[i].actualstate == true );
	//		DevicePowerList[i].waiting = false;
			DevicePowerList[i].actualstate = false;
		}
	}
}

bool setActualDevicePower( uint8_t nodeid, uint8_t channel, bool state )
{
	// find the device
	for ( int i = 0; DevicePowerList != None; i++ )
	{
		if ( DevicePowerList[i].nodeid == nodeid && DevicePowerList[i].output == channel )
		{
			DevicePowerList[i].waiting = false;
			DevicePowerList[i].actualstate = state;
			return true;
		}
	}
	return false;
}


bool processPNodeAckData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{

	if ( CANRxData[1] == 1) // power set ACK
	{
		for ( int i=0;PowerRequests[i].nodeid != 0;i++)
		{
			if ( PowerRequests[i].nodeid == CANRxData[0] ) // check for power request change.
			{
				PowerRequests[i].output ^= CANRxData[2]; // XOR the input with the request. if the reply is set, this will zero out the request.
				PowerRequests[i].state ^= CANRxData[3]; // update the status of which outputs are on.

				// update the device power table for current actual reported state.
				for ( int j=0;j<6;j++)
				{
					if ( CANRxData[2] & ( 1 << j ) )
					{
						setActualDevicePower(PowerRequests[i].nodeid, j, CANRxData[3] & ( 1 << j ) );
					}
				}

				return true; // request has been processed, stop iterating.
			}
		}
	} else
	if ( CANRxData[1] == 2) // PWM set ack
	{
		if ( nodefanpwmreqs[0].nodeid == nodefanpwmreqs[1].nodeid )
		{
			if ( CANRxData[0] == nodefanpwmreqs[0].nodeid )
			{
				queuedfanpwmLeft = false;
				queuedfanpwmRight = false;
			}
		} else if ( CANRxData[0] == nodefanpwmreqs[0].nodeid )
		{
			queuedfanpwmLeft = false;
		} else if ( CANRxData[0] == nodefanpwmreqs[1].nodeid )
		{
			queuedfanpwmRight = false;
		}
	} else
	if ( CANRxData[1] == 23) // Error reset
		{
			if ( nodefanpwmreqs[0].nodeid == nodefanpwmreqs[1].nodeid )
			{
				if ( CANRxData[0] == nodefanpwmreqs[0].nodeid )
				{
					queuedfanpwmLeft = false;
					queuedfanpwmRight = false;
				}
			} else if ( CANRxData[0] == nodefanpwmreqs[0].nodeid )
			{
				queuedfanpwmLeft = false;
			} else if ( CANRxData[0] == nodefanpwmreqs[1].nodeid )
			{
				queuedfanpwmRight = false;
			}
		}

	return true;
}

// ERROR list from powernode main.h

#define ERR_CAN_BUFFER_FULL			1
#define ERR_CAN_FIFO_FULL			2
#define ERR_MESSAGE_DISABLED		3
#define ERR_DLC_0					4
#define ERR_DLC_LONG				5
#define ERR_SEND_FAILED				6
#define ERR_RECIEVE_FAILED			7
#define ERR_INVALID_COMMAND			8
#define ERR_COMMAND_SHORT			9
#define ERR_RECIEVED_INVALID_ID 	10
#define ERR_CANBUSOFFLINE			11

#define ERR_MODIFY_INVALID_MESSAGE	33
#define ERR_MODIFY_INVALID_THING	34
#define ERR_CLEAR_INVALID_ERROR		35

#define ERR_MESS_INVALID_BYTES		97
#define ERR_MESS_UNDEFINED			98


#define WARN_UNDERVOLT_U5				193
#define WARN_OVERVOLT_U5				194
#define WARN_UNDERTEMP_U5				195
#define WARN_OVERTEMP_U5				196
#define WARN_UNDERCURR_U5I0				197
#define WARN_OVERCURR_U5I0				198
#define WARN_UNDERCURR_U5I1				199
#define WARN_OVERCURR_U5I1				200
#define ERROR_OVERCURR_TRIP_U5_0		201 //PO0
#define ERROR_OVERCURR_TRIP_U5_1		202 //PO1
#define WARN_UNDERVOLT_U6				203
#define WARN_OVERVOLT_U6				204
#define WARN_UNDERTEMP_U6				205
#define WARN_OVERTEMP_U6				206
#define WARN_UNDERCURR_U6I0				207
#define WARN_OVERCURR_U6I0				208
#define WARN_UNDERCURR_U6I1				209
#define WARN_OVERCURR_U6I1				210
#define ERROR_OVERCURR_TRIP_U6_0		211 //PO2
#define ERROR_OVERCURR_TRIP_U6_1		212 //PO3
#define WARN_UNDERVOLT_U7				213
#define WARN_OVERVOLT_U7				214
#define WARN_UNDERTEMP_U7				215
#define WARN_OVERTEMP_U7				216
#define WARN_UNDERCURR_U7I0				217
#define WARN_OVERCURR_U7I0				218
#define WARN_UNDERCURR_U7I1				219
#define WARN_OVERCURR_U7I1				220
#define ERROR_OVERCURR_TRIP_U7_0		221 //PO4
#define ERROR_OVERCURR_TRIP_U7_1		222 //PO5

#define ERROR_READ_TEMP					224
#define WARN_TEMP_MEASURE_OVERFLOW		225
#define WARN_VOLT_MEASURE_OVERFLOW		226

#define WARN_PWM_INVALID_CHANNEL		257
#define WARN_PWM_CHANNEL_UNINITIALIZED	258
#define WARN_UNDEFINED_GPIO				259
#define WARN_PWM_NOT_ENABLED			260

//
// command 12 is used to clear an error from switch shutoff
// until the error is cleared the node will regularly send a warning
// about which channel has been shutoff, byte 2 should be set to the
// number of which channel to clear (0 to 5)

// for example: [1][12][3] will clear an error on channel 3 from node 1

#define U5I0_SWITCH_OFF				129 //PO0
#define U5I1_SWITCH_OFF				130 //PO1
#define U6I0_SWITCH_OFF				131 //PO2
#define U6I1_SWITCH_OFF				132 //PO3
#define U7I0_SWITCH_OFF				133 //PO4
#define U7I1_SWITCH_OFF				134 //PO5


#define MAXPNODEERRORS		40

struct PowerNodeError
{
	uint8_t nodeid;
	uint32_t error;
} PowerNodeErrors[MAXPNODEERRORS];

uint8_t PowerNodeErrorCount = 0;


char * PNodeGetErrStr( uint32_t error )
{
	switch ( error )
	{
	case U5I0_SWITCH_OFF 				: return "Ch0 Off";
	case U5I1_SWITCH_OFF 				: return "Ch1 Off";
	case U6I0_SWITCH_OFF 				: return "Ch2 Off";
	case U6I1_SWITCH_OFF 				: return "Ch3 Off";
	case U7I0_SWITCH_OFF 				: return "Ch4 Off";
	case U7I1_SWITCH_OFF 				: return "Ch5 Off";

	case ERR_CAN_BUFFER_FULL			: return "CAN_BUFFER_FULL";
	case ERR_CAN_FIFO_FULL				: return "CAN_FIFO_FULL";
	case ERR_MESSAGE_DISABLED			: return "MESSAGE_DISABLED";
	case ERR_DLC_0						: return "DLC_0";
	case ERR_DLC_LONG					: return "DLC_LONG";
	case ERR_SEND_FAILED				: return "SEND_FAILED";
	case ERR_RECIEVE_FAILED				: return "RECIEVE_FAILED";
	case ERR_INVALID_COMMAND			: return "INVALID_COMMAND";
	case ERR_COMMAND_SHORT				: return "COMMAND_SHORT";
	case ERR_RECIEVED_INVALID_ID 		: return "RECIEVED_INVALID_ID";
	case ERR_CANBUSOFFLINE				: return "CANBUSOFFLINE";

	case ERR_MODIFY_INVALID_MESSAGE		: return "MODIFY_INVALID_MESSAGE";
	case ERR_MODIFY_INVALID_THING		: return "MODIFY_INVALID_THING";
	case ERR_CLEAR_INVALID_ERROR		: return "CLEAR_INVALID_ERROR";

	case ERR_MESS_INVALID_BYTES			: return "MESS_INVALID_BYTES";
	case ERR_MESS_UNDEFINED				: return "MESS_UNDEFINED";

	case WARN_UNDERVOLT_U5				: return "UNDERVOLT_CH0-1";
	case WARN_OVERVOLT_U5				: return "OVERVOLT_CH0-1";
	case WARN_UNDERTEMP_U5				: return "UNDERTEMP_CH0-1";
	case WARN_OVERTEMP_U5				: return "OVERTEMP_CH0-1";
	case WARN_UNDERCURR_U5I0			: return "UNDERCURR_CH0";
	case WARN_OVERCURR_U5I0				: return "OVERCURR_CH0";
	case WARN_UNDERCURR_U5I1			: return "UNDERCURR_CH1";
	case WARN_OVERCURR_U5I1				: return "OVERCURR_CH1";
	case ERROR_OVERCURR_TRIP_U5_0		: return "OVERCURR_CH0";
	case ERROR_OVERCURR_TRIP_U5_1		: return "OVERCURR_CH1";
	case WARN_UNDERVOLT_U6				: return "UNDERVOLT_CH2-3";
	case WARN_OVERVOLT_U6				: return "OVERVOLT_CH2-3";
	case WARN_UNDERTEMP_U6				: return "UNDERTEMP_CH2-3";
	case WARN_OVERTEMP_U6				: return "OVERTEMP_CH2-3";
	case WARN_UNDERCURR_U6I0			: return "UNDERCURR_CH2";
	case WARN_OVERCURR_U6I0				: return "OVERCURR_CH2";
	case WARN_UNDERCURR_U6I1			: return "UNDERCURR_CH3";
	case WARN_OVERCURR_U6I1				: return "OVERCURR_CH3";
	case ERROR_OVERCURR_TRIP_U6_0		: return "OVERCURR_CH2";
	case ERROR_OVERCURR_TRIP_U6_1		: return "OVERCURR_CH3";
	case WARN_UNDERVOLT_U7				: return "UNDERVOLT_CH4-5";
	case WARN_OVERVOLT_U7				: return "OVERVOLT_CH4-5";
	case WARN_UNDERTEMP_U7				: return "UNDERTEMP_CH4-5";
	case WARN_OVERTEMP_U7				: return "OVERTEMP_CH4-5";
	case WARN_UNDERCURR_U7I0			: return "UNDERCURR_CH4";
	case WARN_OVERCURR_U7I0				: return "OVERCURR_CH4";
	case WARN_UNDERCURR_U7I1			: return "UNDERCURR_CH5";
	case WARN_OVERCURR_U7I1				: return "OVERCURR_CH5";
	case ERROR_OVERCURR_TRIP_U7_0		: return "OVERCURR_CH4";
	case ERROR_OVERCURR_TRIP_U7_1		: return "OVERCURR_CH5";

	case ERROR_READ_TEMP				: return "READ_TEMP";
	case WARN_TEMP_MEASURE_OVERFLOW		: return "TEMP_MEASURE_OVERFLOW";
	case WARN_VOLT_MEASURE_OVERFLOW		: return "VOLT_MEASURE_OVERFLOW";

	case WARN_PWM_INVALID_CHANNEL		: return "PWM_INVALID_CHANNEL";
	case WARN_PWM_CHANNEL_UNINITIALIZED	: return "PWM_CHANNEL_UNINITIALIZED";
	case WARN_UNDEFINED_GPIO			: return "UNDEFINED_GPIO";
	case WARN_PWM_NOT_ENABLED			: return "PWM_NOT_ENABLED";
	default:
		return "Unknown";
	}
}

bool PNodeGetErrType( uint32_t error )
{
	switch ( error )
	{
	case U5I0_SWITCH_OFF 				:
	case U5I1_SWITCH_OFF 				:
	case U6I0_SWITCH_OFF 				:
	case U6I1_SWITCH_OFF 				:
	case U7I0_SWITCH_OFF 				:
	case U7I1_SWITCH_OFF 				:

	case ERR_CAN_BUFFER_FULL			:
	case ERR_CAN_FIFO_FULL				:
	case ERR_MESSAGE_DISABLED			:
	case ERR_DLC_0						:
	case ERR_DLC_LONG					:
	case ERR_SEND_FAILED				:
	case ERR_RECIEVE_FAILED				:
	case ERR_INVALID_COMMAND			:
	case ERR_COMMAND_SHORT				:
	case ERR_RECIEVED_INVALID_ID 		:
	case ERR_CANBUSOFFLINE				:

	case ERR_MODIFY_INVALID_MESSAGE		:
	case ERR_MODIFY_INVALID_THING		:
	case ERR_CLEAR_INVALID_ERROR		:

	case ERR_MESS_INVALID_BYTES			:
	case ERR_MESS_UNDEFINED				: return true;

	case WARN_UNDERVOLT_U5				:
	case WARN_OVERVOLT_U5				:
	case WARN_UNDERTEMP_U5				:
	case WARN_OVERTEMP_U5				:
	case WARN_UNDERCURR_U5I0			:
	case WARN_OVERCURR_U5I0				:
	case WARN_UNDERCURR_U5I1			:
	case WARN_OVERCURR_U5I1				:

	case WARN_UNDERVOLT_U6				:
	case WARN_OVERVOLT_U6				:
	case WARN_UNDERTEMP_U6				:
	case WARN_OVERTEMP_U6				:
	case WARN_UNDERCURR_U6I0			:
	case WARN_OVERCURR_U6I0				:
	case WARN_UNDERCURR_U6I1			:
	case WARN_OVERCURR_U6I1				:

	case WARN_UNDERVOLT_U7				:
	case WARN_OVERVOLT_U7				:
	case WARN_UNDERTEMP_U7				:
	case WARN_OVERTEMP_U7				:
	case WARN_UNDERCURR_U7I0			:
	case WARN_OVERCURR_U7I0				:
	case WARN_UNDERCURR_U7I1			:
	case WARN_OVERCURR_U7I1				: return false;

	case ERROR_OVERCURR_TRIP_U5_0		:
	case ERROR_OVERCURR_TRIP_U5_1		:
	case ERROR_OVERCURR_TRIP_U6_0		:
	case ERROR_OVERCURR_TRIP_U6_1		:
	case ERROR_OVERCURR_TRIP_U7_0		:
	case ERROR_OVERCURR_TRIP_U7_1		:

	case ERROR_READ_TEMP				: return true;
	case WARN_TEMP_MEASURE_OVERFLOW		:
	case WARN_VOLT_MEASURE_OVERFLOW		:

	case WARN_PWM_INVALID_CHANNEL		:
	case WARN_PWM_CHANNEL_UNINITIALIZED	:
	case WARN_UNDEFINED_GPIO			:
	case WARN_PWM_NOT_ENABLED			: return false;
	default:
		return true;
	}
}

bool processPNodeErr( const uint8_t nodeid, const uint32_t errorcode, const CANData * datahandle )
{
	uint8_t channel = 255;

	// log error to power queue.
	PowerLogError(nodeid, errorcode);

	switch ( errorcode ) // switch off errors, need to be ACK'ed to stop sending error code.
	{
		case U5I0_SWITCH_OFF : channel = 0; break; //PO0
		case U5I1_SWITCH_OFF : channel = 1; break; //PO1
		case U6I0_SWITCH_OFF : channel = 2; break; //PO2
		case U6I1_SWITCH_OFF : channel = 3; break; //PO3
		case U7I0_SWITCH_OFF : channel = 4; break; //PO4
		case U7I1_SWITCH_OFF : channel = 5; break; //PO5
	}

	if ( channel != 255 ) // received error was assigned a channel.
	{

		sendPowerNodeErrReset( nodeid,  channel ); // clear error to stop node transmitting error state.
		for ( int i=0;PowerRequests[i].nodeid != 0;i++)
		{ // find the node record where error occurred.
			if ( PowerRequests[i].nodeid == nodeid )
			{
				if ( PowerRequests[i].error[channel] < 255 )
					PowerRequests[i].error[channel]++;
				setActualDevicePower( nodeid, channel, false );
				// keep a count of how many times an error has occurred.
		//		blinkOutput(LED, blinkingrate, time)
			}
		}

		return true; // handled error.
	}

	return false;
}


char PNodeStr[10] = "";

char * getPNodeStr( void )
{
	if ( PNodeStr[0] == 0) return "";
	else return PNodeStr;
}

void setPowerNodeStr( uint32_t nodesonline )
{
	PNodeStr[0] = 0;

	uint8_t pos = 0;
#ifndef HPF2023
	if ( PNodeAllBit & ( 0x1 << PNode33Bit ) )
	if ( !(nodesonline & ( 0x1 << PNode33Bit )) )
	{
		PNodeStr[pos] = '3';
		PNodeStr[pos+1] = '\0';
		pos++;
	}
#endif
	if ( PNodeAllBit & ( 0x1 << PNode34Bit ) )
	if ( !(nodesonline & ( 0x1 << PNode34Bit )) )
	{
		PNodeStr[pos] = '4';
		PNodeStr[pos+1] = '\0';
		pos++;
	}
#ifndef HPF2023
	if ( PNodeAllBit & ( 0x1 << PNode35Bit ) )
	if ( !(nodesonline & ( 0x1 << PNode35Bit )) )
	{
		PNodeStr[pos] = '5';
		PNodeStr[pos+1] = '\0';
		pos++;
	}


	if ( PNodeAllBit & ( 0x1 << PNode36Bit ) )
	if ( !(nodesonline & ( 0x1 << PNode36Bit )) )
	{
		PNodeStr[pos] = '6';
		PNodeStr[pos+1] = '\0';
		pos++;
	}
#endif
	if ( PNodeAllBit & ( 0x1 << PNode37Bit ) )
	if ( !(nodesonline & ( 0x1 << PNode37Bit )) )
	{
		PNodeStr[pos] = '7';
		PNodeStr[pos+1] = '\0';
		pos++;
	}
}


int setinitialdevicepower( void )
{
	return 0;
}

int average(int count, ...)
{
	return 0;
}

bool getNodeDevicePower(DevicePower device )
{
	for ( int i=0;DevicePowerList[i].device != None;i++)
	{
		if ( DevicePowerList[i].device == device )
			return DevicePowerList[i].actualstate;
	}

	// device not found
	return false;
}

bool getNodeDeviceExpectedPower(DevicePower device )
{
	for ( int i=0;DevicePowerList[i].device != None;i++)
	{
		if ( DevicePowerList[i].device == device )
			return DevicePowerList[i].expectedstate;
	}

	// device not found
	return false;
}


// Queue up power node requests to be sent.
bool setNodeDevicePower( DevicePower device, bool state, bool reset )
{
//	for ( int i=0;DevicePowerList[i].device != None;i++)
//	{

	int i = getPowerDeviceIndex( device );

	if ( DevicePowerList[i].device == device )
	{ // found the device in list, try to set request.
		DevicePowerList[i].expectedstate = state;

		if ( reset )
		{
			DevicePowerList[i].actualstate = state; // resetting internal state, so current expected actual state is false?
			DevicePowerList[i].waiting = false;
		}
		else
		{
			// check if we're already in the expected state.
			if ( DevicePowerList[i].expectedstate != DevicePowerList[i].actualstate )
				DevicePowerList[i].waiting = true;
			else
				DevicePowerList[i].waiting = false;
		}

		for ( int j=0;PowerRequests[j].nodeid != 0;j++) // search though the list of power nodes
		{
			if ( PowerRequests[j].nodeid == DevicePowerList[i].nodeid ) // check if node on list matches the wanted id.
			{
				// check there isn't a current error on the request.
				if ( PowerRequests[j].error[DevicePowerList[i].output] == 0 || reset)
				{
					if ( reset ) // we had a pending request, cancel it.
					{
						PowerRequests[j].error[DevicePowerList[i].output] = 0;
						PowerRequests[j].output &= ~(0x1 << DevicePowerList[i].output); // set enable output
						PowerRequests[j].state &= ~(0x1 << DevicePowerList[i].output); // set enable output
						return true;
					}

					// no error, can proceed with request.
					// check existing request queued for node..
					bool enabled = PowerRequests[j].output & (0x1 <<  DevicePowerList[i].output);
					if ( !enabled || // no request yet made
						( enabled && (PowerRequests[j].state & (0x1 << DevicePowerList[i].output) ) != state ) // request different to previously not processed request
					)
					{
						PowerRequests[j].output |= (0x1 << DevicePowerList[i].output); // set enable output
						if ( state == true )
							PowerRequests[j].state |=(0x1 << DevicePowerList[i].output); // set bit
						else
							PowerRequests[j].state &= ~(0x1 << DevicePowerList[i].output); // unset bit

						return true;

					} else
					{
						return false; // request already set
					}
				} return false; // error on channel, couldn't set
			}
		}
	}
//	}
	return false; // return if device was found and request set.
}

bool sendFanPWM( uint8_t leftduty, uint8_t rightduty )
{
	// minimum useful duty is 18/255

	// ensure we don't end up in a stall state
	if ( leftduty < 20 && leftduty > 0 ) leftduty = 20;
	if ( rightduty < 20 && rightduty > 0) rightduty = 20;

	uint8_t index = getPowerDeviceIndex(LeftFans);

	nodefanpwmreqs[0].nodeid = DevicePowerList[index].nodeid;
	nodefanpwmreqs[0].output = DevicePowerList[index].output;
	nodefanpwmreqs[0].dutycycle = leftduty;

	index = getPowerDeviceIndex(RightFans);

	nodefanpwmreqs[1].nodeid = DevicePowerList[index].nodeid;
	nodefanpwmreqs[1].output = DevicePowerList[index].output;
	nodefanpwmreqs[1].dutycycle = rightduty;

	queuedfanpwmLeft = true;
	queuedfanpwmRight = true;
	return true;
}

int sendPowerNodeErrReset( uint8_t id, uint8_t channel )
{
	uint8_t candata[8] = { id, 12, channel};
	CAN1Send(NodeCmd_ID, 3, candata );
	return 0;
}


int getPowerDeviceIndex( DevicePower device )
{
	int i = 0;
	for (;DevicePowerList[i].device != None; i++)
	{
		if ( DevicePowerList[i].device == device )
		{
			return i;
		}
	}
	return i;
}

// check and return if an power error has occured on a device.
uint32_t powerErrorOccurred( DevicePower device )
{
//	for ( int i=0;DevicePowerList[i].device != None; i++)
//	{
	int i = getPowerDeviceIndex( device );

	if ( DevicePowerList[i].device == device )
	{ // found the device in list, try to set request.

		for ( int j=0;PowerRequests[j].nodeid != 0;j++)
		{
			if ( PowerRequests[j].nodeid == DevicePowerList[i].nodeid )
			{
				return PowerRequests[j].error[DevicePowerList[i].output];
		//		if ( PowerRequests[j].error[DevicePowerList[i].output] )
		//			return true;
		//		else
		//			return false;
			}
		}
	}
//	}
	return 0;
}


int sendPowerNodeReq( void )
{
	uint8_t candata[8] = { 0, 1, 0, 0 };
	bool senderror = false;

	for ( int i=0;PowerRequests[i].nodeid;i++)
	{
		if ( PowerRequests[i].output != 0 )
		{
			candata[0] = PowerRequests[i].nodeid;
			candata[2] = PowerRequests[i].output;
			candata[3] = PowerRequests[i].state;
			if ( PowerRequests[i].nodeid != 36)
				CAN1Send(NodeCmd_ID, 8, candata );
			// don't reset request until we've seen an ack -> use ack handler to clear request.
			// give a small delay between can power request messages, to allow a tiny bit of leeway for some inrush maybe.
			vTaskDelay(1);
		}
	};

	if ( queuedfanpwmLeft || queuedfanpwmRight )
	{
		candata[1] = 2; // pwm set command.

		if ( nodefanpwmreqs[0].nodeid == nodefanpwmreqs[1].nodeid ) // both requests on same node.
		{
			candata[0] = nodefanpwmreqs[0].nodeid;
			candata[2] = (0x1 << nodefanpwmreqs[0].output) + (0x1 << nodefanpwmreqs[1].output);
			candata[3] = nodefanpwmreqs[0].dutycycle;
			candata[4] = nodefanpwmreqs[1].dutycycle;
			CAN1Send(NodeCmd_ID, 5, candata );
		} else // fans on different nodes
		{
			if ( queuedfanpwmLeft )
			{
				candata[0] = nodefanpwmreqs[0].nodeid;
				candata[2] = (0x1 << nodefanpwmreqs[0].output);
				candata[3] = nodefanpwmreqs[0].dutycycle;
				CAN1Send(NodeCmd_ID, 4, candata );
			}

			if ( queuedfanpwmRight )
			{
				candata[0] = nodefanpwmreqs[1].nodeid;
				candata[2] = (0x1 << nodefanpwmreqs[1].output);
				candata[3] = nodefanpwmreqs[1].dutycycle;
				CAN1Send(NodeCmd_ID, 4, candata );
			}
		}
	}

	return senderror;
}

uint8_t getDevicePowerListSize( void )
{
	int i = 0;
	for (;DevicePowerList[i].device != None;i++);
	devicecount = i;
	return i;
}

DevicePower getDevicePowerFromList( uint32_t i )
{
	if ( i <= devicecount )
		return DevicePowerList[i].device;
	else
		return None;
}

void resetPowerNodes( void )
{
	Shutdown.BOTS = false;
	Shutdown.InertiaSwitch = false;
	Shutdown.BSPDAfter = false;
	Shutdown.BSPDBefore = false;
	Shutdown.CockpitButton = false;
	Shutdown.LeftButton = false;
	Shutdown.RightButton = false;

	Shutdown.BMS = false;
	Shutdown.BMSReason = false;
	Shutdown.IMD = false;
	Shutdown.AIRm = false;
	Shutdown.AIRp = false;
	Shutdown.PRE = false;

// reset errors

	PowerNodeErrorCount = 0;

}

int initPowerNodes( void )
{

	RegisterResetCommand(resetPowerNodes);

	resetPowerNodes();
#ifndef HPF2023
	RegisterCan1Message(&PowerNode33);
#endif
	RegisterCan1Message(&PowerNode34);
	RegisterCan1Message(&PowerNode35);
	RegisterCan2Message(&PowerNode36);
	RegisterCan1Message(&PowerNode37);

	return 0;
}
