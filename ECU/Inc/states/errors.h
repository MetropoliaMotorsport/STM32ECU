/*
 * errors.h
 *
 *  Created on: 29 Apr 2021
 *      Author: Visa
 */

#ifndef STATES_ERRORS_H_
#define STATES_ERRORS_H_

#include "ecumain.h"

typedef struct {
	uint16_t OperationalReceiveError;
	uint16_t State;
	uint8_t  InvAllowReset[MOTORCOUNT];
//	uint8_t  LeftInvAllowReset;
//    uint8_t  RightInvAllowReset;
	uint16_t ErrorReason;
	uint16_t ErrorPlace;

	uint8_t  InverterError;

	uint8_t InverterErrorHistory[8][8];
	uint8_t InverterErrorHistoryPosition;
	uint8_t InverterErrorHistoryID[8];

	uint32_t CANError;
	uint32_t CANCount1;
	uint32_t CANCount2;
	uint32_t CANTimeout;

 // specific devices
	bool	 ADCSent;
	uint32_t eepromerror;

	uint32_t ADCError;
	uint16_t ADCTimeout;
	uint16_t ADCErrorState;

	uint16_t INVReceiveStatus[MOTORCOUNT];
	uint16_t INVReceiveSpd[MOTORCOUNT];
	uint16_t INVReceiveTorque[MOTORCOUNT];

	uint32_t CANSendError1;
	uint32_t CANSendError2;
} ErrorsType;

extern volatile ErrorsType Errors;

extern uint16_t ErrorCode; // global error code.


int ResetErrors( void );
void LogError( char *message );
void SetErrorLogging( bool log );

void SetCriticalError(void);
bool CheckCriticalError(void);
void ClearCriticalError(void);

int OperationalErrorHandler( uint32_t OperationLoops );

int initERRORState( void );

#endif /* STATES_ERRORS_H_ */
