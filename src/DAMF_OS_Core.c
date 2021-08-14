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


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

//#define EDUCIAA		//Implementar Heartbeat en IDLE_TASK

#include "DAMF_OS_FSM.h"

struct DAMF_OS DAMF;

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

/*==================[declaracion prototipos]=================================*/

/*************************************************************************************************
	 *  @brief Seteo de Error para depuracion.
     *
     *  @details
     *  Funcion interna del sistema operativo, utilizada para el registra los errores. Los errores se
     *  encuentran registrados en DAMF_OS_Core.h
     *
	 *  @param
	 *  Error_msg Mensaje de Error a registrar.
	 *
	 *  @return none.
***************************************************************************************************/
void os_set_Error(char* Error_msg);

/*************************************************************************************************
	 *  @brief Recuperacion del ultimo  Error seteado.
     *
     *  @details
     *  Esta funcion debe extrae el ultimo error registrado en la estructura de control del sistema
     *  operativo, se debe haber llamado un error previamente para una depuracion correcta.
     *
	 *  @param none
	 *
	 *  @return none.
***************************************************************************************************/
char* os_getError(void);

/*************************************************************************************************
	 *  @brief Configuracion del stack de tareas
     *
     *  @details
     *  Configuracion de Stacks de las tareas, funciones de retorno y funcion a ejecutar (entrypoint)
     *
	 *  @param
	 *  tarea funcion a ejecutar
	 *  stack_tarea segmento de memoria asignado a la tarea
	 *	stack_pointer puntero al bloque de meoria actual dentro del segmento (stack_tarea) asignado.
	 *
	 *  @return none.
***************************************************************************************************/
void os_InitTarea(void *tarea, uint32_t *stack_tarea, uint32_t *stack_pointer);

/*************************************************************************************************
	 *  @brief Inclusion de tarea IDLE
     *
     *  @details
     *  Inclusion de tarea IDLE, considerando MAX_TASK se utiliza la siguiente posicion disponible
     *  para almacenar la tarea IDLE. Si se define EDUCIAA, la tarea IDLE utiliza el LED0 como
     *  Heartbeat.
     *
	 *  @param none
	 *
	 *  @return none.
***************************************************************************************************/
void os_Include_Idle_Task();

/*************************************************************************************************
	 *  @brief Organizacion de tareas por prioridad
     *
     *  @details
     *  El scheduler requiere que las tareas esten organizadas de mayor a menor segun la priordad.
     *  El sistema NO soporta el cambio de prioridades en ejecucion.
     *
	 *  @param
	 *  order_tasks Elemento para almacenar las tareas ordenadas por prioridad.
	 *
	 *  @return none.
***************************************************************************************************/
void sched_fix(uint8_t* order_tasks);

/*************************************************************************************************
	 *  @brief Lanzamiento de Handler de eventos de APIs
     *
     *  @details
     *  Cada API genera un tipo de evento,que puede modificar el orden del scheduler. El sistema
     *  toma un evento de la cola y lo atiende.
     *
	 *  @param none
	 *
	 *  @return none.
***************************************************************************************************/
void event_dispatcher();

/*************************************************************************************************
	 *  @brief scheduler de tareas
	 *
     *  @details
     *  El sistema operativa se basa en esta funcion para determinar que tarea debe ser ejecutada,
     *  teniendo en cuenta:
     *     - Estado: READY, BLOCKED, RUNNING.
     *     - Prioridad: 0,1,2,3. Siendo 0 el MAX y 3 el valor MIN.
     *     - Round_Robin: En caso de tener tareas de una misma prioridad, se ejecutaran de forma
     *     simultanea.
     *
	 *  @param none
	 *
	 *  @return none.
***************************************************************************************************/
static bool scheduler();

/*************************************************************************************************
	 *  @brief Handler periodico a 1ms
     *
     *  @details
     *  El sistema se basa en interrupciones cada 1ms para gestionar y controlar los eventos del OS.
     *  Esta funcion se encarga de orquestar el control y analisis del OS.
     *
	 *  @param none
	 *
	 *  @return none.
***************************************************************************************************/
void SysTick_Handler(void);

