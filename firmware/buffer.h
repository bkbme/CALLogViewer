/*
 * RingBuffer.h
 *
 * Created: 3/27/2013 5:48:08 PM
 *  Author: mlohse
 */ 


#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdint.h>

typedef struct
{
	uint8_t *data;
	uint8_t capacity;
	uint8_t ri;
	uint8_t wi;
} RingBuffer;

void buffer_init(RingBuffer *buffer, uint8_t *data, uint8_t size);
void buffer_clear(RingBuffer *buffer);

void buffer_write(RingBuffer *buffer, uint8_t ch);
uint8_t buffer_read(RingBuffer *buffer);
uint8_t buffer_peek(RingBuffer *buffer, uint8_t pos);

uint8_t buffer_count(RingBuffer *buffer);
uint8_t buffer_is_full(RingBuffer *buffer);
//uint8_t buffer_is_empty(RingBuffer *buffer);

#endif /* BUFFER_H_ */