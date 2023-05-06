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
#include <stdint.h>

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
static QueueHandle_t xFakeAesInQueue = NULL;

/*--AES-testing--------------------------Vendors/aes.c-------*/
uint8_t state[16] ={0x40,0x41,0x40,0x41,0x40,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,};
uint8_t fake_state[16] ={0x40,0x41,0x40,0x41,0x40,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,};

uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
uint8_t fake_key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
uint8_t puf_key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};

extern int aes_run(uint8_t*, uint8_t*);
/*-----------------------------------------------------------*/

/*
* Blocks dispatcher: put a block address in xAesInQueue
*/
static void prvDispatcherTask( void *pvParameters )
{
	uint8_t *state_address = state;
	uint8_t *fake_state_address = fake_state;

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
		#ifdef QEMU 
			vSendString( buf );
		#endif

	xQueueSend( xAesInQueue, &state_address , 0U );
	xQueueSend( xFakeAesInQueue, &fake_state_address, 0U );

	while(1)
	{
		sprintf( buf, "%d: %s: %s", xGetCoreID(),
				pcTaskGetName( xTaskGetCurrentTaskHandle() ),
				pcMessage);
		#ifdef QEMU 
			vSendString( buf );
		#endif

		/* Place this task in the blocked state until it is time to run again. */
		xTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );

		/* Send to the queue - causing the queue receive task to unblock and
		toggle the LED.  0 is used as the block time so the sending operation
		will not block - it shouldn't need to block as the queue should always
		be empty at this point in the code. */
		xQueueSend( xAesInQueue, &state_address, 0U );
		xQueueSend( xFakeAesInQueue, &state_address, 0U );
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
			#ifdef QEMU 
			vSendString( buf );
			#endif
		}
		
		
		if( receivedState == state )
		{
			aes_run(state, key);
			for (uint8_t i = 0; i < 16; i++)
			{
				sprintf(aes_buf+(4+(2*i)),"%02x",receivedState[i]);
				
			}
			#ifdef QEMU 
				vSendString( buf );
			#endif
		}
		else
		{
			#ifdef QEMU 
				vSendString( pcFailMessage );
			#endif
		}
	}
}

static void prvFakeEncoderTask( void *pvParameters )
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
		if(xQueueReceive( xFakeAesInQueue, &receivedState, portMAX_DELAY ) == pdFALSE)
		{
			sprintf(buf, "task %s: failed receive from queue",
					pcTaskGetName( xTaskGetCurrentTaskHandle()));
			#ifdef QEMU 
				vSendString( buf );
			#endif
		}
		
		
		if( receivedState == state )
		{
			for (uint8_t i = 0; i < 16; i++)
			{
				fake_state[i] = receivedState[i];
			}
			aes_run(fake_state, fake_key);
			for (uint8_t i = 0; i < 16; i++)
			{
				sprintf(aes_buf+(4+(2*i)),"%02x",fake_state[i]);
				
			}
			#ifdef QEMU 
				vSendString( buf );
			#endif
		}
		else
		{
			#ifdef QEMU 
				vSendString( pcFailMessage );
			#endif
		}
	}
}

/*-----------------------------------------------------------*/

int main_aes( void )
{
	#ifdef QEMU 
		vSendString( "FreeRTOS AES QEMU dev bench:" );
		vSendString( "Tasks create start" );
	#endif
	
	
	/* Create the queue. */
	xAesInQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint8_t* ) );
	xFakeAesInQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint8_t* ) );
	
	/* Encode the fake key from the true key with a puf function*/
	aes_run(fake_key,puf_key);

	if( xAesInQueue != NULL )
	{
		/* Start the two tasks as described in the comments at the top of this
		file. */
		xTaskCreate( prvEncoderTask, "AES", configMINIMAL_STACK_SIZE * 2U, NULL,
					mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );
		xTaskCreate( prvFakeEncoderTask, "FAKEAES", configMINIMAL_STACK_SIZE * 2U, NULL,
					mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );
		xTaskCreate( prvDispatcherTask, "Tx", configMINIMAL_STACK_SIZE * 2U, NULL,
					mainQUEUE_SEND_TASK_PRIORITY, NULL );
		#ifdef QEMU			
		//SendString( "Tasks create success" );
		#endif
	}
	#ifdef QEMU
	//vSendString( "Scheduler started" );
	#endif
	vTaskStartScheduler();
	//vSendString( "Failed to start scheduler" );

	return 0;
}
