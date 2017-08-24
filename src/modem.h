#ifndef MODEM_H
#define MODEM_H

/** \file modem.h
\brief Data Transfer API

modem.h consists of user-facing functions to transfer and receive data. Each
function expects at least a valid handle opened by open_handle(), and a
function pointer that indicates how to transfer the data before/after serial
transmit/receive.
*/

#include "serial.h"
#include <stddef.h>

/* Useful macros. */
#define BIT(_x) 1 << _x

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

/** \brief Internal use enum for reading/writing XMODEM packets.
Indices to an array which point to locations within a XMODEM packet.
Pointer arithmetic between offsets gets the number of octets (bytes)
to read or write for a given operation.

\todo This enum is publicly exported for historical reasons. It shouldn't be.
*/
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
}offset_names_t;

/** \brief Status codes for all Data Transfer API functions.

This enum provides error reasons if a data transfer did not complete
successfully. The enum encompasses all possible return values from each
data transfer function. Thus, not all data transfer routines use each
enum value. Consult individual function documentation for which return values
are possible.

\todo Mapping ::BAD_CRC_CHKSUM to ::MODEM_TIMEOUT is an oversight.
*/
typedef enum modem_errors{
	MODEM_NO_ERRORS = 0, /**< Transfer completed successfully. */
	BAD_CRC_CHKSUM, /**< Used to indicate a bad XMODEM CRC or checksum for the
	current received packet. This value is never returned, and gets mapped
	to ::MODEM_TIMEOUT if enough errors occur. */
	SENT_CAN, /**< XMODEM receiver cancelled the transfer. */
	PACKET_MISMATCH, /**< XMODEM packet equals number different from expected. */
	CHANNEL_ERROR, /**< ::input_channel_t or ::output_channel_t callback function
	reported an error. */
	UNDEFINED_ERROR, /**< Placeholder error used in contexts where a conversion
	from ::serial_status_t to ::modem_errors_t failed. */
	MODEM_HW_ERROR, /**< Error reading, writing, or accessing serial device.
	Corresponds to ::SERIAL_HW_ERROR in ::serial_status_t. */
	MODEM_TIMEOUT,	/**< No data was read in expected time frame. Corresponds to
	::SERIAL_TIMEOUT in ::serial_status_t. */
	NOT_IMPLEMENTED
}modem_errors_t;

/** \brief XMODEM transfer mode selection.

This enum is an input parameter into the xmodem_tx() and xmodem_rx() functions
to determine which variant of the XMODEM protocol to use.
*/
typedef enum xfer_modes
{
	XMODEM = 0, /**< Use XMODEM w/ 128 byte packets and 8-bit checksum. */
	XMODEM_CRC, /**< Use XMODEM w/ 128 byte packets and 16-bit CRC. */
	XMODEM_1K /**< Use XMODEM w/ 1024 byte packets and 16-bit CRC. */
}xmodem_xfer_mode_t;

