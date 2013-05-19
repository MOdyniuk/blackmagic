#include "gdb_if.h"
#include "platform.h"

int
gdb_if_init(void) 
{
	return 0;
}

unsigned char
gdb_if_getchar(void)
{
	return UARTgetc();
}

unsigned char
gdb_if_getchar_to(int timeout)
{
	timeout_counter = timeout/100;
	do {
		if( UARTRxBytesAvail() > 0 ) {
			return gdb_if_getchar();
		}
	} while(timeout_counter);

	return -1;
}

void
gdb_if_putchar(unsigned char c, int flush)
{
	UARTwrite(&c, 1);
	if( flush )
		UARTFlushTx(false);
}
