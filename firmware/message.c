/*
 * MessageParser.c
 *
 * Created: 4/4/2013 1:24:47 PM
 *  Author: mlohse
 */ 

#include "message.h"
#include "uart.h"

#define MSG_SIZE_MIN 4 // size of smallest message

MessageParseResult message_parse()
{
	uint8_t bytes_available = uart0_bytes_available();

	if (bytes_available >= MSG_SIZE_MIN && bytes_available >= uart0_peek(1)) // check if next message can be read (entire message must be available in buffer)
	{
		uint8_t checksum;
		if (uart0_peek(0) >= IdInvalidIdentifier)
		{
			return ParseInvalidIdentifier;
		}
		message.identifier = (MessageIdentifier) uart0_read();
		checksum = message.identifier;

		message.data_size = uart0_read(); // holds msg_size now
		checksum += message.data_size;
		message.data_size -= 4; // data_size = msg_size - sizeOf(identifier + size + seq.no + checksum)
		message.sequence = uart0_read();
		checksum += message.sequence;
		
		if (message.data_size > MSG_DATA_SIZE_MAX) // make sure data fits into buffer
		{
			return ParseInvalidSizeError;
		}
		
		for (uint8_t i = 0; i < message.data_size; ++i)
		{
			message.data[i] = uart0_read();
			checksum += message.data[i];
		}
		
		if ((0xFF - checksum) != uart0_read()) // verify checksum
		{
			return ParseChecksumError;
		}
		
		return ParseOk;
	}

	return ParseNotEnoughData;
}