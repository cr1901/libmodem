/* USR_DEF.H- Platform-specific defines which abstract file i/o
 * and access to the serial device. */
#ifndef USR_DEF_H
#define USR_DEF_H
#include "config.h"


/** Abstraction layer **/
/* Each system to which this library is ported should have a
 * nonconflicting macro specifying the system. This macro can either
 * be internal to the compiler or a command line define (though I suggest the former). */

#ifdef __DOS__ /* This is a WATCOM-specific macro. */
	#define SERIAL_HANDLE void *
	/* GNU, Microsoft's requirement, CMake Watcom, and Watcom defined-macros respectively. */
#elif defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__NT__)
	#include <windows.h>
	#define SERIAL_HANDLE HANDLE
#elif defined(__VIC20__)
	#define SERIAL_HANDLE void *
#endif


/* Certain systems may not support redirection of stderr. */
//Doesn't work. Also might not be the right way to do this...
/* #ifdef __DOS__
	FILE * debug_stream = stdout;
	FILE * message_stream = stderr;
#else
	FILE * debug_stream = stderr;
	FILE * message_stream = stdout;
#endif */


/* #ifdef __DOS__
	#define DEBUG_STREAM stdout
	#define MESSAGE_STREAM stderr
#else
	#define DEBUG_STREAM stderr
	#define MESSAGE_STREAM stdout
#endif */




/** Library typedefs affected by platform specific defines. **/
/* I do not suggest modifying the below typedefs, unless absolutely
 * necessary. However, the below code shows which macros need to be
 * defined depending on the target configuration. At the very least,
 * SERIAL_HANDLE must be defined- this is NO default!- W. Jones. */


/* Check to make sure the required macros are defined. */
#ifndef SERIAL_HANDLE
	#error Macro SERIAL_HANDLE must be defined.
#elif !defined USING_STDIO_H && !defined MODEM_FILE
	#error Macro MODEM_FILE must be defined if not using C file I/O.
#endif


/* Change this data to an equivalent if FILE pointer does not exist
 * on target system (for instance, small microcontrollers with limited
 * memory). Comparable routines will need to be implemented by end 
 * user/programmer. */
#ifdef USING_STDIO_H
	#include <stdio.h>
	typedef FILE modem_file_t;
#else
	typedef MODEM_FILE modem_file_t;
#endif

/* Used to get a handle to implementation-specific pointer to 
 * serial port. Direct hardware access may not need this, in which
 * case it is safe to set it to the peripheral's memory-mapped or
 * port-mapped address. NULL is reserved for failure. */
typedef SERIAL_HANDLE serial_handle_t;



#endif
