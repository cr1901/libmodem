#include "timewrap.h"
#include "serial.h"
#include "files.h"
#include "modem.h"

/* #include "config.h"
#include <stdlib.h> */

/* #include <stdio.h> */
#include <stddef.h> /* For size_t, NULL */
#include <time.h>

static void clear_buffer(unsigned char * buf, size_t bufsiz);
/* const doesn't work due to some weird rules in C... */
static void set_packet_offsets(unsigned char ** packet_offsets, unsigned char * packet, unsigned short mode);
static void purge(serial_handle_t serial_device);
static MODEM_ERRORS wait_for_rx_ready(serial_handle_t serial_device, unsigned short flags);
static MODEM_ERRORS serial_to_modem_error(SERIAL_STATUS status);
/* static void assemble_packet(XMODEM_OFFSETS * offsets, )
static void disassemble_packet() */
/* Implement this! */




/** MODEM_RX- transmit file(s) to external equipment. **/
/* Portions of this code depend on having a consistent representation of values
for a given bit pattern between machines, which is only guaranteed for
unsigned types.
Additionally, some portions of this code rely on:
unsigned constant literal cast (particularly hex values >= 0x80), and 
unsigned overflow semantics (checksum and CRC). Therefore, this function
expects an unsigned char * buffer as input. */

MODEM_ERRORS xmodem_tx(O_channel data_out_fcn, unsigned char * tx_buffer, void * chan_state, \
	serial_handle_t serial_device, unsigned short flags)
{	
	/* Array of pointers to the six packet section offsets within the 
	 * buffer holding the packet. */
	unsigned char * offsets[6];
	char rx_code = NUL;
	MODEM_ERRORS modem_status = 0;
	SERIAL_STATUS ser_status = 0;
	/* Logic variables. */
	int eof_detected = MODEM_FALSE, using_128_blocks_in_1k = MODEM_FALSE;
	size_t bytes_read, block_size; /* Check to see if EOF was reached using bytes_read */
	int last_sent_size = 0;
	
	/* Flush the device buffer in case some characters were remaining
	 * to prevent glitches. */ 
	serial_flush(serial_device);
	if((modem_status = wait_for_rx_ready(serial_device, flags)) != MODEM_NO_ERRORS)
	{
		return modem_status;
	}
	
	/* Depending on mode set, set offsets. */	
	set_packet_offsets(offsets, tx_buffer, flags);
	clear_buffer(tx_buffer, (offsets[END] - offsets[START_CHAR]+ 1));
	
	*offsets[START_CHAR] = (flags == XMODEM_1K) ? STX : SOH;
	*offsets[BLOCK_NO] = 0x01;
	*offsets[COMP_BLOCK_NO] = 0xFE;
	block_size = (size_t) (offsets[CHKSUM_CRC] - offsets[DATA]);
	rx_code = NUL;
	
	do{
		int short_read;
		
		/* Read data from IO channel. */
		bytes_read = data_out_fcn((char *) offsets[DATA], block_size, \
			last_sent_size, chan_state);
		short_read = (bytes_read < block_size);
		
		/* If less than 1024 bytes left in XMODEM_1K, switch to
		 * an 128 byte to reduce overhead. */
		if((flags == XMODEM_1K) && !using_128_blocks_in_1k && short_read)
		{
			/* Does not alter flags byte to distinguish XMODEM_CRC
			 * and XMODEM_1K with 128-byte packets. */
			set_packet_offsets(offsets, tx_buffer, XMODEM_CRC);
			*offsets[START_CHAR] = SOH;
			using_128_blocks_in_1k = MODEM_TRUE;
			last_sent_size = block_size = (offsets[CHKSUM_CRC] - offsets[DATA]);
		}
		
		/* Pad a short packet. This also handles the case where the file
		 * ends on a packet-size boundary (write CPMEOF for entire packet). */
		if(short_read)
		{
			unsigned int count;
			
			eof_detected = MODEM_TRUE;
			for(count = 0; count < (block_size - bytes_read); count++)
			{
				*(offsets[DATA] + bytes_read + count) = CPMEOF;
			}
		}
		
		/* Generate the checksum/CRC. */
		if(flags == XMODEM)
		{
			*offsets[CHKSUM_CRC] = generate_chksum(offsets[DATA], \
					block_size);	
		}
		else /* All other protocols use CRC. */
		{
			unsigned int crc16;
			crc16 = generate_crc(offsets[DATA], block_size);
			
			/* if sizeof(int) == 2, then 0xFF00 is unsigned int.
			if sizeof(int) > 2, then 0xFF00 is int.
			However the value ALWAYS remains positive. */
			*(offsets[CHKSUM_CRC]) = ((crc16 & (0xFF00)) >> 8);
			*(offsets[CHKSUM_CRC] + 1) = ((crc16 & (0x00FF)));
		}
		
		/* Send the packet. */
		serial_snd((char *) tx_buffer, block_size, serial_device);
		serial_flush(serial_device);
		
		/* Wait for any character. */
		ser_status = serial_rcv(&rx_code, 1, 60, serial_device);
		
		/* Protocol is completely receiver-driven (will not retransmit
		automatically without receiver intervention)- bail on timeout
		or hardware error. */
		/* All four conditions are mutually exclusive. */
		if(ser_status != SERIAL_NO_ERRORS)
		{
			return serial_to_modem_error(ser_status);
                }
                else if(rx_code == ACK)
		{
			/* Increment the block number and negate the 
			complement block number in one line. */
			(* offsets[COMP_BLOCK_NO]) = ~(++(* offsets[BLOCK_NO]));
			last_sent_size = block_size;
		}
                else if(rx_code == CAN)
		{
			return SENT_CAN;
		}
		else /* if(rx_code == NAK) */
		{
			/* Garbage. Resend. */
			/* FIX! */
			last_sent_size = 0;
			/* If NAK detected on last packet, it needs to
			 * be redone! */
			eof_detected = MODEM_FALSE;
		}
	}while(!eof_detected);
	
	
	/* Wait for ACK or CAN. Is waiting for CAN really necessary? It
	could probably fit under "timeout", as the receiver has acknowledged
	all data to be sent at this point. */
	do{
		char eot_char = EOT;
		
		serial_snd(&eot_char, 1, serial_device);
		ser_status = serial_rcv(&rx_code, 1, 60, serial_device);
	}while(ser_status == SERIAL_NO_ERRORS && !(rx_code == ACK || rx_code == CAN));
	
	if(ser_status != SERIAL_NO_ERRORS)
	{
		return serial_to_modem_error(ser_status);
	}
	else
	{
		return (rx_code == ACK) ? MODEM_NO_ERRORS : SENT_CAN;
	}
}

