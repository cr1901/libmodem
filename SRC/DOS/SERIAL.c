/* This syntax permits the compiler to build from the root of the project
directory. */

#include "cc_config.h"

#include "modem.h" //Todo- get rid of dependency on modem.h
//#include "usr_def.h"
#include "serial.h"
//#include <stdint.h>
//#include <stdlib.h>

/** Begin DOS defines. **/
#ifdef __DOS__
	#include <dos.h>
	#include <comlib.h>
	#include <limits.h>
	
	static int handle_valid(serial_handle_t port);
	
	/* Switch EXIT_SUCCESS to NO_ERRORS at some point.- Done.- W. Jones */
	int serial_init(unsigned short port_no, unsigned long baud_rate, serial_handle_t * port_addr)
	{
		unsigned short comlib_br, comlib_pn;
		void * port_io;
		
		switch(port_no)
		{
			case 1:
				comlib_pn = CO_COM1;
				port_io = (void *) 0x3F8;
				break;
			case 2:	
				comlib_pn = CO_COM2;
				port_io = (void *) 0x2F8;
				break;
			case 3:
				comlib_pn = CO_COM3;
				port_io = (void *) 0x3E8;
				break;
			case 4:
				comlib_pn = CO_COM4;
				port_io = (void *) 0x2E8;
				break;
			default:
				comlib_pn = CO_COM1;
				port_io = (void *) 0x3F8;
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
		}
		
		
		if(!comopen(comlib_pn, comlib_br, CO_NOPARITY, CO_DATA8, CO_STOP1, CO_IRQDEFAULT))
		{
			/* Note to self: Make sure near/far are distinguished. */
			
			(* port_addr) = port_io; /* Dummy memory address to the serial port.
			* Note that a pointer to the address of the serial port is passed
			* because the pointer to the serial port itself may be (in this
			* case, is) a void pointer (which cannot dereferenced). */
			return NO_ERRORS;
		}
		else
		{
			/* See Receive/Send status codes in MODEM.H for difference
			 * between SERIAL_ERROR and TIMEOUT. */
			return SERIAL_ERROR;
		}
	}
	
	int serial_snd(char * data, unsigned int num_bytes, serial_handle_t port)
	{
		if(!handle_valid(port))
		{
			return SERIAL_ERROR;
		}
		
		/* Timeout measured against system timer, 1/18 second per tick. */
		//int prev_timeout = comgettimeout();
		//comsettimeout((int) (timeout * 18));
		/* Guard against out of bounds unsigned to signed conversion. */
		if(num_bytes > INT_MAX)
		{
			return SERIAL_ERROR;
		}
		
		if(!comwrite(data, (int) num_bytes))
		{
			return NO_ERRORS;
		}
		else
		{
			return SERIAL_ERROR;
		}
		//comsettimeout(prev_timeout);
	}
	
	int serial_rcv(char * data, unsigned int num_bytes, int timeout, serial_handle_t port)
	{
		/* Guard against signed overflow in either direction when converting
		to ticks. Additionally, the DOS library may not handle negative 
		timeouts well. A timeout < 0 will be set to 0. */
		unsigned int timeout_ticks;
		int prev_timeout;
		
		if(!handle_valid(port))
		{
			return SERIAL_ERROR;
		}
		
		
		if(timeout < 0)
		{
			timeout = 0;
		}
		
		timeout_ticks = timeout * 18;
		
		/* Guard against out of bounds unsigned to signed conversion. */
		if(num_bytes > INT_MAX || timeout_ticks > INT_MAX)
		{
			return SERIAL_ERROR;
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
			return NO_ERRORS;
		}
		else
		{
			comsettimeout(prev_timeout);
			return TIMEOUT;
		}
	}
	
	int serial_close(serial_handle_t * port_addr)
	{
		if(!comclose())
		{
			(* port_addr) = NULL;
			return NO_ERRORS;
		}
		else
		{
			return SERIAL_ERROR;
		}
	}
	
	int serial_flush(serial_handle_t port)
	{
		if(!handle_valid(port))
		{
			return SERIAL_ERROR;
		}
		
		comflush(); 
		/* Returns number of chars flushed- cannot use as return value
		 * per the serial function requirements. */
		return NO_ERRORS;
	}
	
	static int handle_valid(serial_handle_t port_addr)
	{
		int is_valid;
		
		if(port_addr == (void *) 0x3F8 || port_addr == (void *) 0x2F8 || \
			port_addr == (void *) 0x3E8 || port_addr == (void *) 0x2E8)
		{
			is_valid = 1;
		}
		else
		{
			is_valid = 0;
		}
		
		return is_valid;
	}
			

/* End DOS defines. */
#else
	#error "DOS Target chosen, but compiler does not support DOS!"
#endif
