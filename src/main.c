/*==================[inclusions]=============================================*/

#include "main.h"
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include "../inc/DAMF_OS_Core.h"

/*==================[macros and definitions]=================================*/
#define MILISEC	1000

/*==================[Global data declaration]==============================*/

damf_semaphore Sema1;
damf_semaphore Sema2;
damf_queue     Queue1;

/*==================[internal functions declaration]=========================*/

static void initHardware(void)  {
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / MILISEC);		//systick 1ms
}


/*==================[Definicion de tareas para el OS]==========================*/
void tarea1(void)  {
	uint16_t h = 0;
	uint16_t i = 0;
	uint8_t led_red = 4;
	uint8_t queue_led = 4;

	uint32_t dato_pull = 111111;

	while (1) {
		h++;
		i++;

		dato_pull = 111111;

		os_pull_queue(&Queue1, &dato_pull);
		os_delay(250);//DELAY ms
		Board_LED_Toggle(led_red);
		os_Sema_Free(&Sema1);
		if(dato_pull==55355)
		{
			for(uint8_t i = 0; i<=10 ;i++)
			{
				Board_LED_Toggle(queue_led);
				os_delay(35);//DELAY ms
				Board_LED_Toggle(queue_led);
				os_delay(35);//DELAY ms
			}
		}
		os_Sema_Free(&Sema1);
		os_delay(250);//DELAY ms
		Board_LED_Toggle(led_red);
		os_pull_queue(&Queue1, &dato_pull);
		//os_block();
	}
}

void tarea2(void)  {
	uint16_t j = 0;
	uint16_t k = 0;
	uint8_t led_gre = 5;

	while (1)
	{
		j++;
		k++;
		os_delay(350);//DELAY ms
		Board_LED_Toggle(led_gre);
		os_Sema_Take(&Sema1);
		os_delay(350);//DELAY ms
		os_Sema_Take(&Sema1);
		os_delay(350);//DELAY ms
		os_Sema_Take(&Sema1);
		Board_LED_Toggle(led_gre);
	}
}

void tarea3(void)  {
	uint16_t j = 0;
	uint16_t k = 0;
	uint8_t led_yel = 3;

	uint32_t dato_push = 55355;

	while (1)
	{
		j++;
		k++;
		os_delay(3000);//DELAY ms
		Board_LED_Toggle(led_yel);
		os_push_queue(&Queue1, &dato_push);
	}
}

void woaow(void)
{
	while(1)
	{
		Board_LED_Toggle(0);
		os_delay(200);//DELAY ms
		Board_LED_Toggle(1);
		os_delay(200);//DELAY ms
		Board_LED_Toggle(2);
		os_delay(800);//DELAY ms
	}
}

/*============================================================================*/

int main(void)  {

	initHardware();

	os_Semaphore_Create(&Sema1, 2);
	os_Semaphore_Create(&Sema2, 5);
	os_Queue_Create(&Queue1,5,sizeof(uint32_t));


	os_Init();

	os_Include_Task(&tarea1,"Tarea 1",1);
	os_Include_Task(&tarea2,"Tarea 2",1);
	//TODOMAX_TASKS PRIO3=2
	os_Include_Task(&tarea3,"Tarea 3",1);
	os_Include_Task(&woaow,"Tarea 4",1);

	os_SetIRQ(PIN_INT0_IRQn,&woaow);

	os_Run();

	while(1)
	{
		__WFI();
	}

	//os_print_error();
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
