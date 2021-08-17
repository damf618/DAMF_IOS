#ifndef _TIME_TRACKER_H_
#define _TIME_TRACKER_H_

#include "stdbool.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*==================[macros]=================================================*/
#define INIT_TIME		0

#define LED_RGB_BLUE	2
#define LED_RGB_GREEN	1
#define LED_RGB_RED		0
#define LED_YELLOW		3
#define LED_RED			4
#define LED_GREEN		5

#define GPIO_RGB_BLUE	40
#define GPIO_RGB_GREEN	41
#define GPIO_RGB_RED	42
#define GPIO_YELLOW		43
#define GPIO_RED		44
#define GPIO_GREEN		45


#define MAX_T_EVENTS	50

/******* MSGS ******/
#define	GREEN_LED_MSG	"Led Verde encendido:\n\r"
#define	RED_LED_MSG		"Led Rojo encendido:\n\r"
#define	YELLOW_LED_MSG	"Led Amarillo encendido:\n\r"
#define	BLUE_LED_MSG	"Led Azul encendido:\n\r"

#define	ON_TIME_MSG		"\t Tiempo encendido: "
#define	UNI_MSG			" ms \n\r"

#define	FALLING_MSG		"\t Tiempo entre flancos descendentes: "
#define	RISING_MSG		"\t Tiempo entre flancos ascendentes: "

#define TERMINAL_MSG	"\n\r"

#define GOOGBYE_MSG		"\n\r  *--*  GAME OVER  *--* \n\r"

#define INIT_SEQUENCE	0
#define	FIRST_SEQUENCE	1
#define	SECOND_SEQUENCE	2
#define	THIRD_SEQUENCE	3
#define SEQUENCE_N		4

/*==================[typedef]================================================*/
typedef enum{
	MONITOR_INIT= 0,
	MONITORING 	= 1,
    T1_INIT 	= 2,
    T1_SET 		= 3,
    T2_INIT 	= 4,
	T2_SET 		= 5
} MONITOR_STATE;

typedef enum{
    NORMAL 		= 0,
    RISING 		= 1,
    FALLING 	= 2,
    PRESSED 	= 3
} BUTTON_STATE;

typedef enum{
    BLUE 		= 0,
    RED 		= 1,
    GREEN 		= 2,
    YELLOW 		= 3
} TEST_LED;

typedef struct timing_events_s
{
	uint32_t 		t1_timing;
	uint32_t	 	t2_timing;
	TEST_LED 		led;
	bool_t			active_event;
}timing_event_t;

typedef struct button_s
{
	uint8_t 		key;
	BUTTON_STATE	state;
	float			falling_time;
	float			rising_time;
}button_t;

typedef struct monitor_system_s
{
	button_t 		button1;
	button_t 		button2;
	timing_event_t	events[MAX_T_EVENTS];
	uint32_t 		event_counter;
	MONITOR_STATE	state;
	uint8_t			button_sequence[SEQUENCE_N];
	uint32_t 		sequence_counter;
}monitor_system_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/*==================[functions declaration]=========================*/

bool_t T1_Init(monitor_system_t* monitor, uint8_t key);

bool_t T1_Set(monitor_system_t* monitor);

bool_t T2_Init(monitor_system_t* monitor, uint8_t key);

bool_t T2_Set(monitor_system_t* monitor);

bool_t Sequence_Validation (monitor_system_t* monitor, uint8_t key);

bool_t Monitor_System_update(monitor_system_t* monitor, uint8_t key);

void Monitor_System_Init(monitor_system_t* monitor,uint8_t Key1, uint8_t key2);

bool_t timing_event_generator (monitor_system_t* monitor);

bool_t	timing_action(monitor_system_t* monitor);

void reset_event(monitor_system_t* monitor);

void clear_event(monitor_system_t* monitor);

bool_t set_sequence(monitor_system_t* monitor, uint8_t key);

/*==================[end of file]============================================*/
#endif /* #ifndef _TIME_TRACKER_H_ */

