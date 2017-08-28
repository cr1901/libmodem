#ifndef SERIAL_H
#define SERIAL_H

/** \file serial.h
\brief Serial Port API

serial.h consists of user-facing functions to access an underlying UART.
The functions themselves eventually call into serprim.h, and exist to wrap
potential error conditions in a platform-independent manner using return values
from the primitives.

\todo Add `from_native`/`to_native` for hosted implementations. This allows
opting out of the other API functions while still using modem.h functions. It
should typecast the _address_ of the passed-in variable to/from
serial_device_t.
*/

/** \brief Opaque handle to serial port state.

::serial_handle_t is an alias for a void pointer, passed into each serial
primitive routine. It should point to a global (or allocated) data structure
consisting of all state required to successfully access the serial port when
the given UART interface is open for use by libmodem. See serial_init() for
an example of state that primitives may require.

When wrapping an existing API, ::serial_handle_t can safely be typecast to a
handle returned from the API called within serprim.h functions. In freestanding
environments, ::serial_handle_t can be a pointer to the base address
of the currently-used serial port if no extra state is required. This may
occur, for instance, if the API provided by the platform doesn't return
a handle (hdmi2usb), only one serial port is present (hdmi2usb), or the API
doesn't support multiple opened UARTs at once (PICTOR on dos).

When libmodem does not have access to the serial port, an instance of
::serial_handle_t should point to an invalid value which _need not_ be NULL.
The value of an invalid handle _must_ return `0` from handle_valid().
*/
typedef void * serial_handle_t;

/** \brief Return value for all functions in serial.h

This enum reports the status of all operations performed on a serial port in
a platform-independent manner. Individual enum values are not currently set.
*/
typedef enum serial_status
{
	SERIAL_NO_ERRORS, /**< Underlying primitives reported no errors. */
	SERIAL_TIMEOUT, /**< Underlying read waited too long to receive data. */
	SERIAL_HW_ERROR /**< Catch-all for all other errors. */
}serial_status_t;

/** \brief Initialize underlying serial port.

serial_init() should perform any initialization required to operate on the
initialized UART using the remaining API functions correctly. Such
initialization may include getting exclusive access to the UART and a timer
for measuring timeouts.

It is up to the primitive implementor to decide what resources serial_init()
should require for a given implementation, depending on the features they want.
For instance, libmodem may support yielding to another thread while waiting
for data to arrive on one platform, but not another (although all current
implementations block).

\param[in] port_no Integer corresponding to a UART resource to open.
\param[in] baud_rate Baud rate to set the underlying serial port, if the
platform supports it.
\param[out] port_addr Handle to a serial port.
\returns Status of the operation and a handle to a serial port. \p port_addr
is only valid if ::serial_status_t returned ::SERIAL_NO_ERRORS.
::SERIAL_HW_ERROR is returned if the underlying primitives open_handle() or
init_port() reported an error.
\todo \p port_no is not sufficient for a POSIX implementation as an integer.
Refactor into a typedef that is a `char *` string when `__STDC_HOSTED__` is
defined, `int` otherwise.
\todo Implement 7-bit support- described in ymodem.txt

\sa open_handle() handle_valid() init_port()
*/
serial_status_t serial_init(unsigned short port_no, unsigned long baud_rate, serial_handle_t * port_addr);

/** \brief Send data over serial port.

serial_snd() should send the data pointed to by \p data into the serial port.
An implementation may choose to send the data one character at a time by
directly accessing writing to the send register of the UART, or may choose
to write to an intermediate buffer which will asynchronously be written to
the send register.

\param[in] data Buffer of data to send.
\param[in] num_bytes Number of bytes to send.
\param[in] port Handle to a serial port.
\retval ::SERIAL_NO_ERRORS The sent data is no longer libmodem's
responsibility. Depending on implementation, the data may not have actually
been sent over the wire yet when this function returns (hdmi2usb).
\retval ::SERIAL_HW_ERROR \p port_addr was invalid or the primitive
write_data() reported an error.

\sa handle_valid() write_data()
*/
serial_status_t serial_snd(char * data, unsigned int num_bytes, serial_handle_t port);

/** \brief Receive data over serial port.

serial_receive() should receive data from the serial port into the buffer
pointed to by \p data. An implementation may choose to block while receiving
or yield. Additionally, an implementation may choose to read from a buffer that
is filled asynchronously or incrementally read one character at a time.

\anchor old_time_behavior \p timeout should nominally be measured for the entire read
operation. However, in cases where this is not possible, \p timeout may be
measured one character at a time for a buffer, timing out if any read takes
longer than 1 second.

\param[in] data Buffer of data to receive.
\param[in] num_bytes Number of bytes to receive.
\param[in] timeout Timeout in seconds to wait for data. A timeout of 0 returns
immediately. A timeout < 0 will be converted to 0 by the underlying primitive.
\param[in,out] time_spent If non-NULL, serial_rcv will return the time elapsed
waiting for data in this parameter. If ::SERIAL_TIMEOUT was returned, the
value need not match \p timeout; rely on ::SERIAL_TIMEOUT and \p timeout in
this case. If this param is NULL, then primitive read_data() is called;
otherwise read_data_get_elapsed_time() is called.
\param[in] port Handle to a serial port.
\retval ::SERIAL_NO_ERRORS An entire buffer of data was received successfully.
\retval ::SERIAL_TIMEOUT The underlying primitive timed out waiting for the
entire buffer of data. _Some_ data may have been received depending on
implementation. \p time_spent is invalid.
\retval ::SERIAL_HW_ERROR \p port_addr was invalid, or the primitives
read_data() or read_data_get_elapsed_time() reported an error.
\todo Behavior described in \ref old_time_behavior is historical and unlikely
to work. It should be removed.

\sa handle_valid() read_data() read_data_get_elapsed_time()
*/
serial_status_t serial_rcv(char * data, unsigned int num_bytes, int timeout, int * time_spent, serial_handle_t port);

/** \brief Close serial port.

serial_close() shall deallocate any resources that were previously required for
serial port operation and set the input ::serial_handle_t equal to a value
which returns nonzero from handle_valid(). serial_close() will also call
serial_flush().

\param[in, out] port_addr Handle to a serial port.

\returns ::SERIAL_NO_ERRORS if all three of the following hold,
::SERIAL_HW_ERROR otherwise:
- \p port_addr was a valid handle.
- The serial port was successfully flushed.
- All resources that were acquired during serial_init() were deallocated.

\sa handle_valid() serial_flush() close_handle()
*/
serial_status_t serial_close(serial_handle_t * port_addr);

/** \brief Flush all characters from the receive buffer.

Serial flush will clear a previously-acquired (using serial_init()) buffer
for receiving characters of its contents. If no such buffer exists, this
function is a no-op. As flushing is an operation that rarely fails, an
implementation may choose to to have buffer flushes always succeed.

\param[in] port_addr Handle to a serial port.

\todo A corresponding transmit buffer flush should exist.
\todo At one point in the past, the input to this function was
`serial_handle_t *`. Why did I change it?

\sa handle_valid() flush_device()
*/
serial_status_t serial_flush(serial_handle_t port_addr);

#endif        /*  #ifndef SERIAL_H  */
