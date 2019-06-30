/*
 * ivt.c
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

// 1041 ( 411h )  52 1 0 0 -< turn on

// 1041 ( 411h )  49 0 175 0<- trigger message


#include "ecumain.h"

uint8_t LastIVTI[6] = {0,0,0,0,0,0};
uint8_t LastIVTU1[6] = {0,0,0,0,0,0};
uint8_t LastIVTU2[6] = {0,0,0,0,0,0};
uint8_t LastIVTU3[6] = {0,0,0,0,0,0};
uint8_t LastIVTW[6] = {0,0,0,0,0,0};

// IVTWh = 0x528

uint8_t processIVT(uint8_t CANRxData[8], uint32_t DataLength, uint16_t field )
{
//	static uint8_t receiveerror[8] = {0,0,0,0,0,0,0,0};


	if ( DeviceState.IVTEnabled )
	{
		long value = CANRxData[2]*16777216+ CANRxData[3]*65536+ CANRxData[4]*256+ CANRxData[5];

		uint8_t RxOK[7] = {1,1,1,1,1,1,1};

		CanState.IVT[field - IVTBase_ID].time = gettimer();

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
				if ( CANRxData[0] == 7 )
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
					break;
			}

		//	receiveerror = 0;

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
					CarState.VoltageINV = value;
					for ( int i=0;i<6;i++)
					{
						LastIVTU1[i] = CANRxData[i];
					}
					break;
				case IVTU2_ID :
					CarState.VoltageIVTAccu = value;
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
				case IVTWh_ID : break;
			}
            
#ifdef retransmitIVT
            reTransmitOnCan1(field,CANRxData,DataLength);
#endif

			return 1;

		} else // bad data.
		{
	//		receiveerror++;
			Errors.CANError++;

			switch ( field ) // set state data.
			{
		//		IVTMsg_ID : ;
				case IVTI_ID : Errors.IVTIReceive++; break;
				case IVTU1_ID : Errors.IVTU1Receive++; break;
				case IVTU2_ID : Errors.IVTU2Receive++; break;
		//		case IVTU3_ID : break;
		//		case case IVTT_ID : break;
				case IVTW_ID : Errors.IVTWReceive++; break;
				case IVTAs_ID : break;
				case IVTWh_ID : break;
			}


	/*		if ( receiveerror > 10 ), device is still responding, so don't put it offline.
			{
				CarState.VoltageBMS=0;
				DeviceState.BMS = OFFLINE;
				return 0; // returnval = 0;
			} */
#ifdef SENDBADDATAERROR
			CAN_SendErrorStatus(99,IVTReceived+110+field-IVTBase_ID,99);
#endif
			reTransmitError(99,CANRxData, DataLength >> 16); // put into a retransmit queue?
			return 0; // bad data received, but still heard from device device.
		}

	} else return 1;
}


int receiveIVT( void )
{
	if ( DeviceState.IVTEnabled )
	{
		uint32_t time=gettimer();
		static uint8_t errorsent = 0;

#ifdef NOTIMEOUT
		if ( DeviceState.IVT == OPERATIONAL ) // 5 second timeout should be enough.
		{
			errorsent = 0;
			return 1;
		} else return 0;
#endif
        
		if ( time - CanState.IVT[0].time <= IVTTIMEOUT && DeviceState.IVT == OPERATIONAL )
		{
			errorsent = 0;  // IVT seen within timeout.
			return 1;
		} else
		{

			if ( DeviceState.IVT == OPERATIONAL )
			{
#ifdef errorLED
                blinkOutput(IMDLED_Output,LEDBLINK_FOUR,255);
#endif
				if ( errorsent == 0 )
				{
					CAN_SendErrorStatus(200,IVTReceived+110,(time-CanState.IVT[0].time+IVTTIMEOUT)/10);
					errorsent = 1;
					Errors.CANTimeout++;
					Errors.IVTTimeout++;
					DeviceState.IVT = OFFLINE;
            		CarState.Power=0;
            		CarState.Current=0;
				}

			}

#ifdef NOIVTTIMEOUT
			if ( DeviceState.IVT == OFFLINE )
			{
				// retransmit last messages here to keep BMS operating if IVT Lost.

				 reTransmitOnCan1(IVTI_ID,LastIVTI,FDCAN_DLC_BYTES_6);
				 reTransmitOnCan1(IVTU1_ID,LastIVTU1,FDCAN_DLC_BYTES_6);
				 reTransmitOnCan1(IVTU2_ID,LastIVTU2,FDCAN_DLC_BYTES_6);
			}
			return 1; // never time out, just set data non operational.
#else
			return 0;
#endif
			 // time out
	//
		}
	//	return 0;
	} else // IVT reading disabled, set 'default' values to allow operation regardless.
	{
		DeviceState.IVT = OPERATIONAL;
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

int IVTstate( void ) // not currently being used, not properly functional
{
	if ( CanState.IVTMsg.time > 0 ) // packet has been received
	{
		if ( CanState.IVTMsg.data[0] == 0xBF )
		{
		    DeviceState.IVT = OPERATIONAL;
		}
		return 1;

	} else if (gettimer() - CanState.IVTU1.time < IVTTIMEOUT )
	{
		return 1;
	} else
	return 0;
}

