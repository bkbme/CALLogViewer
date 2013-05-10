/*
 * uart.c
 *
 * Created: 9/23/2012 9:58:39 PM
 *  Author: mlohse
 */ 

#include <avr/interrupt.h>
#include "led.h"
#include "uart.h"
#include "buffer.h"

#ifdef UART0_ENABLE
volatile RingBuffer buffer_tx0;
volatile RingBuffer buffer_rx0;
uint8_t buffer_data_tx0[BUFFER_SIZE_TX0];
uint8_t buffer_data_rx0[BUFFER_SIZE_RX0];
#endif

#ifdef UART1_ENABLE
volatile RingBuffer buffer_tx1;
volatile RingBuffer buffer_rx1;
uint8_t buffer_data_tx1[BUFFER_SIZE_TX1];
uint8_t buffer_data_rx1[BUFFER_SIZE_RX1];
#endif


void uart_init()
{
	cli();
#ifdef UART0_ENABLE
	buffer_init(&buffer_tx0, buffer_data_tx0, BUFFER_SIZE_TX0);
	buffer_init(&buffer_rx0, buffer_data_rx0, BUFFER_SIZE_RX0);

	UBRR0H = 0;
	//UBRRL = 103; // 90600Baud@16MHz, see datasheet table 71 (Examples of UBRR Settings...)
	UBRR0L = 119; // 90600Baud@18.432MHz
	UCSR0B = (1<<TXEN0) | (1<<RXEN0) | (1 << TXCIE0) | (1<<RXCIE0); // Enable receiver and transmitter and receive/transmit complete interrupt
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
#endif
#ifdef UART1_ENABLE
	buffer_init(&buffer_tx1, buffer_data_tx1, BUFFER_SIZE_TX1);
	buffer_init(&buffer_rx1, buffer_data_rx1, BUFFER_SIZE_RX1);

	UBRR1H = 0;
	UBRR1L = 119; // 90600Baud@18.432MHz
	UCSR1B = (1<<TXEN1) | (1<<RXEN1) | (1 << TXCIE1) | (1<<RXCIE1); // Enable receiver and transmitter and receive/transmit complete interrupt
	UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);
#endif
	sei();
}

#ifdef UART0_ENABLE
ISR(USART0_TX_vect)
{
	if (buffer_count(&buffer_tx0))
	{
		UDR0 = buffer_read(&buffer_tx0);
	}
}

ISR(USART0_RX_vect)
{
	if (buffer_is_full(&buffer_rx0))
	{
		// todo: handle this somehow... (send error message?)
		return;
	}
	
	buffer_write(&buffer_rx0, UDR0);
}

void uart0_flush()
{
	cli();
	buffer_clear(&buffer_rx0);
	buffer_clear(&buffer_tx0);
	sei();
}

void uart0_write(uint8_t byte)
{
	if ((UCSR0A & (1 << UDRE0)) && buffer_count(&buffer_tx0) == 0)
	{
		UDR0 = byte; // write buffer empty and no uart write in progress -> write out directly
		return;
	}

	while (buffer_is_full(&buffer_tx0)); // write buffer full - wait!

	cli();
	buffer_write(&buffer_tx0, byte);
	sei();
}

uint8_t uart0_read()
{
	uint8_t byte;
	cli();
	byte = buffer_read(&buffer_rx0);
	sei();
	return byte;
}

uint8_t uart0_bytes_available()
{
	return buffer_count(&buffer_rx0);
}

uint8_t uart0_peek(uint8_t position)
{
	return buffer_peek(&buffer_rx0, position); // doesn't modify buffer, no need to disable interrupts
}
#endif // UART0_ENABLE


#ifdef UART1_ENABLE
ISR(USART1_TX_vect)
{
	if (buffer_count(&buffer_tx1))
	{
		UDR1 = buffer_read(&buffer_tx1);
	}
}

ISR(USART1_RX_vect)
{
	if (buffer_is_full(&buffer_rx1))
	{
		// todo: handle this somehow... (send error message?)
		return;
	}
	
	buffer_write(&buffer_rx1, UDR1);
}

void uart1_flush()
{
	cli();
	buffer_clear(&buffer_rx1);
	buffer_clear(&buffer_tx1);
	sei();
}

void uart1_write(uint8_t byte)
{
	if ((UCSR1A & (1 << UDRE1)) && buffer_count(&buffer_tx1) == 0)
	{
		UDR1 = byte; // write buffer empty and no uart write in progress -> write out directly
		return;
	}

	while (buffer_is_full(&buffer_tx1)); // write buffer full - wait!

	cli();
	buffer_write(&buffer_tx1, byte);
	sei();
}

uint8_t uart1_read()
{
	uint8_t byte;
	cli();
	byte = buffer_read(&buffer_rx1);
	sei();
	return byte;
}

uint8_t uart1_bytes_available()
{
	return buffer_count(&buffer_rx1);
}

uint8_t uart1_peek(uint8_t position)
{
	return buffer_peek(&buffer_rx1, position); // doesn't modify buffer, no need to disable interrupts
}
#endif // UART1_ENABLE