/** MODEM_RX- receive file(s) from external equipment. **/
MODEM_ERRORS xmodem_rx(I_channel data_in_fcn, unsigned char * rx_buffer, void * chan_state, \
	serial_handle_t serial_device, unsigned short flags)
{
	/* Array of pointers to the six packet section offsets within the 
	 * buffer holding the packet. */
	unsigned char * offsets[6];
	char tx_code = NUL;
	unsigned int count, /* Loop variable. */ error_count = 0; 
	
	unsigned char expected_start_char_1 = 0, expected_start_char_2 = 0, \
			expected_block_no = 0, expected_comp_block_no = 0;
	
	/* Logic variables. */
	int eot_detected = MODEM_FALSE, using_128_blocks_in_1k = MODEM_FALSE;
	
	/* Check to see if EOF was reached using bytes_read */
	size_t bytes_written;
	MODEM_ERRORS modem_status;
	SERIAL_STATUS ser_status;
	int in_bufsiz;
	
	/* This function combines all X/Y modem functionality... any
	 * protocol-specific statements are marked in if-else
	 * statements beginning with if(flags == protocol)... */
	/* clear_buffer(rx_buffer, 1034); */
	
	/* Depending on mode set, set offsets. */	
	set_packet_offsets(offsets, rx_buffer, flags);
	
	
	/* modem_fseek(f_ptr, 0); */
	
	/* Set initial transmission code and expected response. */
	if(flags == XMODEM_1K)
	{
		expected_start_char_1 = STX;
		expected_start_char_2 = SOH;
		tx_code = ASCII_C;
	}
	else if(flags == XMODEM_CRC)
	{
		expected_start_char_1 = SOH;
		expected_start_char_2 = SOH;
		tx_code = ASCII_C;
	}
	else if(flags == XMODEM)
	{
		expected_start_char_1 = SOH;
		expected_start_char_2 = SOH;
		tx_code = NAK;
	}
	expected_block_no = 0x01;
	expected_comp_block_no = 0xFE;
	error_count = -1; /* Unsigned warning can be safely ignored. */
	
	/* Begin by sending starting byte to transmitter. */
	serial_snd(&tx_code, 1, serial_device);
	
	do{
		/* wait_for_tx_response() 
		add difftime to calculate timeout. */
		rx_buffer[0] = NUL;
		/* Wait for first character. */
		
		while(MODEM_TRUE)
		{
		/*do{ */
			error_count++; 
			if(error_count > 11)
			{
				tx_code = CAN;
				serial_snd(&tx_code, 1, serial_device);
				/* return SERIAL_ERROR if 11 errors occurred */
				return MODEM_TIMEOUT;
			}
			
			/* Fallback to XMODEM from XMODEM_CRC if conditions
			 * are met. */
			if(flags == XMODEM_CRC && error_count > 2)
			{
				flags = XMODEM;
				tx_code = NAK;
				set_packet_offsets(offsets, rx_buffer, flags);
			} 
			
			ser_status = serial_rcv((char *) rx_buffer, 1, 10, serial_device);
			
			if(rx_buffer[0] == expected_start_char_2 \
				|| rx_buffer[0] == expected_start_char_1)
			{
				break;
			} 
			else if(rx_buffer[0] == EOT)
			{
				eot_detected = MODEM_TRUE;
				break;
			}
			
			if(ser_status == SERIAL_TIMEOUT)
			{
				/* debug-printf eventually?
				printf("Timeout occurred.\n"); */
				serial_snd(&tx_code, 1, serial_device);
			}
			
			/* #ifdef USE_DEBUG_MESSAGES
				printf("Character received while waiting: %c\n", rx_buffer[0]);
				printf("Status inside initial loop: %X\n", status);
			#endif */
		}	
		/* }while((rx_buffer[0] != expected_start_char_1 \
			&& rx_buffer[0] != expected_start_char_2) \
			&& rx_buffer[0] != EOT); */
		/* Get out of the loop AS SOON AS the 
		 * correct character is detected to minimize race
		 * condition potential if using polling I/O. */
		
		/* if(rx_buffer[0] == EOT)
		{
			eot_detected = MODEM_TRUE;
		} */
		
		if(!eot_detected)
		{
			/* Check for small XMODEM-1K packet. */ 
			if(flags == XMODEM_1K)
			{
				if((rx_buffer[0] == SOH) && !using_128_blocks_in_1k)
				{
					set_packet_offsets(offsets, rx_buffer, XMODEM_CRC);
					using_128_blocks_in_1k = MODEM_TRUE;
				}
				else if((rx_buffer[0] == STX) && using_128_blocks_in_1k)
				{
					set_packet_offsets(offsets, rx_buffer, XMODEM_1K);
					using_128_blocks_in_1k = MODEM_FALSE;
				}
			}
			
			
			
			count = 1; /* First character must be preserved because all
				* offsets have been set. If not preserved,
				* all data is shifted to by one element downward. */
			ser_status = serial_rcv((char *) (rx_buffer + 1), (offsets[END] - offsets[BLOCK_NO]), 1, serial_device);
			modem_status = serial_to_modem_error(ser_status);
			
			/* Check for small XMODEM-1K packet. */ 
			/* if(flags == XMODEM_1K)
			{
				if((rx_buffer[0] == SOH) && !using_128_blocks_in_1k)
				{
					set_packet_offsets(offsets, rx_buffer, XMODEM_CRC);
					using_128_blocks_in_1k = MODEM_TRUE;
					status = NO_ERROR;
				}
				else if((rx_buffer[0] == STX) && using_128_blocks_in_1k)
				{
					set_packet_offsets(offsets, rx_buffer, XMODEM_1K);
					using_128_blocks_in_1k = MODEM_FALSE;
					status = NO_ERROR;
				}
			} */
			
			/* do{
				status = serial_rcv(rx_buffer + count, 1, 1, serial_device);
				count++;
				#ifdef USE_DEBUG_MESSAGES
					printf("Getting byte %u...\r", count);
					fflush(stdout);
				#endif
			}while((count <= (offsets[END] - offsets[BLOCK_NO])) && status == NO_ERRORS); */
			
			
			/* Check for common errors. */
			if(ser_status != SERIAL_NO_ERRORS) /* For now, only TIMEOUT is expected here. */
			{
				/* If error occurs cause RX timeout before sending status code,
				 * since transmitter flushes UART buffer after sending packet. */
				purge(serial_device);
			}			
				/* If expected block numbers weren't received (either current or
				 * previous packet number) synchronicity was lost- unrecoverable. */
			else if(((*offsets[COMP_BLOCK_NO] != expected_comp_block_no) && \
				(*offsets[BLOCK_NO] != expected_block_no))) /* || \
				((*offsets[COMP_BLOCK_NO] != expected_comp_block_no + 1) && \
				(*offsets[BLOCK_NO] != expected_block_no - 1))) */
			{
				/* Look up YMODEM.txt to determine how the receiver
				handles receiving the previous packet again. */
				
				/* printf("Packet Mismatch\n", chksum); */
				modem_status = PACKET_MISMATCH;
			}
			
			/* This ridiculous else if statement can be read as:
			 * "If using XMODEM and the checksum is bad, set bad
			 * checksum error." */
			else if((flags == XMODEM) && \
				generate_chksum(offsets[DATA], \
				(size_t) ((offsets[CHKSUM_CRC]) - offsets[DATA])) \
				!= *offsets[CHKSUM_CRC])
			{
				modem_status = BAD_CRC_CHKSUM;	
			}
			
			/* Ditto, except for CRC errors. */
			else if((flags != XMODEM) && \
				generate_crc(offsets[DATA], (size_t) (offsets[END] - offsets[DATA])) != 0)
			{
				modem_status = BAD_CRC_CHKSUM;
			}
			
			/* printf("Current Status Code: %X\n", status); */	
			switch(modem_status)
			{
				case BAD_CRC_CHKSUM:
				case MODEM_TIMEOUT:
					/* modem_fseek(f_ptr, current_offset); */
					tx_code = NAK;
					serial_snd(&tx_code, 1, serial_device);
					break;
				case PACKET_MISMATCH:
					tx_code = CAN;
					serial_snd(&tx_code, 1, serial_device);
					return PACKET_MISMATCH;
					break;				
				case MODEM_NO_ERRORS:			
					expected_comp_block_no = ~(++expected_block_no);
					/* bytes_written = modem_fwrite(offsets[DATA], (size_t) (offsets[CHKSUM_CRC] - offsets[DATA]), f_ptr); */
					bytes_written = data_in_fcn((char *) offsets[DATA], (size_t) (offsets[CHKSUM_CRC] - offsets[DATA]), eot_detected, chan_state);
					/* printf("Number bytes written: %d\n", bytes_written); */
					if(bytes_written < (size_t) (offsets[CHKSUM_CRC] - offsets[DATA]))
					{
						tx_code = CAN;
						serial_snd(&tx_code, 1, serial_device);
						return FILE_ERROR;
					}
					else
					{
						/* current_offset += (offsets[CHKSUM_CRC] - offsets[DATA]); */
						error_count = -1; 
						/* Reset error count if entire packet successfully
						 * sent (all errors retried 10 times). */
						tx_code = ACK;
						serial_snd(&tx_code, 1, serial_device);
					}
					break;
				default:
					tx_code = CAN;
					serial_snd(&tx_code, 1, serial_device);
					return UNDEFINED_ERROR;
			}
			/* #ifdef USE_DEBUG_MESSAGES
				printf("Status receiving remainder of packet: %X\n", status);
			#endif
			
			#ifdef DISPLAY_MESSAGES */
				/* printf("%lu bytes received...\r", current_offset); */
				/* This will display the buffer each line, instead
				of each block. */
				/* fflush(stdout); */
			/* #endif */	
			tx_code = NAK; /* Make sure the receiver is ready to send
							* NAK in case of timeout after looping. */
		} /* End if(!eot_detected) */
	}while(!eot_detected);
	
	tx_code = ACK; 
	ser_status = serial_snd(&tx_code, 1, serial_device);
	/* Discard for now, but perhaps add an error code for failure at end? */
	
	return MODEM_NO_ERRORS;
}


