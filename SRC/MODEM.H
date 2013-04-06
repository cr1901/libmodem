/* 08.15.2012 01:21:18
 * MODEM.H
 * 
 * Copyright 2012 William D. Jones <thor0505@comcast.net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
#ifndef MODEM_H
#define MODEM_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "config.h"
#include "usr_def.h"


/*** Macros ***/
/* Useful macros. */
#define BIT(_x) 1 << _x
#define MODEM_FALSE 0
#define MODEM_TRUE 1


/* ASCII defines. */
#define NUL 0x00
#define SOH 0x01
#define STX 0x02
#define EOT 0x04
#define ACK 0x06
#define TAB 0x09
#define CR  0x0D
#define LF  0x0A
#define NAK 0x15
#define CAN 0x18
#define SUB 0x1A
#define ASCII_C 0x43

/* EOF = SUB in XModem at least. This was for CP/M and DOS
 * compatibility. The platform specific EOF should be checked for,
 * while XMODEM_EOF is sent in response. Referenced in x/ymodem.txt,
 * so underscored is omitted. */
#define CPMEOF SUB

/** Constants which define offsets into TXRX buffer. **/
/* Indices to an array which point to locations within a packet.
 * Pointer arithmetic between offsets gets the number of octets (bytes)
 * to read or write for a given operation. */
typedef enum offset_names
{
	START_CHAR = 0,
	BLOCK_NO,
	COMP_BLOCK_NO,
	DATA,
	CHKSUM_CRC,
	END
}OFFSET_NAMES;

/** Receive/Send status codes. **/
typedef enum modem_errors{
	NO_ERRORS = 0,
	BAD_CRC_CHKSUM,
	SENT_NAK,
	PACKET_MISMATCH, /* Packet equals number different from expected. */
	ASYNC_XFER_INCOMPLETE, /* May not be useful. */
	FILE_ERROR,
	UNDEFINED_ERROR, /* Placeholder error. */
	SERIAL_ERROR, /* Error reading, writing, or accessing serial device. */
	TIMEOUT,	/* No data was read in expected time frame. */
	INVALID_MODE, /* Invalid xfer mode for function (i.e. using YMODEM_G in XMODEM). */
	NOT_IMPLEMENTED
}MODEM_ERRORS;

/** RX/TX input flags. **/
typedef enum xfer_modes
{
	XMODEM = 0,
	XMODEM_CRC,
	XMODEM_1K,
	XMODEM7,
	YMODEM,
	YMODEM_1K,
	YMODEM_G,
	ZMODEM,
	KERMIT
}XFER_MODES;

typedef struct modem_status{
	uint32_t bytes_transferred;
	uint16_t time_elapsed;
	char * filename;	
}MODEM_STATUS;


/* Global enums and status struct declarations */

/* Declare in functions which need it. Look up purpose of passing enums. */
//MODEM_ERRORS current_error;
//MODEM_STATUS current_status;
//XFER_MODES current_mode;

/* Fallback refers to downgrading from XMODEM-1K to XMODEM-CRC, which
 * may not be standard. Additionally, a distinction is made between
 * XMODEM-CRC and XMODEM-1K with 128-byte packets, though they are
 * effectively transfer the same bytes for a given packet. */
 
 
 /* * Removed to make protocol receiver-driven. Distinction between
 * XMODEM-1K with 128 packets and XMODEM-CRC is still made- W. Jones. */
//#define FALLBACK BIT(3)



/* Currently, ignore contradictions in the code when reading this section.
 * I just haven't taken the time to implement the return values yet!
 * - W. Jones */

/* Serial status codes */
//#define TIMEOUT BIT(7)
/* Use SERIAL_ERROR for any other situation besides timeout.
 * However, if a function is capable of timing out (rcv), return 
 * TIMEOUT on error. */
//#define SERIAL_ERROR BIT(6)

/* May not be useful... */
//#define ASYNC_XFER_INCOMPLETE BIT(5)

/* Critical Errors */
//#define FILE_ERROR BIT(4)
//#define PACKET_MISMATCH BIT(3)
//#define UNDEFINED_ERROR BIT(2)

/* Recoverable Errors */

/* Xfer status codes */

//#define SENT_NAK BIT(1)



//#define BAD_CRC_CHKSUM BIT(0)
//#define NO_ERRORS 0x00

/* XMODEM 7 should go here as well. */

/* Add YMODEM/ZMODEM/Kermit later. */


/*** Function prototypes. ***/
/* Can async/sync tx/rx use the same function with the same return
 * values (nonzero on error vs. specific flags)? */
 /* Yes: SYNC- Send/receive all bytes at once, exec idle at end of loop.
  * ASYNC- Send/receive all bytes at once or in packets, exec idle, return to check
  * status (use counter variables to check what still needs to be sent/received).
  * Callback- Routine to send single byte. */
  /* flags- XYZ Modem, ASYNC. */
  
  /* Check timeout- TIMER INTERRUPT required... how to handle referencing variables? 
   * Start timer, exec rcv... if no error, stop/reset timer, else loop (if timeout by end, break). */
