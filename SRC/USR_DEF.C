/* User-defined routines go in this file. Use especially for
 * wrapper functions to existing routines used as callbacks (i.e.
 * serial library) and possibly interrupts updating flags. */
 
 /* Static global variables for accessing platform-specific data,
  * such as the serial port handle, should go here. */
  
 /* Obviously, portability requirements can be relaxed if linking
  * against external support libraries, but try to use standard types
  * from <stdint.h> as much as possible. */

#include "modem.h"
#include "config.h"
#include "usr_def.h"
#include <stdint.h>
#include <stdlib.h>

/** Begin DOS defines. **/
#ifdef __DOS__
#include <dos.h>
#include <comlib.h>

/* Switch EXIT_SUCCESS to NO_ERRORS at some point. */
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
	
	
	if(!comopen(comlib_pn, comlib_br, CO_NOPARITY, 8, CO_STOP1, CO_IRQDEFAULT))
	{
		/* Note to self: Make sure near/far are distinguished. */
		
		(* port_addr) = (void *) 0xCAFE; /* Dummy memory address to the serial port.
		* Note that a pointer to the address of the serial port is passed
		* because the pointer to the serial port itself may be (in this
		* case, is) a void pointer (which cannot dereferenced). */
		return EXIT_SUCCESS;
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
		return EXIT_SUCCESS;
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
	comsettimeout((int) (timeout * 18));
	if(!comread((char *) data, (int) num_bytes))
	{
		return EXIT_SUCCESS;
	}
	else
	{
		return TIMEOUT;
	}
	comsettimeout(prev_timeout);
}