/* void assemble_xmodem_packet(unsigned char * data, unsigned char block_no, size_t size, \
	unsigned short flags)
{
		
	
	
} */


unsigned char generate_chksum(unsigned char * data, size_t size)
{
	unsigned char chksum = 0;
	register unsigned int count;
	for(count = 0; count < size; count++)
	{
		chksum += *(data + count);
	}
	return chksum;
}

/* Use CRC-16-CCITT. XMODEM sends MSB first, so initial
 * CRC value should be 0. */
unsigned short generate_crc(unsigned char * data, size_t size)
{
	static const unsigned int crc_poly = 0x1021;
	unsigned int crc = 0x0000;
	
	register unsigned int octet_count;
	register unsigned char bit_count;
	for(octet_count = 0; octet_count < size; octet_count++)
	{
		crc = (crc ^ (unsigned int) (data[octet_count] & (0xFF)) << 8);
		for(bit_count = 1; bit_count <= 8; bit_count++)
		{
			/* Taken from ymodem.txt/wikipedia... I don't have the patience
			 * to develop my own CRC algorithm currently. */
			if(crc & 0x8000)
			{
				crc = (crc << 1) ^ crc_poly;
			}
			else
			{
				crc <<= 1;
			}	
		}
	}
	return crc;
}


/** Static functions. **/
/* Yes, memset would work, but do not depend on existence of string.h */
static void clear_buffer(unsigned char * buf, size_t bufsiz)
{
	size_t count;
	for(count = 0; count < bufsiz; count++)
	{
		buf[count] = NUL;
	}
}


