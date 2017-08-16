#include "minunit.h"
#include "shared.h"

#include "serial.h"
#include "modem.h"

#include <stddef.h>
#include <limits.h>

typedef struct tx_params
{
	char * data_source;
	char * data_sink;
	size_t source_size;
	size_t source_pos;
	int force_bad_checksum;
}TX_PARAMS;

typedef struct rx_params
{
	char * data_source;
	char * data_sink;
	int force_resend;
}RX_PARAMS;


/* For future full duplex consideration, have tx/rx bufs for both. */
char local_source[STATIC_BUFSIZ] = {'\0'};
char local_sink[STATIC_BUFSIZ] = {'\0'};
char remote_source[STATIC_BUFSIZ] = {'\0'};
char remote_sink[STATIC_BUFSIZ] = {'\0'};
char cpmeof_buf[1024] = {CPMEOF};

unsigned char temp_buf[X1K_END + 1]; /* A dummy buffer to make the xmodem routines happy. */

serial_handle_t local_port, remote_port;
TX_PARAMS tx_opts = {local_source, local_sink, 0, 0, 0};
RX_PARAMS rx_opts = {remote_source, remote_sink, 0};

/* Data xfer fcns used as XMODEM callbacks. */
static int data_out_fcn(char * buf, const int request_size, const int last_sent_size, void * const chan_state);
/* static int data_in_fcn(char * buf, const int request_size, const int last_sent_size, void * const chan_state); */
/* Helper functions. */
static void fill_buf(char * buf, unsigned int num_chars);
static int buf_cmp(char * buf1, char * buf2, int len);
static void * buf_cpy(char * dest, char * src, size_t len);
static void buf_clr(char * buf, unsigned int num_chars);

static void verify_packet(char * packet, unsigned char packet_no, char * payload, \
	unsigned int payload_len, int using_1k);

/* Setup/teardown functions for each test. */
/* Test setup clears all buffers and assumes a working serial port. */
void ser_test_setup()
{
	serial_init(LOCAL, 115200, &local_port);
	serial_init(REMOTE, 115200, &remote_port);
	buf_clr(tx_opts.data_source, STATIC_BUFSIZ);
	buf_clr(tx_opts.data_sink, STATIC_BUFSIZ);
	buf_clr(rx_opts.data_source, STATIC_BUFSIZ);
	buf_clr(rx_opts.data_sink, STATIC_BUFSIZ);
}

void ser_test_teardown()
{
	serial_close(&local_port);
	serial_close(&remote_port);
}

void xmodem_test_setup()
{
	tx_opts.force_bad_checksum = 0;
	tx_opts.source_size = 0;
	tx_opts.source_pos = 0;
	rx_opts.force_resend = 0;
	ser_test_setup();
}

void xmodem_test_teardown()
{
	ser_test_teardown();
}


/* Actual tests begin here. */
MU_TEST(test_ser_bad_handle)
{
	mu_check(serial_close(&local_port) == SERIAL_NO_ERRORS);
	fill_buf(tx_opts.data_source, 128);
	mu_check(serial_init(BAD_DEVICE, 115200, &local_port) == SERIAL_HW_ERROR);
	/* mu_check(local_port == &port_model[BAD_DEVICE]); Make no assumptions about port value on init error?
	Or set to NULL/guaranteed invalid handle? The former implies "no guarantees about what happens
	when calling other functions with a bad handle." Any need to have access to/otherwise
	manipulate handles directly? */
	mu_check(serial_snd(tx_opts.data_source, 128, local_port) == SERIAL_HW_ERROR);
	mu_check(serial_close(&local_port) == SERIAL_HW_ERROR);

	mu_check(serial_close(&remote_port) == SERIAL_NO_ERRORS);
	mu_check(serial_init(BAD_HANDLE, 115200, &remote_port) == SERIAL_HW_ERROR);
	mu_check(remote_port == NULL);
	mu_check(serial_rcv(rx_opts.data_sink, 128, 1, NULL, remote_port) == SERIAL_HW_ERROR);
	mu_check(serial_flush(remote_port) == SERIAL_HW_ERROR);
	mu_check(serial_close(&remote_port) == SERIAL_HW_ERROR);

	mu_check(buf_cmp(tx_opts.data_source, rx_opts.data_sink, 128) == 0);
}

