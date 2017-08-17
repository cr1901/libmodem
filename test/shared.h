#ifndef SHARED_H
#define SHARED_H

/* Shared variables and stuff between the dummy serial port class and the
unit test functions. */

#define STATIC_BUFSIZ 3076

#define LOCAL 0
#define REMOTE 1
#define BAD_DEVICE 2 /* Bad device should be treated as a device that exists,
but is having permanent hardware issues. */
#define BAD_HANDLE 3

typedef struct port_desc
{
	char * tx_line;
	char * rx_line;
	int bad_write; /* Automatic fail for snd. */
	int bad_read; /* Automatic fail for rcv. */
	int force_rx_timeout; /* Automatic timeout. */
	int bad_close;
	int bad_flush;
	unsigned int buf_pos_tx;
	unsigned int buf_pos_rx;
}port_desc_t;

#define VOID_TO_PORT(x, y) ((port_desc_t *) x)->y
extern port_desc_t port_model[3];

#endif        /*  #ifndef SHARED_H  */
