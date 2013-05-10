/*
 * RingBuffer.c
 *
 * Created: 3/27/2013 5:48:28 PM
 *  Author: mlohse
 */ 

#include "buffer.h"

void buffer_init(RingBuffer *buffer, uint8_t *data, uint8_t size)
{
	buffer->data = data;
	buffer->capacity = size;
	buffer->ri = 0;
	buffer->wi = 0;
}

void buffer_clear(RingBuffer *buffer)
{
	buffer->ri = 0;
	buffer->wi = 0;
}

void buffer_write(RingBuffer *buffer, uint8_t ch)
{	
	if ((buffer->wi + 1) == buffer->ri || ((buffer->wi + 1) == buffer->capacity && buffer->ri == 0))
	{
		return; // buffer full;
	}
	
	buffer->data[buffer->wi++] = ch;

	if (buffer->capacity == buffer->wi)
	{
		buffer->wi = 0;
	}
}

uint8_t buffer_read(RingBuffer *buffer)
{
	if (buffer->ri == buffer->wi)
	{
		return 0; // buffer empty
	}

	if ((buffer->ri + 1) == buffer->capacity)
	{
		buffer->ri = 0;
		return buffer->data[buffer->capacity - 1];
	}

	return buffer->data[buffer->ri++];
}

uint8_t buffer_peek(RingBuffer *buffer, uint8_t pos)
{
	uint16_t index = pos + buffer->ri;
	while (index >= buffer->capacity)
	{
		index -= buffer->capacity;
	}

	return buffer->data[index];
}

uint8_t buffer_count(RingBuffer *buffer)
{
	return ((buffer->wi < buffer->ri) ? buffer->capacity - buffer->ri + buffer->wi : buffer->wi - buffer->ri);
}

uint8_t buffer_is_full(RingBuffer *buffer)
{
	if (buffer->wi < buffer->ri)
	{
		return ((buffer->wi + 1) == buffer->ri);
	}
	
	return (buffer->ri == 0 && (buffer->wi + 1) == buffer->capacity);
}

/*
uint8_t buffer_is_empty(RingBuffer *buffer)
{
	return (buffer->ri == buffer->wi);
}
*/