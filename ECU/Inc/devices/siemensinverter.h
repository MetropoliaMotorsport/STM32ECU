/*
 * siemensinverter.h
 *
 *  Created on: 01 May 2019
 *      Author: Visa
 */

#ifndef SIEMENSINVERTER_H_
#define SIEMENSINVERTER_H_

#define InverterRL_COBID			(0x7E) // 126 // swap
#define InverterRR_COBID			(0x7F) // 127 // swap

#ifdef HPF20
#define InverterFL_COBID			(0x7C) // 124 // swap
#define InverterFR_COBID			(0x7D) // 125 // swap
#endif


extern CANData InverterCANErr[];;
extern CANData InverterCANNMT[];
extern CANData InverterCANPDO1[];
extern CANData InverterCANPDO2[];
extern CANData InverterCANPDO3[];
extern CANData InverterCANPDO4[];

#endif /* SIEMENSINVERTER_H_ */

