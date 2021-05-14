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
#define InverterRL_Channel			(false)
#define InverterRR_COBID			(Inverter2_NodeID)
#define InverterRR_Channel			(true)

#define InverterFL_COBID			(Inverter1_NodeID)
#define InverterFL_Channel			(false)
#define InverterFR_COBID			(Inverter2_NodeID)
#define InverterFR_Channel			(true)

#endif /* LENZEINVERTER_H_ */

