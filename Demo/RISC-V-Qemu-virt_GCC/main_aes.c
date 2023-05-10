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
#define mainQUEUE_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainQUEUE_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

#define mainAES_TASK_PRIOTIY (tskIDLE_PRIORITY + 1)
#define mainPRIO_MANAGER_TASK_PRIOTIY (tskIDLE_PRIORITY + 2)
/* The rate at which data is sent to the queue.  The 200ms value is converted
to ticks using the pdMS_TO_TICKS() macro. */
#define mainQUEUE_SEND_FREQUENCY_MS pdMS_TO_TICKS(1000)

/* The maximum number items the queue can hold.  The priority of the receiving
task is above the priority of the sending task, so the receiving task will
preempt the sending task and remove the queue items each time the sending task
writes to the queue.  Therefore the queue will never have more than one item in
it at any time, and even with a queue length of 1, the sending task will never
find the queue full. */
#define mainQUEUE_LENGTH (1)
#define aesFAKE_COUNT (1)
#define init_state                                                                                     \
	{                                                                                                  \
		0x40, 0x41, 0x40, 0x41, 0x40, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41 \
	}
#define true_key                                                                                       \
	{                                                                                                  \
		0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c \
	}
#define puf_key                                                                                        \
	{                                                                                                  \
		0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c \
	}

typedef struct aes_param
{
	uint8_t tag;
	uint8_t *key;
	uint8_t *state;
	TaskHandle_t *task;
} aes_param_t;

uint8_t states[aesFAKE_COUNT +1][16];
uint8_t keys[aesFAKE_COUNT +1][16];
aes_param_t aes_CB[aesFAKE_COUNT + 1];

/* The queue used by both tasks. */
static QueueHandle_t xAesInQueue = NULL;
static QueueHandle_t xFakeAesInQueue = NULL;

extern int aes_run(uint8_t *state, uint8_t *key);
/*-----------------------------------------------------------*/

static void prvPriorityManagerTask(void *pvParameters)
{
	aes_param_t *aes_current_cb = (aes_param_t *) pvParameters;
	uint8_t selector = 0;
	while (1)
	{	
		portENTER_CRITICAL()
		selector += 1;
		selector = selector % (aesFAKE_COUNT +1 );
		for (uint8_t i = 0; i < (aesFAKE_COUNT + 1); i++)
		{
			TaskHandle_t aes_selected_handler = *aes_current_cb[i].task;
			if (i != selector)
			{
				vTaskPrioritySet(aes_selected_handler, mainAES_TASK_PRIOTIY);
			}
			else{
				vTaskPrioritySet(aes_selected_handler, mainPRIO_MANAGER_TASK_PRIOTIY);
			}
		}
		portEXIT_CRITICAL()
		taskYIELD()
	}
}

static void prvEncoderTask(void *pvParameters)
{
	aes_param_t *aes_current_cb = (aes_param_t *) pvParameters;

	while (1)
	{
		char buf[40];
		char aes_buf[4 + (2 * 16)];
		aes_run(aes_current_cb->state, aes_current_cb->key);
		for (uint8_t i = 0; i < 16; i++)
		{
			sprintf(aes_buf + (4 + (2 * i)), "%02x", aes_current_cb->state[i]);
		}
#ifdef QEMU
			sprintf(aes_buf, "AES%d:", aes_current_cb->tag);
			vSendString(buf);
#endif
	}
}

/*
 * AES environment setup, generates necessary aes states and keys:
 *	a state and key pair for the real aes
 *	aesFAKE_COUNT state and key pairs for the fake ones, each key is generated running a puf N times,
 *	with N being different for each key
 */
static void aesSetup()
{

	uint8_t temp_puf[16] = puf_key;

	for (uint8_t i = 0; i < (aesFAKE_COUNT + 1); i++)
	{
		uint8_t temp[16] = init_state;
		uint8_t temp_key[16] = true_key;

		for (uint8_t j = 0; j < 16; j++)
		{
			states[i][j] = temp[j];
			keys[i][j] = temp_key[j];
		}

		if (i != 0)
		{
			for (uint8_t k = 0; k < i; k++)
			{
				aes_run(keys[i], temp_puf);
			}
		}
	}
}
/*-----------------------------------------------------------*/

int main_aes(void)
{
#ifdef QEMU
	vSendString("FreeRTOS AES QEMU dev bench:");
	vSendString("AES setup start");
#endif	
	aesSetup();
#ifdef QEMU
	vSendString("AES setup completed");
	vSendString("Tasks setup start");
#endif	
	for (uint8_t i = 0; i < (aesFAKE_COUNT + 1); i++)
	{
		aes_CB[i].tag = i;
		aes_CB[i].key = keys[i];
		aes_CB[i].state = states[i];
		xTaskCreate(	
			prvEncoderTask, 
			"AES", 
			configMINIMAL_STACK_SIZE * 2U, 
			(void *) &(aes_CB[i]),
			mainAES_TASK_PRIOTIY, 
			aes_CB[i].task
		);
		#ifdef QEMU
			char buf[30];
			sprintf(buf, "Tasks %d created", i);
		#endif
	}
	xTaskCreate(	
		prvPriorityManagerTask, 
		"PRIORITY MANAGER", 
		configMINIMAL_STACK_SIZE * 2U, 
		(void *) aes_CB,
		mainPRIO_MANAGER_TASK_PRIOTIY, 
		NULL
	);
#ifdef QEMU
	vSendString( "Tasks setup completed" );
	vSendString( "Scheduler started" );
#endif
	vTaskStartScheduler();
	return 0;
}
