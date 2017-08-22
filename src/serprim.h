#ifndef SERPRIM_H
#define SERPRIM_H

#include "serial.h"

/** \file serprim.h
\brief Serial Port Primitives

serprim.h consists of functions that either wrap around an existing functions
to access the serial port or implement a mechanism to _exclusively_ access
a particular UART interface in a given environment.
*/

/**
\brief Acquire resources for a serial port.

This function shall allocate and create a pointer to data which holds all
the serial port state required for accessing a UART. If open_handle() succeeds,
the returned handle will return nonzero if passed into handle_valid().

\param[in] port_no An integer representing a UART identifier.
\returns A valid ::serial_handle_t if resources, including the UART was
acquired, an invalid handle otherwise.

\sa serial_init()
*/
serial_handle_t open_handle(unsigned short port_no);

/**
\brief Check if serial port handle points to valid data.

This function determines whether a given ::serial_handle_t pointer is valid.
A ::serial_handle_t is valid if the _value of the pointer_ either represents a
valid pointer to port state, or a magic constant (such as the address of the
UART peripheral for a given microcontroller) indicating the handle is in use.

\param[in] port Opaque representation of port state (or port address if no
state).
\returns Nonzero (preferably `1`) if the handle is valid, 0 otherwise.

\sa serial_init() serial_snd() serial_rcv() serial_close() serial_flush()
*/
int handle_valid(serial_handle_t port);

/** \brief Set UART parameters.

init_port() sets parameters for the serial port, such as baud rates, flow
control options, and default timeouts from the UART's point of view
(if necessary). Currently only setting the baud rate is supported, and common
default parameters should be set for other behavior (if any- some UARTs
are hardcoded for one mode of operation). _Interrupt handlers for transmit and
receive, if any, should also be enabled in this function if they are not
already._

The difference between init_port() and open_handle() is that open_handle()
allocates/acquires resources for accessing the serial port, while init_port()
(eventually) accesses the underlying serial port and other hardware
to initialize its registers for future operations. Thus, open_handle() and
init_port() must both be called before performing read/write operations.

\param[in] port Handle to a serial port.
\param[in] baud_rate Baud rate to set UART. An implementation is free to
ignore this parameter.
\returns Nonzero if initialization of the UART failed (perhaps after a sanity
check), 0 on success. An implementation may return 0 unconditionally.

\sa serial_init()
*/
int init_port(serial_handle_t port, unsigned long baud_rate);

/** \brief Do serial port write.

This function does the actual write to either a transmit buffer or the transmit
register of a UART. Exactly \p num_bytes shall be transferred into a buffer
or to the UART directly from the source buffer \p data. The number of bytes
transmitted over the UART from \p data is undefined if this function fails.

\param[in] port Handle to a serial port.
\param[in] data Buffer of data to send.
\param[in] num_bytes Number of bytes to send.
\returns 0 if all bytes were transferred to a transmit buffer _or_ transmitted
to the UART transmit register successfully, nonzero (`-1`) otherwise.

\sa serial_snd()
*/
int write_data(serial_handle_t port, char * data, unsigned int num_bytes);

/** \brief Do serial port read.

This function does the actual read from a asynchronously-filled receive buffer
or the receive register of UART. Exactly \p num_bytes shall be transferred into
the destination buffer \p data. The number of bytes received into \p data is
undefined if this function fails.

\param[in] port Handle to a serial port.
\param[out] data Buffer of data to receive.
\param[in] num_bytes Number of bytes to receive.
\param[in] timeout Timeout in seconds to wait for data.
\retval 0 The read completed successfully.
\retval -1 The read timed out before \p num_bytes were read.
\retval -2 All other possible errors reading (hardware failure, etc). All other
nonzero values are acceptable.

\sa serial_rcv()
*/
int read_data(serial_handle_t port, char * data, unsigned int num_bytes, int timeout);

/** \brief Do serial port read, and report elapsed time.

This function is required because some data transfer protocols (such as an
xmodem transmitter) need to time out after a certain amount of time has
passed without receiving a _correct_ acknowledgement value, regardless of the
number of bytes received while waiting.

For simplicity of implementation, \p elapsed need not be valid if the read
timed out. This allows an implementor to wrap read_data() within
this function if they so choose.

An example of an implementation using only the ANSI C standard library may look
like this:

\code{.c}
#include <time.h>

int read_data_get_elapsed_time(serial_handle_t port, char * data, unsigned int num_bytes, int timeout, int * elapsed)
{
	int rc;
	time_t start, end;

	time(&start)
	rc = read_data(port, data, num_bytes, timeout);
	time(&end);

	*elapsed = (int) difftime(end, start);
	return rc;
}
\endcode

\param[in] port Handle to a serial port.
\param[out] data Buffer of data to receive.
\param[in] num_bytes Number of bytes to receive.
\param[in] timeout Timeout in seconds to wait for data.
\param[out] elapsed Time spent in this function (and children), from invocation
to return, in seconds.
\returns Return value semantics are identical to read_data().

\bug \p elapsed should have greater granularity to prevent \p elapsed from
returning 0 if data is received close to but not greater than one
second apart. Data transfer functions rely on adding elapsed to a global
`time_spent` counter each invocation.

\sa read_data() serial_rcv()
*/
int read_data_get_elapsed_time(serial_handle_t port, char * data, unsigned int num_bytes, int timeout, int * elapsed);

/** \brief Deallocate resources for a serial port.

close_handle() shall deallocate the resources acquired from open_handle() that
were required to successfully access an underlying UART. No further operations
on the UART may be performed using this handle. It is up to the implementor
whether they want to set serial port hardware registers back to defaults in
this function.

\param [in] port Handle to a serial port.
\returns 0 if resources were successfully deallocated, nonzero (preferably
`-1`) otherwise.

\bug close_handle() should take a `::serial_handle_t *` as input, which returns
an invalid handle on success.

\sa serial_close()
*/
int close_handle(serial_handle_t port);

/** \brief Flush received data buffers from serial port.

This function invalidates any buffers used for receiving data from a serial
port, if any. Any time after this function is called, the next character
received from the UART hardware itself into an asynchronously-filled buffer
(interrupt or thread) will be the first character in the \p data array
(index `0`) of read_data() or read_data_get_elapsed_time() (whichever is called
first).

Although rare, failure can occur, for instance, if resources are in use by
another thread and yielding/busy-waiting is not implemented. Note that
data transfer API functions do not gracefully handle buffer flush failure
and abort transfer instead of trying a few times.

On implementations without receive buffers, this function is a no-op that
returns success.

\param [in] port Handle to a serial port.
\returns 0 if buffers were successfully flushed, nonzero (preferably `-1`)
on any failure. The next received character is undefined if flush fails.

\todo Remove undefined behavior from a bad flush.

\sa serial_flush() serial_close()
*/
int flush_device(serial_handle_t port);

#endif        /*  #ifndef SERPRIM_H  */
