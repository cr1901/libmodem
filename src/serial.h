#ifndef SERIAL_H
#define SERIAL_H

typedef void * serial_handle_t;

typedef enum serial_status
{
	SERIAL_NO_ERRORS,
	SERIAL_TIMEOUT,
	SERIAL_HW_ERROR
}SERIAL_STATUS;

/* Future- implement 7-bit support- described in ymodem.txt */
SERIAL_STATUS serial_init(unsigned short port_no, unsigned long baud_rate, serial_handle_t * port_addr);
SERIAL_STATUS serial_snd(char * data, unsigned int num_bytes, serial_handle_t port);
SERIAL_STATUS serial_rcv(char * data, unsigned int num_bytes, int timeout, int * time_spent, serial_handle_t port);
SERIAL_STATUS serial_close(serial_handle_t * port_addr);
SERIAL_STATUS serial_flush(serial_handle_t port_addr);
#endif        /*  #ifndef SERIAL_H  */
