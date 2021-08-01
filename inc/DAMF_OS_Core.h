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


/************************************************************************************
 * 			Tamaño del stack predefinido para cada tarea expresado en bytes
 ***********************************************************************************/

#define STACK_SIZE 256

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
#define TRUE		   		        1

#define HEARTBEAT_TIMING	        50
#define INIT_TASK 			        0
#define MAX_LOG  			        100	                         //Maximo numero de cambios de conextos registrables
#define MAX_TAG_LENGTH		        50	                         //Maximo numero de caracteres del tag de la tarea
#define MAX_TASKS  			        8	                         //Maximo numero de tareas implementadas
#define MAX_NUMBER_TASKS			MAX_TASKS+1
#define IDLE_TASK_INDEX				8
#define RESET_TIME 			        0
#define RESET_COUNTER		        0
#define RESET_TASK   		        0

#define IDLE_TASK_TAG	            "DAMF_OS IDLE TASK "
#define DEFAULT_TAG		            "TASK #"                     //En caso de que no se defina un tag a la tarea
#define ERROR_STACK		            "STACK MEMORY UNDETERMINED"  //Si no es posible adquirir la memoria disponible
#define FREE_STACK_MSG	            "THE MEMORY AVAILABLE IS: "
#define MAX_TASK_MSG	            "MAX NUMBER OF TASKS REACHED"



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

typedef enum{
    INIT = 1,
    WORKING = 0,
    CHECKING = 2,
	ERROR_H = 3
} OS_STATE;


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
};

struct DAMF_OS {
	uint8_t running_task;
	uint8_t next_task;
	uint8_t tasks_log[MAX_LOG];
	struct Tasks OS_Tasks[MAX_NUMBER_TASKS];
	uint8_t task_counter;
	char error_tag[MAX_TAG_LENGTH];
	OS_STATE state;
	bool os_tick_led;
	int32_t os_tick_counter;
};

/*==================[definicion de prototipos]=================================*/

void os_Include_Task(void *tarea, const char * tag);

void os_Init(void);

void os_yield(void);

void os_block(void);

void os_Run(void);

#endif /* ISO_I_2020_MSE_OS_INC_DAMF_OS_CORE_H_ */
