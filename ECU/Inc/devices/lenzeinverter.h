/*
 * lenzeinverter.h
 *
 *  Created on: 21 Mar 2021
 *      Author: Visa
 */

#ifndef LENZEINVERTER_H_
#define LENZEINVERTER_H_

#define INVERTERCOUNT				(MOTORCOUNT/2)

#define Inverter1_NodeID			(1)
#define Inverter2_NodeID			(2)

// define which wheel is which

#define InverterRL_COBID			(Inverter1_NodeID)
#define InverterRL_Channel			(0)
#define InverterRR_COBID			(Inverter1_NodeID)
#define InverterRR_Channel			(1)

#define InverterFL_COBID			(Inverter2_NodeID)
#define InverterFL_Channel			(0)
#define InverterFR_COBID			(Inverter2_NodeID)
#define InverterFR_Channel			(1)

#endif /* LENZEINVERTER_H_ */

