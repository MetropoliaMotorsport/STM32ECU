/*
 * ivt.c
 *
 *  Created on: 01 May 2019
 *      Author: Visa
 */

// 1041 ( 411h )  52 1 0 0 -< turn on

// 1041 ( 411h )  49 0 175 0<- trigger message


#include "ecumain.h"
#include "ivt.h"
#include "timerecu.h"
#include "errors.h"
#include "eeprom.h"

uint8_t LastIVTI[6] = {0,0,0,0,0,0};
uint8_t LastIVTU1[6] = {0,0,0,0,0,0};
uint8_t LastIVTU2[6] = {0,0,0,0,0,0};
uint8_t LastIVTU3[6] = {0,0,0,0,0,0};
uint8_t LastIVTW[6] = {0,0,0,0,0,0};

bool processIVTData( const uint8_t * CANRxData, const uint32_t DataLength, const uint16_t field );

bool processIVTMsgData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

bool processIVTIData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIVTU1Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle);
bool processIVTU2Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIVTU3Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIVTTData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIVTWData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIVTAsData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIVTWhData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );


void IVTTimeout( uint16_t id );

char CAN_SendIVTTurnon( void );
char CAN_SendIVTTrigger( void );

CANData IVTMsg=	{ &DeviceState.IVT, IVTMsg_ID, 6, processIVTMsgData, NULL, 0 };

CANData IVTCan[8] = {
	{ &DeviceState.IVT,	IVTBase_ID,   6, processIVTIData, IVTTimeout, IVTTIMEOUT },
	{ &DeviceState.IVT, IVTBase_ID+1, 6, processIVTU1Data, NULL, 0 },
	{ &DeviceState.IVT, IVTBase_ID+2, 6, processIVTU2Data, NULL, 0 },
	{ &DeviceState.IVT, IVTBase_ID+3, 6, processIVTU3Data, NULL, 0 },
	{ &DeviceState.IVT, IVTBase_ID+4, 6, processIVTTData,  NULL, 0 },
	{ &DeviceState.IVT, IVTBase_ID+5, 6, processIVTWData,  NULL, 0 },
	{ &DeviceState.IVT, IVTBase_ID+6, 6, processIVTAsData, NULL, 0 },
	{ &DeviceState.IVT, IVTBase_ID+7, 6, processIVTWhData, NULL, 0 }
};

uint8_t processIVT(uint8_t CANRxData[8], uint32_t DataLength, uint16_t field )
{
	CANData * datahandle = &IVTCan[field - IVTBase_ID];
	processCANData(datahandle, CANRxData, DataLength );
	return 0;
}


// IVTWh = 0x528
bool processIVTIData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	if ( DeviceState.IVTEnabled )
	{
		return processIVTData( CANRxData, DataLength, IVTI_ID);
	} else return true;
}

bool processIVTU1Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	if ( DeviceState.IVTEnabled )
	{
		return processIVTData( CANRxData, DataLength, IVTU1_ID);
	} else return true;
}

bool processIVTU2Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	if ( DeviceState.IVTEnabled )
	{
		return processIVTData( CANRxData, DataLength, IVTU2_ID);
	} else return true;
}

bool processIVTU3Data( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	if ( DeviceState.IVTEnabled )
	{
		return processIVTData( CANRxData, DataLength, IVTU3_ID);
	} else return true;
}

bool processIVTTData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	if ( DeviceState.IVTEnabled )
	{
		return processIVTData( CANRxData, DataLength, IVTT_ID);
	} else return true;
}

bool processIVTWData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	if ( DeviceState.IVTEnabled )
	{
		return processIVTData( CANRxData, DataLength, IVTW_ID);
	} else return true;
}

bool processIVTAsData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	if ( DeviceState.IVTEnabled )
	{
		return processIVTData( CANRxData, DataLength, IVTAs_ID);
	} else return true;
}

bool processIVTWhData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	if ( DeviceState.IVTEnabled )
	{
		return processIVTData( CANRxData, DataLength, IVTWh_ID);
	} else return true;
}



