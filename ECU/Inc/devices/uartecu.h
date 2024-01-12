/*
 * uartecu.h
 *
 *  Created on: 16 May 2021
 *      Author: visa
 */

#ifndef DEVICES_UARTECU_H_
#define DEVICES_UARTECU_H_

typedef enum UARTType {
	NOUART,
	UART1,
	UART2,
	UART7_
} UART;


bool UART_Transmit(UART UART, uint8_t* buffer, uint16_t n);

bool UART_Receive(UART UART, uint8_t* ch, uint16_t n);

bool UART_WaitTXDone( UART UART, uint32_t wait );

bool UART_WaitRXDone( UART UART, uint32_t wait );

// uart needs, handle, semaphores for tx/rx..

int initUART( void );


#endif /* DEVICES_UARTECU_H_ */
