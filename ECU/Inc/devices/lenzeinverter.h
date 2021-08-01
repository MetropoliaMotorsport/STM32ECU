/*
 * lenzeinverter.h
 *
 *  Created on: 21 Mar 2021
 *      Author: Visa
 */

#ifndef LENZEINVERTER_H_
#define LENZEINVERTER_H_

// Inverter specific definitions. Called functions are defined in Inverter module for some level of device independance abstraction.

#define INVERTERCOUNT				(MOTORCOUNT/2)

#define LENZE_RPDO1_ID				(0x200)
#define LENZE_RPDO2_ID				(0x300)
#define LENZE_RPDO3_ID				(0x400)
#define LENZE_RPDO4_ID				(0x500)
#define LENZE_RPDO5_ID				(0x540)

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

#define Inverter1_NodeID			(2)
#define Inverter2_NodeID			(0xE)

// define which wheel is which, allows to keep the order same within arrays without being concerned about order of connection.

#define InverterRL_COBID			(Inverter1_NodeID) // which inverter is it on.
#define InverterRL_Channel			(0)				   // is it motor A or B
#define InverterRR_COBID			(Inverter1_NodeID)
#define InverterRR_Channel			(1)

#define InverterFL_COBID			(Inverter2_NodeID)
#define InverterFL_Channel			(0)
#define InverterFR_COBID			(Inverter2_NodeID)
#define InverterFR_Channel			(1)

#define TORQUESLOPESCALING			(90)
#define NMSCALING					(90)

#endif /* LENZEINVERTER_H_ */

