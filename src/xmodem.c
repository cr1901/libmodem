#include "serial.h"
#include "modem.h"

#include <stddef.h> /* For size_t, NULL */

static void pad_buffer(unsigned char * buf, size_t bufsiz, unsigned char val);
/* const doesn't work due to some weird rules in C... */
/* static void set_packet_offsets(unsigned char ** packet_offsets, unsigned char * packet, unsigned short mode); */
static void purge(serial_handle_t serial_device);
static modem_errors_t wait_for_rx_ready(serial_handle_t serial_device, xmodem_xfer_mode_t flags);
static modem_errors_t wait_for_tx_response(serial_handle_t serial_device, xmodem_xfer_mode_t flags);
static modem_errors_t serial_to_modem_error(serial_status_t status);
static offset_names_t get_checksum_offset(unsigned short flags);


/* Portions of this code depend on having a consistent representation of values
for a given bit pattern between machines, which is only guaranteed for
unsigned types.
Additionally, some portions of this code rely on:
unsigned constant literal cast (particularly hex values >= 0x80), and
unsigned overflow semantics (checksum and CRC). Therefore, this function
expects an unsigned char * buffer as input. */
modem_errors_t xmodem_tx(output_channel_t data_out_fcn, unsigned char * tx_buffer, void * chan_state, \
	serial_handle_t serial_device, xmodem_xfer_mode_t flags)
{
	char rx_code = NUL;
	modem_errors_t modem_status = 0;
	serial_status_t ser_status = 0;
	offset_names_t chksum_offset;
	/* Logic variables. */
	int eof_detected = 0;
	size_t block_size, packet_size; /* Check to see if EOF was reached using bytes_read */
	int last_sent_size = 0;

	/* Flush the device buffer in case some characters were remaining
	to prevent glitches. */
	serial_flush(serial_device);
	if((modem_status = wait_for_rx_ready(serial_device, flags)) != \
		MODEM_NO_ERRORS)
	{
		return modem_status;
	}

	tx_buffer[START_CHAR] = (flags == XMODEM_1K) ? STX : SOH;
	tx_buffer[BLOCK_NO] = 0x01;
	tx_buffer[COMP_BLOCK_NO] = 0xFE;
	block_size = (flags == XMODEM_1K) ? 1024 : 128;
	chksum_offset = (flags == XMODEM_1K) ? X1K_CRC : CHKSUM_CRC;
	packet_size = (flags == XMODEM) ? chksum_offset + 1 : chksum_offset + 2;

	do{
		int bytes_read;

		/* Read data from IO channel. */
		if((bytes_read = data_out_fcn((char *) &tx_buffer[DATA], block_size, \
			last_sent_size, chan_state)) < 0)
		{
			return CHANNEL_ERROR;
		}

		/* If less than 1024 bytes left in XMODEM_1K, switch to 128 byte
		to reduce overhead. */
		if((flags == XMODEM_1K) && ((size_t) bytes_read < block_size))
		{
			tx_buffer[START_CHAR] = SOH;
			chksum_offset = CHKSUM_CRC;
			block_size = 128;
			packet_size = chksum_offset + 2;
			flags = XMODEM_CRC;
		}

		/* Pad a short packet. This also handles the case where the file
		ends on a packet-size boundary (write CPMEOF for entire packet). */
		if((size_t) bytes_read < block_size)
		{
			eof_detected = 1;
			pad_buffer(&tx_buffer[DATA + bytes_read], block_size - bytes_read, CPMEOF);
		}

		/* Generate the checksum/CRC. */
		if(flags == XMODEM)
		{
			tx_buffer[chksum_offset] = generate_chksum(&tx_buffer[DATA], \
					block_size);
		}
		else /* All other protocols use CRC. */
		{
			unsigned int crc16 = generate_crc(&tx_buffer[DATA], block_size);

			/* if sizeof(int) == 2, then 0xFF00 is unsigned int.
			if sizeof(int) > 2, then 0xFF00 is int.
			However the value ALWAYS remains positive. */
			tx_buffer[chksum_offset] = ((crc16 & (0xFF00)) >> 8);
			tx_buffer[chksum_offset + 1] = ((crc16 & (0x00FF)));
		}

		/* Send the packet. Wait for any character. Protocol is
		completely receiver-driven (will not retransmit automatically
		without receiver intervention)- bail on timeout or hardware error. */
		serial_snd((char *) tx_buffer, packet_size, serial_device);
		serial_flush(serial_device);
		if((ser_status = serial_rcv(&rx_code, 1, 60, NULL, serial_device)) != \
			SERIAL_NO_ERRORS)
		{
			return serial_to_modem_error(ser_status);
		}

		/* Interpret the response. */
		if(rx_code == ACK)
		{
			/* Increment the block number and negate the
			complement block number in one line. */
			tx_buffer[COMP_BLOCK_NO] = ~(++tx_buffer[BLOCK_NO]);
			last_sent_size = block_size;
		}
		else if(rx_code == NAK)
		{
			last_sent_size = 0; /* Garbage. Resend. */
			eof_detected = 0; /* If NAK detected on
			last packet, it needs to be redone! */
		}
		else /* if(rx_code == CAN) */
		{
			return SENT_CAN;
		}
	}while(!eof_detected);

	/* Wait for ACK or CAN. Is waiting for CAN really necessary? It
	could probably fit under "timeout", as the receiver has acknowledged
	all data to be sent at this point. */
	do{
		char eot_char = EOT;

		serial_snd(&eot_char, 1, serial_device);
		if((ser_status = serial_rcv(&rx_code, 1, 60, NULL, serial_device)) != \
			SERIAL_NO_ERRORS)
		{
			return serial_to_modem_error(ser_status);
		}
	}while(!(rx_code == ACK || rx_code == CAN));

	return (rx_code == ACK) ? MODEM_NO_ERRORS : SENT_CAN;
}

