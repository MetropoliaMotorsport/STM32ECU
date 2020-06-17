/*
 * ecu.c
 *
 *  Created on: 30 Dec 2018
 *      Author: Visa
 */

#include "ecumain.h"

#ifdef LCD
  #include "vhd44780.h"
#endif

/** deal with endianness for canbus
 * function from https://stackoverflow.com/questions/39622332/reading-big-endian-files-in-little-endian-system
 * not actually being currently used
 */
void swapByteOrder_int16(double *current, const int16_t *rawsignal, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        int16_t x = rawsignal[2*i];
        x = (x*1u << 8) | (x*1u >> 8);
        current[i] = x;
    }
}

void RearSpeedCalculation( long leftdata, long rightdata )
{
/*	CarState.Wheel_Speed_Right_Calculated = Speed_Right_Inverter.data.longint * (1/4194304) * 60; // resolution 4194304 for one revolution
	CarState.Wheel_Speed_Left_Calculated = Speed_Left_Inverter.data.longint * (1/4194304) * 60;
	CarState.Wheel_Speed_Rear_Average = (CarState.Wheel_Speed_Right_Calculated  + CarState.Wheel_Speed_Left_Calculated)/2;
*/
}

uint8_t getByte(uint32_t input, int8_t returnbyte)
{
	union {
		uint32_t integer;
		unsigned char bytearray[4];
	} data;
	data.integer = input;
	return data.bytearray[returnbyte];
}


void storeBEint32(uint32_t input, uint8_t Data[4])
{
	Data[0] = getByte(input,3);
	Data[1] = getByte(input,2);
	Data[2] = getByte(input,1);
	Data[3] = getByte(input,0);
}


void storeBEint16(uint16_t input, uint8_t Data[2])
{
	Data[0] = getByte(input,1);
	Data[1] = getByte(input,0);
}


void storeLEint32(uint32_t input, uint8_t Data[4])
{
	Data[0] = getByte(input,0);
	Data[1] = getByte(input,1);
	Data[2] = getByte(input,2);
	Data[3] = getByte(input,3);
}


void storeLEint16(uint16_t input, uint8_t Data[2])
{
	Data[0] = getByte(input,0);
	Data[1] = getByte(input,1);
}
