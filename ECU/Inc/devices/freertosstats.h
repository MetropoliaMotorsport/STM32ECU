/*
 * freertosstats.h
 *
 *  Created on: 26 May 2021
 *      Author: visa
 */

#ifndef DEVICES_FREERTOSSTATS_H_
#define DEVICES_FREERTOSSTATS_H_


void vTaskGetRunTimeStatsNoDyn( char *pcWriteBuffer );
void vTaskListNoDyn( char * pcWriteBuffer );


#endif /* DEVICES_FREERTOSSTATS_H_ */