//session_function(parameters);

/* Wrapper function for all possible xfer modes (wrapper.c). 
(Possibly open serial port as well?) */
//uint16_t modem_tx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags);
//uint16_t modem_rx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags);
uint16_t xmodem_tx(modem_file_t * f_ptr, serial_handle_t device, uint8_t flags);
uint16_t xmodem_rx(modem_file_t * f_ptr, serial_handle_t device, uint8_t flags);
//uint16_t ymodem_tx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags);
//uint16_t ymodem_rx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags);
//uint16_t zmodem_tx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags);
//uint16_t zmodem_rx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags);
//uint16_t kermit_tx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags);
//uint16_t kermit_rx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags);

/* xmodem_tx, xmodem_rx, xmodem_crc_tx, etc... or
return NOT_IMPLEMENTED status if user optimizes for space. Leaning towards 
xmodem_tx, xmodem_rx, ymodem_tx, ymodem_rx, etc... */

/* Change data sizes all to uint32_t? */
uint8_t generate_chksum(uint8_t * data, uint16_t size);
uint16_t generate_crc(uint8_t * data, uint16_t size);


/** User-implemented functions. **/
/* File opening and closing. If stdio is not used, comparable
 * routines need to be implemented. Use the macros below as
 * an example on how to implement such routines. */
 //fread/fwrite, size_t issues...
#ifndef USING_STDIO_H
	extern modem_file_t * modem_fopen_read(const char * name);
	extern modem_file_t * modem_fopen_write(const char * name);
	extern int modem_fclose(modem_file_t * stream);
	extern int modem_fseek(modem_file_t * stream, long int offset);
	extern size_t modem_fread(void * ptr, size_t count, modem_file_t * stream);
	extern size_t modem_fwrite(const void * ptr, size_t count, modem_file_t * stream);
	extern int modem_feof(modem_file_t * stream);
#else
	#define modem_fopen_read(_x) fopen(_x, "rb")
	#define modem_fopen_write(_x) fopen(_x, "wb")
	#define modem_fclose(_x) fclose(_x)
	#define modem_fseek(_x, _y) fseek(_x, _y, SEEK_SET)
	#define modem_fread(_x, _y, _z) fread(_x, sizeof(uint8_t), _y, _z)
	#define modem_fwrite(_x, _y, _z) fwrite(_x, sizeof(uint8_t), _y, _z)
	#define modem_feof(_x) feof(_x)
#endif

#ifndef USING_DIFFTIME
	extern (uint16_t) modem_difftime(time_t time_2, time_t time_1);
#else
	#define modem_difftime(_x, _y) (uint16_t) difftime(_x, _y)
#endif

/* If using asynchronous I/O (interrupt-driven), use an idle function to
 * get other small things done within the context of an application
 * program (NOT the OS- let the OS handle multitasking with it's own
 * mechanisms).
 * Also includes an error handler 
 * Examples... update a status bar/report errors using stdout. */
extern void idle(void);
extern void error_handler(uint16_t status);

/* File attributes functions (take in filename, return info). */

//extern uint32 get_file_size(char * filename);
//extern void get_modification_date(char * filename, char * mod_date);

/* The next routines send and receive data in the context of a 
 * MODEM session. 
 * Requirements:
 * 1. Send/Receive Functions must take four parameters:
 * 	A. A pointer to arbitrary data bytes (meaning 8 bits, not necessarily 
 *     smallest addressable unit) to be transmitted.
 *  B. The number of bytes to be transmitted in the current function call. 
 *     Cast from uint32_t should be acceptable.
 *  C. Time IN SECONDS before the serial routine times out while waiting
 *     for a byte from the opposite end.
 *  D. Void pointer (serial_handle_t) to the serial port to be
 *     used for transmission.
 * 2. ALL below functions must return 0 on success, nonzero value on failure for
 *    any reason (at present time, either return TIMEOUT, SERIAL_ERROR, or
 * 	  NO_ERRORS).
 * 3. serial_handle_t must be set to NULL if error setting up 
 *    access to hardware occurs, or the port has been closed 
 *    (value at initialization not considered). */

/* THANK GOD FOR <time.h>! */
//extern uint16_t check_timer(timer_handle_t t_port);
//extern timer_handle_t init_timer(void);
//extern start_timer(timer_handle_t t_port); 
//extern uint16_t reset_timer(timer_handle_t t_port);
//extern uint16_t stop_timer(timer_handle_t t_port);

/* Future- implement 7-bit support- described in ymodem.txt */
extern uint16_t serial_init(uint8_t port_no, uint32_t baud_rate, serial_handle_t * port_addr);
extern uint16_t serial_snd(uint8_t * data, uint16_t num_bytes, serial_handle_t port);
extern uint16_t serial_rcv(uint8_t * data, uint16_t num_bytes, uint8_t timeout, serial_handle_t port);
extern uint16_t serial_close(serial_handle_t * port_addr);
extern uint16_t serial_flush(serial_handle_t port_addr);

#endif
