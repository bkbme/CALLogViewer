/*
 * led.h
 *
 * Created: 4/9/2013 6:15:58 PM
 *  Author: mlohse
 */ 


#ifndef LED_H_
#define LED_H_

#include <stdint.h>

// Helper Macros
#define bit_set(PORT, PIN)		((PORT) |= (1 << (PIN)))
#define bit_clear(PORT, PIN)	((PORT) &= (uint8_t)~(1 << (PIN)))
#define bit_toggle(PORT, PIN)	((PORT) ^= (1 << (PIN)))
//#define bit_toggle(PORT, PIN)	(PORT) = (uint8_t)((uint8_t)((PORT) & (1 << (PIN))) | (uint8_t)((PORT) & (uint8_t)~(1 << (PIN))))

void led_init(); // must be called after timer has been initialized!

void led_on(uint8_t num);
void led_off(uint8_t num);
void led_toggle(uint8_t num);

#endif /* LED_H_ */