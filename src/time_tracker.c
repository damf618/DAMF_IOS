/*
 * time_tracker.c
 *
 *  Created on: Aug 16, 2021
 *      Author: daniel
 */

#include "board.h"
#include "sapi.h"
#include "stdbool.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../inc/time_tracker.h"

/*==================[external functions]=========================*/
char* itoa(int value, char* result, int base) {
   // check that the base if valid
   if (base < 2 || base > 36) { *result = '\0'; return result; }

   char* ptr = result, *ptr1 = result, tmp_char;
   int tmp_value;

   do {
      tmp_value = value;
      value /= base;
      *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
   } while ( value );

   // Apply negative sign
   if (tmp_value < 0) *ptr++ = '-';
   *ptr-- = '\0';
   while(ptr1 < ptr) {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
   }
   return result;
}

/*==================[functions declaration]=========================*/

bool_t timing_event_generator (monitor_system_t* monitor);
void Event_Msg (monitor_system_t* monitor);

/*==================[functions]=========================*/
void Monitor_System_Init(monitor_system_t* monitor,uint8_t key1, uint8_t key2)
{
	/* Inicializar UART_USB a 115200 baudios */
	uartConfig( UART_USB, 115200 );

	cyclesCounterConfig ( EDU_CIAA_NXP_CLOCK_SPEED );
	monitor->button1.key = key1;
	monitor->button2.key = key2;

	monitor->event_counter 		= 0;
	monitor->sequence_counter 	= 0;


	monitor->button1.falling_time 	= INIT_TIME;
	monitor->button1.rising_time 	= INIT_TIME;

	monitor->button2.falling_time 	= INIT_TIME;
	monitor->button2.rising_time 	= INIT_TIME;

	monitor->state = T1_INIT;

	reset_event(monitor);

	gpioWrite(GPIO_RGB_BLUE	,OFF);
	gpioWrite(GPIO_RGB_RED	,OFF);
	gpioWrite(GPIO_RGB_GREEN,OFF);
	gpioWrite(GPIO_YELLOW	,OFF);
	gpioWrite(GPIO_RED		,OFF);
	gpioWrite(GPIO_GREEN	,OFF);
}

bool_t T1_Init(monitor_system_t* monitor, uint8_t key)
{
	bool_t rtn = FALSE;
	if(1 == monitor->sequence_counter)
	{
		cyclesCounterReset();
		monitor->button1.falling_time = cyclesCounterToMs(cyclesCounterRead());
		monitor->button_sequence[monitor->sequence_counter] = key;
		//monitor->sequence_counter++;
		rtn = TRUE;
	}
	return rtn;
}

bool_t T2_Init(monitor_system_t* monitor, uint8_t key)
{
	bool_t rtn = FALSE;
	if(3 == monitor->sequence_counter)
	{
		cyclesCounterReset();
		monitor->button2.falling_time = cyclesCounterToMs(cyclesCounterRead());
		monitor->button_sequence[monitor->sequence_counter] = key;
		//monitor->sequence_counter++;
		rtn = TRUE;
	}
	return rtn;
}

bool_t T1_Set(monitor_system_t* monitor)
{
	bool_t rtn = FALSE;
	if(2 == monitor->sequence_counter)
	{
		monitor->button1.rising_time = cyclesCounterToMs(cyclesCounterRead());
		//monitor->sequence_counter++;
		rtn = TRUE;
	}
	return rtn;
}

bool_t T2_Set(monitor_system_t* monitor)
{
	bool_t rtn = FALSE;
	if(4 == monitor->sequence_counter)
	{
		monitor->button2.rising_time = cyclesCounterToMs(cyclesCounterRead());
		//monitor->sequence_counter++;
		rtn = TRUE;
	}
	return rtn;
}

bool_t Monitor_System_update(monitor_system_t* monitor, uint8_t key)
{
	bool_t rtn = FALSE;
	switch(monitor->state)
	{
	case MONITOR_INIT:
		Monitor_System_Init(monitor,monitor->button1.key,monitor->button2.key);
		monitor->state = MONITORING;
		break;

	case MONITORING:
		reset_event(monitor);
		monitor->state = T1_INIT;
		break;

	case T1_INIT:
		if(T1_Init(monitor,key))
		{
			monitor->state = T1_SET;
		}
		break;

	case T1_SET:
		if(T1_Set(monitor))
		{
			monitor->state = T2_INIT;
		}
		break;

	case T2_INIT:
		if(T2_Init(monitor,key))
		{
			monitor->state = T2_SET;
		}
		break;

	case T2_SET:
		if(T2_Set(monitor))
		{
			monitor->state = MONITORING;
			rtn = TRUE;
		}
		break;

	default:
		monitor->state = MONITOR_INIT;
		break;
	}
	return rtn;
}

