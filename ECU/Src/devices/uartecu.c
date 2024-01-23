/*
 * uartecu.c
 *
 *  Created on: 16 May 2021
 *      Author: visa
 */

#include "debug.h"
#include "ecumain.h"
#include "uartecu.h"

#include "usart.h"

// freeRTOS
#include "semphr.h"

// ADC conversion buffer, should be aligned in memory for faster DMA?
DMA_BUFFER ALIGN_32BYTES (static uint8_t UART1TXBuffer[1024]);

SemaphoreHandle_t UART1TxDone;
SemaphoreHandle_t UART1RxDone;

SemaphoreHandle_t UART2TxDone;
SemaphoreHandle_t UART2RxDone;

SemaphoreHandle_t UART7TxDone;
SemaphoreHandle_t UART7RxDone;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;

	if (huart == &huart1) {
		// receive complete
		xSemaphoreGiveFromISR(UART1TxDone, &xHigherPriorityWoken);
	}

	if (huart == &huart2) {
		// receive complete
		xSemaphoreGiveFromISR(UART2TxDone, &xHigherPriorityWoken);
	}

	if (huart == &huart7) {
		// receive complete
		xSemaphoreGiveFromISR(UART7TxDone, &xHigherPriorityWoken);
	}

	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;

	if (huart == &huart1) {
		// receive complete
		xSemaphoreGiveFromISR(UART1RxDone, &xHigherPriorityWoken);
	}

	if (huart == &huart2) {
		// receive complete
		xSemaphoreGiveFromISR(UART2RxDone, &xHigherPriorityWoken);
	}

	if (huart == &huart7) {
		// receive complete
		xSemaphoreGiveFromISR(UART7RxDone, &xHigherPriorityWoken);
	}

	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

bool UART_Transmit(UART UART, uint8_t *buffer, uint16_t n) {
//	static volatile int err = 0; // TODO need to maybe setup a send transmit queue for UART1 if its to be used as a canbus relay.
	if (UART == UART1) {
		memcpy(UART1TXBuffer, buffer, n);

		if (HAL_UART_Transmit_DMA(&huart1, UART1TXBuffer, n) != HAL_OK) {
//			err = 1;
			return false;
		} else {
//			err = 0;
			return true;
		}
	}

	if (UART == UART2) {
		if (HAL_UART_Transmit_IT(&huart2, buffer, n) != HAL_OK) {
			return false;
		} else {
			return true;
		}
	}

	if (UART == UART7_) {
		if (HAL_UART_Transmit_IT(&huart7, buffer, n) != HAL_OK) {
			return false;
		} else {
			return true;
		}
	}

	return false;
}

bool UART_Receive(UART UART, uint8_t *ch, uint16_t n) {
	if (UART == UART1) {
		if (HAL_UART_Receive_IT(&huart1, ch, n) != HAL_OK) {
			return false;
		}
		return true;
	}

	if (UART == UART2) {
		if (HAL_UART_Receive_IT(&huart2, ch, n) != HAL_OK) {
			return false;
		}
		return true;
	}

	if (UART == UART7_) {
		if (HAL_UART_Receive_IT(&huart7, ch, n) != HAL_OK) {
			return false;
		}
		return true;
	}

	return false;
}

bool UART_WaitTXDone(UART UART, uint32_t wait) {
	if (UART == UART1) {
		if ( xSemaphoreTake(UART1TxDone, wait) == pdTRUE) {
			return true;
		}
	}

	if (UART == UART2) {
		if ( xSemaphoreTake(UART2TxDone, wait) == pdTRUE) {
			return true;
		}
	}

	if (UART == UART7_) {
		if ( xSemaphoreTake(UART7TxDone, wait) == pdTRUE) {
			return true;
		}
	}

	return false;
}

bool UART_WaitRXDone(UART UART, uint32_t wait) {
	if (UART == UART1) {
		if ( xSemaphoreTake(UART1RxDone, 0) == pdTRUE) {
			return true;
		} else
			return false;
	}

	if (UART == UART2) {
		if ( xSemaphoreTake(UART2RxDone, 0) == pdTRUE) {
			return true;
		} else
			return false;
	}

	if (UART == UART7_) {
		if ( xSemaphoreTake(UART7RxDone, 0) == pdTRUE) {
			return true;
		} else
			return false;
	}

	return false;
}

// uart needs, handle, semaphores for tx/rx..

int initUART(void) {

	UART1TxDone = xSemaphoreCreateBinary();
	UART1RxDone = xSemaphoreCreateBinary();

	UART2TxDone = xSemaphoreCreateBinary();
	UART2RxDone = xSemaphoreCreateBinary();

	UART7TxDone = xSemaphoreCreateBinary();
	UART7RxDone = xSemaphoreCreateBinary();

	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_UART7_Init();

	return 0;
}

