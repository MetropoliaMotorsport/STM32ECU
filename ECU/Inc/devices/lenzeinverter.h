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

#define LENZE_TPDO2_ID				(0x1C0)
#define LENZE_TPDO3_ID				(0x240)
#define LENZE_TPDO4_ID				(0x280)


#define LENZE_RDO_ID				(0x580)

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

