/*
 * imu.c
 *
 *  Created on: 28 Jul 2020
 *      Author: Visa
 */

#include "ecumain.h"
#include "imu.h"

bool processIMUStatus( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIMUUTC( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIMUInfo( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIMUAccel( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIMUGyro( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIMUDeltaV( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIMUDeltaA( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIMUEuler( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIMUVel( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
//bool processIMUVelAcc( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIMUVelBody(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processIMUGPS( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );


void IMUTimeout( uint16_t id );

// System
CANData IMUStatus = { &DeviceState.IMU, IMUBase_ID, 6, processIMUStatus, IMUTimeout, IMUTIMEOUT };
CANData IMUUTC = { &DeviceState.IMU, IMUUTC_ID, 8, processIMUUTC, IMUTimeout, IMUTIMEOUT };

//IMU:
CANData IMUInfo = { &DeviceState.IMU, IMUInfo_ID, 8, processIMUInfo, IMUTimeout, IMUTIMEOUT };
CANData IMUAccel = { &DeviceState.IMU, IMUAccel_ID, 6, processIMUAccel, IMUTimeout, IMUTIMEOUT };
CANData IMUGyro = { &DeviceState.IMU, IMUGyro_ID, 6, processIMUGyro, IMUTimeout, IMUTIMEOUT };
CANData IMUDeltaV = { &DeviceState.IMU, IMUDeltaV_ID, 6, processIMUDeltaV, IMUTimeout, IMUTIMEOUT };
CANData IMUDeltaA = { &DeviceState.IMU, IMUDeltaA_ID, 6, processIMUDeltaA, IMUTimeout, IMUTIMEOUT };

//EKF:

CANData IMUEuler = { &DeviceState.IMU, IMUEuler_ID, 6, processIMUEuler, IMUTimeout, IMUTIMEOUT };
CANData IMUVel = { &DeviceState.IMU, IMUVel_ID, 6, processIMUVel, IMUTimeout, IMUTIMEOUT };
//CANData IMUVelAcc = { &DeviceState.IMU, IMUVelAcc_ID, 6, processIMUVelAcc, IMUTimeout, IMUTIMEOUT };
CANData IMUVelBody = { &DeviceState.IMU, IMUVELBody_ID, 8, processIMUVelBody, IMUTimeout, IMUTIMEOUT };
CANData IMUGPS = { &DeviceState.IMU, IMUGPS_ID, 8, processIMUGPS, IMUTimeout, IMUTIMEOUT };
// gps retursn all FF when no lock.
//CANData IMUAUTO = { &DeviceState.IMU, IMUAUTO_ID, 8, processIMUAUTO, IMUTimeout, IMUTIMEOUT };

enum { COUNTER_BASE = __COUNTER__ };

#define LOCAL_COUNTER (__COUNTER__ - COUNTER_BASE)

IMUData IMUReceived;

bool processIMUStatus( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
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

#ifdef IMUNONUSED
bool processIMUUTC( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
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
#endif

bool processIMUInfo( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
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

bool processIMUAccel( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
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

bool processIMUGyro( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
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

bool processIMUDeltaV( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
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

bool processIMUDeltaA( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
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

bool processIMUEuler( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
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

bool processIMUVel( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
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
bool processIMUVelAcc( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
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

bool processIMUVelBody(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
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


bool processIMUGPS( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
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

bool processIMUAUTO( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle )
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
	return receivedCANData(&IMUStatus);
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
	RegisterResetCommand(resetIMU);

	resetIMU();

#ifdef IMUNONUSED
	RegisterCan2Message(&IMUStatus);
	RegisterCan2Message(&IMUUTC);
#endif
	RegisterCan2Message(&IMUInfo);
	RegisterCan2Message(&IMUAccel);
	RegisterCan2Message(&IMUGyro);

	RegisterCan2Message(&IMUEuler);
	RegisterCan2Message(&IMUDeltaV);
	RegisterCan2Message(&IMUDeltaA);

	RegisterCan2Message(&IMUVel);
#ifdef IMUNONUSED
	RegisterCan2Message(&IMUVelAcc);
#endif

	RegisterCan2Message(&IMUVelBody);
	RegisterCan2Message(&IMUGPS);
	return 0;
}

