/* This syntax permits the compiler to build from the root of the project
directory. */

/* #include "cc_config.h" */

#include "serial.h"
#include "serprim.h"

/** Begin DOS defines. **/
#include <dos.h>
#include <comlib.h>
#include <limits.h>
#include <stddef.h>

/* Todo- Add logic to check that the serial port in fact exists. Eventually,
remove dependencies off PICTOR lib. */
serial_handle_t open_handle(unsigned short port_no)
{
	serial_handle_t port_io;
	
	switch(port_no)
	{
		case 1:
			port_io = (void *) 0x3F8;
			break;
		case 2:	
			port_io = (void *) 0x2F8;
			break;
		case 3:
			port_io = (void *) 0x3E8;
			break;
		case 4:
			port_io = (void *) 0x2E8;
			break;
		default:
			port_io = NULL;
			break;
	}
	
	return port_io;	
}

int handle_valid(serial_handle_t port_addr)
{
	return (port_addr == (void *) 0x3F8 || port_addr == (void *) 0x2F8 || \
		port_addr == (void *) 0x3E8 || port_addr == (void *) 0x2E8);
}

int init_port(serial_handle_t port, unsigned long baud_rate)
{
	unsigned short comlib_br, comlib_pn;
	
	/* Page 195 of Watcom C Library Reference says conversion from ptr
	to int is valid/will work. */
	int io_addr = (int) port;
	
	switch(io_addr)
	{
		case 1:
			comlib_pn = CO_COM1;
			break;
		case 2:	
			comlib_pn = CO_COM2;
			break;
		case 3:
			comlib_pn = CO_COM3;
			break;
		case 4:
			comlib_pn = CO_COM4;
			break;
			/* In practice, one should not reach here, but
			kept as precaution. */
		default:
			comlib_pn = CO_COM1;
			break;
	}
	
	switch(baud_rate)
	{
		case 300:
			comlib_br = CO_BAUD300;
			break;
		case 1200:
			comlib_br = CO_BAUD1200;
			break;
		case 2400:
			comlib_br = CO_BAUD2400;
			break;
		case 4800:
			comlib_br = CO_BAUD4800;
			break;
		case 9600:
			comlib_br = CO_BAUD9600;
			break;
		case 19200:
			comlib_br = CO_BAUD19200;
			break;
		case 38400:
			comlib_br = CO_BAUD38400;
			break;
		case 57600:
			comlib_br = CO_BAUD57600;
			break;						
		case 115200:
			comlib_br = CO_BAUD115200;
			break;
		default:
			comlib_br = CO_BAUD9600;
			break;
	}
	
	/* Note to self: Make sure near/far are distinguished. */
	return comopen(comlib_pn, comlib_br, CO_NOPARITY, CO_DATA8, \
		CO_STOP1, CO_IRQDEFAULT) ? -1 : 0;
}

int write_data(serial_handle_t port, char * data, unsigned int num_bytes)
{
	(void) port;
	
	/* Guard against out of bounds unsigned to signed conversion. */
	if(num_bytes > INT_MAX)
	{
		return -2;
	}
	else
	{
		return comwrite(data, (int) num_bytes) ? -1 : 0;
	}	
}

int read_data(serial_handle_t port, char * data, unsigned int num_bytes, int timeout)
{
	/* Guard against signed overflow in either direction when converting
	to ticks. Additionally, the DOS library may not handle negative 
	timeouts well. A timeout < 0 will be set to 0. */
	unsigned int timeout_ticks;
	int prev_timeout;
	
	(void) port;
	
	if(timeout < 0)
	{
		timeout = 0;
	}
	
	timeout_ticks = timeout * 18;
	
	/* Guard against out of bounds unsigned to signed conversion. */
	if(num_bytes > INT_MAX || timeout_ticks > INT_MAX)
	{
		return -2;
	}
	
	/* Timeout measured against system timer, 1/18 second per tick. */
	prev_timeout = comgettimeout();
	
	/* if(_PL_comoverflow)
	{
		printf("Overflow of buffer!\n");
	} */
	
	comsettimeout((int) timeout_ticks);
	if(!comread((char *) data, (int) num_bytes))
	{
		comsettimeout(prev_timeout);
		/* Add overflow check somewhere. */
		return 0;
	}
	else
	{
		comsettimeout(prev_timeout);
		return -1;
	}
	
	
}

int close_handle(serial_handle_t port)
{
	(void) port;
	
	return comclose() ? -1 : 0;
}

int flush_device(serial_handle_t port)
{
	(void) port;
	
	/* Returns number of chars flushed- cannot use as return value
	 * per the serial function requirements. */
	comflush();
	return 0;
}

