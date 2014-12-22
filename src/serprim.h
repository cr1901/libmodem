#ifndef SERPRIM_H
#define SERPRIM_H

serial_handle_t open_handle(unsigned short port_no);
int handle_valid(serial_handle_t port);
int init_port(serial_handle_t port, unsigned long baud_rate);
int write_data(serial_handle_t port, char * data, unsigned int num_bytes);
int read_data(serial_handle_t port, char * data, unsigned int num_bytes, int timeout);
int close_handle(serial_handle_t port);
int flush_device(serial_handle_t port);

#endif        /*  #ifndef SERPRIM_H  */

