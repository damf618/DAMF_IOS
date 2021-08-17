/*
 * MSE_OS_Core.h
 *
 *  Created on: 26 mar. 2020
 *      Author: gonza
 */

#ifndef ISO_I_2020_MSE_OS_INC_DAMF_OS_CORE_H_
#define ISO_I_2020_MSE_OS_INC_DAMF_OS_CORE_H_

#define EDUCIAA		//Implementar Heartbeat en IDLE_TASK

#include "stdbool.h"
#include <stdint.h>
#include "board.h"
#include "stdio.h"
#include "string.h"
#include "DAMF_OS_IRQ.h"
#include "GPIOInt_Setup.h"

/************************************************************************************
 * 			Definiciones de Tipo de Variables
 ***********************************************************************************/
#define damf_semaphore        semaphore_event_t
#define damf_queue			  queue_event_t

/************************************************************************************
 * 			Tamaño del stack predefinido para cada tarea expresado en bytes
 ***********************************************************************************/

#define STACK_SIZE 512

//----------------------------------------------------------------------------------



/************************************************************************************
 * 	Posiciones dentro del stack de los registros que lo conforman
 ***********************************************************************************/

#define XPSR			1
#define PC_REG			2
#define LR				3
#define R12				4
#define R3				5
#define R2				6
#define R1				7
#define R0				8
#define LR_PREV_VALUE	9
#define R4				10
#define R5				11
#define R6				12
#define R7				13
#define R8				14
#define R9				15
#define R10 			16
#define R11 			17

//----------------------------------------------------------------------------------


/************************************************************************************
 * 			Valores necesarios para registros del stack frame inicial
 ***********************************************************************************/

#define INIT_XPSR 	1 << 24				//xPSR.T = 1
#define EXEC_RETURN	0xFFFFFFF9			//retornar a modo thread con MSP, FPU no utilizada

//----------------------------------------------------------------------------------


/************************************************************************************
 * 						Definiciones varias
 ***********************************************************************************/
#define STACK_FRAME_SIZE			8
#define FULL_STACKING_SIZE 			17	                         //16 core registers + valor previo de LR

#define FALSE   			        0

#define CLEAN                       0
#define PREV                       -1

#define MAX_N_EVENTS                100
#define MAX_N_SEMA		            16
#define MAX_QUEUE_SIZE				350
#define MIN_QUEUE_LENGTH			1

#define MIN_SEMA					1
#define MAX_SEMA					10
#define EVAL_PRIO                   0
#define NO_PRIO                     -1
#define MIN_PRIO                    1
#define MAX_PRIO                    3
#define CANT_PRIO					4
#define HEARTBEAT_TIMING	        50
#define INIT_TASK 			        0
#define MAX_LOG  			        100	                         //Maximo numero de cambios de conextos registrables
#define MAX_TAG_LENGTH		        80	                         //Maximo numero de caracteres del tag de la tarea
#define MAX_TASKS  			        8	                         //Maximo numero de tareas implementadas
#define MAX_NUMBER_TASKS			MAX_TASKS+1
#define IDLE_TASK_INDEX				8
#define RESET_TIME 			        0
#define RESET_COUNTER		        0
#define RESET_TASK   		        0
#define TASK_ROUND_ROB				55

#define IDLE_TASK_TAG				"DAMF_OS IDLE TASK"
#define MAX_TASK_MSG	            "MAX NUMBER OF TASKS REACHED"
#define TASK_RETURN_MSG				"TASK HAS RETURNED"
#define IDLE_TASK_RETURN_MSG		"IDLE TASK HAS RETURNED"
#define NO_INIT_MSG					"OS_INIT MUST BE CALLED BEFORE"
#define NO_QUEUE_MSG				"NO QUEUE POINTER WAS FIND"
#define NO_SEMA_MSG					"NO SEMA POINTER WAS FIND"


/*==================[definicion de datos externa]=================================*/

extern uint32_t sp_tarea1;					//Stack Pointer para la tarea 1
extern uint32_t sp_tarea2;					//Stack Pointer para la tarea 2
extern struct Tasks Task1;
extern struct Tasks Task2;
extern struct Tasks Task3;



/*==================[definicion de estructuras]=================================*/


typedef enum{
    RUNNING = 1,
    READY = 0,
    BLOCKED = 2,
    SUSPENDED = 3
} TASK_STATE;

//VALIDFAR CAMBIO INIT
typedef enum{
    INIT = 63,
    WORKING = 0,
	IRQ = 4,
    CHECKING = 2,
	ERROR_H = 3
} OS_STATE;

typedef struct semaphore_events
{
	uint8_t Sema_counter;
	uint8_t Total_counter;
	uint8_t origin_task;
}semaphore_event_t;


typedef struct queue_events
{
	void* prmt;
	uint32_t n_slots;
	uint32_t slot_size;
	uint8_t queue_counter;
	uint32_t queue_array[MAX_QUEUE_SIZE/4];
	uint8_t origin_task;
}queue_event_t;


typedef struct delay_events
{
	uint32_t time_delay;
	uint32_t origin_task;
}delay_event_t;

typedef struct priority_struct
{
	uint8_t OS_Tasks_Prio[MAX_TASKS];
	uint8_t n_task_counter;
}priority_t;


struct Tasks {
	uint32_t stack_pointer;
	uint32_t stack[STACK_SIZE/4];
	uint32_t exec_count;
	uint32_t time_count;
	uint32_t function;
	uint8_t id;
	uint8_t prior;
	char tag[MAX_TAG_LENGTH];
	uint8_t free_stack;
	TASK_STATE state;
	struct delay_events delay_event;
	bool round_robin;
};

struct Events {
	void* prmtr;
	bool (*event_handler)(void*);
};

struct DAMF_OS {
	uint8_t running_task;
	uint8_t next_task;
	uint8_t tasks_log[MAX_LOG];
	uint8_t OS_Prior[MAX_TASKS];
	struct Tasks OS_Tasks[MAX_NUMBER_TASKS];
	uint8_t task_counter;
	char error_tag[MAX_TAG_LENGTH];
	OS_STATE state;
	bool os_tick_led;
	int32_t os_tick_counter;
	uint16_t events_index;
	struct Events OS_Events[MAX_N_EVENTS];
	bool scheduler_flag;
	uint8_t critical_counter;
	priority_t OS_Task_Arrange[CANT_PRIO];
};

/*==================[definicion de prototipos]=================================*/

void os_Include_Task(void *tarea, const char * tag, const uint8_t Priority);

void os_Running_Include_Task(void *tarea, const char * tag, const uint8_t Priority);

void os_Init(void);

void os_yield(void);

void os_block(void);

void os_Run(void);

void os_Semaphore_Create(semaphore_event_t * pointer, uint8_t N_config);

void os_Sema_Take(semaphore_event_t * pointer);

void os_Sema_Free(semaphore_event_t * pointer);

void os_delay( const uint32_t time_delay );

void os_Queue_Create(queue_event_t * queue_p, uint8_t n_data, uint32_t size_data);

void os_push_queue(queue_event_t * queue_p, void* data);

void os_pull_queue(queue_event_t * queue_p, void* vari);


#endif /* ISO_I_2020_MSE_OS_INC_DAMF_OS_CORE_H_ */
