#ifndef MSE_OS_DAMF_OS_IRQ_H_
#define MSE_OS_DAMF_OS_IRQ_H_

#include "DAMF_OS_Core.h"
#include "board.h"
#include "cmsis_43xx.h"

#define CANT_IRQ	53

typedef struct Interrupt_event
{
	void (*interrupt_function)(void*) ;
	void * prmtr;
}interrupt_t;

bool os_SetIRQ(LPC43XX_IRQn_Type irq, void* usr_isr, void* usr_isr_prmtr);
bool os_ClearIRQ(LPC43XX_IRQn_Type irq);


#endif /* MSE_OS_DAMF_OS_IRQ_H_ */
