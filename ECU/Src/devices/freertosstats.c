/*
 * freertosstats.c
 *
 *  Created on: 26 May 2021
 *      Author: visa
 */

#include "ecumain.h"
#include "freertosstats.h"

#define MAXNOTASKS (50)
#define TASKSTATUSARRAYSIZE   MAXNOTASKS*sizeof( TaskStatus_t )
RAM_D1 TaskStatus_t xTaskStatusArray[TASKSTATUSARRAYSIZE];

// modified from freertos example for static allocation.

/*
 * Macros used by vListTask to indicate which state a task is in.
 */
#define tskRUNNING_CHAR		( 'X' )
#define tskBLOCKED_CHAR		( 'B' )
#define tskREADY_CHAR		( 'R' )
#define tskDELETED_CHAR		( 'D' )
#define tskSUSPENDED_CHAR	( 'S' )

static char* prvWriteNameToBuffer(char *pcBuffer, const char *pcTaskName) {
	size_t x;

	/* Start by copying the entire string. */
	strcpy(pcBuffer, pcTaskName);

	/* Pad the end of the string with spaces to ensure columns line up when
	 printed out. */
	for (x = strlen(pcBuffer); x < (size_t) ( configMAX_TASK_NAME_LEN - 1);
			x++) {
		pcBuffer[x] = ' ';
	}

	/* Terminate. */
	pcBuffer[x] = (char) 0x00;

	/* Return the new end of string. */
	return &(pcBuffer[x]);
}

/* This example demonstrates how a human readable table of run time stats
 information is generated from raw data provided by uxTaskGetSystemState().
 The human readable table is written to pcWriteBuffer.  (see the vTaskList()
 API function which actually does just this). */
void vTaskGetRunTimeStatsNoDyn(char *pcWriteBuffer) {
	TaskStatus_t *pxTaskStatusArray;
	UBaseType_t uxArraySize, x;
	uint32_t ulTotalTime, ulStatsAsPercentage;

#if( configUSE_TRACE_FACILITY != 1 )
	{
		#error configUSE_TRACE_FACILITY must also be set to 1 in FreeRTOSConfig.h to use vTaskGetRunTimeStats().
	}
	#endif

	/*
	 * PLEASE NOTE:
	 *
	 * This function is provided for convenience only, and is used by many
	 * of the demo applications.  Do not consider it to be part of the
	 * scheduler.
	 *
	 * vTaskGetRunTimeStats() calls uxTaskGetSystemState(), then formats part
	 * of the uxTaskGetSystemState() output into a human readable table that
	 * displays the amount of time each task has spent in the Running state
	 * in both absolute and percentage terms.
	 *
	 * vTaskGetRunTimeStats() has a dependency on the sprintf() C library
	 * function that might bloat the code size, use a lot of stack, and
	 * provide different results on different platforms.  An alternative,
	 * tiny, third party, and limited functionality implementation of
	 * sprintf() is provided in many of the FreeRTOS/Demo sub-directories in
	 * a file called printf-stdarg.c (note printf-stdarg.c does not provide
	 * a full snprintf() implementation!).
	 *
	 * It is recommended that production systems call uxTaskGetSystemState()
	 * directly to get access to raw stats data, rather than indirectly
	 * through a call to vTaskGetRunTimeStats().
	 */

	/* Make sure the write buffer does not contain a string. */
	*pcWriteBuffer = (char) 0x00;

	/* Take a snapshot of the number of tasks in case it changes while this
	 function is executing. */
	uxArraySize = uxTaskGetNumberOfTasks();

	/* Allocate an array index for each task.  NOTE!  If
	 configSUPPORT_DYNAMIC_ALLOCATION is set to 0 then pvPortMalloc() will
	 equate to NULL. */
	pxTaskStatusArray = xTaskStatusArray; //pvPortMalloc( uxCurrentNumberOfTasks * sizeof( TaskStatus_t ) ); /*lint !e9079 All values returned by pvPortMalloc() have at least the alignment required by the MCU's stack and this allocation allocates a struct that has the alignment requirements of a pointer. */

	if (pxTaskStatusArray != NULL) {
		/* Generate the (binary) data. */
		uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize,
				&ulTotalTime);

		/* For percentage calculations. */
		ulTotalTime /= 100UL;

		/* Avoid divide by zero errors. */
		if (ulTotalTime > 0UL) {
			/* Create a human readable table from the binary data. */
			for (x = 0; x < uxArraySize; x++) {
				/* What percentage of the total run time has the task used?
				 This will always be rounded down to the nearest integer.
				 ulTotalRunTimeDiv100 has already been divided by 100. */
				ulStatsAsPercentage = pxTaskStatusArray[x].ulRunTimeCounter
						/ ulTotalTime;

				/* Write the task name to the string, padding with
				 spaces so it can be printed in tabular form more
				 easily. */
				pcWriteBuffer = prvWriteNameToBuffer(pcWriteBuffer,
						pxTaskStatusArray[x].pcTaskName);

				if (ulStatsAsPercentage > 0UL) {
#ifdef portLU_PRINTF_SPECIFIER_REQUIRED
					{
						sprintf( pcWriteBuffer, "\t%lu\t\t%lu%%\r\n", pxTaskStatusArray[ x ].ulRunTimeCounter, ulStatsAsPercentage );
					}
					#else
					{
						/* sizeof( int ) == sizeof( long ) so a smaller
						 printf() library can be used. */
						sprintf(pcWriteBuffer, "\t%u\t\t%u%%\r\n",
								(unsigned int) pxTaskStatusArray[x].ulRunTimeCounter,
								(unsigned int) ulStatsAsPercentage); /*lint !e586 sprintf() allowed as this is compiled with many compilers and this is a utility function only - not part of the core kernel implementation. */
					}
#endif
				} else {
					/* If the percentage is zero here then the task has
					 consumed less than 1% of the total run time. */
#ifdef portLU_PRINTF_SPECIFIER_REQUIRED
					{
						sprintf( pcWriteBuffer, "\t%lu\t\t<1%%\r\n", pxTaskStatusArray[ x ].ulRunTimeCounter );
					}
					#else
					{
						/* sizeof( int ) == sizeof( long ) so a smaller
						 printf() library can be used. */
						sprintf(pcWriteBuffer, "\t%u\t\t<1%%\r\n",
								(unsigned int) pxTaskStatusArray[x].ulRunTimeCounter); /*lint !e586 sprintf() allowed as this is compiled with many compilers and this is a utility function only - not part of the core kernel implementation. */
					}
#endif
				}

				pcWriteBuffer += strlen(pcWriteBuffer); /*lint !e9016 Pointer arithmetic ok on char pointers especially as in this case where it best denotes the intent of the code. */
			}
		} else {
			mtCOVERAGE_TEST_MARKER();
		}

		/* Free the array again.  NOTE!  If configSUPPORT_DYNAMIC_ALLOCATION
		 is 0 then vPortFree() will be #defined to nothing. */
//		vPortFree( pxTaskStatusArray );
	} else {
		mtCOVERAGE_TEST_MARKER();
	}
}

