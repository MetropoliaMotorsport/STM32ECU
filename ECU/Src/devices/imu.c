/*
 * imu.c
 *
 *  Created on: 28 Jul 2020
 *      Author: Visa
 */

#include "ecumain.h"


void IMUTimeout( uint16_t id );

enum { COUNTER_BASE = __COUNTER__ };

#define LOCAL_COUNTER (__COUNTER__ - COUNTER_BASE)

IMUData IMUReceived;

inline uint32_t getLEint32( uint8_t data[4] )
{
  return data[3]*16777216+data[2]*65536+data[1]*256+data[0];
}

inline uint16_t getLEint16( uint8_t data[2] )
{
  return data[1]*256+data[0];
}

inline uint32_t getBEint32( uint8_t data[4] )
{
  return data[0]*16777216+data[1]*65536+data[2]*256+data[3];
}

inline uint16_t getBEint16( uint8_t data[2] )
{
  return data[0]*256+data[1];
}


bool processIMUStatus(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
  if ( true // verify data format.
  )
  {
	  IMUReceived.SolutionStatus =  getLEint32(&CANRxData[0]); // 0x102
	  IMUReceived.HeaveStatus = getLEint16(&CANRxData[4]);

//	  IMUReceived.Received &= ~(0x1 << LOCAL_COUNTER);
	  return true;
  }
  else
	return false;
}

bool processIMUUTC(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
  if ( true // verify data format.
  )
  {
	  IMUReceived.Year = CANRxData[0];
	  IMUReceived.Month = CANRxData[1];
	  IMUReceived.Day = CANRxData[2];
	  IMUReceived.Hour = CANRxData[3];
	  IMUReceived.Min = CANRxData[4];
	  IMUReceived.Sec = CANRxData[5];
	  IMUReceived.tenthsms = getLEint16(&CANRxData[6]);

	  IMUReceived.Received &= ~(0x1 << LOCAL_COUNTER);
	  return true;
  }
  else
	return false;
}

bool processIMUInfo(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
  if ( true // verify data format.
  )
  {
		//0x120
	  IMUReceived.TimeStamp = getLEint32(&CANRxData[0]);
	  IMUReceived.IMU_Status = getLEint16(&CANRxData[4]);
	  IMUReceived.Temperature = getLEint32(&CANRxData[6]);

	  IMUReceived.Received &= ~(0x1 << LOCAL_COUNTER);
	  return true;
  }
  else
	return false;
}

bool processIMUAccel(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
  if ( true // verify data format.
  )
  {
		//0x121
	  IMUReceived.AccelX = getLEint16(&CANRxData[0]);
	  IMUReceived.AccelY = getLEint16(&CANRxData[2]);
	  IMUReceived.AccelZ = getLEint16(&CANRxData[4]);

	  IMUReceived.Received &= ~(0x1 << LOCAL_COUNTER);
	  return true;
  }
  else
	return false;
}

bool processIMUGyro(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
  if ( true // verify data format.
  )
  {
	  IMUReceived.GyroX = getLEint16(&CANRxData[0]);
	  IMUReceived.GyroY = getLEint16(&CANRxData[2]);
	  IMUReceived.GyroZ = getLEint16(&CANRxData[4]);

	  IMUReceived.Received &= ~(0x1 << LOCAL_COUNTER);
	  return true;
  }
  else
	return false;
}

bool processIMUDeltaV(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
  if ( true // verify data format.
  )
  {
	  IMUReceived.DeltaVX = getLEint16(&CANRxData[0]);
	  IMUReceived.DeltaVY = getLEint16(&CANRxData[2]);
	  IMUReceived.DeltaVZ = getLEint16(&CANRxData[4]);

	  IMUReceived.Received &= ~(0x1 << LOCAL_COUNTER);
	  return true;
  }
  else
	return false;
}

