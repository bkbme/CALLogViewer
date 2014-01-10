/*
 * uart.h
 *
 * Created: 9/23/2012 9:58:15 PM
 *  Author: mlohse
 */ 

#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include "config.h"

void uart_init();

#ifdef UART0_ENABLE
void uart0_flush();
uint8_t uart0_read();
uint8_t uart0_bytes_available();
uint8_t uart0_peek(uint8_t position);
void uart0_write(uint8_t byte);
void uart0_write_buffer(const uint8_t *data, uint8_t size);
#endif

#ifdef UART1_ENABLE
void uart1_flush();
uint8_t uart1_read();
uint8_t uart1_bytes_available();
uint8_t uart1_peek(uint8_t position);
void uart1_write(uint8_t byte);
#endif

// Check whether buffer size is within range...
#ifdef UART0_ENABLE
	#if (BUFFER_SIZE_TX0 > 254 || BUFFER_SIZE_RX0 > 254)
		#error Maximum uart buffer size is 254 bytes!
	#endif

	#if (BUFFER_SIZE_TX0 < 1 || BUFFER_SIZE_RX0 < 1)
		#error Minimum uart buffer size is 1 byte!
	#endif
#endif

#ifdef UART1_ENABLE
	#if (BUFFER_SIZE_TX1 > 254 || BUFFER_SIZE_RX1 > 254)
		#error Maximum uart buffer size is 254 bytes!
	#endif

	#if (BUFFER_SIZE_TX1 < 1 || BUFFER_SIZE_RX1 < 1)
		#error Minimum uart buffer size is 1 byte!
	#endif
#endif

#endif /* UART_H_ */