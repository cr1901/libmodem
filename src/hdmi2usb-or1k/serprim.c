#include "serial.h"
#include "serprim.h"

#include <time.h>
#include <uart.h>
#include <stddef.h>
#include <generated/csr.h>
#include <hw/common.h>


serial_handle_t open_handle(unsigned short port_no)
{
    return (port_no == 0) ? (serial_handle_t) CSR_UART_BASE : NULL;
}

int handle_valid(serial_handle_t port)
{
    return ((serial_handle_t) CSR_UART_BASE == port);
}

int init_port(serial_handle_t port, unsigned long baud_rate)
{
    /* BIOS will initialize uart and timer, but we need to measure
    intervals greater than 2 seconds (up to 10). */
    int t;
    (void) port;
    (void) baud_rate;

    timer0_en_write(0);
    t = 11*SYSTEM_CLOCK_FREQUENCY;
    timer0_reload_write(t);
    timer0_load_write(t);
    timer0_en_write(1);

    return 0;
}

int write_data(serial_handle_t port, char * data, unsigned int num_bytes)
{
    (void) port;
    unsigned int count;

    for(count = 0; count < num_bytes; count++)
    {
        uart_write(data[count]);
    }

    return 0;
}

int read_data(serial_handle_t port, char * data, unsigned int num_bytes, int timeout)
{
    unsigned int count = 0;
    int start_time = 0;

    (void) port;

    elapsed(&start_time, -1); /* Initialize elapsed function. */
    while(!elapsed(&start_time, timeout * SYSTEM_CLOCK_FREQUENCY))
    {
        if(uart_read_nonblock())
        {
            data[count++] = uart_read();
            if(count >= num_bytes)
            {
                return 0;
            }
        }
    }

    return -1;
}

int read_data_get_elapsed_time(serial_handle_t port, char * data, unsigned int num_bytes, int timeout, int * elapsed)
{
    int start_time, end_time;
    int rv;

    start_time = timer0_value_read();
    rv = read_data(port, data, num_bytes, timeout);
    end_time = timer0_value_read();

    if(start_time < end_time)
    {
        /* Down counter. Timer rolled over if start < end. */
        end_time += timer0_reload_read();
    }

    *elapsed = (end_time - start_time) / SYSTEM_CLOCK_FREQUENCY;
    return rv;
}

int close_handle(serial_handle_t port)
{
    (void) port;
    time_init(); /* Restore BIOS defaults. */
    return 0;
}

int flush_device(serial_handle_t port)
{
    (void) port;
    while(uart_read_nonblock())
    {
        (void) uart_read();
    }

    return 0;
}
