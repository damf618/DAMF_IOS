/*
 * DAMF_OS_Core.c
 *
 * Created on: 10/07/2021
 *
 * based on:
 * MSE_OS_Core.c
 *
 *  Created on: 26 mar. 2020
 *      Author: gonza
 */

#include "DAMF_OS_Core.h"

struct DAMF_OS DAMF;

/*************************************************************************************************
	 *  @brief Inclusion de Tareas el DAMF_OS.
     *
     *  @details
     *   Incluye la tarea al listado de tareas internas del DAMF_OS.
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
/*
void os_Include_Task(void *tarea, uint32_t *stack, uint32_t *stack_pointer) {
	os_InitTarea(tarea->function,&tarea->stack, &tarea->stack_pointer);
	DAMF.OS_Tasks[DAMF.task_counter] = tarea;
	DAMF.task_counter ++;

}*/
void os_Include_Task(void *tarea, const char * tag) {
	//struct Tasks tempTask;
	DAMF.OS_Tasks[DAMF.task_counter].function = (uint32_t) tarea;
	DAMF.OS_Tasks[DAMF.task_counter].state = READY;
	memset(DAMF.OS_Tasks[DAMF.task_counter].tag,0,MAX_TAG_LENGTH);
	memcpy(DAMF.OS_Tasks[DAMF.task_counter].tag,tag,strlen(tag)+1);
	os_InitTarea(DAMF.OS_Tasks[DAMF.task_counter].function,&DAMF.OS_Tasks[DAMF.task_counter].stack, &DAMF.OS_Tasks[DAMF.task_counter].stack_pointer);
	DAMF.task_counter ++;

}



/*************************************************************************************************
	 *  @brief Inicializa las tareas que correran en el OS.
     *
     *  @details
     *   Inicializa una tarea para que pueda correr en el OS implementado.
     *   Es necesario llamar a esta funcion para cada tarea antes que inicie
     *   el OS.
     *
	 *  @param *tarea			Puntero a la tarea que se desea inicializar.
	 *  @param *stack			Puntero al espacio reservado como stack para la tarea.
	 *  @param *stack_pointer   Puntero a la variable que almacena el stack pointer de la tarea.
	 *  @return     None.
***************************************************************************************************/
void os_InitTarea(void *tarea, uint32_t *stack_tarea, uint32_t *stack_pointer)  {

	stack_tarea[STACK_SIZE/4 - XPSR] = INIT_XPSR;				//necesario para bit thumb
	stack_tarea[STACK_SIZE/4 - PC_REG] = (uint32_t)tarea;		//direccion de la tarea (ENTRY_POINT)


	/**
	 * El valor previo de LR (que es EXEC_RETURN en este caso) es necesario dado que
	 * en esta implementacion, se llama a una funcion desde dentro del handler de PendSV
	 * con lo que el valor de LR se modifica por la direccion de retorno para cuando
	 * se termina de ejecutar getContextoSiguiente
	 */
	stack_tarea[STACK_SIZE/4 - LR_PREV_VALUE] = EXEC_RETURN;

	/**
	 * Notar que ahora, al agregar un registro mas al stack, la definicion de FULL_STACKING_SIZE
	 * paso de ser 16 a ser 17
	 */

	*stack_pointer = (uint32_t) (stack_tarea + STACK_SIZE/4 - FULL_STACKING_SIZE);

}


/*************************************************************************************************
	 *  @brief Inicializa el OS.
     *
     *  @details
     *   Inicializa el OS seteando la prioridad de PendSV como la mas baja posible. Es necesario
     *   llamar esta funcion antes de que inicie el sistema. Es recomendable llamarla luego de
     *   inicializar las tareas
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void os_Init(void)  {
	/*
	 * Todas las interrupciones tienen prioridad 0 (la maxima) al iniciar la ejecucion. Para que
	 * no se de la condicion de fault mencionada en la teoria, debemos bajar su prioridad en el
	 * NVIC. La cuenta matematica que se observa da la probabilidad mas baja posible.
	 */
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS)-1); // No existe esta  prioridad existe la anterior 0->a este numero -1
	DAMF.task_counter = RESET_TASK;
}