bool processIMUDeltaA(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
  if ( true // verify data format.
  )
  {
	  IMUReceived.DeltaAX = getLEint16(&CANRxData[0]);
	  IMUReceived.DeltaAY = getLEint16(&CANRxData[2]);
	  IMUReceived.DeltaAZ = getLEint16(&CANRxData[4]);

	  IMUReceived.Received &= ~(0x1 << LOCAL_COUNTER);
	  return true;
  }
  else
	return false;
}

bool processIMUEuler(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
  if ( true // verify data format.
  )
  {
		//0x137
	  IMUReceived.Yaw = getLEint16(&CANRxData[4]);
	  IMUReceived.Received &= ~(0x1 << LOCAL_COUNTER);
	  return true;
  }
  else
	return false;
}

bool processIMUVel(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
  if ( true // verify data format.
  )
  {
		//0x137
	  IMUReceived.VelN = getLEint16(&CANRxData[0]);
	  IMUReceived.VelE = getLEint16(&CANRxData[2]);
	  IMUReceived.VelD = getLEint16(&CANRxData[4]);

	  IMUReceived.Received &= ~(0x1 << LOCAL_COUNTER);
	  return true;
  }
  else
	return false;
}

#ifdef IMUNONUSED
bool processIMUVelAcc(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
  if ( true // verify data format.
  )
  {
	  IMUReceived.VelAccN = getLEint16(&CANRxData[0]);
	  IMUReceived.VelAccE = getLEint16(&CANRxData[2]);
	  IMUReceived.VelAccD = getLEint16(&CANRxData[4]);

	  IMUReceived.Received &= ~(0x1 << LOCAL_COUNTER);
	  return true;
  }
  else
	return false;
}
#endif


bool processIMUVelBody(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
  if ( true // verify data format.
  )
  {
	  IMUReceived.VelBodyX = getLEint16(&CANRxData[0]);
	  IMUReceived.VelBodyY = getLEint16(&CANRxData[2]);
	  IMUReceived.VelBodyZ = getLEint16(&CANRxData[4]);

	  IMUReceived.Received &= ~(0x1 << LOCAL_COUNTER);
	  return true;
  }
  else
	return false;
}


bool processIMUGPS(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
  if ( true // verify data format.
  )
  {
		//0x177
	  IMUReceived.Lat_Accur = getLEint16(&CANRxData[0]); // *10^-2 m
	  IMUReceived.LONG_Accur = getLEint16(&CANRxData[2]); // *10^-2
	  IMUReceived.ALT_Accur = getLEint16(&CANRxData[4]); // *10^-2
//		uint16_t BASE_STATION_ID;

	  IMUReceived.Received &= ~(0x1 << LOCAL_COUNTER);
	  return true;
  }
  else
	return false;
}

bool processIMUAUTO(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle )
{
  if ( true // verify data format.
  )
  {
		//0x177
	  IMUReceived.Lat_Accur = getLEint16(&CANRxData[0]); // *10^-2 m
	  IMUReceived.LONG_Accur = getLEint16(&CANRxData[2]); // *10^-2
	  IMUReceived.ALT_Accur = getLEint16(&CANRxData[4]); // *10^-2
//		uint16_t BASE_STATION_ID;

	  IMUReceived.Received &= ~(0x1 << LOCAL_COUNTER);
	  return true;
  }
  else
	return false;
}



void IMUTimeout( uint16_t id )
{

}


int receiveIMU( void )
{
//	return receivedCANData(&IMUStatus);
	return 0;
}


int requestIMU( int nodeid )
{
	return 0; // this is operating cyclically, no extra request needed.
}

int returnCounter( void )
{
	return  LOCAL_COUNTER;
}

void resetIMU( void )
{
//	IMUData temp = {0};
//	volatile size= sizeof( IMUData );
	memset( (void *) &IMUReceived, 0, sizeof( IMUData ) );
}

int initIMU( void )
{
	resetIMU();

	return 0;
}

