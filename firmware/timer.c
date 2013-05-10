/*
 * timer.c
 *
 * Created: 4/4/2013 5:41:31 PM
 *  Author: mlohse
 */ 

//#include "dms.h"
#include "dock.h"
#include "timer.h"
#include "config.h"
#include "protocol.h"

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t servo_clk = 0;
volatile uint8_t servo_pos;
volatile uint16_t servo_stop_timer;

void timer_init()
{
	uptime = 0;
	
	//TCCR0 = 0x0B; // WGM01=1 (CTC mode), CS00, CS01 = 1 (clk/64 prescaler)
	TCCR0A = (1 << WGM01); // CTC-Mode
	TCCR0B = (1 << CS01) | (1 << CS00); // clk/64 prescaler

	TIMSK0 &= (uint8_t)~(1 << TOIE0); // TOIE0 = 0 (Timer0 overflow IRQ)
	TIMSK0 |= (1 << OCIE0A);  // OCIE0 = 1 (Timer0 Compare IRQ)

	servo_pos = DOCK_SERVO_STOP;
}

void servo_set_position(uint8_t position)
{
		servo_pos = position;
}

void servo_set_position_timeout(uint8_t position, uint16_t timeout)
{
	if (timeout > 20) // minimum timer resolution 20ms
	{
		servo_stop_timer = timeout;
		servo_pos = position;
	}
}

uint8_t servo_get_position()
{
	return servo_pos;
}

/*************************************************************************
 * Timer0
 * 16MHz: prescaler 64: 250000 inc/sec, -> 976.5625 overflows (1.024ms)
 * 18.4321 MHz: pre 64: 288002 inc/sec, -> 1125.006 overflows (0.889ms)
 *
 * OCR0: 153   256x2 17x255
 * t_ms: 0.612 2.048 17.34 = 20ms
 * OC0:  1     x/0   0     = 50Hz PWM
 *************************************************************************/
ISR (SIG_OUTPUT_COMPARE0A)
{
	switch (OCR0A) // generate 50Hz pwm for servos
	{
		case TMG_SERVO_PWM: //                         1.77777 ms (interval: 0.00347ms)
			if (servo_pos < servo_clk)
			{
				PORT_SERVO0 &= (uint8_t)~(1 << PIN_SERVO0);
			}
			if (!++servo_clk)
			{
				OCR0A = TMG_SERVO_STOP;
			}
			break;
		case TMG_SERVO_START: //                       0.4479 ms
			OCR0A = TMG_SERVO_PWM;
			servo_clk = 0;
			break;
		case TMG_SERVO_STOP: // fall                  17.7777 ms
		default: //                                  -------------------------------
			if (++servo_clk > TMG_SERVO_STOP_MULT) // 20.0034 ms (~50 Hz)
			{
				OCR0A = TMG_SERVO_START;
				if ((servo_pos < 110 || servo_pos > 120))
				{
					PORT_SERVO0 |= (1 << PIN_SERVO0);
				}

				uptime += 20; // every 20ms
				if (uptime % DOCK_UPDATE_INTERVAL == 0) // twice per sec...
				{
					//dms_update();
					dock_update_force();
				}
				if (servo_stop_timer)
				{
					servo_stop_timer -= 20;
					if (servo_stop_timer < 20)
					{
						servo_stop_timer = 0;
						servo_pos = DOCK_SERVO_STOP;
					}
				}
			}
			break;
	}
}
