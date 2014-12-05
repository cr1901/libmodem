/* Dummy serial device class. Has access to extern vars to control the output
destination, whether the device is working properly, etc. */

#include "serial.h"
#include <stddef.h> /* For NULL. */
#include <string.h>

/* void windows_error(char *); */
SERIAL_STATUS serial_init(unsigned short port_no, unsigned long baud_rate, serial_handle_t * port_addr)
{
	return SERIAL_NO_ERRORS;
}

SERIAL_STATUS serial_snd(char * data, unsigned int num_bytes, serial_handle_t port)
{
	return SERIAL_NO_ERRORS;
}

SERIAL_STATUS serial_rcv(char * data, unsigned int num_bytes, int timeout, serial_handle_t port)
{	
	return SERIAL_NO_ERRORS;
}

SERIAL_STATUS serial_close(serial_handle_t * port_addr)
{
	return SERIAL_NO_ERRORS;
}

SERIAL_STATUS serial_flush(serial_handle_t port_addr)
{
	return SERIAL_NO_ERRORS;
}

/* #else
	#error "Windows target chosen, but compiler does not support Windows!"
#endif */