/*************************************************************************************************
	 *  @brief SysTick Handler.
     *
     *  @details
     *   El handler del Systick no debe estar a la vista del usuario. Dentro se setea como
     *   pendiente la excepcion PendSV.
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void SysTick_Handler(void)  {

	/**
	 * Se setea el bit correspondiente a la excepcion PendSV
	 */
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

	/**
	 * Instruction Synchronization Barrier; flushes the pipeline and ensures that
	 * all previous instructions are completed before executing new instructions
	 */
	__ISB();

	/**
	 * Data Synchronization Barrier; ensures that all memory accesses are
	 * completed before next instruction is executed
	 */
	__DSB();
}




/*************************************************************************************************
	 *  @brief Funcion para determinar el proximo contexto.
     *
     *  @details
     *   Esta funcion en este momento hace las veces de scheduler y tambien obtiene el siguiente
     *   contexto a ser cargado. El cambio de contexto se ejecuta en el handler de PendSV, dentro
     *   del cual se llama a esta funcion
     *
	 *  @param 		sp_actual	Este valor es una copia del contenido de MSP al momento en
	 *  			que la funcion es invocada.
	 *  @return     El valor a cargar en MSP para apuntar al contexto de la tarea siguiente.
***************************************************************************************************/
uint32_t getContextoSiguiente(uint32_t sp_actual)  {
	static int32_t tarea_actual = -1;
	static uint8_t search_index = 0;
	//static int32_t tarea_actual = 0;
	uint32_t sp_siguiente;

	if((0<=tarea_actual)&&(tarea_actual<=DAMF.task_counter)){

		DAMF.OS_Tasks[tarea_actual].stack_pointer = sp_actual;           //Guardar el MSP actual para la tarea en marcha
		/*
		search_index = tarea_actual+1;                                   //Tarea siguiente en el listado
		if(search_index >= DAMF.task_counter){
			search_index = INIT_TASK;
		}
		*/
		for(int8_t index=0;index<DAMF.task_counter;index++){           //Scheduler con Starvation
		//for(int8_t index=search_index;index<DAMF.task_counter;index++){  //Scheduler V2
			if(DAMF.OS_Tasks[index].state == READY){
				DAMF.OS_Tasks[tarea_actual].state = READY;
				tarea_actual = index;
				break;
			}
		}
		sp_siguiente = DAMF.OS_Tasks[tarea_actual].stack_pointer;        //Asignacion de nuevo stackpointer
		DAMF.OS_Tasks[tarea_actual].state = RUNNING;
	}
	else {
		sp_siguiente = DAMF.OS_Tasks[INIT_TASK].stack_pointer;           //En caso inicial entra aqui y comienza con la primera tarea
		DAMF.OS_Tasks[INIT_TASK].state = RUNNING;
		tarea_actual = INIT_TASK;
	}



	/**
	 * Este bloque switch-case hace las veces de scheduler. Es el mismo codigo que
	 * estaba anteriormente implementado en el Systick handler
	 */
//	switch(tarea_actual)  {

	/**
	 * Tarea actual es tarea1. Recuperamos el stack pointer (MSP) y lo
	 * almacenamos en sp_tarea1. Luego cargamos en la variable de retorno
	 * sp_siguiente el valor del stack pointer de la tarea2
	 */
/*	case 1:
		Task1.stack_pointer = sp_actual;
		//sp_tarea1 = sp_actual;
		//sp_siguiente = sp_tarea2;
		//tarea_actual = 2;
		sp_siguiente = Task2.stack_pointer;
		tarea_actual = 2;
		break;

	/**
	 * Tarea actual es tarea2. Recuperamos el stack pointer (MSP) y lo
	 * almacenamos en sp_tarea2. Luego cargamos en la variable de retorno
	 * sp_siguiente el valor del stack pointer de la tarea1
	 */
/*	case 2:
		Task2.stack_pointer = sp_actual;
		//sp_tarea2 = sp_actual;
		//sp_siguiente = sp_tarea1;
		//tarea_actual = 1;
		sp_siguiente = Task3.stack_pointer;
		tarea_actual = 3;
		break;

	case 3:
		Task3.stack_pointer = sp_actual;
		//sp_siguiente = sp_tarea1;
		//tarea_actual = 1;
		sp_siguiente = Task1.stack_pointer;
		tarea_actual = 1;
		break;

	/**
	 * Este es el caso del inicio del sistema, donde no se ha llegado aun a la
	 * primer ejecucion de tarea1. Por lo que se cargan los valores correspondientes
	 */

/*	default:
		sp_siguiente = Task1.stack_pointer;
		tarea_actual = 1;
		break;
	}
*/
	return sp_siguiente;
}
