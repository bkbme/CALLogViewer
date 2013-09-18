/*
 * dock.c
 *
 * Created: 4/30/2013 3:16:29 PM
 *  Author: mlohse
 */ 

#include "led.h"
#include "dock.h"
#include "uart.h"
#include "timer.h"
#include "config.h"
#include "protocol.h"

#include <avr/interrupt.h>
#include <avr/io.h>

volatile int16_t dock_force;
volatile int16_t dock_force_delta;
volatile uint8_t dock_force_steady;
volatile uint32_t dock_force_timestamp;
volatile uint8_t dock_wait_update;

int16_t lower_docking_limit;
int16_t upper_docking_limit;

DockConnectivityState con_state;
DockingType dock_type;
DockingState dock_state;


void set_docking_state(DockingState state)
{
	if (state != dock_state)
	{
		dock_state = state;
		led_off(DOCK_LED_BOT);
		led_off(DOCK_LED_TOP);
		led_off(DOCK_LED_FORCE);

		switch (state)
		{
			case DockedBottom:
				led_on(DOCK_LED_BOT);
				break;
			case DockedTop:
				led_on(DOCK_LED_TOP);
				break;
			case DockedLimit:
				led_on(DOCK_LED_FORCE);
				break;
			case DockError:
				led_on(DOCK_LED_BOT);
				led_on(DOCK_LED_TOP);
				led_on(DOCK_LED_FORCE);
				break;
			default: // DockMovingUp, DockMovingDown, DockedManually
				break;
		}

		if (connection_state() == StateConnected)
		{
			send_docking_state(state);
		}
	}
}

void dock_init()
{
	dock_force = 0;
	dock_force_delta = 0;
	dock_force_steady = 0;
	dock_type = ManualDocking;
	dock_state = DockedManually;
	con_state = DockDisconnected;
	lower_docking_limit = INIT_LOWER_DOCKING;
	upper_docking_limit = INIT_UPPER_DOCKING;

	/// @todo dock auto-detection

//	if (v > DMS_REF_MIN && v < DMS_REF_MAX)
//	{
		con_state = DockConnected;
//	}
}

void dock_tare()
{
	if (con_state == DockConnected)
	{
		uart1_write('t');
	}
}

void dock_update_force()
{
	if (con_state == DockConnected && connection_state() == StateConnected)
	{
		uart1_write('w');
	}
}

void dock_parse_forcedata()
{
	if ((uint8_t)uart1_bytes_available() >= 18)
	{
		if (uart1_peek(11) == 'E')
		{
			send_error(DockCommunicationError, 0);
			uart1_flush();
			return;
		}
		uart1_read(); // discard first byte

		uint8_t negative = (uart1_read() == '-');
		dock_force_steady = 0;
		int16_t force = 0;
		for (uint8_t i = 0; i < 10; ++i)
		{
			uint8_t ch = uart1_read();
			if (ch == '.')
			{
				continue;
			}
			force *= 10;
			if (ch > 0x30 && ch <= 0x39) // parse digit (1..9)
			{
				force += (ch - 0x30);
			}
		}
		for (uint8_t i = 0; i < 6; ++i)
		{
			if (uart1_read() == 'g')
			{
				dock_force_steady = 1;
			}
		}
		if (negative)
		{
			force = -force;
		}

		dock_wait_update = 0;

		dock_force_delta += force - dock_force;
		dock_force_delta /= 2;
		dock_force_timestamp = uptime;
		dock_force = force;
		send_docking_force(dock_force, dock_force_steady);
	}
}