static void set_packet_offsets(unsigned char ** packet_offsets, unsigned char * packet, unsigned short mode)
{
	/* int START_CHAR = 0; Not causing error- why? */
	packet_offsets[START_CHAR] = packet;
	packet_offsets[BLOCK_NO] = packet + 1;
	packet_offsets[COMP_BLOCK_NO] = packet + 2;
	packet_offsets[DATA] = packet + 3;
	switch(mode & 0x07)
	{
		/* Add YMODEM, etc. later */
		case YMODEM_1K:
		case XMODEM_1K:
			packet_offsets[CHKSUM_CRC] = packet + 1027;
			packet_offsets[END] = packet + 1029;
			break;
		
		case YMODEM:
		case XMODEM_CRC:
			packet_offsets[CHKSUM_CRC] = packet + 131;
			packet_offsets[END] = packet + 133;
			break;
		
		case XMODEM:
			packet_offsets[CHKSUM_CRC] = packet + 131;
			packet_offsets[END] = packet + 132;
			break;
			
		case YMODEM_G:
			/* Implement later. */
			break;
	}
}

static void purge(serial_handle_t serial_dev)
{
	SERIAL_STATUS timeout_status = SERIAL_NO_ERRORS;
	char dummy_byte;
	do{
		timeout_status = serial_rcv(&dummy_byte, 1, 1, serial_dev);
	}while(timeout_status != SERIAL_TIMEOUT);
}

