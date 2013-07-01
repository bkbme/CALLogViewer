/*
 * timer.h
 *
 * Created: 4/4/2013 5:41:19 PM
 *  Author: mlohse
 */ 


#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

volatile uint32_t uptime; // uptime in ms

void timer_init();

void servo_set_position(uint8_t id, uint8_t position);
void servo_set_position_timeout(uint8_t id, uint8_t position, uint16_t timeout);
uint8_t servo_get_position(uint8_t id);


#endif /* TIMER_H_ */