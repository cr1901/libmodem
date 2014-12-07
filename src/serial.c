#include "serial.h"
#include "serprim.h"

#include <stddef.h> /* For NULL. */

/* TODO: Serial close within routines? */

/* At some point, baud_rate will likely become a struct of serial parameters. */
SERIAL_STATUS serial_init(unsigned short port_no, unsigned long baud_rate, serial_handle_t * port_addr)
{
	SERIAL_STATUS ser_stat = SERIAL_NO_ERRORS;
	
	(* port_addr) = open_handle(port_no);
	if(!handle_valid((* port_addr)) || init_port((* port_addr), baud_rate))
	{
		(* port_addr) = NULL;
		ser_stat = SERIAL_HW_ERROR;
	}
	
	return ser_stat;
}
	
SERIAL_STATUS serial_snd(char * data, unsigned int num_bytes, serial_handle_t port)
{
	SERIAL_STATUS ser_stat = SERIAL_NO_ERRORS;
	if(!handle_valid(port) || write_data(port, data, num_bytes))
	{
		ser_stat = SERIAL_HW_ERROR;
	}
	
	return ser_stat;
}

SERIAL_STATUS serial_rcv(char * data, unsigned int num_bytes, int timeout, serial_handle_t port)
{
	SERIAL_STATUS ser_stat;
	int read_stat;

	/* Guard against negative values being converted to ridiculous timeouts. 
	A timeout < 0 will be set to 0. */	
	if(timeout < 0)
	{
		timeout = 0;
	}
	
	if(handle_valid(port))
	{
		read_stat = read_data(port, data, num_bytes, timeout);
	}
	else
	{
		read_stat = -2;
	}
	
	/* TODO: eventually handle case where some chars received, but not all
	requested and then timeout or hw error occurs. */
	switch(read_stat)
	{
		case 0:
			ser_stat = SERIAL_NO_ERRORS;
			break;
		case -1:
			ser_stat = SERIAL_TIMEOUT;
			break;
		case -2:
		default:
			ser_stat = SERIAL_HW_ERROR;
			break;
	}
	
	return ser_stat;
}

SERIAL_STATUS serial_close(serial_handle_t * port_addr)
{
	SERIAL_STATUS ser_stat = SERIAL_NO_ERRORS;
	/* Both the flush and close must succeed to return without error. */
	 
	if(!handle_valid((* port_addr)) || (serial_flush((* port_addr)) != SERIAL_NO_ERRORS) \
		|| close_handle((* port_addr)))
	{
		ser_stat = SERIAL_HW_ERROR;
	}
	else
	{
		(* port_addr) = NULL;
	}
	
	return ser_stat;
}

SERIAL_STATUS serial_flush(serial_handle_t port_addr)
{
	SERIAL_STATUS ser_stat = SERIAL_NO_ERRORS;
		
	if(!handle_valid(port_addr) || flush_device(port_addr))
	{
		ser_stat = SERIAL_HW_ERROR;
	}
	
	return ser_stat;
}