MU_TEST(test_ser_tx)
{
	fill_buf(tx_opts.data_source, 128);
	mu_check(serial_snd(tx_opts.data_source, 128, local_port) == SERIAL_NO_ERRORS);
	mu_check(serial_rcv(rx_opts.data_sink, 128, 1, NULL, remote_port) == SERIAL_NO_ERRORS);
	mu_check(buf_cmp(tx_opts.data_source, rx_opts.data_sink, 128) == 1);
}

MU_TEST(test_ser_rx_hw_error_bad_read)
{
	VOID_TO_PORT(remote_port, bad_read) = 1;
	fill_buf(tx_opts.data_source, 128);
	mu_check(serial_snd(tx_opts.data_source, 128, local_port) == SERIAL_NO_ERRORS);
	mu_check(serial_rcv(rx_opts.data_sink, 128, 1, NULL, remote_port) == SERIAL_HW_ERROR);
	mu_check(buf_cmp(tx_opts.data_source, rx_opts.data_sink, 128) == 0);
}

MU_TEST(test_ser_tx_hw_error_bad_write)
{
	VOID_TO_PORT(local_port, bad_write) = 1;
	fill_buf(tx_opts.data_source, 128);
	/* Nothing was sent in this example, but the point is there was an error
	when trying to fulfill the request. */
	mu_check(serial_snd(tx_opts.data_source, 128, local_port) == SERIAL_HW_ERROR);

	mu_check(buf_cmp(tx_opts.data_source, rx_opts.data_sink, 128) == 0);
	mu_check(serial_rcv(rx_opts.data_sink, 128, 1, NULL, remote_port) == SERIAL_NO_ERRORS);
	mu_check(buf_cmp(tx_opts.data_source, rx_opts.data_sink, 128) == 0);
}

MU_TEST(test_ser_rx_timeout)
{
	VOID_TO_PORT(remote_port, force_rx_timeout) = 1;
	fill_buf(tx_opts.data_source, 128);
	mu_check(serial_snd(tx_opts.data_source, 128, local_port) == SERIAL_NO_ERRORS);
	mu_check(serial_rcv(rx_opts.data_sink, 128, 1, NULL, remote_port) == SERIAL_TIMEOUT);
	mu_check(buf_cmp(tx_opts.data_source, rx_opts.data_sink, 128) == 0);
}

MU_TEST(test_ser_rx_flush_close)
{
	fill_buf(tx_opts.data_source, 128);
	mu_check(serial_snd(tx_opts.data_source, 128, local_port) == SERIAL_NO_ERRORS);
	/* Assume: at this point, the data has been sent, but has not been
	retrieved from the remote's internal rcv buffer. */
	mu_check(serial_flush(remote_port) == SERIAL_NO_ERRORS);
	mu_check(serial_rcv(rx_opts.data_sink, 128, 1, NULL, remote_port) == SERIAL_NO_ERRORS);
	/* Because of the flush, the buffers should not be equal... serial_rcv
	should have received a fresh buffer full of '\0' :). */
	mu_check(buf_cmp(tx_opts.data_source, rx_opts.data_sink, 128) == 0);

	/* Only use the first 128 bytes of the port's buffers for now. */
	VOID_TO_PORT(local_port, buf_pos_tx) = 0;
	VOID_TO_PORT(remote_port, buf_pos_rx) = 0;

	/* Simulate a bad flush. */
	VOID_TO_PORT(remote_port, bad_flush) = 1;
	mu_check(serial_snd(tx_opts.data_source, 128, local_port) == SERIAL_NO_ERRORS);
	mu_check(serial_flush(remote_port) == SERIAL_HW_ERROR);
	mu_check(serial_rcv(rx_opts.data_sink, 128, 1, NULL, remote_port) == SERIAL_NO_ERRORS);

	/* Because the flush failed, the buffers should be equal now (in this
	test suite, flush is all or nothing). */
	mu_check(buf_cmp(tx_opts.data_source, rx_opts.data_sink, 128) == 1);
	mu_check(serial_close(&remote_port) == SERIAL_HW_ERROR);
	VOID_TO_PORT(remote_port, bad_flush) = 0;
}

