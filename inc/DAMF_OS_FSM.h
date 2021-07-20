/*
 * MSE_OS_FSM.h
 *
 *  Created on: 26 mar. 2020
 *      Author: DAMF
 */

#ifndef ISO_I_2020_MSE_OS_INC_DAMF_OS_FSM_H_
#define ISO_I_2020_MSE_OS_INC_DAMF_OS_FSM_H_


#include "DAMF_OS_Core.h"
#include "stdbool.h"

/************************************************************************************
 * 			Tamaño del stack predefinido para cada tarea expresado en bytes
 ***********************************************************************************/

/*==================[definicion de variables]=================================*/

extern struct DAMF_OS DAMF;


/*==================[definicion de prototipos]=================================*/

void os_fsm_Init(void);

bool os_fsm_Running(void);

void os_fsm_Checking(void);

void os_fsm_Error(void);

#endif /* ISO_I_2020_MSE_OS_INC_DAMF_OS_CORE_H_ */
