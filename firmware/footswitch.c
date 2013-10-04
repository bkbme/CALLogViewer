/*
 * footswitch.c
 *
 * Created: 4/5/2013 3:51:49 PM
 *  Author: mlohse
 */ 

#include "footswitch.h"
#include "config.h"
#include "led.h"

#include <avr/io.h>
#include <avr/interrupt.h>

void footswitch_init()
{
	footswitch_set(INIT_FOOTSWITCH_STATE);
}

void footswitch_set(uint8_t state)
{
	// debug start (visualize footswitch state through led 5 and 6)
	switch (state)
	{
		case 0x00: // intermediate state
			led_off(FS_LED_STATE0);
			led_off(FS_LED_STATE1);
			break;
		case 0x01: // pedal down
			led_on(FS_LED_STATE0);
			led_off(FS_LED_STATE1);
			break;
		case 0x02: // pedal up
			led_off(FS_LED_STATE0);
			led_on(FS_LED_STATE1);
			break;
		default: // error state
			led_on(FS_LED_STATE0);
			led_on(FS_LED_STATE1);
			break;
	}
	// debug end

	state = ((state << PIN_FOOTSWITCH0) & (1 << PIN_FOOTSWITCH0)) | (((state >> 1) << PIN_FOOTSWITCH1) & (1 << PIN_FOOTSWITCH1));
	cli();
	uint8_t temp = PORT_FOOTSWITCH & (uint8_t)~((1 << PIN_FOOTSWITCH0) | (1 << PIN_FOOTSWITCH1));
	temp |= state;
	PORT_FOOTSWITCH = temp;
	sei();
}
