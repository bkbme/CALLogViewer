/*
 * protocol.c
 *
 * Created: 4/5/2013 2:07:06 PM
 *  Author: mlohse
 */

#include <stdint.h>
#include <avr/interrupt.h>

#include "led.h"
#include "dock.h"
#include "uart.h"
#include "timer.h"
#include "config.h"
#include "message.h"
#include "protocol.h"
#include "footswitch.h"

const char version[] = PROTOCOL_VERSION;
ConnectionState state = StateDisconnected;
uint8_t sequence = 0;

void send_error(ErrorCode code, uint8_t seq)
{
	// flush read buffer?
	uint8_t size = (seq ? 0x06 : 0x05);
	uint8_t checksum = IdErrorMessage + size + sequence + code;
	state = StateDisconnected;
	led_off(5);
	servo_set_position(DOCK_SERVO_ZAXIS_ID, DOCK_SERVO_POWER_OFF);
	servo_set_position(DOCK_SERVO_XAXIS_ID, DOCK_SERVO_POWER_OFF);
	cli();
	uart0_write(IdErrorMessage);
	uart0_write(size);
	uart0_write(sequence++);
	uart0_write(code);

	if (seq)
	{
		uart0_write(seq);
		checksum += seq;
	}

	uart0_write(0xFF - checksum);
	sei();
}

void send_docking_force(uint16_t force, uint8_t steady)
{
	const uint8_t size = 0x07;
	uint8_t checksum = IdDockingForceMessage + size + sequence;
	uint8_t temp;

	cli();
	uart0_write(IdDockingForceMessage);
	uart0_write(size);
	uart0_write(sequence++);

	temp = (uint8_t)(force >> 8);
	checksum += temp;
	uart0_write(temp);
	temp = (uint8_t)(force);
	checksum += temp;
	uart0_write(temp);

	checksum += steady;
	uart0_write(steady);

	uart0_write(0xFF - checksum);
	sei();
}

void send_docking_state(uint8_t state)
{
	const uint8_t size = 0x05;
	const uint8_t checksum = IdDockingStateMessage + size + sequence + state;

	cli();
	uart0_write(IdDockingStateMessage);
	uart0_write(size);
	uart0_write(sequence++);
	uart0_write(state);
	uart0_write(0xFF - checksum);
	sei();
}

void send_ack(uint8_t seq)
{
	const uint8_t size = 0x05;
	uint8_t checksum = IdAckMessage + size + sequence + seq;
	cli();
	uart0_write(IdAckMessage);
	uart0_write(size);
	uart0_write(sequence++);
	uart0_write(seq);
	uart0_write(0xFF - checksum);
	sei();
}

void send_version()
{
	const uint8_t size = 0x0C;
	uint8_t checksum = IdVersionMessage + size + sequence;

	cli();
	uart0_write(IdVersionMessage);
	uart0_write(size);
	uart0_write(sequence++);

	for (uint8_t i = 0; i < 8; ++i)
	{
		uart0_write(version[i]);
		checksum += version[i];
	}

	uart0_write(0xFF - checksum);
	sei();
}

void handle_ack()
{
	// ignore ack's for now...
}

void handle_error()
{
	state = StateDisconnected;
	led_off(5);
	servo_set_position(DOCK_SERVO_ZAXIS_ID, DOCK_SERVO_POWER_OFF);
	servo_set_position(DOCK_SERVO_XAXIS_ID, DOCK_SERVO_POWER_OFF);
	cli();
	uart0_flush();
	sei();
}

void handle_init()
{
	send_ack(message.sequence);
	send_version();
	state = StateConnected;
	led_on(5);
}

void handle_footswitch()
{
	if (state != StateConnected)
	{
		send_error(ErrorDisconnect, message.sequence);
		return;
	}
	
	if (message.data_size != 1)
	{
		send_error(ErrorParser, message.sequence);
		return;
	}

	footswitch_set(message.data[0]);
	send_ack(message.sequence);
}

void handle_int_reset()
{
	if (state != StateConnected)
	{
		send_error(ErrorDisconnect, message.sequence);
		return;
	}
	
	/// @todo: implement me
	
	send_ack(message.sequence);
}

void handle_docking_limit()
{
	if (state != StateConnected)
	{
		send_error(ErrorDisconnect, message.sequence);
		return;
	}

	if (message.data_size != 4)
	{
		send_error(ErrorParser, message.sequence);
		return;
	}

	int16_t lowerLimit = ((message.data[0] << 8) | message.data[1]);
	int16_t upperLimit = ((message.data[2] << 8) | message.data[3]);

	dock_set_dockinglimits(lowerLimit, upperLimit);
	send_ack(message.sequence);
}

void handle_servo_ctrl()
{
	if (state != StateConnected)
	{
		send_error(ErrorDisconnect, message.sequence);
		return;
	}

	if (message.data_size != 2)
	{
		send_error(ErrorParser, message.sequence);
		return;
	}

	uint8_t id = message.data[0];
	if (id == DOCK_SERVO_ZAXIS_ID)
	{
		dock_set_docktype(ManualDocking);
	}

	servo_set_position(id, message.data[1]);
	send_ack(message.sequence);
}

void handle_docking_tare()
{
	if (state != StateConnected)
	{
		send_error(ErrorDisconnect, message.sequence);
		return;
	}

	dock_tare();
	send_ack(message.sequence);
}

void process_messages()
{
	MessageParseResult res;
	while ((res = message_parse()) == ParseOk)
	{
		switch (message.identifier)
		{
			case IdAckMessage:
				handle_ack();
				break;
			case IdInitMessage:
				handle_init();
				break;
			case IdFootSwitchMessage:
				handle_footswitch();
				break;
			case IdInterlockResetMessage:
				handle_int_reset();
				break;
			case IdServoCtrlMessage:
				handle_servo_ctrl();
				break;
			case IdDockingLimitMessage:
				handle_docking_limit();
				break;
			case IdErrorMessage:
				handle_error();
				break;
			case IdDockingTareMessage:
				handle_docking_tare();
				break;
			default:
				send_error(ErrorUnhandled, message.sequence);
				return;
		}
	}

	switch (res)
	{
		case ParseNotEnoughData: // buffer empty, done
			break;
//		case ParseInvalidIdentifier: // fall
//		case ParseInvalidSizeError: // fall
//		case ParseChecksumError: // fall
		default:
			send_error(ErrorParser, 0);
			break;
	}
}

ConnectionState connection_state()
{
	return state;
}