/* Potential reimplementation of xmodem_rx. */
modem_errors_t xmodem_rx1(input_channel_t data_in_fcn, unsigned char * rx_buffer, void * chan_state, \
	serial_handle_t serial_device, unsigned short flags)
{
	char tx_code;
	unsigned int error_count = 0;
	unsigned char expected_start_char, expected_block_no, expected_comp_block_no;
	int eot_detected = 0;
	offset_names_t chksum_offset;
	serial_status_t ser_status;


	tx_code = (flags == XMODEM) ? NAK : ASCII_C;
	expected_start_char = (flags == XMODEM_1K) ? STX : SOH;
	expected_block_no = 0x01;
	expected_comp_block_no = 0xFE;
	chksum_offset = (flags == XMODEM_1K) ? X1K_CRC : CHKSUM_CRC;
	serial_snd(&tx_code, 1, serial_device);

	do{
		modem_errors_t modem_status;

		if((modem_status = wait_for_tx_response(serial_device, flags)) != MODEM_NO_ERRORS)
		{
			return modem_status;
		}

	}while(!eot_detected);

	tx_code = ACK;
	ser_status = serial_snd(&tx_code, 1, serial_device);
	/* Discard for now, but perhaps add an error code for failure at end? */

	return MODEM_NO_ERRORS;
}


