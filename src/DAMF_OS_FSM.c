/*
 * MSE_OS_FSM.h
 *
 *  Created on: 26 mar. 2020
 *      Author: DAMF
 */


#include <stdint.h>
#include "DAMF_OS_FSM.h"
#include "DAMF_OS_Core.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"


/************************************************************************************
 * 			Tamaño del stack predefinido para cada tarea expresado en bytes
 ***********************************************************************************/

void os_fsm_Init()
{
	DAMF.state = WORKING;
	DAMF.next_task = INIT_TASK;
}

bool os_fsm_Running(void)
{
	bool aux = FALSE;

	for(int8_t index=0;index<DAMF.task_counter;index++)           //Scheduler con Starvation
	{
		//for(int8_t index=search_index;index<DAMF.task_counter;index++){  //Scheduler V2
		if(DAMF.OS_Tasks[index].state == READY)
		{
			DAMF.OS_Tasks[DAMF.running_task].state = READY;
			DAMF.next_task = index;
			aux = TRUE;
			break;
		}
	}
/*
	static int32_t tarea_actual = -1;
	static uint8_t search_index = 0;
	uint32_t sp_siguiente;

	if((0<=tarea_actual)&&(tarea_actual<=DAMF.task_counter)){

		DAMF.OS_Tasks[tarea_actual].stack_pointer = sp_actual;           //Guardar el MSP actual para la tarea en marcha

		for(int8_t index=0;index<DAMF.task_counter;index++){           //Scheduler con Starvation
		//for(int8_t index=search_index;index<DAMF.task_counter;index++){  //Scheduler V2
			if(DAMF.OS_Tasks[index].state == READY)
			{
				DAMF.OS_Tasks[tarea_actual].state = READY;
				tarea_actual = index;
				break;
			}
		}
		sp_siguiente = DAMF.OS_Tasks[tarea_actual].stack_pointer;        //Asignacion de nuevo stackpointer
		DAMF.OS_Tasks[tarea_actual].state = RUNNING;
		DAMF.running_task = DAMF.OS_Tasks[tarea_actual].id;
	}
	else {
		sp_siguiente = DAMF.OS_Tasks[INIT_TASK].stack_pointer;           //En caso inicial entra aqui y comienza con la primera tarea
		DAMF.OS_Tasks[INIT_TASK].state = RUNNING;
		tarea_actual = INIT_TASK;
	}

	return sp_siguiente;
	*/
	return aux;
}

void os_fsm_Checking(void)
{

}

void os_fsm_Error(void)
{

}
