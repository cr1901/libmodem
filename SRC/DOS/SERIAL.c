 /* Obviously, portability requirements can be relaxed if linking
  * against external support libraries, but try to use standard types
  * from <stdint.h> as much as possible. */
  
/* This syntax permits the compiler to build from the root of the project
directory. */

#include "cc_config.h"

#include "modem.h" //Todo- get rid of dependency on modem.h
//#include "usr_def.h"
#include "serial.h"
#include <stdint.h>
//#include <stdlib.h>

/** Begin DOS defines. **/
#ifdef __DOS__
	#include <dos.h>
	#include <comlib.h>
	#include <stdio.h>
	
	/* Switch EXIT_SUCCESS to NO_ERRORS at some point.- Done.- W. Jones */
	uint16_t serial_init(uint8_t port_no, uint32_t baud_rate, serial_handle_t * port_addr)
	{
		uint16_t comlib_br, comlib_pn;
		
		switch(port_no)
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
		}
		
		
		if(!comopen(comlib_pn, comlib_br, CO_NOPARITY, CO_DATA8, CO_STOP1, CO_IRQDEFAULT))
		{
			/* Note to self: Make sure near/far are distinguished. */
			
			(* port_addr) = (void *) 0xCAFE; /* Dummy memory address to the serial port.
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
	
	extern uint16_t serial_snd(uint8_t * data, uint16_t num_bytes, /* uint8_t timeout, */ serial_handle_t port)
	{
		/* Timeout measured against system timer, 1/18 second per tick. */
		//int prev_timeout = comgettimeout();
		//comsettimeout((int) (timeout * 18));
		if(!comwrite((char *) data, (int) num_bytes))
		{
			return NO_ERRORS;
		}
		else
		{
			return SERIAL_ERROR;
		}
		//comsettimeout(prev_timeout);
	}
	
	extern uint16_t serial_rcv(uint8_t * data, uint16_t num_bytes, uint8_t timeout, serial_handle_t port)
	{
		/* Timeout measured against system timer, 1/18 second per tick. */
		int prev_timeout = comgettimeout();
		
		/* if(_PL_comoverflow)
		{
			printf("Overflow of buffer!\n");
		} */
		
		comsettimeout((int) (timeout * 18));
		if(!comread((char *) data, (int) num_bytes))
		{
			comsettimeout(prev_timeout);
			return NO_ERRORS;
		}
		else
		{
			comsettimeout(prev_timeout);
			return TIMEOUT;
		}
	}
	
	uint16_t serial_close(serial_handle_t * port_addr)
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
	
	uint16_t serial_flush(serial_handle_t port_addr)
	{
		comflush(); 
		/* Returns number of chars flushed- cannot use as return value
		 * per the serial function requirements. */
		return NO_ERRORS;
		/* if(FlushFileBuffers(port_addr))
		{
			return NO_ERRORS;
		}
		else
		{
			return SERIAL_ERROR;
		} */
	}

/* End DOS defines. */
#else
	#error "DOS Target chosen, but compiler does not support DOS!"
#endif
