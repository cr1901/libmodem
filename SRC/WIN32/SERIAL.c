/* Obviously, portability requirements can be relaxed if linking
  * against external support libraries, but try to use standard types
  * from <stdint.h> as much as possible. */

#include "modem.h"
//#include "config.h"
//#include "usr_def.h"
#include "files.h"
#include "serial.h"
#include <stdint.h>
#include <stdlib.h>


/** Begin Windows defines... **/
/* See USR_DEF.H for the reasoning of each macro. */
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__NT__)
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
	DCB dcbSerialParams;
	COMMTIMEOUTS timeouts;

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
	//printf("Baud rate: %lu\n", baud_rate);
	
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
	COMMTIMEOUTS prev_timeouts;
	COMMTIMEOUTS curr_timeouts;
	
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
	if((serial_flush(port_addr) == NO_ERRORS) && CloseHandle((* port_addr)))
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
#else
	#error "Windows target chosen, but compiler does not support Windows!"
#endif