uint16_t serial_close(serial_handle_t * port_addr)
{
	if(!comclose())
	{
		(* port_addr) = NULL;
		return EXIT_SUCCESS;
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


/** Begin Windows defines... **/
#elif defined __WIN32__
/* Serial routines ripped from:
 * http://www.robbayer.com/files/serial-win.pdf
 * without whose permission this was made possible :)...
 * Seriously, thanks! */


#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

//void windows_error(char *);
uint16_t serial_init(uint8_t port_no, uint32_t baud_rate, serial_handle_t * port_addr)
{
	char com_string[12] = "\\\\.\\COM\0\0\0\0";
	DCB dcbSerialParams = {0};
	COMMTIMEOUTS timeouts= {0};

	sprintf(&com_string[7], "%d", port_no);
	(* port_addr) = CreateFile(com_string, GENERIC_READ | GENERIC_WRITE, 0, NULL, \
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	
	/* After this check, serial_close must be called on error. */
	if((* port_addr) == INVALID_HANDLE_VALUE)
	{
		(* port_addr) = NULL;
		/* See Receive/Send status codes in MODEM.H for difference
		 * between SERIAL_ERROR and TIMEOUT. */
		return SERIAL_ERROR;
	}
	
	
	/* Set the COM port parameters. */
	if (!GetCommState((* port_addr), &dcbSerialParams)) 
	{
		serial_close(port_addr); /* Sets port to null. */
		return SERIAL_ERROR;
	}	
		
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	/* Using real values instead of CBR_BAUD_RATE for simplicity.
	* Allowed according to MSDN:
	* http://msdn.microsoft.com/en-us/library/windows/desktop/aa363214(v=vs.85).aspx */
	dcbSerialParams.BaudRate=baud_rate;
	dcbSerialParams.ByteSize=8;
	dcbSerialParams.StopBits=ONESTOPBIT;
	dcbSerialParams.Parity=NOPARITY;
	dcbSerialParams.fBinary = TRUE;
	dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
	dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;
	dcbSerialParams.fOutxCtsFlow = FALSE;
	dcbSerialParams.fOutxDsrFlow = FALSE;
	dcbSerialParams.fDsrSensitivity= FALSE;
	dcbSerialParams.fAbortOnError = TRUE;
	
	/* Note to self: Forgot the indirection operator here- W. Jones... */
	if(!SetCommState((* port_addr), &dcbSerialParams))
	{
		serial_close(port_addr); /* Sets port to null. */
		return SERIAL_ERROR;
	}
	
	/* Add if-else here? */
	GetCommTimeouts((* port_addr),&timeouts);
	timeouts.ReadIntervalTimeout        = MAXDWORD; 
	timeouts.ReadTotalTimeoutMultiplier    = 0;
	timeouts.ReadTotalTimeoutConstant    = 0;
	timeouts.WriteTotalTimeoutMultiplier    = 0;
	timeouts.WriteTotalTimeoutConstant    = 0;
	
	if(!SetCommTimeouts((* port_addr), &timeouts))
	{
		serial_close(port_addr);	
		return SERIAL_ERROR;
	}	
	else
	{
		return NO_ERRORS;
	}
}

extern uint16_t serial_snd(uint8_t * data, uint16_t num_bytes, /* uint8_t timeout, */ serial_handle_t port)
{
	DWORD dwBytesWritten = 0;
	
	if(WriteFile(port, (char *) data, (DWORD) num_bytes, &dwBytesWritten, NULL))
	{
		return NO_ERRORS;
	}
	else
	{
		return SERIAL_ERROR;
	}
}

extern uint16_t serial_rcv(uint8_t * data, uint16_t num_bytes, uint8_t timeout, serial_handle_t port)
{	
	DWORD dwBytesRead = 0;
	COMMTIMEOUTS prev_timeouts= {0};
	COMMTIMEOUTS curr_timeouts= {0};
	
	//printf("Time left in timeout: %d\n", timeout);
	
	/* Timeout measured in milliseconds */
	if(!GetCommTimeouts(port, &prev_timeouts))
	{
		//windows_error("serial_rcv_1()");	
		return SERIAL_ERROR;
	}
	
	curr_timeouts = prev_timeouts;
	curr_timeouts.ReadIntervalTimeout        = 1000 * timeout; 
	curr_timeouts.ReadTotalTimeoutMultiplier    = 0;
	curr_timeouts.ReadTotalTimeoutConstant    = 1000 * timeout;
	curr_timeouts.WriteTotalTimeoutMultiplier    = 10;
	curr_timeouts.WriteTotalTimeoutConstant    = 50;
	
	if(!SetCommTimeouts(port, &curr_timeouts))
	{
		return SERIAL_ERROR;
	}

	if(ReadFile((HANDLE) port, (char *) data, (DWORD) num_bytes, &dwBytesRead, NULL))
	{
		/* If no error occurred, check to see that all bytes requested
		 * were received... if not, this is considered a timeout. */
		SetCommTimeouts(port, &prev_timeouts);
		if(dwBytesRead == (DWORD) num_bytes)
		{	
			return NO_ERRORS;
		}
		else
		{
			return TIMEOUT;
		}
	}
	/* Otherwise a more serious error occurred. */
	else
	{
		SetCommTimeouts(port, &prev_timeouts);
		return SERIAL_ERROR;
	}
		
	
	/* Handle this somehow... */
	/* if(dwBytesRead < num_bytes)
	{
		return SERIAL_ERROR;
	} */
}

uint16_t serial_close(serial_handle_t * port_addr)
{
	/* Both the flush and close must succeed to return
	 * without error. */
	if(!serial_flush(port_addr) && CloseHandle((* port_addr)))
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
	if(FlushFileBuffers(port_addr))
	{
		return NO_ERRORS;
	}
	else
	{
		return SERIAL_ERROR;
	}
}

/** Begin __VIC20__ defines... **/
#elif defined __VIC20__
/* Switch EXIT_SUCCESS to NO_ERRORS at some point. */
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
	
	
	if(!comopen(comlib_pn, comlib_br, CO_NOPARITY, 8, CO_STOP1, CO_IRQDEFAULT))
	{
		/* Note to self: Make sure near/far are distinguished. */
		
		(* port_addr) = (void *) 0xCAFE; /* Dummy memory address to the serial port.
		* Note that a pointer to the address of the serial port is passed
		* because the pointer to the serial port itself may be (in this
		* case, is) a void pointer (which cannot dereferenced). */
		return EXIT_SUCCESS;
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
		return EXIT_SUCCESS;
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
	comsettimeout((int) (timeout * 18));
	if(!comread((char *) data, (int) num_bytes))
	{
		return EXIT_SUCCESS;
	}
	else
	{
		return TIMEOUT;
	}
	comsettimeout(prev_timeout);
}

uint16_t serial_close(serial_handle_t * port_addr)
{
	if(!comclose())
	{
		(* port_addr) = NULL;
		return EXIT_SUCCESS;
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



/* End VIC20 defines. */

#endif