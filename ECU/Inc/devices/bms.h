/*
 * bms.h
 *
 *  Created on: 01 May 2019
 *      Author: drago
 */

#ifndef BMS_H_
#define BMS_H_

//uint8_t processBMS(uint8_t CANRxData[8], uint32_t DataLength );
uint8_t processBMSVoltage(uint8_t CANRxData[8], uint32_t DataLength );
uint8_t processBMSOpMode(uint8_t CANRxData[8], uint32_t DataLength );
uint8_t processBMSError(uint8_t CANRxData[8], uint32_t DataLength );
int receiveBMS( void );
void sendBMS( void );

#endif /* BMS_H_ */

