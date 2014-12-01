#ifndef SERIAL_H
#define SERIAL_H

//#include "config.h"
//#include "usr_def.h"

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

typedef void * serial_handle_t;

/* Future- implement 7-bit support- described in ymodem.txt */
extern int serial_init(unsigned short port_no, unsigned long baud_rate, serial_handle_t * port_addr);
extern int serial_snd(char * data, unsigned int num_bytes, serial_handle_t port);
extern int serial_rcv(char * data, unsigned int num_bytes, int timeout, serial_handle_t port);
extern int serial_close(serial_handle_t * port_addr);
extern int serial_flush(serial_handle_t port_addr);
#endif        //  #ifndef SERIAL_H

