/*
 * FreeRTOS V202112.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://www.github.com/FreeRTOS
 *
 * 1 tab == 4 spaces!
 */

/* FreeRTOS kernel includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <stdio.h>

#include "riscv-virt.h"

/* Priorities used by the tasks. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2)
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1)

/* The rate at which data is sent to the queue.  The 200ms value is converted
to ticks using the pdMS_TO_TICKS() macro. */
#define mainQUEUE_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 1000 )

/* The maximum number items the queue can hold.  The priority of the receiving
task is above the priority of the sending task, so the receiving task will
preempt the sending task and remove the queue items each time the sending task
writes to the queue.  Therefore the queue will never have more than one item in
it at any time, and even with a queue length of 1, the sending task will never
find the queue full. */
#define mainQUEUE_LENGTH					( 1 )

/* The queue used by both tasks. */
static QueueHandle_t xAesInQueue = NULL;

/*--AES-testing--------------------------Vendors/aes.c-------*/
uint8_t state[16] ={0x40,0x41,0x40,0x41,0x40,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,};
extern int aes_run(uint8_t*);
/*-----------------------------------------------------------*/

/*
* Blocks dispatcher: put a block address in xAesInQueue
*/
static void prvDispatcherTask( void *pvParameters )
{
	uint8_t *bufferAddrres;
	bufferAddrres = state;

	TickType_t xNextWakeTime;
	const char * const pcMessage = "Block enqueued";
	char buf[40];

	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;
	/* Initialise xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	sprintf( buf, "%d: %s: %s", xGetCoreID(),
				pcTaskGetName( xTaskGetCurrentTaskHandle() ),
				pcMessage);
		vSendString( buf );
	xQueueSend( xAesInQueue, &bufferAddrres , 0U );

	while(1)
	{
		sprintf( buf, "%d: %s: %s", xGetCoreID(),
				pcTaskGetName( xTaskGetCurrentTaskHandle() ),
				pcMessage);
		vSendString( buf );

		/* Place this task in the blocked state until it is time to run again. */
		xTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );

		/* Send to the queue - causing the queue receive task to unblock and
		toggle the LED.  0 is used as the block time so the sending operation
		will not block - it shouldn't need to block as the queue should always
		be empty at this point in the code. */
		xQueueSend( xAesInQueue, &bufferAddrres, 0U );
	}
}

/*-----------------------------------------------------------*/

static void prvEncoderTask( void *pvParameters )
{
	uint8_t *receivedState;
	const char * const pcFailMessage = "Unexpected value received\r\n";
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;

	while(1)
	{
		char buf[40];
		char aes_buf[4+(2*16)];
		sprintf(aes_buf,"AES:");
		/* Wait until something arrives in the queue - this task will block
		indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		FreeRTOSConfig.h. */
		if(xQueueReceive( xAesInQueue, &receivedState, portMAX_DELAY ) == pdFALSE)
		{
			sprintf(buf, "task %s: failed receive from queue",
					pcTaskGetName( xTaskGetCurrentTaskHandle()));
			vSendString(buf);
		}
		/*  To get here something must have been received from the queue, but
		is it the expected value?  If it is, toggle the LED. */
		if( receivedState == state )
		{
			aes_run(receivedState);
			for (uint8_t i = 0; i < 16; i++)
			{
				sprintf(aes_buf+(4+(2*i)),"%02x",receivedState[i]);
				
			}
			vSendString( aes_buf);
		}
		else
		{
			vSendString( pcFailMessage );
		}
	}
}

/*-----------------------------------------------------------*/

int main_aes( void )
{
	vSendString( "FreeRTOS AES QEMU dev bench:" );
	vSendString( "Tasks create start" );
	/* Create the queue. */
	xAesInQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint8_t* ) );

	if( xAesInQueue != NULL )
	{
		/* Start the two tasks as described in the comments at the top of this
		file. */
		xTaskCreate( prvEncoderTask, "AES", configMINIMAL_STACK_SIZE * 2U, NULL,
					mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );
		xTaskCreate( prvDispatcherTask, "Tx", configMINIMAL_STACK_SIZE * 2U, NULL,
					mainQUEUE_SEND_TASK_PRIORITY, NULL );
		vSendString( "Tasks create success" );
	}

	vSendString( "Scheduler started" );
	vTaskStartScheduler();
	vSendString( "Failed to start scheduler" );

	return 0;
}
