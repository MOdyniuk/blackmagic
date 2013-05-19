#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>

#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/systick.h>
#include <driverlib/uart.h>
#include <driverlib/interrupt.h>

#include <utils/uartstdio.h>
#include <utils/ustdlib.h>

#include "platform.h"

#define CPU_FREQ	(48000000)

#define SYSTICKHZ	10
#define SYSTICKMS	(1000 / SYSTICKHZ)

jmp_buf fatal_error_jmpbuf;
uint8_t running_status;
volatile uint32_t timeout_counter;

static void morse_update(void);

/* Morse code patterns and lengths */
static const struct {
	uint16_t code;
	uint8_t bits;
} morse_letter[] = {
	{        0b00011101,  8}, // 'A' .-
	{    0b000101010111, 12}, // 'B' -...
	{  0b00010111010111, 14}, // 'C' -.-.
	{      0b0001010111, 10}, // 'D' -..
	{            0b0001,  4}, // 'E' .
	{    0b000101110101, 12}, // 'F' ..-.
	{    0b000101110111, 12}, // 'G' --.
	{      0b0001010101, 10}, // 'H' ....
	{          0b000101,  6}, // 'I' ..
	{0b0001110111011101, 16}, // 'J' .---
	{    0b000111010111, 12}, // 'K' -.-
	{    0b000101011101, 12}, // 'L' .-..
	{      0b0001110111, 10}, // 'M' --
	{        0b00010111,  8}, // 'N' -.
	{  0b00011101110111, 14}, // 'O' ---
	{  0b00010111011101, 14}, // 'P' .--.
	{0b0001110101110111, 16}, // 'Q' --.-
	{      0b0001011101, 10}, // 'R' .-.
	{        0b00010101,  8}, // 'S' ...
	{          0b000111,  6}, // 'T' -
	{      0b0001110101, 10}, // 'U' ..-
	{    0b000111010101, 12}, // 'V' ...-
	{    0b000111011101, 12}, // 'W' .--
	{  0b00011101010111, 14}, // 'X' -..-
	{0b0001110111010111, 16}, // 'Y' -.--
	{  0b00010101110111, 14}, // 'Z' --..
};

const char *morse_msg;
static const char * volatile morse_ptr;
static char morse_repeat;

void morse(const char *msg, char repeat)
{
	morse_msg = morse_ptr = msg;
	morse_repeat = repeat;
	SET_ERROR_STATE(0);
}

static void morse_update(void)
{
	static uint16_t code;
	static uint8_t bits;

	if(!morse_ptr) return;

	if(!bits) {
		char c = *morse_ptr++;
		if(!c) {
			if(morse_repeat) {
				morse_ptr = morse_msg;
				c = *morse_ptr++;
			} else {
				morse_ptr = 0;
				return;
			}
		}
		if((c >= 'A') && (c <= 'Z')) {
			c -= 'A';
			code = morse_letter[c].code;
			bits = morse_letter[c].bits;
		} else {
			code = 0; bits = 4;
		}
	}
	SET_ERROR_STATE(code & 1);
	code >>= 1; bits--;
}


void sys_tick_handler(void)
{
	if(running_status) {
		if( MAP_GPIOPinRead(LED_PORT, LED_IDLE_RUN) > 0 ) {
			MAP_GPIOPinWrite(LED_PORT, LED_IDLE_RUN, 0);
		} else {
			MAP_GPIOPinWrite(LED_PORT, LED_IDLE_RUN, LED_IDLE_RUN);
		}
	}

	if(timeout_counter)
		timeout_counter--;

	morse_update();
}

void uart0_isr(void)
{
	UARTStdioIntHandler();
}

static void
uart_init(void)
{
        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

        // Configure PD0 and PD1 for UART
        MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
        MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
        MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
        UARTStdioInitExpClk(0, 115200);
}

int
platform_init(void)
{
        int i;
        for(i=0; i<1000000; i++);

        // Setup for 16MHZ external crystal, use 200MHz PLL and divide by 4 = 50MHz
        MAP_SysCtlClockSet(SYSCTL_SYSDIV_16 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                SYSCTL_XTAL_16MHZ);
	
	uart_init();

	// Enable LED
        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	MAP_GPIOPinTypeGPIOOutput(LED_PORT, LED_IDLE_RUN);
	MAP_GPIOPinTypeGPIOOutput(LED_PORT, LED_ERROR);

	// Enable all JTAG ports and set pins to output
        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

	MAP_GPIOPinTypeGPIOOutput(TMS_PORT, TMS_PIN);
	MAP_GPIOPinTypeGPIOOutput(TCK_PORT, TCK_PIN);
	MAP_GPIOPinTypeGPIOOutput(TDI_PORT, TDI_PIN);
	MAP_GPIOPinTypeGPIOOutput(TDO_PORT, TDO_PIN);
	MAP_GPIOPinTypeGPIOOutput(TRST_PORT, TRST_PIN);
	MAP_GPIOPinTypeGPIOOutput(SRST_PORT, SRST_PIN);

	//UARTprintf("Initializing systick\r\n");
	/*char s[] = "Initializing systick\n";
	UARTwrite(s, strlen(s));*/
	UARTEchoSet(false);
	UARTFlushTx(false);

	MAP_SysTickPeriodSet(MAP_SysCtlClockGet() / SYSTICKHZ);
	MAP_SysTickEnable();
	MAP_SysTickIntEnable();

	MAP_IntMasterEnable();

	return 0;
}

void platform_delay(uint32_t delay)
{
	timeout_counter = delay;
	while(timeout_counter);
}

const char *platform_target_voltage(void)
{
	return "0.0V";
}

void
gpio_set_val(uint32_t port, uint8_t pin, uint8_t val)
{
	if(val)
		MAP_GPIOPinWrite(port, pin, pin); 
	else 
		MAP_GPIOPinWrite(port, pin, 0);
}

uint8_t
gpio_get(uint32_t port, uint8_t pin)
{
	if( MAP_GPIOPinRead(port, pin) > 0)
		return 1;
	else
		return 0;
}
