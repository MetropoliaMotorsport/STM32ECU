/*
 * lenzeinverter.h
 *
 *  Created on: 21 Mar 2021
 *      Author: Visa
 */

#ifndef LENZEINVERTER_H_
#define LENZEINVERTER_H_

#include "ecu.h"

// Inverter specific definitions. Called functions are defined in Inverter module for some level of device independance abstraction.

#define INVERTERCOUNT				(MOTORCOUNT/2)

#define LENZE_RPDO1_ID				(0x200) //RxPDO1_InverterASetpoint1_14
#define LENZE_RPDO2_ID				(0x300) //RxPDO2_InverterASetpoint2_14
#define LENZE_RPDO3_ID				(0x400) //RxPDO3_InverterBSetpoint1_14
#define LENZE_RPDO4_ID				(0x500) //RxPDO4_InverterBSetpoint2_14
#define LENZE_RPDO5_ID				(0x540) //RxPDO5_DeviceSetpoint_6 //RxPDO5_DeviceSetpoint_14

#define LENZE_TPDO1_ID				(0x180)
#define LENZE_TPDO2_ID				(0x1C0)
#define LENZE_TPDO3_ID				(0x240)
#define LENZE_TPDO4_ID				(0x280)
#define LENZE_TPDO5_ID				(0x2C0)
#define LENZE_TPDO6_ID				(0x340)
#define LENZE_TPDO7_ID				(0x380)

#define LENZE_RDO_ID				(0x580)
#define LENZE_SDO_ID				(0x600)

#define LENZE_MOTORA_OFFSET			(0)
#define LENZE_APPC_OFFSET			(31)
#define LENZE_MOTORB_OFFSET			(63)

#ifdef TWOWHEELS
    #define Inverter1_NodeID            (0xE)
    #define Inverter2_NodeID			(1)
#else
    #ifdef HPF2023
    #define Inverter1_NodeID			(6)
    #else
    #define Inverter1_NodeID			(2)
    #endif
    #define Inverter2_NodeID			(0xE)
#endif

// define which wheel is which, allows to keep the order same within arrays without being concerned about order of connection.

//#define InverterRL_COBID			(Inverter1_NodeID) // which inverter is it on.
//#define InverterRL_Channel			(0)				   // is it motor A or B

//#define InverterRL_COBID			(Inverter2_NodeID) // which inverter is it on.
//#define InverterRL_Channel			(1)				   // is it motor A or B
//#define InverterRR_COBID			(Inverter1_NodeID)
//#define InverterRR_Channel			(1)

//#define InverterFL_COBID			(Inverter2_NodeID)
//#define InverterFL_Channel			(0)
//#define InverterFR_COBID			(Inverter2_NodeID)
//#define InverterFR_Channel			(1)


//#define InverterFR_COBID			(Inverter1_NodeID)
//#define InverterFR_Channel			(0)

#ifdef HPF2023

#ifdef TWOWHEELS
#define invFL						(2)
#define invFR						(3)
#define invRL						(0)
#define invRR						(1)
#else

#define invFL						(0)
#define invFR						(1)
#define invRL						(2)
#define invRR						(3)
#endif

#else

#define invFL						(2)
#define invFR						(0)
#define invRL						(3)
#define invRR						(1)

#endif


#define TORQUESLOPESCALING			(90)
#define NMSCALING					(90)
#define SPEEDSCALING				(0x4000)

#endif /* LENZEINVERTER_H_ */

