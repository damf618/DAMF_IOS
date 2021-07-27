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
	//DEBE SER DIFERENTE DE LA INICIAL PARA QUE EL SCHEDULER FUNCIONE CORRECTAMENTE
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
	//IDLE Task activa, no hay tareas en estado READY
	if(!aux)
	{
		if(DAMF.running_task!=IDLE_TASK_INDEX)
		{
			DAMF.next_task = IDLE_TASK_INDEX;
			aux = TRUE;
		}
	}

	return aux;
}

void os_fsm_Checking(void)
{

}

void os_fsm_Error(void)
{

}