/*************************************************************************************************
	 *  @brief Cambio de Contexto
     *
     *  @details
     *  Esta funcion se utiliza durante la insterrupcion PendSV, dentro de una seccion critica.
     *  La funcion toma los stack pointers de las taraeas y los carga en el sack del MSP para
     *  realizar un cambio de contexto "inadvertido" por el CPU.
     *
	 *  @param
	 *  sp_actual	stack pointer de la tarea en ejecucion actual.
	 *
	 *  @return none.
***************************************************************************************************/
uint32_t getContextoSiguiente(uint32_t sp_actual);

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

/*************************************************************************************************
	 *  @brief Tarea Idle
     *
     *  @details
     *	Tarea a ejecutar cuando todas las tareas definidas por el usuario se encuentran en estado
     *	BLOCKED. El sistema operativo incluye la posibilidad de utilizar un Heartbeat en LED0 al
     *	definir EDUCIAA previo a la inclusion de DAMF_OS_Core.h
     *
	 *  @param none
	 *
	 *  @return none.
***************************************************************************************************/
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

/*==================[OS_APIS]=================================*/

/*
 * Esta seccion contiene las funciones que permiten al usuario interactuar con
 * el sistema operativo.
 */

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

/*************************************************************************************************
	 *  @brief Blockeo Manual de Tareas Tareas el DAMF_OS.
     *
     *  @details
     *  Utilizada en su mayoria para DEBUG del OS, hace el bloqueo de la tarea actualmente en
     *  ejecucion.
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void os_block()
{
	DAMF.OS_Tasks[DAMF.running_task].state = BLOCKED;
	os_yield();
}

//

// Include_Task
void os_Include_Task(void *tarea, const char * tag, const uint8_t Priority) {

	if(DAMF.task_counter < MAX_TASKS)
	{
		DAMF.OS_Tasks[DAMF.task_counter].function = (uint32_t) tarea;
		DAMF.OS_Tasks[DAMF.task_counter].state = READY;
		DAMF.OS_Tasks[DAMF.task_counter].id = DAMF.task_counter;
		memset(DAMF.OS_Tasks[DAMF.task_counter].tag,0,MAX_TAG_LENGTH);
		memcpy(DAMF.OS_Tasks[DAMF.task_counter].tag,tag,strlen(tag)+1);

		DAMF.OS_Tasks[DAMF.task_counter].round_robin = FALSE;


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

		os_InitTarea((void*)DAMF.OS_Tasks[DAMF.task_counter].function,DAMF.OS_Tasks[DAMF.task_counter].stack, &DAMF.OS_Tasks[DAMF.task_counter].stack_pointer);
		DAMF.task_counter ++;
	}
	else
	{
		os_set_Error(MAX_TASK_MSG);
	}
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
	DAMF.critical_counter = CLEAN;

	for(uint8_t i=0 ;i<MAX_PRIO ;i++)
	{
		DAMF.OS_Task_Arrange[i].n_task_counter = 0;
		for(uint8_t k = 0;k<MAX_TASKS;k++)
		{
				DAMF.OS_Task_Arrange[i].OS_Tasks_Prio[k] = TASK_ROUND_ROB;
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
	 *  @brief Marca el inicio de una seccion como seccion critica.
     *
     *  @details
     *   Las secciones criticas son aquellas que deben ejecutar operaciones atomicas, es decir que
     *   no pueden ser interrumpidas. Con llamar a esta funcion, se otorga soporte en el OS
     *   para marcar un bloque de codigo como atomico
     *
	 *  @param 		None
	 *  @return     None
	 *  @see 		os_enter_critical
***************************************************************************************************/
/***************CRITICAL********************/

//
static inline void os_enter_critical()  {
	__disable_irq();
	DAMF.critical_counter++;
}