MU_TEST(test_ser_close)
{
	fill_buf(tx_opts.data_source, 128);
	mu_check(serial_snd(tx_opts.data_source, 128, local_port) == SERIAL_NO_ERRORS);

	/* Simulate bad close due to bad flush. (If closing flush failed, this
	test suite assumes the serial port is still open for use). */
	VOID_TO_PORT(remote_port, bad_flush) = 1;
	mu_check(serial_close(&remote_port) == SERIAL_HW_ERROR);
	mu_check(serial_rcv(rx_opts.data_sink, 128, 1, NULL, remote_port) == SERIAL_NO_ERRORS);
	mu_check(buf_cmp(tx_opts.data_source, rx_opts.data_sink, 128) == 1);

	VOID_TO_PORT(local_port, bad_close) = 1;
	mu_check(serial_close(&local_port) == SERIAL_HW_ERROR);

	VOID_TO_PORT(remote_port, bad_close) = 1;
	mu_check(serial_close(&remote_port) == SERIAL_HW_ERROR);

	VOID_TO_PORT(remote_port, bad_flush) = 0;
	VOID_TO_PORT(local_port, bad_close) = 0;
	VOID_TO_PORT(remote_port, bad_close) = 0;

	mu_check(serial_close(&local_port) == SERIAL_NO_ERRORS);
	mu_check(serial_close(&remote_port) == SERIAL_NO_ERRORS);
}

/* MU_TEST(test_ser_edge)
{
	Reserved. For ex: valid_size test, perhaps in the future.
} */

/* For xmodem tests, tx_params.data_source/sink is the local_tx/remote_rx "file" buffer.
The xmodem packets get stored in local_tx_line/remote_rx_line, which is aliased to
port_desc tx_line/rx_line.
rx_opts.data_sink simply holds the control codes that will be sent back to
the transmitter. */
MU_TEST(test_xmodem_packet)
{
	int count, rx_okay;

	rx_opts.data_source[0] = NAK;
	rx_opts.data_source[1] = ACK;
	rx_opts.data_source[2] = ACK;
	rx_opts.data_source[3] = ACK; /* Fourth ACK is for 128 boundary test. */

	VOID_TO_PORT(local_port, bad_flush) = 1; /* The transmit routine
	will flush its rx buffer occassionally. In the context of a simulation,
	we need these flushes not to occur, because the rx buffer is filled with
	simulated responses before the tx even starts. If serial_flush RV
	is checked within the xmodem routine, add a test override macro. */

	/* Check that the tx simulation is prepared properly. */
	mu_check(serial_snd(rx_opts.data_source, 3, remote_port) == SERIAL_NO_ERRORS);
	mu_assert_int_eq(NAK, VOID_TO_PORT(local_port, rx_line)[0]);

	/* Fill the local source data buffer and make sure it's size is correctly
	set for input into the xmodem_tx fcn. */
	fill_buf(tx_opts.data_source, tx_opts.source_size = 127);
	xmodem_tx(data_out_fcn, temp_buf, &tx_opts, local_port, XMODEM);

	/* Intercept the packet directly and test the transmit routine! */
	serial_rcv(rx_opts.data_sink, CHKSUM_END, 1, NULL, remote_port);
	verify_packet(&rx_opts.data_sink[0], 1, tx_opts.data_source, 127, 0);

	/* Reset the port state. */
	VOID_TO_PORT(local_port, bad_flush) = 0;
	serial_flush(local_port);
	VOID_TO_PORT(local_port, bad_flush) = 1;
	serial_flush(remote_port);

	VOID_TO_PORT(local_port, buf_pos_tx) = 0;
	VOID_TO_PORT(remote_port, buf_pos_tx) = 0;

	/* Now do a bounary test- 128 byte file. Should result in 128 bytes
	extra data of CPMEOF appended. */
	mu_check(serial_snd(rx_opts.data_source, 4, remote_port) == SERIAL_NO_ERRORS);
	mu_assert_int_eq(NAK, VOID_TO_PORT(local_port, rx_line)[0]);
	fill_buf(tx_opts.data_source, tx_opts.source_size = 128);
	xmodem_tx(data_out_fcn, temp_buf, &tx_opts, local_port, XMODEM);

	serial_rcv(rx_opts.data_sink, 2*CHKSUM_END, 1, NULL, remote_port);
	verify_packet(&rx_opts.data_sink[0], 1, tx_opts.data_source, 128, 0);
	verify_packet(&rx_opts.data_sink[CHKSUM_END], 2, NULL, 0, 0);
}

