/*
 * ivt.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef IVT_H_
#define IVT_H_

#define IVTCmd_ID		    	0x411
#define IVTMsg_ID			    0x511
#define IVTBase_ID 				0x521
#define IVTI_ID			    	IVTBase_ID   //0x521
#define IVTU1_ID		    	IVTBase_ID+1 //0x522
#define IVTU2_ID		    	IVTBase_ID+2 //0x523
#define IVTU3_ID				IVTBase_ID+3 //0x524
#define IVTT_ID					IVTBase_ID+4 //0x525
#define IVTW_ID			    	IVTBase_ID+5 //0x526
#define IVTAs_ID		    	IVTBase_ID+6 //0x527
#define IVTWh_ID				IVTBase_ID+7 //0x528

uint8_t processIVT(uint8_t CANRxData[8], uint32_t DataLength, uint16_t field );

int receiveIVT( void );

int requestIVT( void );

int initIVT( void );

#endif /* IVT_H_ */