/*************************************************************************************************
	 *  @brief Marca el final de una seccion como seccion critica.
     *
     *  @details
     *   Las secciones criticas son aquellas que deben ejecutar operaciones atomicas, es decir que
     *   no pueden ser interrumpidas. Con llamar a esta funcion, se otorga soporte en el OS
     *   para marcar un bloque de codigo como atomico
     *
	 *  @param 		None
	 *  @return     None
	 *  @see 		os_enter_critical
***************************************************************************************************/
static inline void os_exit_critical()  {
	if (--DAMF.critical_counter <= 0)  {
		DAMF.critical_counter = 0;
		__enable_irq();
	}
}

/*************************************************************************************************
	 *  @brief API de Creacion de Colas
     *
     *  @details
     *  El OS incluye un manejo de Colas para la transmision de informacion entre tareas.
     *
	 *  @param
	 *  queue_p    puntero a cola a configurar.
	 *  n_data     maximo numeros de datos a guardar en la cola
	 *  size_data  tamano en Bytes del tiupo de dato a guardar.
	 *
	 *
	 *  @return none.
***************************************************************************************************/
/***************QUEUE***********************/

//
void os_Queue_Create(queue_event_t * queue_p, uint8_t n_data, uint32_t size_data)
{
	uint8_t queue_lenght = 0;
	uint8_t queue_max_length = (MAX_QUEUE_SIZE/4)/size_data;

	if(size_data <= (MAX_QUEUE_SIZE/4))
	{

		if(n_data < MIN_QUEUE_LENGTH)
		{
			queue_lenght = MIN_QUEUE_LENGTH;
		}
		//TODO Error de memoria insuficiente
		else if (n_data > queue_max_length)
		{
			queue_lenght = queue_max_length;
		}
		else
		{
			queue_lenght = n_data;
		}

		queue_p->n_slots = queue_lenght;
		queue_p->queue_counter = 0;
		queue_p->slot_size = size_data;
	}
	/*TODO Error de memoria insuficiente
	else
	{

	}
	*/
}

//

//TODO CHECK Casos extremos
/*************************************************************************************************
	 *  @brief Handler de los Eventos Asociados a las Colas del OS
     *
     *  @details
     *  Pueden generarse eventos en la cola como por ejemplo: carga en cola FULL o vaciado en cola vacia.
     *  Estas situaciones son controladas por el Event_Habdler.
     *
	 *  @param none
	 *
	 *  @return none.
***************************************************************************************************/
bool queue_handler(void * prmtr)
{
	bool task_finished = FALSE;

	queue_event_t* event_data = (queue_event_t *)prmtr;

	if((event_data->queue_counter > 0) && (event_data->queue_counter <= event_data->n_slots))
	{
		DAMF.OS_Tasks[event_data->origin_task].state = READY;
		DAMF.scheduler_flag = TRUE;
		task_finished = TRUE;
	}
	return task_finished;
}

//

/*************************************************************************************************
	 *  @brief Creacion de evento de colas
     *
     *  @details
     *
     *
     *
	 *  @param
	 *  queue_p puntero a la cola asociada al evento.
	 *  running_task tarea asociada al evento de cola.
	 *
	 *  @return none.
***************************************************************************************************/
void os_queue_event_t(queue_event_t * queue_p, uint8_t running_task)
{
	queue_p->origin_task = running_task;

	DAMF.OS_Events[DAMF.events_index].prmtr =    (void *) queue_p;
	DAMF.OS_Events[DAMF.events_index].event_handler = (void*) queue_handler;
	DAMF.events_index ++;
}

//

/*************************************************************************************************
	 *  @brief Carga de datos en cola
     *
     *  @details
     *  Carga de datos en la cola indicada como parametro, en caso de que la cola no disponga espacio
     *  se procedera a bloquear la tarea y generar un evento de colas. El evento sera verificado
     *  durante los SysTick Hanlder para verificar posibles cambios en el scheduler.
     *
	 *  @param
	 *  queue_p puntero a la cola que se desea insertar el dato.
	 *  data  data a cargar en la cola
	 *
	 *  @return none.
***************************************************************************************************/
void os_push_queue(queue_event_t * queue_p, void* data)
{

	//TODO Validar espacio libre
	if(queue_p->queue_counter >= queue_p->n_slots)
	{
		DAMF.OS_Tasks[DAMF.running_task].state = BLOCKED;
		os_queue_event_t(queue_p, DAMF.running_task);
		__WFI();
	}

	uint32_t offset = queue_p->queue_counter * queue_p->slot_size;
	uint32_t slot_pointer = (uint32_t) queue_p->queue_array + offset;

	memcpy((void *)slot_pointer, data, queue_p->slot_size);
	queue_p->queue_counter++;
}

