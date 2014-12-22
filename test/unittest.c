#include "minunit.h"
#include "shared.h"

#include "serial.h"
#include "modem.h"

#include <limits.h>

typedef struct verify_params
{
	int force_resend;
	
}VERIFY_PARAMS;



/* For future full duplex consideration, have tx/rx bufs for both. */
char local_tx_file_buf[STATIC_BUFSIZ] = {'\0'};
char local_rx_file_buf[STATIC_BUFSIZ] = {'\0'};
char remote_tx_file_buf[STATIC_BUFSIZ] = {'\0'};
char remote_rx_file_buf[STATIC_BUFSIZ] = {'\0'};

serial_handle_t local_port, remote_port;
VERIFY_PARAMS ver_params;

/* Data xfer fcns used as XMODEM callbacks. */
/* int verify_data_out_fcn(char * buf, const int request_size, const int last_sent_size, void * const chan_state); */
/* int verify_data_in_fcn(char * buf, const int request_size, const int last_sent_size, void * const chan_state); */

/* Helper functions. */
static void fill_buf(char * buf, unsigned int num_chars);
static int buf_cmp(char * buf1, char * buf2);
static void buf_clr(char * buf, unsigned int num_chars);


/* Test setup clears all buffers and assumes a working serial port. */
void test_setup() {
	serial_init(LOCAL, 115200, &local_port);
	serial_init(REMOTE, 115200, &remote_port);
	buf_clr(local_tx_file_buf, STATIC_BUFSIZ);
	buf_clr(local_rx_file_buf, STATIC_BUFSIZ);
	buf_clr(remote_tx_file_buf, STATIC_BUFSIZ);
	buf_clr(remote_rx_file_buf, STATIC_BUFSIZ);
}

void test_teardown() {
	serial_close(&local_port);
	serial_close(&remote_port);
}

MU_TEST(test_ser_bad_handle)
{
	mu_check(serial_close(&local_port) == SERIAL_NO_ERRORS);
	fill_buf(local_tx_file_buf, 128);
	mu_check(serial_init(BAD_DEVICE, 115200, &local_port) == SERIAL_HW_ERROR);
	/* mu_check(local_port == &port_model[BAD_DEVICE]); Make no assumptions about port value on init error? 
	Or set to NULL/guaranteed invalid handle? The former implies "no guarantees about what happens 
	when calling other functions with a bad handle." Any need to have access to/otherwise
	manipulate handles directly? */
	mu_check(serial_snd(local_tx_file_buf, 128, local_port) == SERIAL_HW_ERROR);
	mu_check(serial_close(&local_port) == SERIAL_HW_ERROR);
	
	mu_check(serial_close(&remote_port) == SERIAL_NO_ERRORS);
	mu_check(serial_init(BAD_HANDLE, 115200, &remote_port) == SERIAL_HW_ERROR);
	mu_check(remote_port == NULL);
	mu_check(serial_rcv(remote_rx_file_buf, 128, 1, remote_port) == SERIAL_HW_ERROR);
	mu_check(serial_flush(remote_port) == SERIAL_HW_ERROR);	
	mu_check(serial_close(&remote_port) == SERIAL_HW_ERROR);
	
	mu_check(buf_cmp(local_tx_file_buf, remote_rx_file_buf) == 0);
}

MU_TEST(test_ser_tx)
{
	fill_buf(local_tx_file_buf, 128);	
	mu_check(serial_snd(local_tx_file_buf, 128, local_port) == SERIAL_NO_ERRORS);
	mu_check(serial_rcv(remote_rx_file_buf, 128, 1, remote_port) == SERIAL_NO_ERRORS);
	mu_check(buf_cmp(local_tx_file_buf, remote_rx_file_buf) == 1);
}

MU_TEST(test_ser_rx_hw_error_bad_read)
{
	VOID_TO_PORT(remote_port, bad_read) = 1;
	fill_buf(local_tx_file_buf, 128);
	mu_check(serial_snd(local_tx_file_buf, 128, local_port) == SERIAL_NO_ERRORS);
	mu_check(serial_rcv(remote_rx_file_buf, 128, 1, remote_port) == SERIAL_HW_ERROR);
	mu_check(buf_cmp(local_tx_file_buf, remote_rx_file_buf) == 0);	
}

MU_TEST(test_ser_tx_hw_error_bad_write)
{
	VOID_TO_PORT(local_port, bad_write) = 1;
	fill_buf(local_tx_file_buf, 128);
	/* Nothing was sent in this example, but the point is there was an error
	when trying to fulfill the request. */
	mu_check(serial_snd(local_tx_file_buf, 128, local_port) == SERIAL_HW_ERROR);
	
	mu_check(buf_cmp(local_tx_file_buf, remote_rx_file_buf) == 0); 
	mu_check(serial_rcv(remote_rx_file_buf, 128, 1, remote_port) == SERIAL_NO_ERRORS);
	mu_check(buf_cmp(local_tx_file_buf, remote_rx_file_buf) == 0);	
}