bool processIVTData( const uint8_t * CANRxData, const uint32_t DataLength, const uint16_t field )
{
	if ( DeviceState.IVTEnabled )
	{
		long value = CANRxData[2]*16777216+ CANRxData[3]*65536+ CANRxData[4]*256+ CANRxData[5];

		uint8_t RxOK[7] = {1,1,1,1,1,1,1};

		IVTCan[field - IVTBase_ID].time = gettimer();

		if ( DataLength == FDCAN_DLC_BYTES_6) RxOK[6] = 0;

		if ( CANRxData[1] < 16 ) RxOK[1] = 0;

		switch ( field )
		{
	//		IVTMsg_ID : ;
			case IVTI_ID :
				value = value /1000;
				//	if ( value < 10000000)
				{ RxOK[2] = 0;  RxOK[3] = 0;  RxOK[4] = 0;  RxOK[5] = 0; };
				if ( CANRxData[0] == 0 )
				  RxOK[0] = 0;
				break;
			case IVTU1_ID :
				value = value /1000;
				// if ( value > 20 && value < 600 )
				{ RxOK[2] = 0;  RxOK[3] = 0;  RxOK[4] = 0;  RxOK[5] = 0; };
				if ( CANRxData[0] ==1 )
					RxOK[0] = 0;
				break;
			case IVTU2_ID :
				value = value /1000;
				// if ( value > 20 && value < 600 )
				{ RxOK[2] = 0;  RxOK[3] = 0;  RxOK[4] = 0;  RxOK[5] = 0; }
				if ( CANRxData[0] == 2 )
					RxOK[0] = 0;
				break;
			case IVTU3_ID :
				value = value /1000;
				//	if ( value > 20 && value < 600 )
				{ RxOK[2] = 0;  RxOK[3] = 0;  RxOK[4] = 0;  RxOK[5] = 0; }
				if ( CANRxData[0] == 3 )
					RxOK[0] = 0;
				break;
	//		case case IVTT_ID :
	//			if ( value 0 ) { RxOK[2] = 0;  RxOK[3] = 0;  RxOK[4] = 0;  RxOK[5] = 0; } break;
			case IVTW_ID :
				//if ( value < 500000 )
				{ RxOK[2] = 0;  RxOK[3] = 0;  RxOK[4] = 0;  RxOK[5] = 0; }
				if ( CANRxData[0] == 5 )
					RxOK[0] = 0;
				break;
	//		case IVTAs_ID :
	//			if ( value ) { RxOK[2] = 0;  RxOK[3] = 0;  RxOK[4] = 0;  RxOK[5] = 0; }
	//			break;
			case IVTWh_ID : if ( value < 100000)
				{ RxOK[2] = 0;  RxOK[3] = 0;  RxOK[4] = 0;  RxOK[5] = 0; }
				if ( CANRxData[0] == 7 ) // TODO check value
					RxOK[0] = 0;
				break;

			default :
				break;
		}

		uint8_t proceed=0;

		for ( int i=0;i<7;i++)
		{
			if ( RxOK[i] == 0 ) proceed++;
		}

		if ( proceed == 7 ) // packet passed basic data checks.
		{
			switch ( field ) // reset error data.
			{
		//		IVTMsg_ID : ;
				case IVTI_ID :
		//			Errors.IVTIReceive = 0;
					DeviceState.IVT = OPERATIONAL;
					if ( value < runtimedata_p->maxIVTI )
					{
						runtimedata_p->maxIVTI = value;
						runtimedata_p->time = getTime();
					}
					break;
			}

			switch ( field ) // set state data.
			{
		//		IVTMsg_ID : ;
				case IVTI_ID :
					CarState.Current = value;
					for ( int i=0;i<6;i++)
					{
						LastIVTI[i] = CANRxData[i];
					}
					break;
				case IVTU1_ID :
#ifdef HPF2023
					CarState.VoltageIVTAccu = value;
#else
					CarState.VoltageINV = value;
#endif

					for ( int i=0;i<6;i++)
					{
						LastIVTU1[i] = CANRxData[i];
					}
					break;
				case IVTU2_ID :
#ifdef HPF2023
					CarState.VoltageINV = value;
#else
					CarState.VoltageIVTAccu = value;
#endif


					for ( int i=0;i<6;i++)
					{
						LastIVTU2[i] = CANRxData[i];
					}
					break;
		//		case IVTU3_ID : break;
		//		case case IVTT_ID : break;
				case IVTW_ID :
					CarState.Power = value;
					for ( int i = 0;i<6;i++)
					{
						LastIVTW[i] = CANRxData[i];
					}
					break;
		//		case IVTAs_ID : break;
				case IVTWh_ID :
					CarState.Wh = value;
					break;
			}
            
#ifdef retransmitIVT
            reTransmitOnCan1(field,CANRxData,DataLength);
#endif
			return true;

		} return false;
	} return true;
}


