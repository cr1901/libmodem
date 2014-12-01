#include "serial.h"
#include "modem.h"
/* #include "config.h"
#include "usr_def.h" */

/* Do not use these! */
/* static void get_ymodem_filepath(char * fullpath, char * filename, uint8_t numchars); */

/* #if TIME_MEASURES_UNIXTIME
	#define static void get_ymodem_time as time
endif
static void get_ymodem_time
void get_ymodem_permissions(modem_file_t * file_pointer);
void get_ymodem_filesize(modem_file_t * file_pointer); */

/* ymodem_tx/rx wraps around xmodem_tx/rx- check options assemble packet 0, send/rx packet,
wait for/send response (TX the latter- rx packet then send response), start xmodem routine */
MODEM_ERRORS ymodem_tx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags)
{
	return NOT_IMPLEMENTED;
}

MODEM_ERRORS ymodem_rx(modem_file_t ** f_ptr, serial_handle_t device, uint8_t flags)
{
	return NOT_IMPLEMENTED;
}
