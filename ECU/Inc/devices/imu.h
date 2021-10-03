/*
 * imu.h
 *
 *  Created on: 28 Jul 2020
 *      Author: Visa
 */

#ifndef IMU_H_
#define IMU_H_



typedef volatile struct IMUDataType {
	uint16_t Received;

	uint32_t SolutionStatus; // 0x102
	uint16_t HeaveStatus;

	//0x100  being sent
	//0x101
	//0x110

	//0x170
	//0x171

	//0x111

	uint8_t Year;
	uint8_t Month;
	uint8_t Day;
	uint8_t Hour;
	uint8_t Min;
	uint8_t Sec;
	uint16_t tenthsms; // *10^-4

	//0x120
	uint32_t TimeStamp; // us
	uint16_t IMU_Status; // status bitmask
	int16_t Temperature; // *10^-2 C

	//0x121
	int16_t AccelX; // *10^-2 m/s^2
	int16_t AccelY; // *10^-2
	int16_t AccelZ; // *10^-2

	//0x122
	int16_t GyroX; // *10^-3 // rad/s
	int16_t GyroY; // *10^-3
	int16_t GyroZ; // *10^-3 // yaw rate?

	//0x123
	int16_t DeltaVX; // *10^-2 m/s^2
	int16_t DeltaVY; // *10^-2
	int16_t DeltaVZ; // *10^-2

	//0x124
	int16_t DeltaAX; // *10^-3 // rad/s
	int16_t DeltaAY; // *10^-3
	int16_t DeltaAZ; // *10^-3

	//0x132
	int16_t Yaw; //*10^-4 // rad

	//0x137
	int16_t VelN; // *10^-2 m/s  North
	int16_t VelE; // *10^-2		 East
	int16_t VelD; // *10^-2		 Down

	//0x138
	int16_t VelAccN; // *10^-2 m/s^2   North   -- not being sent
	int16_t VelAccE; // *10^-2		    East
	int16_t VelAccD; // *10^-2			Down

	//0x139
	int16_t VelBodyX; // *10^-2 m/s
	int16_t VelBodyY; // *10^-2 m/s
	int16_t VelBodyZ; // *10^-2 m/s

	//0x177
	uint16_t Lat_Accur; // *10^-2 m
	uint16_t LONG_Accur; // *10^-2
	uint16_t ALT_Accur; // *10^-2
	uint16_t BASE_STATION_ID;

	//0x220
	int16_t Angle_Track; // *10^-4 rad
	int16_t Angle_Slip; // *10^-4 rad
	uint16_t Curvature_Radius; //10^-2 m
	uint8_t Auto_Status;
	// bit 0 SBG_ECOM_AUTO_TRACK_VALID Set to 1 if the track angle is valid
	// bit 1 SBG_ECOM_AUTO_SLIP_VALID Set to 1 if the slip angle is valid
	// bit 2 SBG_ECOM_AUTO_CURVATURE_VALID Set to 1 if the curvature radius is valid

} IMUData;


extern IMUData IMUReceived;

#define IMUBase_ID 				(0x102)
#define IMUUTC_ID 				(0x111)
#define IMUInfo_ID 				(0x120)
#define IMUAccel_ID				(0x121)
#define IMUGyro_ID				(0x122)
#define IMUDeltaV_ID			(0x123)
#define IMUDeltaA_ID			(0x124)
#define IMUEuler_ID				(0x132)
#define IMUVel_ID				(0x137)
#define IMUVelAcc_ID			(0x138)
#define IMUVELBody_ID			(0x139)
#define IMUGPS_ID				(0x177)
#define IMUAUTO_ID				(0x220)

// System
extern CANData IMUStatus;//	0x00000102
extern CANData IMUUTC;	//	0x00000111

//IMU:
extern CANData IMUInfo;	 //0x00000120
extern CANData IMUAccel; //0x00000121
extern CANData IMUGyro;	 //0x00000122

extern CANData IMUDeltaV;	//0x00000123
extern CANData IMUDeltaA;	//0x00000124

//EKF:
extern CANData IMUVel;	   //0x00000137
extern CANData IMUVelAcc;  //0x00000138
extern CANData IMUGPS;	   //0x00000177
extern CANData IMUAuto;    //0x00000220

int receiveIMU( void );

int returnCounter( void );

int initIMU( void );

#endif /* IMU_H_ */

