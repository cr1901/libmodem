#include "modem.h"
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

/* Indices to an array which point to locations within a packet.
 * Pointer arithmetic between offsets gets the number of octets (bytes)
 * to read or write for a given operation. */
typedef enum offset_names
{
	START_CHAR = 0,
	BLOCK_NO,
	COMP_BLOCK_NO,
	DATA,
	CHKSUM_CRC,
	END
}OFFSET_NAMES;


static void set_packet_offsets(OFFSET_NAMES names, uint8_t ** packet_offsets, uint8_t * packet, uint8_t mode);
static void purge(serial_handle_t serial_device);
//static void assemble_packet(XMODEM_OFFSETS * offsets, )
//static void disassemble_packet()

/* Implement this! */
//static uint16_t modem_difftime()

/** MODEM_RX- transmit file(s) to external equipment. **/
uint16_t modem_tx(modem_file_t * f_ptr, serial_handle_t serial_device, uint8_t flags)
{
	/* Array of pointers to the six packet section offsets within the 
	 * buffer holding the packet. */
	static uint8_t * offsets[6];
	
	/* Buffer for the packet to be sent. */
	static uint8_t tx_buffer[1034] = {NUL};
	static uint8_t rx_code = NUL;
	
	/* May be better practice then using a single static global enum. */
	static OFFSET_NAMES names;
	
	/* fseek(SEEK_CUR) may not be portable, as described here:
	 * http://www.cplusplus.com/reference/clibrary/cstdio/fseek/.
	 * Additionally, one is probably not going to use XMODEM to transfer
	 * 4G+ files... */
	uint32_t current_offset = 0;
	
	uint16_t count; /* Loop variable. */
	
	uint16_t bytes_written = 0, status = 0, elapsed_time = 0, \
			crc16 = 0, num_1k_xfer_failures = 0;
	
	/* Logic variables. */
	uint8_t eof_detected = MODEM_FALSE, expected_rx_detected = MODEM_FALSE, \
			using_128_blocks_in_1k = MODEM_FALSE;
	
	/* Check to see if EOF was reached using bytes_read */
	size_t bytes_read;
	time_t start,end;
	
	/* This function combines all X/Y modem functionality... any
	 * protocol-specific statements are marked in if-else
	 * statements beginning with if(flags == protocol)... */
	
	/* Flush the buffer in case some characters were remaining
	 * to prevent glitches. */ 
	serial_flush(serial_device);
	
	/* Wait for NAK or 'C', timeout after 1 minute. */
	time(&start);
	while(status != TIMEOUT && !(expected_rx_detected))
	{	
		time(&end);
		elapsed_time = (uint16_t) modem_difftime(end, start);	
		status = serial_rcv(&rx_code, 1, 60 - elapsed_time, serial_device);
		//printf("Char detected:\t%X\n", rx_code);
		

		/* What happens if error on setting timeouts? */
		if(status == TIMEOUT || status == SERIAL_ERROR)
		{
			return status;
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
		else
		{
			/* Do nothing. */
		}
	}
	

	
	
	/* Depending on mode set, set offsets. */	
	set_packet_offsets(names, offsets, tx_buffer, flags);
	/* set_packet_offsets(names, &offsets, tx_buffer, flags); Why wasn't this error caught? */
	modem_fseek(f_ptr, 0);
	
	
	if(flags == XMODEM_1K)
	{
		*offsets[START_CHAR] = STX;
	}
	else
	{
		*offsets[START_CHAR] = SOH;
	}
	*offsets[BLOCK_NO] = 0x01;
	*offsets[COMP_BLOCK_NO] = 0xFE;
	
	rx_code = NUL;
	do{
		/* Fallback removed to make the protocol
		 * completely receiver driven.- W. Jones. */

		/* if(flags == (FALLBACK | XMODEM_1K) && num_xfer_1k_failures > 3)
		{
			flags &= XMODEM_CRC;
			set_packet_offsets(names, &offsets, tx_buffer, flags);
			*offsets[START_CHAR] = SOH;
		} */
		
		/* If EOF was previously detected using XMODEM-1K, make sure 
		 * to switch back to using 1024k blocks in case new data came 
		 * in before processing new packet. 
		 * Without multi-threaded applications or opening a file
		 * for both reading and writing (which requires buffer flushing),
		 * this is likely an unnecessary check.
		 * Commented out for now.- W. Jones */
		
		/* else if((flags == XMODEM_1K) && using_128_blocks_in_1k)
		{
			set_packet_offsets(names, &offsets, tx_buffer, XMODEM_1K);
			*offsets[START_CHAR] = STX;
			using_128_blocks_in_1k = MODEM_FALSE;
		} */
		
		bytes_read = modem_fread(offsets[DATA], (size_t) (offsets[CHKSUM_CRC] - offsets[DATA]), f_ptr);	
		 
		/* If less than 1024 bytes left in XMODEM_1K, switch to
		 * an 128 byte to reduce overhead. */
		if((flags == XMODEM_1K) && (bytes_written < offsets[CHKSUM_CRC] - offsets[DATA]) && !using_128_blocks_in_1k)
		{
			/* Does not alter flags byte to distinguish XMODEM_CRC
			 * and XMODEM_1K with 128-byte packets. */
			set_packet_offsets(names, offsets, tx_buffer, XMODEM_CRC);
			*offsets[START_CHAR] = SOH;
			using_128_blocks_in_1k = MODEM_TRUE;
		}
		
		/* If not all bytes were READ from file for current packet, 
		 * check to see if EOF was reached. If not, error occurred 
		 * and abort. This also handles the case where the file
		 * ends on a packet-size boundary (write CPMEOF for entire packet). */
		if(bytes_read < offsets[CHKSUM_CRC] - offsets[DATA])
		{
			if(modem_feof(f_ptr))
			{
				eof_detected = MODEM_TRUE;
				for(count = 0; count < ((offsets[CHKSUM_CRC] - offsets[DATA]) - bytes_written); count++)
				{
					*(offsets[DATA] + bytes_written + count) = CPMEOF;
				}
			}
			else
			{
				//printf("File read error.\n");
				return FILE_ERROR;
			}
		}
		else
		{
			/* In the case the EOF was not detected */
		}
		//else if(set_packet_offsets(names, &offsets, tx_buffer, flags);


		switch(flags)
		{
			case XMODEM:
			/* Typecasting needed? */
				*offsets[CHKSUM_CRC] = generate_chksum(offsets[DATA], (uint16_t) (offsets[CHKSUM_CRC] - offsets[DATA]));
				break;
			/* All other protocols use CRC. */	
			default:
				crc16 = generate_crc(offsets[DATA], (uint16_t) (offsets[CHKSUM_CRC] - offsets[DATA]));
				#ifdef LITTLE_ENDIAN
					/* Recall that CRC is 2 bytes, and so it needs to
					 * be stored in the array of bytes. */
					*(offsets[CHKSUM_CRC]) = ((crc16 & (0xFF00)) >> 8);
					*(offsets[CHKSUM_CRC] + 1) = ((crc16 & (0x00FF)));
				#else
					*(offsets[CHKSUM_CRC]) = (crc16 & (0xFF00));
					*(offsets[CHKSUM_CRC] + 1) = (crc16 & (0x00FF));	
				#endif
		}
		
		//printf("Packet_size: %d\n", (offsets[END] - offsets[START_CHAR]));
		serial_snd(tx_buffer, (offsets[END] - offsets[START_CHAR]), serial_device);
		serial_flush(serial_device);
		//flush the buffer
		
		/* Wait for any character. */
		do{
			status = serial_rcv(&rx_code, 1, 60, serial_device);
			//printf("Char detected:\t%X\n", rx_code);

			if(rx_code == NAK) //FIX!
			{
				status = SENT_NAK;
				//printf("NAK detected\n");
				modem_fseek(f_ptr, current_offset);
				/* If NAK detected on last packet, it needs to
				 * be redone! */
				eof_detected = MODEM_FALSE;
			}
			else if(rx_code == CAN)
			{
				return EXIT_FAILURE;
			}
			else if(rx_code == ACK)
			{
				/* Increment the block number and negate the complement block number in one line. */
				(* offsets[COMP_BLOCK_NO]) = ~(++(* offsets[BLOCK_NO]));
				current_offset += (offsets[END] - offsets[START_CHAR]);
			}
		}while(!(rx_code == ACK || rx_code == NAK));
	}while(!eof_detected);
	
	
	/* Wait for ACK or CAN. */
	tx_buffer[START_CHAR] = EOT;
	rx_code = NUL; /* Reset the rx_code. */
	while(!(rx_code == ACK || rx_code == CAN)) 
	{	
		serial_snd(tx_buffer, 1, serial_device);
		status = serial_rcv(&rx_code, 1, 60, serial_device);
	}
	
	return rx_code == ACK ? EXIT_SUCCESS : EXIT_FAILURE;
}

/** MODEM_RX- receive file(s) from external equipment. **/
uint16_t modem_rx(modem_file_t * f_ptr, serial_handle_t serial_device, uint8_t flags)
{
	/* Array of pointers to the six packet section offsets within the 
	 * buffer holding the packet. */
	static uint8_t * offsets[6];
	
	/* Buffer for the packet to be received. */
	static uint8_t rx_buffer[1034] = {NUL};
	static uint8_t tx_code = NUL;
	
	/* May be better practice then using a single static global enum. */
	static OFFSET_NAMES names;
	
	/* fseek(SEEK_CUR) may not be portable, as described here:
	 * http://www.cplusplus.com/reference/clibrary/cstdio/fseek/.
	 * Additionally, one is probably not going to use XMODEM to transfer
	 * 4G+ files... */
	uint32_t current_offset = 0;
	
	uint16_t count; /* Loop variable. */
	
	uint16_t status = 0, elapsed_time = 0, \
			crc16 = 0, error_count = 0, chksum = 0;
	
	uint8_t expected_start_char_1 = 0, expected_start_char_2 = 0, \
			expected_block_no = 0, expected_comp_block_no = 0;
	
	/* Logic variables. */
	uint8_t eot_detected = MODEM_FALSE, expected_rx_detected = MODEM_FALSE, \
			using_128_blocks_in_1k = MODEM_FALSE;
	
	/* Check to see if EOF was reached using bytes_read */
	size_t bytes_written;
	//time_t start,end;
	
	/* This function combines all X/Y modem functionality... any
	 * protocol-specific statements are marked in if-else
	 * statements beginning with if(flags == protocol)... */
	
	
	/* Depending on mode set, set offsets. */	
	set_packet_offsets(names, offsets, rx_buffer, flags);
	modem_fseek(f_ptr, 0);
	
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
		rx_buffer[0] = NUL;
		/* Wait for first character. */
		
		do{
			error_count++; 
			if(error_count > 11)
			{
				tx_code = CAN;
				serial_snd(&tx_code, 1, serial_device);
				return TIMEOUT;
			}
			
			/* Fallback to XMODEM from XMODEM_CRC if conditions
			 * are met. */
			if(error_count > 2 && flags == XMODEM_CRC)
			{
				flags = XMODEM;
				tx_code = NAK;
				set_packet_offsets(names, offsets, rx_buffer, flags);
			} 
			
			status = serial_rcv(rx_buffer, 1, 10, serial_device);
			if(status == TIMEOUT)
			{
				//printf("Timeout occurred.\n");
				serial_snd(&tx_code, 1, serial_device);
			}
			
			printf("Character received while waiting: %c\n", rx_buffer[0]);
			//printf("Status inside initial loop: %X\n", status);
		}while((rx_buffer[0] != expected_start_char_1 \
			&& rx_buffer[0] != expected_start_char_2) \
			&& rx_buffer[0] != EOT);
		/* Get out of the loop AS SOON AS the 
		 * correct character is detected to minimize race
		 * condition potential if using polling I/O. */
		
		if(rx_buffer[0] == EOT)
		{
			eot_detected = MODEM_TRUE;
		}
		
		if(!eot_detected)
		{
			/* Check for small XMODEM-1K packet. */ 
			if(flags == XMODEM_1K)
			{
				if((rx_buffer[0] == SOH) && !using_128_blocks_in_1k)
				{
					set_packet_offsets(names, offsets, rx_buffer, XMODEM_CRC);
					using_128_blocks_in_1k = MODEM_TRUE;
				}
				else if((rx_buffer[0] == STX) && using_128_blocks_in_1k)
				{
					set_packet_offsets(names, offsets, rx_buffer, XMODEM_1K);
					using_128_blocks_in_1k = MODEM_FALSE;
				}
			}
			 
			count = 1; /* First character must be preserved because all
						* offsets have been set. If not preserved,
						* all data is shifted to by one element downward. */
			do{
				status = serial_rcv(rx_buffer + count, 1, 1, serial_device);
				count++;
			}while((count <= (offsets[END] - offsets[BLOCK_NO])) && status == NO_ERRORS);
			
			
			/* Check for common errors. */
			if(status != NO_ERRORS) /* For now, only TIMEOUT is expected here. */
			{
				/* If error occurs cause RX timeout before sending status code,
				 * since transmitter flushes UART buffer after sending packet. */
				purge(serial_device);
			}			
				/* If expected block numbers weren't received (either current or
				 * previous packet number) synchronicity was lost- unrecoverable. */
			else if((*offsets[COMP_BLOCK_NO] != expected_comp_block_no) && \
				(*offsets[BLOCK_NO] != expected_block_no))
			{
				//printf("Packet Mismatch\n", chksum);
				status = PACKET_MISMATCH;
			}
			
			/* This ridiculous else if statement can be read as:
			 * "If using XMODEM and the checksum is bad, set bad
			 * checksum error." */
			else if((flags == XMODEM) && \
				generate_chksum(offsets[DATA], (uint16_t) ((offsets[CHKSUM_CRC]) - offsets[DATA])) != *offsets[CHKSUM_CRC])
			{
				status = BAD_CRC_CHKSUM;	
			}
			
			/* Ditto, except for CRC errors. */
			else if((flags != XMODEM) && \
				generate_crc(offsets[DATA], (uint16_t) (offsets[END] - offsets[DATA])) != 0)
			{
				status = BAD_CRC_CHKSUM;
			}
			
			//printf("Current Status Code: %X\n", status);	
			switch(status)
			{
				case BAD_CRC_CHKSUM:
				case TIMEOUT:
					modem_fseek(f_ptr, current_offset);
					tx_code = NAK;
					serial_snd(&tx_code, 1, serial_device);
					break;
				case PACKET_MISMATCH:
					tx_code = CAN;
					serial_snd(&tx_code, 1, serial_device);
					return PACKET_MISMATCH;
					break;				
				case NO_ERRORS:			
					expected_comp_block_no = ~(++expected_block_no);
					bytes_written = modem_fwrite(offsets[DATA], (size_t) (offsets[CHKSUM_CRC] - offsets[DATA]), f_ptr);
					//printf("Number bytes written: %d\n", bytes_written);
					if(bytes_written < offsets[CHKSUM_CRC] - offsets[DATA])
					{
						tx_code = CAN;
						serial_snd(&tx_code, 1, serial_device);
						return FILE_ERROR;
					}
					else
					{
						current_offset += (offsets[END] - offsets[START_CHAR]);
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
			tx_code = NAK; /* Make sure the receiver is ready to send
							* NAK in case of timeout after looping. */
		} /* End if(!eot_detected) */
	}while(!eot_detected);
	
	tx_code = ACK; 
	status = serial_snd(&tx_code, 1, serial_device);
	
	return EXIT_SUCCESS;
}


uint8_t generate_chksum(uint8_t * data, uint16_t size)
{
	uint8_t chksum = 0;
	register uint16_t count;
	for(count = 0; count < size; count++)
	{
		chksum += *(data + count);
	}
	return chksum;
}

/* Use CRC-16-CCITT. XMODEM sends MSB first, so initial
 * CRC value should be 0. */
uint16_t generate_crc(uint8_t * data, uint16_t size)
{
	static const uint16_t crc_poly = 0x1021;
	uint16_t crc = 0x0000;
	
	register uint16_t octet_count;
	register uint8_t  bit_count;
	for(octet_count = 0; octet_count < size; octet_count++)
	{
		crc = (crc ^ (uint16_t) (data[octet_count] & (0xFF)) << 8);
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
static void set_packet_offsets(OFFSET_NAMES names, uint8_t ** packet_offsets, uint8_t * packet, uint8_t mode)
{
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
	uint16_t timeout_status = NO_ERRORS;
	uint8_t dummy_byte;
	do{
		timeout_status = serial_rcv(&dummy_byte, 1, 1, serial_dev);
	}while(timeout_status != TIMEOUT);
}
