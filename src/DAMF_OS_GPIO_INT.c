/*
 * DAMF_OS_GPIO_INT.c
 *
 *  Created on: Aug 15, 2021
 *      Author: daniel
 */

#include "chip.h"
#include "GPIOInt_Setup.h"
#include "board.h"

void GPIO_Interrupt_Setup(void)
{
/****************TEC1*************************/
	/*GPIO TEC1 INTERRUPT SETUP*/
	// Configure interrupt channel for the GPIO pin
	Chip_SCU_GPIOIntPinSel( PININT_INDEX1, GPIO0_GPIO_PORT1, GPIO0_GPIO_PIN1);

	// Configure channel interrupt as edge sensitive and falling edge interrupt
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX1));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX1));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX1));
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX1));

	// Enable interrupt in the NVIC
	NVIC_ClearPendingIRQ( TEC1_INT);
	NVIC_SetPriority(TEC1_INT, 5);
	NVIC_EnableIRQ( TEC1_INT);
}


