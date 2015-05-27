/* Dummy serial device class. Has access to extern vars to control the output
destination, whether the device is working properly, etc. */

#include "shared.h"

#include <stdio.h>
#include "serial.h"
#include "serprim.h"
#include <stddef.h> /* For NULL. */

/* The buffers used to test the modem routines. May be split from file_bufs
eventually. */
/* extern char local_tx_file_buf[STATIC_BUFSIZ];
extern char local_rx_file_buf[STATIC_BUFSIZ];
extern char remote_tx_file_buf[STATIC_BUFSIZ];
extern char remote_rx_file_buf[STATIC_BUFSIZ]; */


/* Simulate device buffers at some point- some UARTs (glares at 8250) don't have one! */
/* extern char local_tx_device_buf[STATIC_BUFSIZ];
extern char local_rx_device_buf[STATIC_BUFSIZ];
extern char remote_tx_device_buf[STATIC_BUFSIZ];
extern char remote_rx_device_buf[STATIC_BUFSIZ]; */

/* Line is modelled by a an array- abstract representation of characters being 
sent down the wire before being acknowledged by receiever. When appropriate, perhaps
model individual tx/rx using multitasking? *shrug* */
char local_tx_line[STATIC_BUFSIZ];
char local_rx_line[STATIC_BUFSIZ];
char * remote_tx_line = local_rx_line; /* Connect the virtual serial lines to each other. */
char * remote_rx_line = local_tx_line;

PORT_DESC port_model[3] = { {NULL, NULL, 0, 0, 0, 0, 0, 0, 0} };


/* Needed as dummy target of void ptr at some point? */
/* static int local_handle = 0;
static int remote_handle = 1; */

serial_handle_t open_handle(unsigned short port_no)
{	
	return port_model + port_no;
}

int handle_valid(serial_handle_t port)
{
	return ((port == port_model) || (port == port_model + 1) || (port == port_model + 2));
}

/* We can assume a valid handle here. However, on error, the handle will
be globally set to invalid. */
int init_port(serial_handle_t port, unsigned long baud_rate)
{
	(void) baud_rate;
	
	if(port == &port_model[0])
	{
		VOID_TO_PORT(port, tx_line) = local_tx_line;
		VOID_TO_PORT(port, rx_line) = local_rx_line;
	}
	else if(port == &port_model[1])
	{
		VOID_TO_PORT(port, tx_line) = remote_tx_line;
		VOID_TO_PORT(port, rx_line) = remote_rx_line;
	}
	else
	{
		return -1;
	}
	
	VOID_TO_PORT(port, bad_write) = 0;
	VOID_TO_PORT(port, bad_read) = 0;
	VOID_TO_PORT(port, force_rx_timeout) = 0;
	VOID_TO_PORT(port, buf_pos_tx) = 0;
	VOID_TO_PORT(port, buf_pos_rx) = 0;
	
	return 0;
}

int valid_size(unsigned int num_bytes)
{	
	return (num_bytes <= STATIC_BUFSIZ);
}

int write_data(serial_handle_t port, char * data, unsigned int num_bytes)
{
	if(!VOID_TO_PORT(port, bad_write))
	{
		unsigned int count;
		unsigned int curr_offset = VOID_TO_PORT(port, buf_pos_tx);
		/* printf("buf_pos_write: %d\n", VOID_TO_PORT(port, buf_pos_tx)); */
		for(count = 0; count < num_bytes; count++)
		{
			((PORT_DESC *) port)->tx_line[curr_offset + count] = data[count];
		}
		
		VOID_TO_PORT(port, buf_pos_tx) += num_bytes;
		return 0;
	}
	else
	{
		return -1;
	}
}

int read_data(serial_handle_t port, char * data, unsigned int num_bytes, int timeout)
{
	(void) timeout;
	
	if(VOID_TO_PORT(port, bad_read))
	{
		return -2;
	}
	else if(VOID_TO_PORT(port, force_rx_timeout))
	{
		return -1;
	}
	else
	{
		unsigned int count;
		unsigned int curr_offset = VOID_TO_PORT(port, buf_pos_rx);
		/* printf("buf_pos_read: %d\n", VOID_TO_PORT(port, buf_pos_rx)); */
		for(count = 0; count < num_bytes; count++)
		{
			 data[count] = ((PORT_DESC *) port)->rx_line[curr_offset + count];
		}
		
		VOID_TO_PORT(port, buf_pos_rx) += num_bytes;
		return 0;
	}
}

int close_handle(serial_handle_t port)
{
	return VOID_TO_PORT(port, bad_close);
}

int flush_device(serial_handle_t port)
{	
	if(!VOID_TO_PORT(port, bad_flush))
	{
		unsigned int count;
		for(count = 0; count < STATIC_BUFSIZ; count++)
		{
			((PORT_DESC *) port)->rx_line[count] = '\0';
		}
		VOID_TO_PORT(port, buf_pos_rx) = 0;
	}
	else
	{
		return -1;
	}
	return 0;
}