/**
\typedef output_channel_t
\brief Transmit function pointer for data transfer routines.

A function of type ::output_channel_t describes how to send data from an
application to the serial port to be transmitted via a data transfer protocol.
Each data transfer protocol will supply valid input parameters as needed.

\param[out] buf Buffer of received serial port data. The callback can assume
the buffer length is greater than or equal to \p request_size.
\param[in] request_size Size of the payload the data trasmitter requested.
\param[in] last_sent_size Size of the last payload sent over the serial port.
This parameter will be < \p request_size of the previous iteration if a
payload couldn't be fully sent. This input _must_ be zero before any data was
sent over the serial port (in case the initial transmit failed).
\param[in,out] chan_state An opaque pointer to the state required to send data
properly.
\returns Value of \p request_size if the entire buffer was copied from the
transmit source to \p buf.
\returns A nonnegative integer less than \p buf_size if less than
\p request_size was transferred without error. _An ::output_channel_t callback
may be called again in this condition (on, for instance, transmit failure of
the current packet)._
\returns A negative value on unrecoverable errors.

Each data transfer routine checks the return value from an ::output_channel_t
callback to determine what to do next.

An example of a generic transmit function for a global or preallocated
buffer looks like the following. Note that memcpy is used for convenience.
An error is returned if the is insufficient space in the source buffer to
satisfy the request size, which a data transfer routine will detect:

\code{.c}
typedef struct buf_reader {
	unsigned char * buf;
	size_t bufpos;
	size_t buflen;
} buf_reader_t;

int read_from_buf(char * buf, const int request_size, const int last_sent_size, void * const chan_state) {
	int space_left, size_read;
	buf_read_t * reader = chan_state;

	reader->bufpos += last_sent_size;
	space_left = reader->buflen - reader->bufpos;

	size_read = space_left < buf_size ? space_left : request_size;
	memcpy(buf, reader->buf + reader->bufpos, size_read);

	return size_read;
}
\endcode

\todo There is currently no way for a function of ::output_channel_t to know
whether it will be called again after it has reported filling \b buf with
data < \p request_size. This means it's not possible to clean up \p chan_state
resources within an ::output_channel_t, contrast to ::input_channel_t, which
has an \p eof parameter. Thus, a user must wait until after a transmit routine
returns to deallocate resources. There should be feature parity between the two
functions.
*/
typedef int (* output_channel_t)(char * buf, const int request_size, const int last_sent_size, void * const chan_state);

/**
\typedef input_channel_t
\brief Receive function pointer for data transfer routines.

A function of type ::input_channel_t describes how to receive data from the
serial port via a data transfer protocol for further use by an application.
Each data transfer protocol will supply valid input parameters as needed.

\param[in] buf Buffer of received serial port data.
\param[in] buf_size Size of the buffer with received data.
\param[in] eof End of transfer was detected by the protocol. This parameter is
useful for closing a FILE pointer within the data transfer routine.
\param[in,out] chan_state An opaque pointer to the state required to receive
data properly.
\returns Value of \p buf_size if the entire buffer was copied to the
destination properly, an integer less than \p buf_size (possibly negative) if
any error occurred. Each data transfer routine checks for equality on return
from an ::input_channel_t callback to determine what to do next.

An example of a generic receive function for a global or preallocated
buffer looks like the following. Note that memcpy is used for convenience,
and \p eof is not used in this case. An error is returned if the is
insufficient space in the destination buffer, which a data transfer routine
will detect:

\code{.c}
typedef struct buf_writer {
	unsigned char * buf;
	size_t bufpos;
	size_t buflen;
} buf_writer_t;

int write_to_buf(const char * buf, const int buf_size, const int eof, void * const chan_state) {
	int space_left, size_sent;
	buf_writer_t * writer = chan_state;

	space_left = writer->buflen - writer->bufpos;

	size_sent = space_left < buf_size ? space_left : buf_size;
	memcpy(writer->buf + writer->bufpos, buf, size_sent);

	writer->bufpos += size_sent;
	return size_sent;
}
\endcode
*/
typedef int (* input_channel_t)(const char * buf, const int buf_size, const int eof, void * const chan_state);

/* Wrapper function for all possible xfer modes (wrapper.c).
(Possibly open serial port as well?) */
/* uint16_t modem_tx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags);
uint16_t modem_rx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags); */

/** \brief XMODEM transmitter implementation.

xmodem_tx() will initiate a data transmit session over the opened serial device
\p device. The function will call callback \p data_out to request data for a
new packet to send. For each packet to be sent, xmodem_tx() will pass a pointer
within \p buf, along with appropriately-sized number of bytes for \p data_out
to fill. Data transfer ends when \p data_out indicates an end-of-data
condition.

\param[in,out] data_out Callback that xmodem_tx() uses to obtain more data.
\param[in] buf Intermediate buffer, used to hold a full XMODEM packet. A
pointer offset into the buffer where the data payload starts is passed into
\p data_out's \p buf parameter. It is up to the caller to ensure the buffer
is appropriately sized:
- 132 bytes if ::XMODEM is used.
- 133 bytes if ::XMODEM_CRC is used.
- 1029 bytes if ::XMODEM_1K is used.
\param[in,out] chan_state State for callback \p data_out, passed into the \p
chan_state parameter of \p data_out. xmodem_tx() itself does not modify
\p chan_state.
\param[in] device Handle to a serial port.
\param[in] flags XMODEM protocol variant to use. ::XMODEM_1K will be converted
to ::XMODEM_CRC when less than 1024 bytes are left to send.

\sa output_channel_t xmodem_xfer_mode_t
*/
modem_errors_t xmodem_tx(output_channel_t data_out, unsigned char * buf, void * chan_state, serial_handle_t device, const xmodem_xfer_mode_t flags);