modem_errors_t xmodem_rx(input_channel_t data_in_fcn, unsigned char * rx_buffer, void * chan_state, \
	serial_handle_t serial_device, xmodem_xfer_mode_t flags)
{
	/* Array of pointers to the six packet section offsets within the
	buffer holding the packet. */
	char tx_code = NUL;
	unsigned int error_count = 0;
	unsigned char expected_start_char_1 = 0, expected_start_char_2 = 0, \
			expected_block_no = 0, expected_comp_block_no = 0;
	/* Logic variables. */
	int eot_detected = 0, using_128_blocks_in_1k = 0;
	size_t bytes_written;
	modem_errors_t modem_status;
	serial_status_t ser_status;
	offset_names_t chksum_offset, packet_end;
	/* int in_bufsiz; */


	/* Depending on mode set, set offsets. */
	chksum_offset = (flags == XMODEM_1K) ? X1K_CRC : CHKSUM_CRC;
	/* clear_buffer(rx_buffer, 1034); */

	/* Set initial transmission code and expected response. */
	if(flags == XMODEM_1K)
	{
		expected_start_char_1 = STX;
		expected_start_char_2 = SOH;
		tx_code = ASCII_C;
		chksum_offset = X1K_CRC;
		packet_end = X1K_END;
	}
	else if(flags == XMODEM_CRC)
	{
		expected_start_char_1 = SOH;
		expected_start_char_2 = SOH;
		tx_code = ASCII_C;
		chksum_offset = CHKSUM_CRC;
		packet_end = CRC_END;
	}
	else /* if(flags == XMODEM) */
	{
		expected_start_char_1 = SOH;
		expected_start_char_2 = SOH;
		tx_code = NAK;
		chksum_offset = CHKSUM_CRC;
		packet_end = CHKSUM_END;
	}
	expected_block_no = 0x01;
	expected_comp_block_no = 0xFE;
	error_count = -1; /* Unsigned warning can be safely ignored. */

	/* Begin by sending starting byte to transmitter. */
	serial_snd(&tx_code, 1, serial_device);

	do{
		/* wait_for_tx_response()
		add difftime to calculate timeout. */
		size_t expected_size, data_size, data_plus_crc_size;
		rx_buffer[0] = NUL;
		/* Wait for first character. */

		while(1)
		{
			error_count++;
			if(error_count > 11)
			{
				tx_code = CAN;
				serial_snd(&tx_code, 1, serial_device);
				/* return SERIAL_ERROR if 11 errors occurred */
				return MODEM_TIMEOUT;
			}

			/* Fallback to XMODEM from XMODEM_CRC if conditions
			are met. */
			if(flags == XMODEM_CRC && error_count > 2)
			{
				flags = XMODEM;
				tx_code = NAK;
				packet_end = CHKSUM_END;
			}

			ser_status = serial_rcv((char *) rx_buffer, 1, 10, NULL, serial_device);

			if(rx_buffer[0] == expected_start_char_2 \
				|| rx_buffer[0] == expected_start_char_1)
			{
				break;
			}
			else if(rx_buffer[0] == EOT)
			{
				eot_detected = 1;
				break;
			}

			if(ser_status == SERIAL_TIMEOUT)
			{
				serial_snd(&tx_code, 1, serial_device);
			}
		}

		if(!eot_detected)
		{
			/* Check for small XMODEM-1K packet. */
			if(flags == XMODEM_1K)
			{
				if((rx_buffer[0] == SOH) && !using_128_blocks_in_1k)
				{
					/* set_packet_offsets(offsets, rx_buffer, XMODEM_CRC); */
					chksum_offset = CHKSUM_CRC;
					packet_end = CRC_END;
					using_128_blocks_in_1k = 1;
				}
				else if((rx_buffer[0] == STX) && using_128_blocks_in_1k)
				{
					/* set_packet_offsets(offsets, rx_buffer, XMODEM_1K); */
					chksum_offset = X1K_CRC;
					packet_end = X1K_END;
					using_128_blocks_in_1k = 0;
				}
			}

			/* These are guaranteed to be positive- see enum offset_names_t */
			expected_size = packet_end - BLOCK_NO;
			data_size = chksum_offset - DATA;
			data_plus_crc_size = packet_end - DATA;

			ser_status = serial_rcv((char *) (rx_buffer + 1), \
				expected_size, 1, NULL, serial_device);
			modem_status = serial_to_modem_error(ser_status);

			/* Check for common errors. */
			if(ser_status != SERIAL_NO_ERRORS) /* For now, only TIMEOUT is expected here. */
			{
				/* If error occurs cause RX timeout before sending status code,
				since transmitter flushes UART buffer after sending packet. */
				purge(serial_device);
			}
			/* If expected block numbers weren't received (either current or
			previous packet number) synchronicity was lost- unrecoverable. */
			else if(((rx_buffer[COMP_BLOCK_NO] != expected_comp_block_no) && \
				(rx_buffer[BLOCK_NO] != expected_block_no))) /* || \
				((*offsets[COMP_BLOCK_NO] != expected_comp_block_no + 1) && \
				(*offsets[BLOCK_NO] != expected_block_no - 1))) */
			{
				char tx_code = CAN;
				/* Look up YMODEM.txt to determine how the receiver
				handles receiving the previous packet again. */

				serial_snd(&tx_code, 1, serial_device);
				return PACKET_MISMATCH;
			}

			/* This ridiculous else if statement can be read as:
			"If using XMODEM and the checksum is bad, set bad
			checksum error." */
			else if((flags == XMODEM) && \
				generate_chksum(&rx_buffer[DATA], data_size) \
				!= rx_buffer[CHKSUM_CRC])
			{
				modem_status = BAD_CRC_CHKSUM;
			}

			/* Ditto, except for CRC errors. */
			else if((flags != XMODEM) && \
				generate_crc(&rx_buffer[DATA], data_plus_crc_size) != 0)
			{
				modem_status = BAD_CRC_CHKSUM;
			}

			switch(modem_status)
			{
				case BAD_CRC_CHKSUM:
				case MODEM_TIMEOUT:
					tx_code = NAK;
					serial_snd(&tx_code, 1, serial_device);
					break;
				case MODEM_NO_ERRORS:
					expected_comp_block_no = ~(++expected_block_no);
					bytes_written = data_in_fcn((char *) &rx_buffer[DATA], data_size, eot_detected, chan_state);
					if(bytes_written < data_size)
					{
						tx_code = CAN;
						serial_snd(&tx_code, 1, serial_device);
						return CHANNEL_ERROR;
					}
					else
					{
						error_count = -1;
						/* Reset error count if entire packet successfully
						sent (all errors retried 10 times). */
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
						   NAK in case of timeout after looping. */
		} /* End if(!eot_detected) */
	}while(!eot_detected);

	tx_code = ACK;
	ser_status = serial_snd(&tx_code, 1, serial_device);
	/* Discard for now, but perhaps add an error code for failure at end? */

	return MODEM_NO_ERRORS;
}

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
CRC value should be 0. */
unsigned short generate_crc(unsigned char * data, size_t size)
{
	const unsigned int crc_poly = 0x1021;
	unsigned int crc = 0x0000;

	unsigned int octet_count;
	unsigned char bit_count;
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


/* Private functions begin here. */
/* Do not depend on existence of string.h */
static void pad_buffer(unsigned char * buf, size_t bufsiz, unsigned char val)
{
	size_t count;
	for(count = 0; count < bufsiz; count++)
	{
		buf[count] = val;
	}
}

static void purge(serial_handle_t serial_dev)
{
	serial_status_t timeout_status = SERIAL_NO_ERRORS;
	char dummy_byte;
	do{
		timeout_status = serial_rcv(&dummy_byte, 1, 1, NULL, serial_dev);
	}while(timeout_status != SERIAL_TIMEOUT);
}

static modem_errors_t wait_for_rx_ready(serial_handle_t serial_device, xmodem_xfer_mode_t flags)
{
	unsigned int elapsed_time;
	serial_status_t ser_status = SERIAL_NO_ERRORS;
	int expected_rx_detected = 0;
	char rx_code = NUL;

	/* Wait for NAK or 'C', timeout after 1 minute. */
	elapsed_time = 0;
	while(!(expected_rx_detected))
	{
		int time_to_recv;
		ser_status = serial_rcv(&rx_code, 1, 60 - elapsed_time, &time_to_recv, serial_device);
		elapsed_time += time_to_recv;

		/* What happens if error on setting timeouts? */
		if(ser_status != SERIAL_NO_ERRORS)
		{
			break;
		}
		/* ASCII_C is only correct for XMODEM_1K and XMODEM_CRC. */
		else if((flags == XMODEM_1K || flags == XMODEM_CRC) && rx_code == ASCII_C)
		{
			expected_rx_detected = 1;
		}
		/* Else, wait for NAK. */
		else if((flags == XMODEM) && rx_code == NAK)
		{
			expected_rx_detected = 1;
		}
	}

	return serial_to_modem_error(ser_status);
}

static modem_errors_t wait_for_tx_response(serial_handle_t serial_device, xmodem_xfer_mode_t flags)
{
	return MODEM_NO_ERRORS;
}

static offset_names_t get_checksum_offset(unsigned short flags)
{
	return (flags == XMODEM_1K) ? X1K_CRC : CHKSUM_CRC;
}

/* static int wait_for_rx_ack(serial_handle_t serial_device)
{
	char rx_code;


} */

static modem_errors_t serial_to_modem_error(serial_status_t status)
{
	modem_errors_t equiv_status;
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