void vTaskListNoDyn(char *pcWriteBuffer) {
	TaskStatus_t *pxTaskStatusArray;
	UBaseType_t uxArraySize, x;
	char cStatus;

	/*
	 * PLEASE NOTE:
	 *
	 * This function is provided for convenience only, and is used by many
	 * of the demo applications.  Do not consider it to be part of the
	 * scheduler.
	 *
	 * vTaskList() calls uxTaskGetSystemState(), then formats part of the
	 * uxTaskGetSystemState() output into a human readable table that
	 * displays task names, states and stack usage.
	 *
	 * vTaskList() has a dependency on the sprintf() C library function that
	 * might bloat the code size, use a lot of stack, and provide different
	 * results on different platforms.  An alternative, tiny, third party,
	 * and limited functionality implementation of sprintf() is provided in
	 * many of the FreeRTOS/Demo sub-directories in a file called
	 * printf-stdarg.c (note printf-stdarg.c does not provide a full
	 * snprintf() implementation!).
	 *
	 * It is recommended that production systems call uxTaskGetSystemState()
	 * directly to get access to raw stats data, rather than indirectly
	 * through a call to vTaskList().
	 */

	/* Make sure the write buffer does not contain a string. */
	*pcWriteBuffer = (char) 0x00;

	/* Take a snapshot of the number of tasks in case it changes while this
	 function is executing. */
	uxArraySize = uxTaskGetNumberOfTasks();

	/* Allocate an array index for each task.  NOTE!  if
	 configSUPPORT_DYNAMIC_ALLOCATION is set to 0 then pvPortMalloc() will
	 equate to NULL. */
	pxTaskStatusArray = xTaskStatusArray; //pvPortMalloc( uxCurrentNumberOfTasks * sizeof( TaskStatus_t ) ); /*lint !e9079 All values returned by pvPortMalloc() have at least the alignment required by the MCU's stack and this allocation allocates a struct that has the alignment requirements of a pointer. */

	if (pxTaskStatusArray != NULL) {
		/* Generate the (binary) data. */
		uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize,
		NULL);

		/* Create a human readable table from the binary data. */
		for (x = 0; x < uxArraySize; x++) {
			switch (pxTaskStatusArray[x].eCurrentState) {
			case eRunning:
				cStatus = tskRUNNING_CHAR;
				break;

			case eReady:
				cStatus = tskREADY_CHAR;
				break;

			case eBlocked:
				cStatus = tskBLOCKED_CHAR;
				break;

			case eSuspended:
				cStatus = tskSUSPENDED_CHAR;
				break;

			case eDeleted:
				cStatus = tskDELETED_CHAR;
				break;

			case eInvalid: /* Fall through. */
			default: /* Should not get here, but it is included
			 to prevent static checking errors. */
				cStatus = (char) 0x00;
				break;
			}

			/* Write the task name to the string, padding with spaces so it
			 can be printed in tabular form more easily. */
			pcWriteBuffer = prvWriteNameToBuffer(pcWriteBuffer,
					pxTaskStatusArray[x].pcTaskName);

			/* Write the rest of the string. */
			sprintf(pcWriteBuffer, "\t%c\t%u\t%u\t%u\r\n", cStatus,
					(unsigned int) pxTaskStatusArray[x].uxCurrentPriority,
					(unsigned int) pxTaskStatusArray[x].usStackHighWaterMark,
					(unsigned int) pxTaskStatusArray[x].xTaskNumber); /*lint !e586 sprintf() allowed as this is compiled with many compilers and this is a utility function only - not part of the core kernel implementation. */
			pcWriteBuffer += strlen(pcWriteBuffer); /*lint !e9016 Pointer arithmetic ok on char pointers especially as in this case where it best denotes the intent of the code. */
		}

		/* Free the array again.  NOTE!  If configSUPPORT_DYNAMIC_ALLOCATION
		 is 0 then vPortFree() will be #defined to nothing. */
		//		vPortFree( pxTaskStatusArray );
	} else {
		mtCOVERAGE_TEST_MARKER();
	}
}
