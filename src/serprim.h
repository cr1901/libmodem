#ifndef SERPRIM_H
#define SERPRIM_H

serial_handle_t open_handle(unsigned short port_no);
int handle_valid(serial_handle_t port);
int init_port(serial_handle_t port, unsigned long baud_rate);
int write_data(serial_handle_t port, char * data, unsigned int num_bytes);
int read_data(serial_handle_t port, char * data, unsigned int num_bytes, int timeout);
int close_handle(serial_handle_t port);
int flush_device(serial_handle_t port);


/* #include <limits.h>

#if defined(UNITTEST) || defined(DOCS) */
/* For the sake of documentation and unit tests, code that can be written using
macros are defined using functions instead to ease documentation generation. */
/* serial_handle_t open_handle(unsigned short port_no);
int handle_valid(serial_handle_t port);
int init_port(serial_handle_t port, unsigned long baud_rate);
int valid_size(unsigned int num_bytes);
int write_data(serial_handle_t port, char * data, unsigned int num_bytes);
int read_data(serial_handle_t port, char * data, unsigned int num_bytes, int timeout);
int close_handle(serial_handle_t port);
int flush_device(serial_handle_t port);

#elif defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__NT__)
serial_handle_t open_handle(unsigned short port_no);
int handle_valid(serial_handle_t port);
int init_port(serial_handle_t port, unsigned long baud_rate);
#define valid_size(nb) ((unsigned int) nb <= ULONG_MAX) */ /* This better be always true! */

/* int write_data(serial_handle_t port, char * data, unsigned int num_bytes);
int read_data(serial_handle_t port, char * data, unsigned int num_bytes, int timeout);
int close_handle(serial_handle_t port);
int flush_device(serial_handle_t port);


#elif defined(__DOS__)
serial_handle_t open_handle(unsigned short port_no);
int handle_valid(serial_handle_t port);
int init_port(serial_handle_t port, unsigned long baud_rate);
#define valid_size(nb) ((unsigned int) nb <= INT_MAX)
int write_data(serial_handle_t port, char * data, unsigned int num_bytes);
int read_data(serial_handle_t port, char * data, unsigned int num_bytes, int timeout);
int close_handle(serial_handle_t port);
int flush_device(serial_handle_t port); 


#endif */


#endif        /*  #ifndef SERPRIM_H  */

