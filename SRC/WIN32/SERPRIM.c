#include "serial.h"
#include "serprim.h"


/** Begin Windows defines... **/
/* Serial routines ripped from:
 * http://www.robbayer.com/files/serial-win.pdf
 * without whose permission this was made possible :)...
 * Seriously, thanks! */
#include <windows.h>
#include <stdio.h> /* For sprintf. Since we're on windows, we should use it. */
#include <stddef.h> /* For NULL. */


serial_handle_t open_handle(unsigned short port_no)
{
	char com_string[12] = "\\\\.\\COM\0\0\0\0";
	HANDLE port_addr;
	
	sprintf(&com_string[7], "%d", port_no);
	port_addr = CreateFile(com_string, GENERIC_READ | GENERIC_WRITE, 0, NULL, \
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	return port_addr;
}

int handle_valid(serial_handle_t port)
{
	return (port != INVALID_HANDLE_VALUE);
}

int init_port(serial_handle_t port, unsigned long baud_rate)
{
	DCB dcbSerialParams;
	COMMTIMEOUTS timeouts;
	
	if(!GetCommState(port, &dcbSerialParams)) 
	{
		serial_close(port); /* Sets port to null. */
		return -1;
	}
	
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	/* Using real values instead of CBR_BAUD_RATE for simplicity.
	* Allowed according to MSDN:
	* http://msdn.microsoft.com/en-us/library/windows/desktop/aa363214(v=vs.85).aspx */
	/* printf("Baud rate: %lu\n", baud_rate); */
	
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
	if(!SetCommState(port, &dcbSerialParams))
	{
		return -2;
	}
	
	/* Add if-else here? */
	GetCommTimeouts(port, &timeouts);
	timeouts.ReadIntervalTimeout = MAXDWORD; 
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	
	if(!SetCommTimeouts(port, &timeouts))
	{
		return -3;
	}
	
	return 0;
}

int write_data(serial_handle_t port, char * data, unsigned int num_bytes)
{
	DWORD dwBytesWritten = 0;
	
	/* No INT_MAX check necessary- num_bytes will fit into DWORD always. */	
	return WriteFile(port, data, (DWORD) num_bytes, &dwBytesWritten, NULL) ? 0 : -1;
}

int read_data(serial_handle_t port, char * data, unsigned int num_bytes, int timeout)
{
	DWORD timeout_ticks;
	DWORD dwBytesRead = 0;
	COMMTIMEOUTS prev_timeouts;
	COMMTIMEOUTS curr_timeouts;
	
	/* printf("Time left in timeout: %d\n", timeout); */
	
	/* Guard against negative values being converted to ridiculous timeouts. 
	A timeout < 0 will be set to 0. */
	/* All values of num_bytes and 1000*timeout can be represented in a DWORD
	which windows expects, so no INT_MAX check is necessary. */
	if(timeout < 0)
	{
		timeout = 0;
	}
	
	/* Protect against signed overflow by casting timeout to DWORD. */
	timeout_ticks = 1000uL * timeout;
	
	/* Timeout measured in milliseconds */
	if(!GetCommTimeouts(port, &prev_timeouts))
	{
		/* windows_error("serial_rcv_1()"); */	
		return -2;
	}
	
	curr_timeouts = prev_timeouts;
	curr_timeouts.ReadIntervalTimeout = timeout_ticks; 
	curr_timeouts.ReadTotalTimeoutMultiplier = 0;
	curr_timeouts.ReadTotalTimeoutConstant = timeout_ticks;
	curr_timeouts.WriteTotalTimeoutMultiplier = 10;
	curr_timeouts.WriteTotalTimeoutConstant = 50;
	
	if(!SetCommTimeouts(port, &curr_timeouts))
	{
		return -2;
	}

	if(ReadFile((HANDLE) port, data, (DWORD) num_bytes, &dwBytesRead, NULL))
	{
		/* If no error occurred, check to see that all bytes requested
		 * were received... if not, this is considered a timeout. */
		SetCommTimeouts(port, &prev_timeouts);
		return (dwBytesRead == (DWORD) num_bytes) ? 0 : -1;
	}
	/* Otherwise a more serious error occurred. */
	else
	{
		SetCommTimeouts(port, &prev_timeouts);
		return -2;
	}
}

int close_handle(serial_handle_t port)
{
	return CloseHandle(port) ? 0 : -1;
}

int flush_device(serial_handle_t port)
{
	return FlushFileBuffers(port) ? 0 : -1;
}

