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


#include "stdio.h"
#include <stdint.h>
#include <stdbool.h>
#include "DAMF_OS_FSM.h"

struct DAMF_OS DAMF;

/*==================[definicion de prototipos]=================================*/

static void os_InitTarea(void *tarea, uint32_t *stack, uint32_t *stack_pointer);
static bool scheduler(void);
static void os_set_Error(char* Error_msg);

/*=============================================================================*/



/*==================[definicion de hooks debiles]=================================*/

/*
 * Esta seccion contiene los hooks de sistema, los cuales el usuario del OS puede
 * redefinir dentro de su codigo y poblarlos segun necesidad
 */

/*************************************************************************************************
	 *  @brief Hook de retorno de tareas
     *
     *  @details
     *   Esta funcion no deberia accederse bajo ningun concepto, porque ninguna tarea del OS
     *   debe retornar. Si lo hace, es un comportamiento anormal y debe ser tratado.
     *
	 *  @param none
	 *
	 *  @return none.
***************************************************************************************************/
void __attribute__((weak)) returnHook(void)  {
	while(1);
}

/*************************************************************************************************
	 *  @brief Hook de tick de sistema
     *
     *  @details
     *   Se ejecuta cada vez que se produce un tick de sistema. Es llamada desde el handler de
     *   SysTick.
     *
	 *  @param none
	 *
	 *  @return none.
	 *
	 *  @warning	Esta funcion debe ser lo mas corta posible porque se ejecuta dentro del handler
     *   			mencionado, por lo que tiene prioridad sobre el cambio de contexto y otras IRQ.
	 *
	 *  @warning 	Esta funcion no debe bajo ninguna circunstancia utilizar APIs del OS dado
	 *  			que podria dar lugar a un nuevo scheduling.
***************************************************************************************************/
void __attribute__((weak)) tickHook(void)  {
	__asm volatile( "nop" );
}

/*************************************************************************************************
	 *  @brief Hook de error de sistema
     *
     *  @details
     *   Esta funcion es llamada en caso de error del sistema, y puede ser utilizada a fin de hacer
     *   debug. El puntero de la funcion que llama a errorHook es pasado como parametro para tener
     *   informacion de quien la esta llamando, y dentro de ella puede verse el codigo de error
     *   en la estructura de control de sistema. Si ha de implementarse por el usuario para manejo
     *   de errores, es importante tener en cuenta que la estructura de control solo esta disponible
     *   dentro de este archivo.
     *
	 *  @param caller		Puntero a la funcion donde fue llamado errorHook. Implementado solo a
	 *  					fines de trazabilidad de errores
	 *
	 *  @return none.
***************************************************************************************************/
void __attribute__((weak)) errorHook(void *caller)  {
	/*
	 * Revisar el contenido de control_OS.error para obtener informacion. Utilizar os_getError()
	 */
	while(1);
}

/*
 *
 *
 * */
void __attribute__((weak)) Idle_Task(void)  {

#ifdef EDUCIAA
	#include "board.h"
	#define LED 0

	while(1)
	{
		if(DAMF.os_tick_counter % HEARTBEAT_TIMING == CLEAN )
		{
			Board_LED_Toggle(LED);
		}
		//NO esta bloqueando y se va al retorno
		__WFI();
	}
#else
	__WFI();
#endif
}

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

//
void os_block()
{
	DAMF.OS_Tasks[DAMF.running_task].state = BLOCKED;
	os_yield();
}

/*
 * En caos de la prioridad si es menor o mayor del rango maximo MIN_PRIO (1) y MAX_PRIO (5).
 * */
