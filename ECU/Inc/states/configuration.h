/*
 * configuration.h
 *
 *  Created on: 13 Apr 2019
 *      Author: drago
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

int CheckConfigurationRequest( void );
void setDriveMode(void);

bool GetConfigCmd(uint8_t CANRxData[8], uint32_t DataLength );
void ConfigReset( void );
bool checkConfigReset( void );

#endif /* CONFIGURATION_H_ */
