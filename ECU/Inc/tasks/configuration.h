/*
 * configuration.h
 *
 *  Created on: 13 Apr 2019
 *      Author: Visa
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

// priority level of config menu LCD output.
#define MENUPRIORITY  (4)



int CheckConfigurationRequest( void );
void setCurConfig(void);

void ConfigReset( void );
bool checkConfigReset( void );

char * getConfStr( void); // gets current short configuration line for display.
bool ConfigInput( uint16_t input);

bool initConfig( void );
bool inConfig( void );

#endif /* CONFIGURATION_H_ */
