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
	//DEBE SER DIFERENTE DE LA INICIAL PARA QUE EL SCHEDULER FUNCIONE CORRECTAMENTE
	DAMF.next_task = IDLE_TASK_INDEX;
}

bool os_fsm_Running(void)
{
	bool any_ready = FALSE;
	bool any_running = FALSE;
	bool Round_Robin_Reset = FALSE;

	uint8_t next_ready_prior;

	uint8_t possible_tasks[MAX_TASKS];

	//Determinar la siguiente tarea en READY
	for(int8_t index=0;index<DAMF.task_counter;index++)           //Scheduler con Starvation
	{
		if(DAMF.OS_Tasks[DAMF.OS_Prior[index]].state == READY)
		{
			//next_ready = DAMF.OS_Prior[index];
			next_ready_prior = DAMF.OS_Tasks[DAMF.OS_Prior[index]].prior;
			any_ready = TRUE;
			break;
		}
		else if(DAMF.OS_Tasks[DAMF.OS_Prior[index]].state == RUNNING)
		{
			any_running = TRUE;
		}
	}

	if(any_ready)
	{
		if( (DAMF.OS_Tasks[DAMF.running_task].state==BLOCKED)||(next_ready_prior >= DAMF.OS_Tasks[DAMF.running_task].prior))
		{

			//ajustedescheduler
			//memcpy(possible_tasks,DAMF.OS_Tasks_Prio[next_ready_prior],sizeof(possible_tasks));
			//DAMF.OS_Task_Arrange[tasks_prio[i]].OS_Tasks_Prio[DAMF.OS_Task_Arrange[tasks_prio[i]].n_task_counter]
			memcpy(possible_tasks,DAMF.OS_Task_Arrange[next_ready_prior].OS_Tasks_Prio,sizeof(possible_tasks));

			while(!Round_Robin_Reset)
			{
				//for(uint8_t k = 0;k<DAMF.task_counter;k++)
				for(uint8_t k = 0;k<DAMF.OS_Task_Arrange[next_ready_prior].n_task_counter;k++)
				{
					if((DAMF.OS_Tasks[possible_tasks[k]].state==READY)
							&&(DAMF.OS_Tasks[possible_tasks[k]].round_robin == FALSE ))
					{
						DAMF.OS_Tasks[possible_tasks[k]].round_robin = TRUE;
						Round_Robin_Reset = TRUE;
						if(DAMF.OS_Tasks[DAMF.running_task].state!=BLOCKED)
						{
							DAMF.OS_Tasks[DAMF.running_task].state = READY;
						}
						DAMF.next_task = possible_tasks[k];
						break;
					}
				}
				if(!Round_Robin_Reset)
				{
					//for(uint8_t k = 0;k<DAMF.task_counter;k++)
					for(uint8_t k = 0;k<DAMF.OS_Task_Arrange[next_ready_prior].n_task_counter;k++)
					{
						DAMF.OS_Tasks[possible_tasks[k]].round_robin = CLEAN;
					}
				}
			}

			return TRUE;
		}
	}
	else if(!any_running)
	{
		//IDLE Task activa, no hay tareas en estado READY
		if(DAMF.running_task!=IDLE_TASK_INDEX)
		{
			DAMF.next_task = IDLE_TASK_INDEX;
			return TRUE;
		}
	}
	return FALSE;
}

void os_fsm_Checking(void)
{


}

void os_fsm_Error(void)
{

}