MU_TEST(test_ser_rx_timeout)
{
	VOID_TO_PORT(remote_port, force_rx_timeout) = 1;
	fill_buf(local_tx_file_buf, 128);
	mu_check(serial_snd(local_tx_file_buf, 128, local_port) == SERIAL_NO_ERRORS);
	mu_check(serial_rcv(remote_rx_file_buf, 128, 1, remote_port) == SERIAL_TIMEOUT);
	mu_check(buf_cmp(local_tx_file_buf, remote_rx_file_buf) == 0);	
}

MU_TEST(test_ser_rx_flush_close)
{
	fill_buf(local_tx_file_buf, 128);	
	mu_check(serial_snd(local_tx_file_buf, 128, local_port) == SERIAL_NO_ERRORS);
	/* Assume: at this point, the data has been sent, but has not been 
	retrieved from the remote's internal rcv buffer. */
	mu_check(serial_flush(remote_port) == SERIAL_NO_ERRORS);
	mu_check(serial_rcv(remote_rx_file_buf, 128, 1, remote_port) == SERIAL_NO_ERRORS);
	/* Because of the flush, the buffers should not be equal... serial_rcv
	should have received a fresh buffer full of '\0' :). */
	mu_check(buf_cmp(local_tx_file_buf, remote_rx_file_buf) == 0);
	
	/* Simulate a bad flush. */
	VOID_TO_PORT(remote_port, bad_flush) = 1;
	mu_check(serial_snd(local_tx_file_buf, 128, local_port) == SERIAL_NO_ERRORS);
	mu_check(serial_flush(remote_port) == SERIAL_HW_ERROR);
	mu_check(serial_rcv(remote_rx_file_buf, 128, 1, remote_port) == SERIAL_NO_ERRORS);
	
	/* Because the flush failed, the buffers should be equal now (in this
	test suite, flush is all or nothing). */
	mu_check(buf_cmp(local_tx_file_buf, remote_rx_file_buf) == 1);
	mu_check(serial_close(&remote_port) == SERIAL_HW_ERROR);
	VOID_TO_PORT(remote_port, bad_flush) = 0;
}

MU_TEST(test_ser_close)
{
	fill_buf(local_tx_file_buf, 128);	
	mu_check(serial_snd(local_tx_file_buf, 128, local_port) == SERIAL_NO_ERRORS);
	
	/* Simulate bad close due to bad flush. (If closing flush failed, this
	test suite assumes the serial port is still open for use). */
	VOID_TO_PORT(remote_port, bad_flush) = 1;
	mu_check(serial_close(&remote_port) == SERIAL_HW_ERROR);
	mu_check(serial_rcv(remote_rx_file_buf, 128, 1, remote_port) == SERIAL_NO_ERRORS);
	mu_check(buf_cmp(local_tx_file_buf, remote_rx_file_buf) == 1);
	
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

/* MU_TEST(test_xmodem_packet)
{
	fill_buf(local_tx_file_buf, 128);
} */

MU_TEST_SUITE(ser_test_suite) 
{
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
	
	MU_RUN_TEST(test_ser_tx);
	MU_RUN_TEST(test_ser_tx_hw_error_bad_write);
	
	MU_RUN_TEST(test_ser_rx_hw_error_bad_read);
	MU_RUN_TEST(test_ser_rx_timeout);
	MU_RUN_TEST(test_ser_rx_flush_close);
	
	MU_RUN_TEST(test_ser_bad_handle);
	MU_RUN_TEST(test_ser_close);
	/* MU_RUN_TEST(test_ser_edge); */
	
	
	/* MU_RUN_TEST(xmodem_test); */
}


int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	
	MU_RUN_SUITE(ser_test_suite);
	MU_REPORT();
	return 0;
}


/* int verify_data_out_fcn(char * buf, const int request_size, const int last_sent_size, void * const chan_state)
{
	
	
} */


static void fill_buf(char * buf, unsigned int num_chars)
{
	unsigned int cur;
	  
	for(cur = 0; cur < num_chars; cur++)
	{
		buf[cur] = cur;
	}
}

static int buf_cmp(char * buf1, char * buf2)
{
	int cur;
	int tx_okay = 1;
	
	for(cur = 0; (tx_okay && cur < 128); cur++)
	{
		tx_okay = (((unsigned char *) buf1)[cur] == ((unsigned char *) buf2)[cur]);
	}
	
	return tx_okay;
}

static void buf_clr(char * buf, unsigned int num_chars)
{
	unsigned int cur;
	  
	for(cur = 0; cur < num_chars; cur++)
	{
		buf[cur] = '\0';
	}
}
