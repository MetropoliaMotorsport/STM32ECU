/*
 * ecu.c
 *
 *  Created on: 30 Dec 2018
 *      Author: Visa
 */

#include "ecumain.h"

CANData ECUCAN = { NULL, 21, 8, NULL, NULL, 0 };


inline uint32_t getLEint32( const uint8_t data[4] )
{
  return (data[3]<<24)+(data[2]<<16)+(data[1]<<8)+data[0];
}

inline uint16_t getLEint16( const uint8_t data[2] )
{
  return (data[1]<<8)+data[0];
}

inline uint32_t getBEint32( const uint8_t data[4] )
{
  return (data[0]<<24)+(data[1]<<16)+(data[2]<<8)+data[3];
}

inline uint16_t getBEint16( const uint8_t data[2] )
{
  return (data[0]<<8)+data[1];
}

inline uint8_t getByte( const uint32_t input, const int8_t returnbyte )
{
	union {
		uint32_t integer;
		unsigned char bytearray[4];
	} data;
	data.integer = input;
	return data.bytearray[returnbyte];
}


inline void storeBEint32( const uint32_t input, uint8_t Data[4] )
{
	Data[0] = getByte(input,3);
	Data[1] = getByte(input,2);
	Data[2] = getByte(input,1);
	Data[3] = getByte(input,0);
}


inline void storeBEint16( const uint16_t input, uint8_t Data[2] )
{
	Data[0] = getByte(input,1);
	Data[1] = getByte(input,0);
}


inline void storeLEint32( const uint32_t input, uint8_t Data[4] )
{
	Data[0] = getByte(input,0);
	Data[1] = getByte(input,1);
	Data[2] = getByte(input,2);
	Data[3] = getByte(input,3);
}


inline void storeLEint16( const uint16_t input, uint8_t Data[2] )
{
	Data[0] = getByte(input,0);
	Data[1] = getByte(input,1);
}


char * getDeviceStatusStr( const DeviceStatus status )
{
	switch ( status )
	{
	case INERROR : return "Error";
	case OFFLINE : return "Offline";
	case BOOTUP : return "Bootup";
	case STOPPED : return "Stopped";
	case PREOPERATIONAL : return "PreOperational";
	case OPERATIONAL : return "Operational";
	default:
		return "BADSTATUS";
	}
}


int initECU( void )
{
	RegisterCan1Message(&ECUCAN);
	return 0;
}