static MODEM_ERRORS wait_for_rx_ready(serial_handle_t serial_device, unsigned short flags)
{
	time_t start,end;
	unsigned int elapsed_time;
	SERIAL_STATUS ser_status = SERIAL_NO_ERRORS;
	int expected_rx_detected = MODEM_FALSE;
	char rx_code = NUL;
	
	/* Wait for NAK or 'C', timeout after 1 minute. */
	time(&start);
	while(!(expected_rx_detected))
	{	
		time(&end);
		elapsed_time = modem_difftime(end, start);	
		ser_status = serial_rcv(&rx_code, 1, 60 - elapsed_time, serial_device);
		/* printf("Char detected:\t%X\n", rx_code); */
		

		/* What happens if error on setting timeouts? */
		if(ser_status != SERIAL_NO_ERRORS)
		{
			break;
		}
		/* ASCII_C is only correct for XMODEM_1K and XMODEM_CRC. */
		else if((flags == XMODEM_1K || flags == XMODEM_CRC) && rx_code == ASCII_C)
		{
			expected_rx_detected = MODEM_TRUE;
		}
		/* Else, wait for NAK. */
		else if((flags == XMODEM) && rx_code == NAK)
		{
			expected_rx_detected = MODEM_TRUE;
		}
	}
	
	return serial_to_modem_error(ser_status);
}

static int wait_for_rx_ack(serial_handle_t serial_device)
{
	char rx_code;
	
	
}


static MODEM_ERRORS serial_to_modem_error(SERIAL_STATUS status)
{
	MODEM_ERRORS equiv_status;
	switch(status)
	{
		case SERIAL_NO_ERRORS:
			equiv_status = MODEM_NO_ERRORS;
			break;
		case SERIAL_TIMEOUT:
			equiv_status = MODEM_TIMEOUT;
			break;
		case SERIAL_HW_ERROR:
			equiv_status = MODEM_HW_ERROR;
			break;
		default:
			equiv_status = UNDEFINED_ERROR;
			break;
	}
	
	return equiv_status;
}