//

/*************************************************************************************************
	 *  @brief EXtraccion de datos en cola
     *
     *  @details
     *  Retiro de datos en la cola indicada como parametro, en caso de que la cola no disponga
     *   de datos almacenados, se procedera a bloquear la tarea y generar un evento de colas. El
     *  evento sera verificado durante los SysTick Hanlder para verificar posibles cambios en el
     *  scheduler.
     *
	 *  @param
	 *  queue_p puntero a la cola que se desea insertar el dato.
	 *  vari  variable donde se procedera a cargar el dato proveniente de la cola
	 *
	 *  @return none.
***************************************************************************************************/
void os_pull_queue(queue_event_t * queue_p, void* vari)
{
	//TODO Validar espacio lleno
	if(queue_p->queue_counter <= 0)
	{
		DAMF.OS_Tasks[DAMF.running_task].state = BLOCKED;
		os_queue_event_t(queue_p, DAMF.running_task);
		__WFI();
	}

	uint32_t offset = (queue_p->queue_counter+PREV) * queue_p->slot_size;
	uint32_t slot_pointer = (uint32_t) queue_p->queue_array + offset;

	memcpy(vari, (void *)slot_pointer, queue_p->slot_size);
	queue_p->queue_counter--;
}

/**************DELAY************************/

//
bool delay_handler(void* prmtr)
{
	bool task_finished = FALSE;

	delay_event_t* event_data = (delay_event_t *)prmtr;

	if(DAMF.os_tick_counter >= event_data->time_delay)
	{
		DAMF.OS_Tasks[event_data->origin_task].state = READY;
		DAMF.scheduler_flag = TRUE;
		task_finished = TRUE;
	}
	return task_finished;
}

//
void os_delay_event(const uint32_t time_delay, uint8_t running_task)
{
	DAMF.OS_Tasks[running_task].delay_event.origin_task = running_task;
	DAMF.OS_Tasks[running_task].delay_event.time_delay  = time_delay + DAMF.os_tick_counter;


	DAMF.OS_Events[DAMF.events_index].prmtr =    (void *) &DAMF.OS_Tasks[running_task].delay_event;
	DAMF.OS_Events[DAMF.events_index].event_handler = (void*) delay_handler;
	DAMF.events_index ++;
}

//
void os_delay( const uint32_t time_delay )
{
	// Bloqueo de tarea actual
	DAMF.OS_Tasks[DAMF.running_task].state = BLOCKED;
	// Creacion de evento a validar posteriormente
	os_delay_event(time_delay,DAMF.running_task);
	__WFI();
}

/**************SEMAPHORE*******************/

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

/*==================[OS_INTERNAL_APIS]=================================*/

/*
 * Esta seccion contiene las funciones internas que mantienen el sistema operativo.
 */

// Gardado de error
void os_set_Error(char* Error_msg)
{
	memcpy(DAMF.error_tag,Error_msg,strlen(Error_msg)+1);
	errorHook(os_InitTarea);
}

// Obtener el ultimo error registrado
char* os_getError(void)
{
	return DAMF.error_tag;
}

// Inicializacion de Tarea
void os_InitTarea(void *tarea, uint32_t *stack_tarea, uint32_t *stack_pointer)  {

	stack_tarea[STACK_SIZE/4 - XPSR] = INIT_XPSR;				//necesario para bit thumb
	stack_tarea[STACK_SIZE/4 - PC_REG] = (uint32_t)tarea;		//direccion de la tarea (ENTRY_POINT)

	stack_tarea[STACK_SIZE/4 - LR] = (uint32_t)returnHook;

	stack_tarea[STACK_SIZE/4 - LR_PREV_VALUE] = EXEC_RETURN;

	*stack_pointer = (uint32_t) (stack_tarea + STACK_SIZE/4 - FULL_STACKING_SIZE);
}

