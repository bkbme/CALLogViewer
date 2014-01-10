/*
 * ArgesInterlockDebug.c
 *
 * Created: 11/21/2012 2:29:06 AM
 *  Author: mlohse
 */ 

#include <avr/interrupt.h>
#include <avr/io.h>

#include "led.h"
#include "dock.h"
#include "uart.h"
#include "timer.h"
#include "config.h"
#include "settings.h"
#include "protocol.h"
#include "footswitch.h"


void init()
{
	// configure & initialize io ports
	DDRA = IO_DDRA;
	DDRB = IO_DDRB;
	DDRC = IO_DDRC;
	DDRD = IO_DDRD;
	
	PORTA = INIT_PORTA;
	PORTB = INIT_PORTB;
	PORTC = INIT_PORTC;
	PORTD = INIT_PORTD;

	uart_init();
	timer_init();
	led_init();
	footswitch_init();
	dock_init();
	uart0_flush();
}

int main(void)
{
	init();

	while(1)
	{
#if TARGET == VICTUS_DOCK_TARGET || TARGET == EITECH_DOCK_TARGET
		if (settings_changed() & SettingsByte) // ignore Word/Array type changes
		{
				dock_load_settings();
		}

		dock_parse_forcedata();
		dock_check_limits();
#endif
		process_messages();
	}
}