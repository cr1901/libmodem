#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include "serial.h"
#include "modem.h"
#include "files.h"

#define EVER ;;

void print_banner(void);
//void print_error(void);
void print_usage(void);
int16_t byte_swap(int16_t standard);

int main(int argc, char * argv[])
{
	/* Add macros for choosing sizes */
	uint8_t txrx_flags = XMODEM, rx_mode = MODEM_FALSE, \
		port_no = 1, count, device_mode;
	
	//Unused variables	
	/* uint8_t use_1k = MODEM_FALSE, malformed = MODEM_FALSE, \
		unrecognized = MODEM_FALSE, com_name[8] = {'\0'}, \
		xmodem_buffer[133] = {'\0'}; */
		
	
	/* Needs to be signed due to string literals... */
	char filename[128] = {'\0'}, * current_arg, * previous_arg;
	
	uint16_t status;
	uint32_t baud_rate = 9600;	
	
	modem_file_t * trx_file;
	serial_handle_t com_port;
	
	print_banner();
	
	/* Usage: TRXCOM [[/T | /R] [/S]] [[/X [/CRC] | /Y [/G]] [/1K [/F]]]
	[/B:b] [/P:n] FILENAME[...]*/
	
	/* Parse command line arguments. */
	if(argc < 2)
	{
		print_usage();
		return 0;
	}
	else
	{
		//FIX- this isnt' working...
		for(count = 1; count < argc; count++)
		{
			/* Last parameter is dedicated to filenames */
			if(count + 1 != argc)
			{
				current_arg = argv[count];
				previous_arg = argv[count - 1];
				if(!strcmp(current_arg, "/T"))
				{
					/* Do nothing. */
				}
				else if(!strcmp(current_arg, "/R"))
				{
					rx_mode = MODEM_TRUE;
				}
				else if(!strcmp(current_arg, "/X"))
				{
					/* Do nothing. */
				}
				else if(!strcmp(current_arg, "/Y"))
				{
					//txrx_flags = YMODEM;
					printf("YMODEM not implemented yet!- W. Jones\n");
				}
				else if(!strncmp(current_arg, "/B:", 3))
				{
					baud_rate = (uint32_t) atol(current_arg + 3);
				}
				else if(!strncmp(current_arg, "/P:", 3))
				{
					port_no = (uint8_t) atoi(current_arg + 3);
				}
				
				/* Begin else-if logic dependent on previous switches. */			
				/* If the previous switches were either '/T' or '/R'. */
				else if(!strcmp(current_arg, "/S") && (!strcmp(previous_arg, "/T") || !strcmp(previous_arg, "/R")))
				{
					/* Switch is valid. */
					device_mode = MODEM_TRUE;
				}
				/* And so on... */
				else if(!strcmp(current_arg, "/CRC") && (!strcmp(previous_arg, "/X")))
				{
					txrx_flags = XMODEM_CRC;
				}
				else if(!strcmp(current_arg, "/G") && (!strcmp(previous_arg, "/Y")))
				{
					//txrx_flags = YMODEM_G;
					printf("YMODEM-G not implemented yet!- W. Jones\n");
				}
				else if(!strcmp(current_arg, "/1K") && (!strcmp(previous_arg, "/X") || !strcmp(previous_arg, "/CRC")))
				{
					/* CRC option is implied here. */
					txrx_flags = XMODEM_1K;
				}
				else if(!strcmp(current_arg, "/1K") && (!strcmp(previous_arg, "/Y")))
				{
					//txrx_flags = YMODEM_1K;
					printf("YMODEM_1K not implemented yet!- W. Jones\n");
				}
				else if(!strcmp(current_arg, "/F") && (!strcmp(previous_arg, "/1K")))
				{
					printf("Fallback disabled- W. Jones\n");
				}
				else
				{
					printf("Error: %s- Unrecognized, malformed, or misplaced command line parameter.\n", current_arg);
					return EXIT_FAILURE;
				}							
			}
			else
			{
				strcpy(filename, argv[count]);
				/* Parse filename(s) */
			}
			

		}	
	}
	
	if(serial_init(port_no, baud_rate, &com_port) == SERIAL_ERROR)
	{
		printf("Error initializing serial port- aborting.\n");
	}
	else
	{	
		/* DOS support broken... trashes serial registers...
		while(test < 'F')
		{
			computc(test);
			sleep(1);
			test++;
		}
		
		return 0;	*/
		
		if(rx_mode == MODEM_TRUE)
		{
			trx_file = modem_fopen_write(filename);
			if(trx_file != NULL)
			{
				status = xmodem_rx(trx_file, com_port, txrx_flags);
				printf("Returned status code: %X", status);
			}
			else
			{
				printf("Error: %s cannot be written.", filename);
				return EXIT_FAILURE;
			}
		}
		else
		{
			trx_file = modem_fopen_read(filename);
			if(trx_file != NULL)
			{
				status = xmodem_tx(trx_file, com_port, txrx_flags);
				printf("Returned status code: %X", status);
			}
			else
			{
				printf("Error: %s not found.", filename);
				return EXIT_FAILURE;
			}
		}
		modem_fclose(trx_file);
	}
	
	if(serial_close(&com_port) != NO_ERRORS)
	{
		printf("\nError closing serial port!");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


void print_banner(void)
{
	printf("TRXCOM- Transit and Receive files via COM ports\n"
	"Copyright 2013 William D. Jones\n");
}

//Implement redirection to stdout (look up redirection in C).
void print_usage(void)
{
	printf("Usage: TRXCOM [[/T | /R] [/S]] [[/X [/CRC] | /Y [/G]] [/1K [/F]]]\n"
	"  [/B:b] [/P:n] FILENAME[...]\n"
	"  /T\tTransmit file(s) to external equipment (default).\n"
	"  /R\tReceive file(s) from external equipment.\n"
	"  /S\tRedirect transmission to/from device.\n"
	"  /X\tXMODEM: Use classic XMODEM protocol (default).\n"
	"  /Y\tYMODEM: Use YMODEM protocol.\n"
	"  /CRC\tXMODEM-CRC: Use CRC-16 check for XMODEM, fall back to checksum "
		"after\n\t3 failed retries.\n"
	"  /1K\tX/YMODEM-1K: Use 1K packets for XMODEM-CRC/YMODEM. Implies "
		"'/X /CRC'\n\tif used with '/X' alone. '/G' option is ignored "
		"if used with '/Y /G'.\n"
	"  /F\tX/YMODEM-1K => XMODEM-CRC/YMODEM: Fall back to 128 byte packets "
		"after\n\t3 failed retries (possibly non-standard).\n"
	"  /G\tUse YMODEM-G protocol.\n"
	"  /B:b\tBaud Rate (must be supported by both sender and receiver). "
		"Default\n\t9600 baud.\n"
	"  /P:n\tCOM port number (between 1 and 4 for DOS).\n");
}

/* int16_t byte_swap(int16_t x)
{
	uint8_t hi_byte, lo_byte;
	hi_byte = (x & 0xff00) >> 8;
	lo_byte = (x & 0xff);
	return lo_byte << 8 | hi_byte;
} */