/** \brief XMODEM receiver implementation.

xmodem_rx() will initiate a data receive session over the opened serial device
\p device. The function will call callback \p data_in as a data sink for
received packets. xmodem_rx() will pass a pointer within \p buf, along with a
valid buffer size for \p data_in to send to a destination. Data transfer has
ended when \p eof indicates an end-of-data condition (`1`).

\param[in,out] data_in Callback that xmodem_rx() uses to send data.
\param[in] buf Intermediate buffer, used to hold a full XMODEM packet. A
pointer offset into the buffer where the data payload starts is passed into
\p data_in's \p buf parameter. It is up to the caller to ensure the buffer
is appropriately sized:
- 132 bytes if ::XMODEM is used.
- 133 bytes if ::XMODEM_CRC is used.
- 1029 bytes if ::XMODEM_1K is used.
\param[in,out] chan_state State for callback \p data_in, passed into the \p
chan_state parameter of \p data_in. xmodem_rx() itself does not modify
\p chan_state.
\param[in] device Handle to a serial port.
\param[in] flags XMODEM protocol variant to use. The receiver will accept any
combination of ::XMODEM_1K and ::XMODEM_CRC packets during receipt.
Additionally, during initial negotiation, ::XMODEM_1K and ::XMODEM_CRC will be
downgraded to ::XMODEM if the xmodem_rx() does not receive an appropriate
response within 3 attempts.

\sa input_channel_t xmodem_xfer_mode_t
*/
modem_errors_t xmodem_rx(input_channel_t data_in, unsigned char * buf, void * chan_state, serial_handle_t device, const xmodem_xfer_mode_t flags);

/** \brief SFL receiver implementation.

sfl_rx() will initiate a data receive session over the opened serial device
\p device. The function will call callback \p data_in as a data sink for
received packets. sfl_rx() will pass a pointer within \p buf, along with a
valid buffer size for \p data_in to send to a destination. Data transfer has
ended when \p eof indicates an end-of-data condition (`1`).

\param[in,out] data_in Callback that sfl_rx() uses to send data.
\param[in] buf Intermediate buffer, used to hold a full SFL packet. A
pointer offset into the buffer where the data payload starts is passed into
\p data_in's \p buf parameter. It is up to the caller to ensure the buffer
at least 256 bytes.
\param[in,out] chan_state State for callback \p data_in, passed into the \p
chan_state parameter of \p data_in. sfl_rx() itself does not modify
\p chan_state.
\param[in] device Handle to a serial port.
*/
modem_errors_t sfl_rx(input_channel_t data_in, unsigned char * buf, void * chan_state, serial_handle_t device);

/* Change data sizes all to uint32_t? */
/** \brief Generate 8-bit checksum.

Exported by xmodem.c. Calculate an 8-bit checksum.

\param[in] data Buffer to calculate the checksum.
\param[in] size Size of the input buffer.

\returns 8-bit checksum, usable by XMODEM transmitter and receiver.

*/
unsigned char generate_chksum(unsigned char * data, size_t size);

/** \brief Generate XMODEM CRC16.

Exported by xmodem.c. Calculate the 16-bit CRC required by XMODEM.

\param[in] data Buffer to calculate the CRC.
\param[in] size Size of the input buffer.

\returns 16-bit CRC, usable by XMODEM transmitter and receiver.

*/
unsigned short generate_crc(unsigned char * data, size_t size);

#endif