//

// Inclusion de tarea Idle
void os_Include_Idle_Task() {

	if(DAMF.task_counter <= MAX_TASKS)
	{
		DAMF.OS_Tasks[IDLE_TASK_INDEX].function = (uint32_t) Idle_Task;
		DAMF.OS_Tasks[IDLE_TASK_INDEX].state = READY;
		DAMF.OS_Tasks[IDLE_TASK_INDEX].id = IDLE_TASK_INDEX;
		memset(DAMF.OS_Tasks[IDLE_TASK_INDEX].tag,0,MAX_TAG_LENGTH);
		memcpy(DAMF.OS_Tasks[IDLE_TASK_INDEX].tag,IDLE_TASK_TAG,strlen(IDLE_TASK_TAG)+1);
		os_InitTarea((void*)DAMF.OS_Tasks[IDLE_TASK_INDEX].function,DAMF.OS_Tasks[IDLE_TASK_INDEX].stack, &DAMF.OS_Tasks[IDLE_TASK_INDEX].stack_pointer);
	}
	else
	{
		os_set_Error(MAX_TASK_MSG);
	}
}

//

// Organizacion de tareas por prioridad
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
		if(tasks_prio[i]>=0)
		{
			DAMF.OS_Task_Arrange[tasks_prio[i]].OS_Tasks_Prio[DAMF.OS_Task_Arrange[tasks_prio[i]].n_task_counter]=i;
			DAMF.OS_Task_Arrange[tasks_prio[i]].n_task_counter++;
		}
/*
		switch(tasks_prio[i])
		{
			case 0:
				DAMF.OS_Task_Arrange[0].OS_Tasks_Prio[DAMF.OS_Task_Arrange[0].task_counter]=i;
				//DAMF.OS_Tasks_Prio[0][counter0]=i;
				DAMF.OS_Task_Arrange[0].task_counter++;
				//counter0++;
				break;

			case 1:
				DAMF.OS_Task_Arrange[1].OS_Tasks_Prio[DAMF.OS_Task_Arrange[1].task_counter]=i;
				DAMF.OS_Task_Arrange[1].task_counter++;
				break;

			case 2:
				DAMF.OS_Task_Arrange[2].OS_Tasks_Prio[DAMF.OS_Task_Arrange[2].task_counter]=i;
				DAMF.OS_Task_Arrange[2].task_counter++;
				break;

			case 3:
				DAMF.OS_Task_Arrange[3].OS_Tasks_Prio[DAMF.OS_Task_Arrange[3].task_counter]=i;
				DAMF.OS_Task_Arrange[3].task_counter++;
				break;

			default:
				break;
		}
*/
	}


	while(actual_index-1>=counter)
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

//

// Funcion de lanzamiento de eventos
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

// scheduler del os
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

// handler del evento systick
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

// contexto siguiente
uint32_t getContextoSiguiente(uint32_t sp_actual)  {
	uint32_t sp_siguiente;

	if(DAMF.state == INIT)
	{
		DAMF.state = WORKING;
		sp_siguiente = DAMF.OS_Tasks[DAMF.next_task].stack_pointer;        //Asignacion de nuevo stackpointer
		DAMF.OS_Tasks[DAMF.next_task].state = RUNNING;
		DAMF.OS_Tasks[DAMF.next_task].round_robin = TRUE;
		DAMF.running_task = DAMF.next_task;

		return sp_siguiente;
	}
	DAMF.OS_Tasks[DAMF.running_task].stack_pointer = sp_actual;
	sp_siguiente = DAMF.OS_Tasks[DAMF.next_task].stack_pointer;        //Asignacion de nuevo stackpointer
	DAMF.OS_Tasks[DAMF.next_task].state = RUNNING;
	DAMF.running_task = DAMF.next_task;

	return sp_siguiente;
}