void os_Include_Task(void *tarea, const char * tag, const uint8_t Priority) {
	//struct Tasks tempTask;
	if(DAMF.task_counter < MAX_TASKS)
	{
		DAMF.OS_Tasks[DAMF.task_counter].function = (uint32_t) tarea;
		DAMF.OS_Tasks[DAMF.task_counter].state = READY;
		DAMF.OS_Tasks[DAMF.task_counter].id = DAMF.task_counter;
		memset(DAMF.OS_Tasks[DAMF.task_counter].tag,0,MAX_TAG_LENGTH);
		memcpy(DAMF.OS_Tasks[DAMF.task_counter].tag,tag,strlen(tag)+1);

		if(Priority<MIN_PRIO)
		{
			DAMF.OS_Tasks[DAMF.task_counter].prior = MIN_PRIO;
		}
		else if (Priority>MAX_PRIO)
		{
			DAMF.OS_Tasks[DAMF.task_counter].prior = MAX_PRIO;
		}
		else
		{
			DAMF.OS_Tasks[DAMF.task_counter].prior = Priority;
		}

		//CHECK TODO
		os_InitTarea((void*)DAMF.OS_Tasks[DAMF.task_counter].function,DAMF.OS_Tasks[DAMF.task_counter].stack, &DAMF.OS_Tasks[DAMF.task_counter].stack_pointer);
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
	errorHook(os_InitTarea);
}

/*************************************************************************************************
	 *  @brief Retorno de mensaje de error.
     *
     *  @details
     *  Permite obtener el último el error cargado por el OS.
     *
	 *  @return  Error_msg   Mensaje que describe el error detectado.
***************************************************************************************************/
char* os_getError(void)
{
	return DAMF.error_tag;
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

	stack_tarea[STACK_SIZE/4 - LR] = (uint32_t)returnHook;


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
	memset(DAMF.error_tag,CLEAN,MAX_TAG_LENGTH);
	DAMF.running_task = IDLE_TASK_INDEX;
	DAMF.next_task = INIT_TASK;
	DAMF.state = INIT;
	DAMF.os_tick_counter = CLEAN;
	DAMF.events_index = CLEAN;
	DAMF.scheduler_flag = FALSE;
}

void os_Include_Idle_Task() {
	//struct Tasks tempTask;

	if(DAMF.task_counter <= MAX_TASKS)
	{
		DAMF.OS_Tasks[IDLE_TASK_INDEX].function = (uint32_t) Idle_Task;
		DAMF.OS_Tasks[IDLE_TASK_INDEX].state = READY;
		DAMF.OS_Tasks[IDLE_TASK_INDEX].id = IDLE_TASK_INDEX;
		memset(DAMF.OS_Tasks[IDLE_TASK_INDEX].tag,0,MAX_TAG_LENGTH);
		memcpy(DAMF.OS_Tasks[IDLE_TASK_INDEX].tag,IDLE_TASK_TAG,strlen(IDLE_TASK_TAG)+1);
		//CHECK TODO
		os_InitTarea((void*)DAMF.OS_Tasks[IDLE_TASK_INDEX].function,DAMF.OS_Tasks[IDLE_TASK_INDEX].stack, &DAMF.OS_Tasks[IDLE_TASK_INDEX].stack_pointer);
	}
	else
	{
		os_set_Error(MAX_TASK_MSG);
	}
}

//TODO VALIDAR REORGANIZAR EL SCHEDULER CON EDU-CIAA
void sched_fix(uint8_t* order_tasks)
{
	int8_t  tasks_prio [MAX_TASKS];
	uint8_t tasks_fix [MAX_TASKS];
	uint8_t max = 0;
	uint8_t max_index = 0;
	uint8_t actual_index = DAMF.task_counter;
	uint8_t counter = 0;

	//create a vector of the tasks priorities
	for(uint8_t i=0;i<MAX_TASKS;i++)
	{
		if(i<actual_index)
		{
			tasks_prio[i]=DAMF.OS_Tasks[i].prior;
		}
		else
		{
			tasks_prio[i]=-1;
		}
	}


	while(actual_index-1>counter)
	{
		//Search for the index of the greater priority value
		for(uint8_t i=0;i<actual_index;i++)
		{
			if(max<=tasks_prio[i])
			{
				max = tasks_prio[i];
				max_index = i;
			}
		}
		//Reset the search and delete the actual max value
		max= EVAL_PRIO;
		tasks_prio[max_index] = NO_PRIO;
		//save the actual max index
		tasks_fix[counter] = max_index;
		counter++;
	}
	memcpy(order_tasks,tasks_fix,sizeof(tasks_fix));
}

//TODO VALIDAR DELAY

bool delay_handler(void* prmtr)
{
	bool task_finished = FALSE;

	delay_event_t* event_data = (delay_event_t *)prmtr;

	if(DAMF.os_tick_counter >= event_data[0].time_delay)
	{
		DAMF.OS_Tasks[event_data[0].origin_task].state = READY;
		DAMF.scheduler_flag = TRUE;
		task_finished = TRUE;
	}
	return task_finished;
}

void os_delay_event(const uint32_t time_delay, uint8_t running_task)
{
	DAMF.OS_Tasks[running_task].delay_event.origin_task = running_task;
	DAMF.OS_Tasks[running_task].delay_event.time_delay  = time_delay + DAMF.os_tick_counter;


	DAMF.OS_Events[DAMF.events_index].prmtr =    (void *) &DAMF.OS_Tasks[running_task].delay_event;
	DAMF.OS_Events[DAMF.events_index].event_handler = (void*) delay_handler;
	DAMF.events_index ++;
}

void os_delay( const uint32_t time_delay )
{
	// Bloqueo de tarea actual
	DAMF.OS_Tasks[DAMF.running_task].state = BLOCKED;
	// Creacion de evento a validar posteriormente
	os_delay_event(time_delay,DAMF.running_task);
	__WFI();
}

void event_dispatcher()
{
	bool finished_event = FALSE;
	if (DAMF.events_index > CLEAN)
	{
		for(uint32_t i=0;i<DAMF.events_index;i++)
		{
			//ejecutar el handler del evento correspondiente con su parametro correspondiente
			finished_event = (*DAMF.OS_Events[i].event_handler)(DAMF.OS_Events[i].prmtr);
			if(finished_event)
			{
				DAMF.events_index--;
				//Rotacion de eventos
				for(uint32_t j=i;j<DAMF.events_index;j++)
				{
					DAMF.OS_Events[j].prmtr = DAMF.OS_Events[j+1].prmtr;
					DAMF.OS_Events[j].event_handler = DAMF.OS_Events[j+1].event_handler;
				}
				i--; //Se reinicia esta ronda del for, para lograr ejecutar la tarea siguiente cuyo indice cambiò
			}
		}
	}
}

/*************************************************************************************************
	 *  @brief Arranque del OS.
     *
     *  @details
     *  Debe ser llamada luego del OS_Init y posterior a la creación de todas las tareas generadas por el ussuario.
     *  Crea la Tarea IDLE para gestionar las tareas y funciones del OS a partir de la ùltima tarea creada.
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void os_Run(void)
{
	os_Include_Idle_Task();
	//TODO Validar
	//Al iniciar ajustar el scheduler
	sched_fix(DAMF.OS_Prior);
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

	bool sche = FALSE;
	DAMF.os_tick_counter++;

	event_dispatcher();

	sche =scheduler();

	/*
		 * Luego de determinar cual es la tarea siguiente segun el scheduler, se ejecuta la funcion
		 * tickhook.
	 */

	tickHook();

	if (sche)
	{
		DAMF.scheduler_flag = FALSE;

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

//
bool sema_handler( void* prmtr )
{
	semaphore_event_t* sema = (semaphore_event_t*) prmtr;

	if(sema->Sema_counter >= sema->Total_counter)
	{
		DAMF.OS_Tasks[sema->origin_task].state = BLOCKED;
	}
	else
	{
		DAMF.OS_Tasks[sema->origin_task].state = READY;
	}

	DAMF.scheduler_flag = TRUE;

	/*
	DAMF.OS_Events[DAMF.events_index+PREV].prmtr = DAMF.OS_Events[DAMF.events_index].prmtr;
	DAMF.OS_Events[DAMF.events_index+PREV].event_handler = DAMF.OS_Events[DAMF.events_index].event_handler;
	DAMF.events_index--;
	*/

	return TRUE;
}

//
void os_Semaphore_Create(semaphore_event_t * pointer, uint8_t N_config)
{
	//TODO VALIDA CREACION
	uint8_t config_n = CLEAN;

	if(N_config < MIN_SEMA)
	{
		config_n = MIN_SEMA;
	}
	else if(N_config > MAX_SEMA)
	{
		config_n = MAX_SEMA;
	}
	else
	{
		config_n = N_config;
	}
	pointer[0].Total_counter = config_n;
	pointer[0].Sema_counter = config_n;

	//TODO Set ERROR else max numero de semaphores alcanzado
}

//
void os_semaphore_event(semaphore_event_t * pointer)
{
	DAMF.OS_Events[DAMF.events_index].prmtr =    (void *) pointer;
	DAMF.OS_Events[DAMF.events_index].event_handler = (void*) sema_handler;
	DAMF.events_index ++;
}

//
void os_Sema_Take(semaphore_event_t * pointer)
{
	semaphore_event_t* sema = pointer;

	if(sema->Sema_counter < sema->Total_counter)
	{
		sema->Sema_counter++;
	}
	else
	{
		sema->origin_task = DAMF.running_task;
		//Si se ha alcanzado el max numero de Takes, genero un evento Semaphore
		os_semaphore_event(pointer);
		__WFI();
	}
}

//
void os_Sema_Free(semaphore_event_t * pointer)
{
	semaphore_event_t* sema = pointer;

	if(sema->Sema_counter > CLEAN)
	{
		sema->Sema_counter--;
		/*Si libero un semaforo, puede que se desbloquee alguna tarea, por lo
		que se genera un evento Semaphore*/
		os_semaphore_event(sema);
		__WFI();
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

	if(DAMF.state == INIT)
	{
		DAMF.state = WORKING;
		sp_siguiente = DAMF.OS_Tasks[DAMF.next_task].stack_pointer;        //Asignacion de nuevo stackpointer
		DAMF.OS_Tasks[DAMF.next_task].state = RUNNING;
		DAMF.running_task = DAMF.next_task;

		return sp_siguiente;
	}
	DAMF.OS_Tasks[DAMF.running_task].stack_pointer = sp_actual;
	sp_siguiente = DAMF.OS_Tasks[DAMF.next_task].stack_pointer;        //Asignacion de nuevo stackpointer
	DAMF.OS_Tasks[DAMF.next_task].state = RUNNING;
	DAMF.running_task = DAMF.next_task;

	return sp_siguiente;
}
