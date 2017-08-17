/* Dummy serial device class. Has access to extern vars to control the output
destination, whether the device is working properly, etc. */

#include "shared.h"

#include <stdio.h>
#include <time.h> /* Add version which doesn't depend on time.h
to run tests on embedded? minunit.h version provided already requires
hosted C. */
#include <stddef.h> /* For NULL. */
#include "serial.h"
#include "serprim.h"

/* Line is modelled by a an array- abstract representation of characters being
sent down the wire before being acknowledged by receiever.
TODO: When appropriate, perhaps model individual tx/rx using multitasking? */
char local_tx_line[STATIC_BUFSIZ];
char local_rx_line[STATIC_BUFSIZ];
char * remote_tx_line = local_rx_line; /* Connect the virtual serial lines to each other. */
char * remote_rx_line = local_tx_line;

port_desc_t port_model[3] = { {NULL, NULL, 0, 0, 0, 0, 0, 0, 0} };

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
		for(count = 0; count < num_bytes; count++)
		{
			((port_desc_t *) port)->tx_line[curr_offset + count] = data[count];
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
		for(count = 0; count < num_bytes; count++)
		{
			 data[count] = ((port_desc_t *) port)->rx_line[curr_offset + count];
		}

		VOID_TO_PORT(port, buf_pos_rx) += num_bytes;
		return 0;
	}
}

int read_data_get_elapsed_time(serial_handle_t port, char * data, unsigned int num_bytes, int timeout, int * elapsed)
{
	int rc;
	time_t start, end;

	time(&start);
	rc = read_data(port, data, num_bytes, timeout);
	time(&end);

	*elapsed = (int) difftime(end, start);
	return rc;
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
			((port_desc_t *) port)->rx_line[count] = '\0';
		}
		VOID_TO_PORT(port, buf_pos_rx) = 0;
	}
	else
	{
		return -1;
	}
	return 0;
}