static void verify_packet(char * packet, unsigned char packet_no, char * payload,
	unsigned int payload_len, int using_1k)
{
	unsigned int max_payload_len = using_1k ? 1024 : 128;
	mu_assert_int_eq(SOH, packet[0]);
	mu_assert_int_eq(packet_no, packet[1]);
	mu_assert_int_eq((unsigned char) ~packet_no, (unsigned char) packet[2]);
	if(payload != NULL)
	{
		mu_check(buf_cmp(&packet[3], payload, payload_len) == 1);
	}

	/* Verify EOF at end of packet if necessary. */
	{
		int rx_okay = 1;
		int count;
		int eof_bytes_left = max_payload_len - payload_len;
		for(count = 0; (rx_okay && count < eof_bytes_left); count++)
		{
			rx_okay = (packet[payload_len + count + 3] == CPMEOF);
		}
		mu_check(rx_okay);
	}
}

MU_TEST_SUITE(ser_test_suite)
{
	MU_SUITE_CONFIGURE(&ser_test_setup, &ser_test_teardown);
	MU_RUN_TEST(test_ser_tx);
	MU_RUN_TEST(test_ser_tx_hw_error_bad_write);

	MU_RUN_TEST(test_ser_rx_hw_error_bad_read);
	MU_RUN_TEST(test_ser_rx_timeout);
	MU_RUN_TEST(test_ser_rx_flush_close);

	MU_RUN_TEST(test_ser_bad_handle);
	MU_RUN_TEST(test_ser_close);
	/* MU_RUN_TEST(test_ser_edge); */

	MU_SUITE_CONFIGURE(&xmodem_test_setup, &xmodem_test_teardown);
	MU_RUN_TEST(test_xmodem_packet);
}


int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;

	MU_RUN_SUITE(ser_test_suite);
	MU_REPORT();
	return 0;
}


static int data_out_fcn(char * buf, const int request_size, const int last_sent_size, void * const chan_state)
{
	TX_PARAMS * const tx_parms = (TX_PARAMS * const) chan_state;
	int size_left;
	int actual_size_sent;


	tx_parms->source_pos += last_sent_size;

	size_left = tx_parms->source_size - tx_parms->source_pos;
	actual_size_sent = (request_size < size_left) ? request_size : size_left;

	buf_cpy(buf, tx_parms->data_source + tx_parms->source_pos, request_size);

	return actual_size_sent;
}


static void fill_buf(char * buf, unsigned int num_chars)
{
	unsigned int cur;

	for(cur = 0; cur < num_chars; cur++)
	{
		buf[cur] = cur;
	}
}

static int buf_cmp(char * buf1, char * buf2, int len)
{
	int cur;
	int tx_okay = 1;

	for(cur = 0; (tx_okay && cur < len); cur++)
	{
		tx_okay = (((unsigned char *) buf1)[cur] == ((unsigned char *) buf2)[cur]);
	}

	return tx_okay;
}

static void * buf_cpy(char * dest, char * src, size_t len)
{
	size_t cur;

	for(cur = 0; cur < len; cur++)
	{
		((unsigned char *) dest)[cur] = ((unsigned char *) src)[cur];
	}

	return dest;
}

static void buf_clr(char * buf, unsigned int num_chars)
{
	unsigned int cur;

	for(cur = 0; cur < num_chars; cur++)
	{
		buf[cur] = '\0';
	}
}
