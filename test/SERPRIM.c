/* Dummy serial device class. Has access to extern vars to control the output
destination, whether the device is working properly, etc. */

#include "serial.h"
#include "serprim.h"
#include <stddef.h> /* For NULL. */

serial_handle_t open_handle(unsigned short port_no)
{
	return (void *) 0xCAFE;
}

int handle_valid(serial_handle_t port)
{
	return 1;
}

int init_port(serial_handle_t port, unsigned long baud_rate)
{
	return 0;
}

int valid_size(unsigned int num_bytes)
{
	return 1;
}

int write_data(serial_handle_t port, char * data, unsigned int num_bytes)
{
	return 0;
}

int read_data(serial_handle_t port, char * data, unsigned int num_bytes, int timeout)
{
	return 0;
}

int close_handle(serial_handle_t port)
{
	return 0;
}

int flush_device(serial_handle_t port)
{
	return 0;
}

