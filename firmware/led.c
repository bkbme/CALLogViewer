/*
 * led.c
 *
 * Created: 4/9/2013 6:16:11 PM
 *  Author: mlohse
 */

#include "led.h"
#include "timer.h"
#include "config.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#if defined PORT_LED7 && defined PIN_LED7
#	define LED_COUNT 8
#else
#	define LED_COUNT 6
#endif
#define LED_FLASH_INTERVAL 100 //ms

void led_init()
{
	for (uint8_t i = 0; i < LED_COUNT; ++i)
	{
		led_on(i);
		uint32_t timeout = (uint32_t)(i + 1) * LED_FLASH_INTERVAL;
		while (uptime < timeout);
	}
	
	for (uint8_t i = 0; i < LED_COUNT; ++i)
	{
		led_off(i);
		uint32_t timeout = (uint32_t)(i + LED_COUNT) * LED_FLASH_INTERVAL;
		while (uptime < timeout);
	}
}

void led_on(uint8_t num)
{
	switch (num)
	{
		case 0: bit_set(PORT_LED1, PIN_LED1); break;
		case 1: bit_set(PORT_LED2, PIN_LED2); break;
		case 2: bit_set(PORT_LED3, PIN_LED3); break;
		case 3: bit_set(PORT_LED4, PIN_LED4); break;
		case 4: bit_set(PORT_LED5, PIN_LED5); break;
		case 5: bit_set(PORT_LED6, PIN_LED6); break;
#if defined PORT_LED7 && defined PIN_LED7
		case 6: bit_set(PORT_LED7, PIN_LED7); break;
#endif
#if defined PORT_LED8 && defined PIN_LED8
		case 7: bit_set(PORT_LED8, PIN_LED8); break;
#endif
		default:
			break;
	}
}

void led_off(uint8_t num)
{
	switch (num)
	{
		case 0: bit_clear(PORT_LED1, PIN_LED1); break;
		case 1: bit_clear(PORT_LED2, PIN_LED2); break;
		case 2: bit_clear(PORT_LED3, PIN_LED3); break;
		case 3: bit_clear(PORT_LED4, PIN_LED4); break;
		case 4: bit_clear(PORT_LED5, PIN_LED5); break;
		case 5: bit_clear(PORT_LED6, PIN_LED6); break;
#if defined PORT_LED7 && defined PIN_LED7
		case 6: bit_clear(PORT_LED7, PIN_LED7); break;
#endif
#if defined PORT_LED8 && defined PIN_LED8
		case 7: bit_clear(PORT_LED8, PIN_LED8); break;
#endif
		default:
			break;
	}
}

void led_toggle(uint8_t num)
{
	switch (num)
	{
		case 0: bit_toggle(PORT_LED1, PIN_LED1); break;
		case 1: bit_toggle(PORT_LED2, PIN_LED2); break;
		case 2: bit_toggle(PORT_LED3, PIN_LED3); break;
		case 3: bit_toggle(PORT_LED4, PIN_LED4); break;
		case 4: bit_toggle(PORT_LED5, PIN_LED5); break;
		case 5: bit_toggle(PORT_LED6, PIN_LED6); break;
#if defined PORT_LED7 && defined PIN_LED7
		case 6: bit_toggle(PORT_LED7, PIN_LED7); break;
#endif
#if defined PORT_LED8 && defined PIN_LED8
		case 7: bit_toggle(PORT_LED8, PIN_LED8); break;
#endif
		default:
			break;
	}
}