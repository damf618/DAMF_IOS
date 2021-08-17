#ifndef GPIO_INT_SET_UP
#define GPIO_INT_SET_UP

#define GPIO_INTERRUPT_PRIORITY 5

/*-*-*-*-*- TEC 1 *-*-*-*-*-*/
//Interrupt Configuration for TEC1
#define PININT_INDEX1			0                  // PININT index used for GPIO mapping
#define TEC1_INT				PIN_INT0_IRQn

//GPIO Info for Interrupt Configuration TEC1
#define GPIO0_GPIO_PORT1  		0
#define GPIO0_GPIO_PIN1   		4

/*-*-*-*-*- TEC 2 *-*-*-*-*-*/
//Interrupt Configuration for TEC2
#define PININT_INDEX2			2                  // PININT index used for GPIO mapping
#define TEC2_INT				PIN_INT2_IRQn

//GPIO Info for Interrupt Configuration TEC2
#define GPIO0_GPIO_PORT2  		0
#define GPIO0_GPIO_PIN2   		8

//Funcion de configuracion de Interrupcion
void GPIO_Interrupt_Setup(void);

#endif //GPIO_INT_SET_UP
