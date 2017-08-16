#ifndef MODEM_H
#define MODEM_H

#include "serial.h"
#include <stddef.h>

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
compatibility. The platform specific EOF should be checked for,
while XMODEM_EOF is sent in response. Referenced in x/ymodem.txt,
so underscored is omitted. */
#define CPMEOF SUB

/* Indices to an array which point to locations within a packet.
Pointer arithmetic between offsets gets the number of octets (bytes)
to read or write for a given operation. */
typedef enum offset_names
{
	START_CHAR = 0,
	BLOCK_NO,
	COMP_BLOCK_NO,
	DATA,
	CHKSUM_CRC=131,
	CHKSUM_END=132,
	CRC_END=133,
	X1K_CRC=1027,
	X1K_END=1029
}OFFSET_NAMES;

/* Receive/Send status codes. */
typedef enum modem_errors{
	MODEM_NO_ERRORS = 0,
	BAD_CRC_CHKSUM,
	SENT_NAK,
	SENT_CAN,
	PACKET_MISMATCH, /* Packet equals number different from expected. */
	ASYNC_XFER_INCOMPLETE, /* May not be useful. */
	CHANNEL_ERROR,
	UNDEFINED_ERROR, /* Placeholder error. */
	MODEM_HW_ERROR, /* Error reading, writing, or accessing serial device. */
	MODEM_TIMEOUT,	/* No data was read in expected time frame. */
	INVALID_MODE, /* Invalid xfer mode for function (i.e. using YMODEM_G in XMODEM). */
	NOT_IMPLEMENTED
}MODEM_ERRORS;

/* RX/TX input flags. */
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
	unsigned long bytes_transferred;
	unsigned short time_elapsed;
	char * filename;
}MODEM_STATUS;

typedef int (* O_channel)(char * buf, const int request_size, const int last_sent_size, void * const chan_state);
typedef int (* I_channel)(const char * buf, const int buf_size, const int eof, void * const chan_state);

/* Wrapper function for all possible xfer modes (wrapper.c).
(Possibly open serial port as well?) */
/* uint16_t modem_tx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags);
uint16_t modem_rx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags); */

MODEM_ERRORS xmodem_tx(O_channel data_out, unsigned char * buf, void * chan_state, serial_handle_t device, const unsigned short flags);
MODEM_ERRORS xmodem_rx(I_channel data_in, unsigned char * buf, void * chan_state, serial_handle_t device, const unsigned short flags);

/* Change data sizes all to uint32_t? */
unsigned char generate_chksum(unsigned char * data, size_t size);
unsigned short generate_crc(unsigned char * data, size_t size);

#endif
