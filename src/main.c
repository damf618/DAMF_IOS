/*==================[inclusions]=============================================*/

#include "main.h"

#include "../inc/DAMF_OS_Core.h"
#include "board.h"



/*==================[macros and definitions]=================================*/

#define MILISEC		1000

/*==================[Global data declaration]==============================*/

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
	while (1) {
		h++;
		i++;
	}
}

void tarea2(void)  {
	uint16_t j = 0;
	uint16_t k = 0;

	while (1) {
		j++;
		k++;
	}
}

/*============================================================================*/

int main(void)  {

	initHardware();

	os_Init();

	os_Include_Task(&tarea1,"Tarea 1");
	os_Include_Task(&tarea2,"Tarea 2");
	os_Include_Task(&tarea2,"Tarea 3");

	while (1)
	{
		__WFI();
		//os_print_error();
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
