#include "serial.h"
#include "serprim.h"

#include <stddef.h>

serial_handle_t open_handle(unsigned short port_no)
{
    return NULL;
}

int handle_valid(serial_handle_t port)
{
    return 0;
}

int init_port(serial_handle_t port, unsigned long baud_rate)
{
    return 0;
}

int write_data(serial_handle_t port, char * data, unsigned int num_bytes)
{
    return 0;
}

int read_data(serial_handle_t port, char * data, unsigned int num_bytes, int timeout)
{
    return 0;
}

int read_data_get_elapsed_time(serial_handle_t port, char * data, unsigned int num_bytes, int timeout, int * elapsed)
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
