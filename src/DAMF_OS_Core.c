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

#include "DAMF_OS_FSM.h"
#include "stdio.h"
#include <stdint.h>
#include <stdbool.h>

struct DAMF_OS DAMF;

/*==================[definicion de prototipos]=================================*/

static void os_InitTarea(void *tarea, uint32_t *stack, uint32_t *stack_pointer);
static bool scheduler(void);
static void os_set_Error(char* Error_msg);

/*=============================================================================*/

/*************************************************************************************************
	 *  @brief Ceder CPU de Tareas el DAMF_OS.
     *
     *  @details
     *  Cede el CPU de la tarea actual y llama al scheduler.
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/

void os_yield()
{
	//TODO LLamada a Interrupción SysTick Handler
	__WFI();
}

void os_block()
{
	DAMF.OS_Tasks[DAMF.running_task].state = BLOCKED;
	os_yield();
}

void os_Include_Task(void *tarea, const char * tag) {
	//struct Tasks tempTask;
	if(DAMF.task_counter < MAX_TASKS)
	{
		DAMF.OS_Tasks[DAMF.task_counter].function = (uint32_t) tarea;
		DAMF.OS_Tasks[DAMF.task_counter].state = READY;
		DAMF.OS_Tasks[DAMF.task_counter].id = DAMF.task_counter;
		memset(DAMF.OS_Tasks[DAMF.task_counter].tag,0,MAX_TAG_LENGTH);
		memcpy(DAMF.OS_Tasks[DAMF.task_counter].tag,tag,strlen(tag)+1);
		os_InitTarea(DAMF.OS_Tasks[DAMF.task_counter].function,&DAMF.OS_Tasks[DAMF.task_counter].stack, &DAMF.OS_Tasks[DAMF.task_counter].stack_pointer);
		DAMF.task_counter ++;
	}
	else
	{
		os_set_Error(MAX_TASK_MSG);
	}
}

/*************************************************************************************************
	 *  @brief Asignacion de mensaje de error.
     *
     *  @details
     *   Permite guardar el error dentro de la estructura para su posterior indicacion al usuario.
     *
	 *  @param Error_msg   Mensaje que describe el error detectado.
	 *  @return     None.
***************************************************************************************************/

static void os_set_Error(char* Error_msg)
{
	memcpy(DAMF.error_tag,Error_msg,strlen(Error_msg)+1);
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
static void os_InitTarea(void *tarea, uint32_t *stack_tarea, uint32_t *stack_pointer)  {

	/*
		 * Al principio se efectua un pequeño checkeo para determinar si llegamos a la cantidad maxima de
		 * tareas que pueden definirse para este OS. En el caso de que se traten de inicializar mas tareas
		 * que el numero maximo soportado, se guarda un codigo de error en la estructura de control del OS
		 * y la tarea no se inicializa.
		 */

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
	memset(DAMF.error_tag,0,MAX_TAG_LENGTH);
	DAMF.running_task = INIT_TASK;
	DAMF.next_task = INIT_TASK;
	DAMF.state = INIT;
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
static bool scheduler()  {
	bool aux = FALSE;
	//FSM DAMF_OS
	if(DAMF.state == INIT)
	{
		os_fsm_Init();
		aux = TRUE;
	}
	else if(DAMF.state == WORKING)
	{
		if(os_fsm_Running())
		{
			aux = TRUE;
		}
	}
	/*
	else if(DAMF.state == CHECKING)
	{
		os_fsm_Checking();
	}
	*/
	else if(DAMF.state == ERROR_H)
	{
		os_fsm_Error();
		aux = TRUE;
	}
	else
	{
		os_fsm_Init();
	}
	return aux;
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

	bool sche =scheduler();
	if (sche)
	{

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
	uint32_t sp_siguiente;

	DAMF.OS_Tasks[DAMF.running_task].stack_pointer = sp_actual;
	sp_siguiente = DAMF.OS_Tasks[DAMF.next_task].stack_pointer;        //Asignacion de nuevo stackpointer
	DAMF.OS_Tasks[DAMF.next_task].state = RUNNING;
	DAMF.running_task = DAMF.next_task;

	return sp_siguiente;
}