void IVTTimeout( uint16_t id )
{
#ifdef errorLED
    blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif
	CarState.Power=0;
	CarState.Current=0;
	CarState.VoltageINV=0;

    SetCriticalError(CRITERIVT);
}

int receiveIVT( void )
{
	if ( DeviceState.IVTEnabled )
	{

#ifdef NOIVTTIMEOUT
			if ( *IVT[0].devicestate == OFFLINE )
			{
				// retransmit last messages here to keep BMS operating if IVT Lost.

				 reTransmitOnCan1(IVTI_ID,LastIVTI,FDCAN_DLC_BYTES_6);
				 reTransmitOnCan1(IVTU1_ID,LastIVTU1,FDCAN_DLC_BYTES_6);
				 reTransmitOnCan1(IVTU2_ID,LastIVTU2,FDCAN_DLC_BYTES_6);
			}
			returnval = 1; // never time out, just set data non operational.
#else
			int returnval = receivedCANData(&IVTCan[0]);
#endif
		return returnval;
	} else // IVT reading disabled, set 'default' values to allow operation regardless.
	{
		*IVTCan[0].devicestate = OPERATIONAL;
		CarState.VoltageINV=540; // set an assumed voltage that forces TSOFF indicator to go out on timeout for SCS.
		CarState.VoltageIVTAccu=540;
		CarState.Power=0;
		CarState.Current=0;
		return 1;
	}
}

int requestIVT( void )
{
	CAN_SendIVTTrigger();
	return 0; // this is operating with , no extra needed.
}


bool processIVTMsgData( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
{
	return true;
}

int IVTstate( void ) // not currently being used, not properly functional
{
	if ( IVTMsg.time > 0 ) // packet has been received
	{
//		if ( CanState.IVTMsg.data[0] == 0xBF )
		{
		    DeviceState.IVT = OPERATIONAL;
		}
		return 1;

	} else if (gettimer() - IVTCan[IVTBase_ID-IVTU1_ID].time < IVTTIMEOUT )
	{
		return 1;
	} else
	return 0;
}


char CAN_SendIVTTrigger( void )
{
	uint8_t CANTxData[8] = { 49, 0, 175,0,0,0,0,0 };
	return CAN1Send( IVTCmd_ID, 8, CANTxData );
}


char CAN_SendIVTTurnon( void )
{
	uint8_t CANTxData[8] = { 52, 1,0,0,0,0,0,0 }; // turn on from pre operation.
	return CAN1Send( IVTCmd_ID, 8, CANTxData );
}


void resetIVT( void )
{
#ifdef IVTEnable
	DeviceState.IVTEnabled = ENABLED;
#else
	DeviceState.IVTEnabled = DISABLED;
#endif

	DeviceState.IVT = OFFLINE;
	CarState.Power=0;
	CarState.Current=0;
}

int initIVT( void )
{
	RegisterResetCommand(resetIVT);

	resetIVT();

	for ( int i=0;i<8;i++)
#if IVT_BUS == CANB1
		RegisterCan1Message(&IVTCan[i]);
#else
		RegisterCan2Message(&IVTCan[i]);
#endif
	return 0;
}
