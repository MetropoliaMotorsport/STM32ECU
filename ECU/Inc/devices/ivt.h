/*
 * ivt.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef IVT_H_
#define IVT_H_

uint8_t processIVT(uint8_t CANRxData[8], uint32_t DataLength, uint16_t field );

bool processIVTMsgData(uint8_t CANRxData[8], uint32_t DataLength );

bool processIVTIData(uint8_t CANRxData[8], uint32_t DataLength );
bool processIVTU1Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processIVTU2Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processIVTU3Data(uint8_t CANRxData[8], uint32_t DataLength );
bool processIVTTData(uint8_t CANRxData[8], uint32_t DataLength );
bool processIVTWData(uint8_t CANRxData[8], uint32_t DataLength );
bool processIVTAsData(uint8_t CANRxData[8], uint32_t DataLength );
bool processIVTWhData(uint8_t CANRxData[8], uint32_t DataLength );


int receiveIVT( void );

int requestIVT( void );


int initIVT( void );

#endif /* IVT_H_ */

