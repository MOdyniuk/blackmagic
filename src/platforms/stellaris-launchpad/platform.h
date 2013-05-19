#ifndef __PLATFORM_H
#define __PLATFORM_H

#include <stdint.h>

#include <setjmp.h>
#include <alloca.h>

#include <inc/hw_types.h>
#include <inc/hw_memmap.h>

#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/gpio.h>

#include "gdb_packet.h"

extern jmp_buf fatal_error_jmpbuf;
extern uint8_t running_status;
extern const char *morse_msg;
extern volatile uint32_t timeout_counter;

#define LED_PORT	GPIO_PORTF_BASE
#define LED_IDLE_RUN	GPIO_PIN_2
#define LED_ERROR	GPIO_PIN_3

#define TMS_PORT	GPIO_PORTB_BASE
#define TMS_PIN		GPIO_PIN_5

#define TCK_PORT	GPIO_PORTB_BASE
#define TCK_PIN		GPIO_PIN_0

#define TDI_PORT	GPIO_PORTB_BASE
#define TDI_PIN		GPIO_PIN_1

#define TDO_PORT	GPIO_PORTE_BASE
#define TDO_PIN		GPIO_PIN_4

#define SWDIO_PORT	TMS_PORT
#define SWDIO_PIN	TMS_PIN

#define SWCLK_PORT	TCK_PORT
#define SWCLK_PIN	TCK_PIN

#define TRST_PORT	GPIO_PORTA_BASE
#define TRST_PIN	GPIO_PIN_5

#define SRST_PORT	GPIO_PORTA_BASE
#define SRST_PIN	GPIO_PIN_6

#define TMS_SET_MODE()	{								\
	MAP_GPIODirModeSet(TMS_PORT, TMS_PIN, GPIO_DIR_MODE_OUT);			\
	MAP_GPIOPadConfigSet(TMS_PORT, TMS_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);	\
}

#define SWDIO_MODE_FLOAT() {								\
	MAP_GPIODirModeSet(SWDIO_PORT, SWDIO_PIN, GPIO_DIR_MODE_IN);			\
}

#define SWDIO_MODE_DRIVE() {									\
	MAP_GPIODirModeSet(SWDIO_PORT, SWDIO_PIN, GPIO_DIR_MODE_OUT);				\
	MAP_GPIOPadConfigSet(SWDIO_PORT, SWDIO_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);	\
}

/* Use newlib provided integer only stdio functions */
#define sscanf siscanf
#define sprintf siprintf
#define vasprintf vasiprintf

#define DEBUG(...)

#define SET_RUN_STATE(state)	{running_status = (state);}
#define SET_IDLE_STATE(state)	{gpio_set_val(LED_PORT, LED_IDLE_RUN, state);}
#define SET_ERROR_STATE(state)	{if(state) MAP_GPIOPinWrite(LED_PORT, LED_ERROR, LED_ERROR); else MAP_GPIOPinWrite(LED_PORT, LED_ERROR, 0);}

#define PLATFORM_SET_FATAL_ERROR_RECOVERY()	{setjmp(fatal_error_jmpbuf);}
#define PLATFORM_FATAL_ERROR(error) {			\
	if( running_status ) gdb_putpacketz("X1D");	\
		else gdb_putpacketz("EFF");		\
	running_status = 0;				\
	target_list_free();				\
	morse("TARGET LOST.", 1);			\
	longjmp(fatal_error_jmpbuf, (error));		\
}

int platform_init(void);
void morse(const char *msg, char repeat);

void gpio_set_val(uint32_t port, uint8_t pin, uint8_t val);

uint8_t gpio_get(uint32_t port, uint8_t pin);

void platform_delay(uint32_t delay);

#endif
