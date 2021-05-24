/*
 * configuration.h
 *
 *  Created on: 13 Apr 2019
 *      Author: Visa
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

CANData ECUConfig;

int CheckConfigurationRequest( void );
void setCurConfig(void);

void ConfigReset( void );
bool checkConfigReset( void );

bool ConfigInput( uint16_t input);

bool initConfig( void );

#endif /* CONFIGURATION_H_ */
