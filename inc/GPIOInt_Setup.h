#ifndef GPIO_INT_SET_UP
#define GPIO_INT_SET_UP

#define PININT_INDEX1			0                  // PININT index used for GPIO mapping
//#define LOGIC1_IRQ_HANDLER		GPIO0_IRQHandler   // GPIO interrupt IRQ function name
#define TEC1_INT				PIN_INT0_IRQn

#define GPIO0_GPIO_PORT1  0
#define GPIO0_GPIO_PIN1   4


//Funcion de configuracion de Interrupcion
void GPIO_Interrupt_Setup(void);

#endif //GPIO_INT_SET_UP
