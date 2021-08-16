# DAMF_OS
Sistema Operativo desarrollado para la materia "Implementacion de sistemas operativos"

Este sistema operativo tiene fines solamente didacticos, no es recomendable para utilizarlo en produccion de dispositivos y o productos.

## Descripcion del Sistema.

El sistema se basa en los concoimientos adquiridos en la materia, y se enfoca en funcionalidad. No nos enfocamos en desempe;o ni en rendimiento sino en el cumplimiento de los requerimientos previstos.

Funciona a partir de la combinacion de las excepciones PendSV y SysTick del ARM Cortex M4, se utiliza el SysTic para definir el Tick interno del sistema y de esta forma realizar las tareas y configuraciones necesarias del Sistema Operativo de forma periodica y exacta. Por otra parte el PendSV se utiliza para realizar el cambio de contexto al momento de forma organizada y controlada.

Las APIs funcionana mediante la generacion de eventos, previo a la llamada al scheduler el sistema ejecuta un ciclo de validacion, donde evalua cada evento y toma las acciones correspondientes, por ejemplo: Cargar un dato en una cola llena, generaria un evento de Cola, que sera evaluado durante el siguiente SysTick. 

# Heartbeat EDUCIAA

Considerando las caracteristicas de educativas del sistema, se incluye un Heartbeat por defecto que permite al usuario identificar el funcionamiento de la placa mediante el encendido del LED "0" (Rojo del LED RGB), cada 20 ciclos de Tick. 

# Caracteristicas Comunes

El DAMF_OS cuenta con los elementos comunes de un Sistema Operativos como lo son: Semaforos, Colas y Delays. Igual que muchos otros sistemas la sincronizacion de las tareas se realizan a paritir de un shceduler, el cual considera :
** El estado de la tarea: existen 3 estados implementados, BLOCKED, READY, RUNNING.
** La prioridad de la tarea: existen 4 niveles de prioridad que van de 0 a 3, donde 0 es el valor MAX y 3 el valor min.
** Funcionamiento Round Robin.

# Listado de de APIS

** Semaforos
** Colas
** Delay

A continuacion se describen algunas consideraciones a tener en cuenta:

** El sistema requiere ser inicializado (os_Init) previo a ser ejecutado (os_Run).
** Se tienen diferentes APIS para el creado de Tareas en momento de configuracion (previo a os_Run) y rl punto de ejecucion (posterior a os_Run).
** Las APIS del sistema pueden ser llamadas durante una excepcion, exceptuando y funcionaran correctamente, excepto por: os_delay, os_Sema_Take y os_pull_queue. Estas funciones han sido obviadas debido a que no es recomendable ser utilizadas durante una IRQ.
** Cada API genera un evento, a su vez cada evento a su vez se compone de un Handler y una estructura asociada. Cada evento es atentido durante la evaluacion de un nuevo SysTick, previo a la llamada del scheduler, de esta forma cualquier cambio que se genere durante una excepcion sera considerada en el siguiente cambio de contexto.
 