bool_t timing_event_generator (monitor_system_t* monitor)
{
	bool_t rtn = FALSE;

	float t1_timing;
	float t2_timing;

	t1_timing = (float)monitor->button1.rising_time - monitor->button1.falling_time;
	t2_timing = (float)monitor->button2.rising_time - monitor->button2.falling_time;

	//Validacion de tiempos t1 y t2 superiores a 0ms
	if((t1_timing > 0)&&(t2_timing > 0))
	{
		monitor->events[monitor->event_counter].t1_timing = t1_timing;
		monitor->events[monitor->event_counter].t2_timing = t2_timing;

		//Se presiono primero el Boton 1
		if(monitor->button_sequence[INIT_SEQUENCE] == monitor->button1.key)
		{
			//Si se solto primero el Boton 1
			if(monitor->button_sequence[SECOND_SEQUENCE] == monitor->button1.key)
			{
				monitor->events[monitor->event_counter].led = LED_GREEN;
			}
			else
			{
				monitor->events[monitor->event_counter].led = LED_RED;
			}
		}
		//Se presiono primero el Boton 2
		else
		{
			//Si se solto primero el Boton 1
			if(monitor->button_sequence[SECOND_SEQUENCE] == monitor->button1.key)
			{
				monitor->events[monitor->event_counter].led = LED_YELLOW;
			}
			else
			{
				monitor->events[monitor->event_counter].led = LED_RGB_BLUE;
			}
		}
		rtn = TRUE;
		monitor->event_counter++;
	}
	reset_event(monitor);
	return rtn;
}

bool_t set_sequence(monitor_system_t* monitor, uint8_t key)
{
	bool_t rtn = FALSE;

	if(monitor->sequence_counter<SEQUENCE_N)
	{
		monitor->button_sequence[monitor->sequence_counter] = key;
		monitor->sequence_counter++;

		rtn = TRUE;
	}

	return rtn;
}

bool_t Sequence_Validation (monitor_system_t* monitor, uint8_t key)
{
	bool_t rtn = FALSE;

	if(monitor->sequence_counter==1)
	{
		//Para que la secuencia sea valida el Boton1 y el Boton 2 deben ser presionados de forma alternante
		if(key != monitor->button_sequence[monitor->sequence_counter-1])
		{
			rtn = TRUE;
		}
	}
	else
	{
		rtn = TRUE;
	}
	return rtn;
}

void clear_event(monitor_system_t* monitor)
{
	monitor->events[monitor->event_counter-1].active_event=FALSE;
	//monitor->event_counter--;
}

void reset_event(monitor_system_t* monitor)
{
	for(uint8_t i = 0;i<SEQUENCE_N;i++)
	{
		monitor->button_sequence[i] = 0;
	}
	monitor->sequence_counter = 0;
	monitor->state = T1_INIT;
}

void Event_Msg(monitor_system_t* monitor)
{
	char msg1 [50];
	char msg2 [50];
	char msg3 [50];
	char msg4 [50];
	char itoa_msg[10];

	memset(msg1,0,50);
	memset(msg2,0,50);
	memset(msg3,0,50);
	memset(msg4,0,50);

	uint32_t time1_millis = monitor->events[monitor->event_counter-1].t1_timing;
	uint32_t time2_millis = monitor->events[monitor->event_counter-1].t2_timing;
	uint32_t total_time_milllis = time1_millis + time2_millis;

	if(LED_RGB_BLUE==monitor->events[monitor->event_counter-1].led)
	{
		strcpy(msg1,BLUE_LED_MSG);
	}
	else if(LED_RED==monitor->events[monitor->event_counter-1].led)
	{
		strcpy(msg1,RED_LED_MSG);
	}
	else if(LED_YELLOW==monitor->events[monitor->event_counter-1].led)
	{
		strcpy(msg1,YELLOW_LED_MSG);
	}
	else if(LED_GREEN==monitor->events[monitor->event_counter-1].led)
	{
		strcpy(msg1,GREEN_LED_MSG);
	}

	strcpy(msg2,ON_TIME_MSG);
	itoa( total_time_milllis, itoa_msg, 10 ); /* base 10 significa decimal */
	strcat(msg2,itoa_msg);
	strcat(msg2,UNI_MSG);

	memset(itoa_msg,0,10);

	strcpy(msg3,FALLING_MSG);
	itoa( time1_millis, itoa_msg, 10 ); /* base 10 significa decimal */
	strcat(msg3,itoa_msg);
	strcat(msg3,UNI_MSG);

	memset(itoa_msg,0,10);

	strcpy(msg4,RISING_MSG);
	itoa( time2_millis, itoa_msg, 10 ); /* base 10 significa decimal */
	strcat(msg4,itoa_msg);
	strcat(msg4,UNI_MSG);

	uartWriteString( UART_USB, msg1 );
	uartWriteString( UART_USB, msg2 );
	uartWriteString( UART_USB, msg3 );
	uartWriteString( UART_USB, msg4 );
}

void Goodbye_Msg()
{
	char goodbye[50];

	strcpy(goodbye,GOOGBYE_MSG);
	uartWriteString( UART_USB, goodbye);
}

bool_t	timing_action(monitor_system_t* monitor)
{
	bool_t rtn = FALSE;
	float led_timing = cyclesCounterToMs(cyclesCounterRead());
	float total_led_timing = monitor->events[monitor->event_counter-1].t1_timing+
			monitor->events[monitor->event_counter-1].t2_timing;

	if(!monitor->events[monitor->event_counter-1].active_event)
	{
		Event_Msg(monitor);
		monitor->events[monitor->event_counter-1].active_event = TRUE;
		cyclesCounterReset();
		Board_LED_Toggle(monitor->events[monitor->event_counter-1].led);
	}
	else
	{
		//Si ya se cumplio el tiempo de encendido del LED
		if(led_timing>=total_led_timing)
		{
			Goodbye_Msg();
			Board_LED_Toggle(monitor->events[monitor->event_counter-1].led);
			rtn = TRUE;
		}
	}
	return rtn;
}
