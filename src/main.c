/*==================[inclusions]=============================================*/

#include "main.h"
#include "board.h"
#include "../inc/DAMF_OS_Core.h"

/*==================[macros and definitions]=================================*/

#define MILISEC		1000


/*==================[Global data declaration]==============================*/

damf_semaphore Sema1;
damf_semaphore Sema2;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/** @brief hardware initialization function
 *	@return none
 */
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
	while (1) {
		h++;
		i++;
		os_delay(600);//DELAY ms
		Board_LED_Toggle(led_red);
		os_Sema_Free(&Sema1);
		os_Sema_Free(&Sema1);
		os_delay(850);//DELAY ms
		Board_LED_Toggle(led_red);
		//os_block();
	}
}

void tarea2(void)  {
	uint16_t j = 0;
	uint16_t k = 0;
	uint8_t led_gre = 5;

	while (1) {
		j++;
		k++;
		os_delay(200);//DELAY ms
		Board_LED_Toggle(led_gre);
		os_Sema_Take(&Sema1);
		os_delay(200);//DELAY ms
		os_Sema_Take(&Sema1);
		os_delay(200);//DELAY ms
		os_Sema_Take(&Sema1);
		Board_LED_Toggle(led_gre);
	}
}

void tarea3(void)  {
	uint16_t j = 0;
	uint16_t k = 0;
	uint8_t led_yel = 3;

	while (1) {
		j++;
		k++;
		os_delay(1500);//DELAY ms
		Board_LED_Toggle(led_yel);
	}
}

/*============================================================================*/

int main(void)  {

	initHardware();

	os_Semaphore_Create(&Sema1, 2);
	os_Semaphore_Create(&Sema2, 5);

	os_Init();

	os_Include_Task(&tarea1,"Tarea 1",1);
	os_Include_Task(&tarea2,"Tarea 2",1);
	os_Include_Task(&tarea3,"Tarea 3",2);

	os_Run();

	while(1)
	{
		__WFI();
	}

	//os_print_error();
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