void dock_check_limits()
{
	// check end positions...
	const uint8_t servo = servo_get_position(DOCK_SERVO_ZAXIS_ID);
	if (servo != DOCK_SERVO_POWER_OFF && servo > DOCK_SERVO_STOP) // moving up
	{
		set_docking_state(DockMovingUp);
#ifdef DOCK_TYPE_EITECH
		if (!(PORT_PHOTO_SENSOR_TOP & (1 << PIN_PHOTO_SENSOR_TOP)))
#else
		if (PORT_PHOTO_SENSOR_TOP & (1 << PIN_PHOTO_SENSOR_TOP))
#endif
		{
			servo_set_position(DOCK_SERVO_ZAXIS_ID, DOCK_SERVO_POWER_OFF);
			dock_type = ManualDocking;
			set_docking_state(DockedTop);
		}
		if (dock_force > DOCK_RANGE_MAX) // safety shutoff when max. docking pressure is reached
		{
			servo_set_position(DOCK_SERVO_ZAXIS_ID, DOCK_SERVO_POWER_OFF);
			set_docking_state(DockedLimit);
			//dock_type = ManualDocking;
		}
		if ((dock_force_timestamp + DOCK_FORCE_TIMEOUT) < uptime) // docking force update timeout
		{
			servo_set_position(DOCK_SERVO_ZAXIS_ID, DOCK_SERVO_POWER_OFF);
			dock_type = ManualDocking;
			set_docking_state(DockError);
			send_error(DockTimeoutError, 0);
		}
	}

	if (servo != DOCK_SERVO_POWER_OFF && servo < DOCK_SERVO_STOP) // moving down
	{
		set_docking_state(DockMovingDown);
		if (PORT_PHOTO_SENSOR_BOT & (1 << PIN_PHOTO_SENSOR_BOT))
		{
			servo_set_position(DOCK_SERVO_ZAXIS_ID, DOCK_SERVO_POWER_OFF);
			dock_type = ManualDocking;
			set_docking_state(DockedBottom);
		}
	}

	if (servo == DOCK_SERVO_POWER_OFF && dock_type == ManualDocking && (dock_state == DockMovingUp || dock_state == DockMovingDown))
	{
		set_docking_state(DockedManually);
	}

	if (dock_type == AutoDocking && !dock_wait_update)
	{
		if (dock_force > upper_docking_limit && lower_docking_limit < 0) // undock, must be fast - overshooting is ok
		{
			servo_set_position(DOCK_SERVO_ZAXIS_ID, DOCK_SERVO_DOWN);
			set_docking_state(DockMovingDown);
			return;
		}

		if (dock_force > upper_docking_limit) // very slowly decrease docking force, wait for next zForce update if close to docking limit...
		{
			if (dock_force_delta > DOCK_FORCE_DELTA_TRH_DEC || dock_force_steady)
			{
				dock_wait_update = ((upper_docking_limit - dock_force) > DOCK_FORCE_SLOWDOWN_TRH) ? 0 : 1;
				servo_set_position_timeout(DOCK_SERVO_ZAXIS_ID, DOCK_SERVO_DOWN_SLOW, 60);
				set_docking_state(DockMovingDown);
			}
			return;
		}

		if (dock_force < lower_docking_limit && dock_force_delta < DOCK_FORCE_DELTA_TRH_INC) // increase docking pressure...
		{
			uint8_t speed = (dock_force_delta > 0 || ((lower_docking_limit - dock_force) < DOCK_FORCE_SLOWDOWN_TRH && dock_force > DOCK_FORCE_ZERO_TRH)) ? DOCK_SERVO_UP_SLOW : DOCK_SERVO_UP;
			servo_set_position(DOCK_SERVO_ZAXIS_ID, speed);
			set_docking_state(DockMovingUp);
			return;
		}

		servo_set_position(DOCK_SERVO_ZAXIS_ID, DOCK_SERVO_POWER_OFF);
		set_docking_state(DockedLimit);
	}
}

void dock_set_docktype(DockingType type)
{
	if (type == AutoDocking || type == ManualDocking)
	{
		dock_type = type;
	}
}

void dock_set_dockinglimits(int16_t lower, int16_t upper)
{
	if (lower <= DOCK_RANGE_MAX && lower >= DOCK_RANGE_MIN &&
		upper <= DOCK_RANGE_MAX && upper >= DOCK_RANGE_MIN &&
		lower < upper)
	{
		lower_docking_limit = lower;
		upper_docking_limit = upper;
		dock_type = AutoDocking;
	}
}
