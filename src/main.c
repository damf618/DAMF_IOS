/*==================[inclusions]=============================================*/

#include "main.h"
#include "board.h"
#include "sapi.h"
#include <stdio.h>
#include <stdlib.h>
#include "../inc/DAMF_OS_Core.h"
#include "../inc/time_tracker.h"

/*==================[macros and definitions]=================================*/
#define MILISEC	1000
//#define DEBUG_CIA

/*==================[Global data declaration]==============================*/

damf_semaphore Event_Sema;
damf_queue     Button_Queue;
monitor_system_t monitor;

/*==================[internal functions declaration]=========================*/
static void initHardware(void)  {
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / MILISEC);		//systick 1ms
}

/*==================[Definicion de tareas para el OS]==========================*/

void B1_Interrupt(void* prmtr)
{
	bool_t sequence_state = FALSE;
	uint8_t key = TEC1;
	//Limpiar bandera de Interrupcion
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX1));

	sequence_state = Sequence_Validation ( &monitor, key);

	if(sequence_state)
	{
		set_sequence(&monitor,key);
		os_push_queue(&Button_Queue,&key);
	}
	else
	{
		reset_event(&monitor);
	}
}

void B2_Interrupt(void* prmtr)
{
	bool_t sequence_state = FALSE;
	uint8_t key = TEC2;

	//Limpiar bandera de Interrupcion
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX2));

	sequence_state = Sequence_Validation (&monitor, key);

	if(sequence_state)
	{
		set_sequence(&monitor,key);
		os_push_queue(&Button_Queue,&key);
	}
	else
	{
		reset_event(&monitor);
	}
}

void woaow(void)
{
	while(1)
	{
		Board_LED_Toggle(0);
		#ifdef DEBUG_CIA
			os_delay(2);//DELAY ms
		#else
			os_delay(200);//DELAY ms
		#endif
		Board_LED_Toggle(1);
		#ifdef DEBUG_CIA
			os_delay(2);//DELAY ms
		#else
			os_delay(200);//DELAY ms
		#endif
		Board_LED_Toggle(2);
		#ifdef DEBUG_CIA
			os_delay(2);//DELAY ms
		#else
			os_delay(800);//DELAY ms
		#endif
	}
}

void Event_Dispatcher(void)
{
	bool_t finished_event = FALSE;

	while (1)
	{
		os_Sema_Take(&Event_Sema);
		while(!finished_event)
		{
			finished_event = timing_action(&monitor);
		}
		clear_event(&monitor);
		finished_event = FALSE;
	}
}

void Monitor_FSM(void)
{
	uint8_t key;
	bool_t event_state = FALSE;

	while (1)
	{
		os_pull_queue(&Button_Queue,&key);
		event_state = Monitor_System_update(&monitor,key);

		if(event_state)
		{
			timing_event_generator(&monitor);
			//os_push_queue(&Event_Queue,&monitor.events[monitor.event_counter-1]);
			os_Sema_Free(&Event_Sema);
			event_state=FALSE;
		}
	}

}

/*============================================================================*/

int main(void)  {

	initHardware();

	os_Semaphore_Create(&Event_Sema, 1);

	//os_Queue_Create(&Event_Queue,10,sizeof(uint32_t));
	os_Queue_Create(&Button_Queue,10,sizeof(uint32_t));

	os_Init();

	os_Include_Task(&Event_Dispatcher	,"Despachador de Eventos",0);
	os_Include_Task(&Monitor_FSM 		,"Monitor FSM",1);

	os_SetIRQ(PIN_INT0_IRQn,&B1_Interrupt,&monitor);
	os_SetIRQ(PIN_INT2_IRQn,&B2_Interrupt,&monitor);

	GPIO_Interrupt_Setup();

	Monitor_System_Init(&monitor,TEC1,TEC2);

	os_Run();

	while(1)
	{
		__WFI();
	}
}

/*==================[end of file]============================================*/
