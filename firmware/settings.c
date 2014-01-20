/*
 * settings.c
 *
 * Created: 10/16/2013 2:21:29 PM
 *  Author: mlohse
 */ 

#include "config.h"
#include "settings.h"

#include <avr/eeprom.h>

// when changing ARRAY_SIZE make sure to initialize 'eeArraySettings' accordingly
// also note: 10 bytes is the maximum size a SettingsMessage can hold
#define ARRAY_SIZE 10

uint8_t change_flags = 0x00;

uint8_t EEMEM eeByteSettings[ByteSettingsKeyCount] = {
	DOCK_SERVO_UP_EE,		// KeyDockServoUp
	DOCK_SERVO_DOWN_EE,		// KeyDockServoDown
	DOCK_SERVO_UP_SLOW_EE,	// KeyDockServoUpSlow
	DOCK_SERVO_DOWN_SLOW_EE,	// KeyDockServoDownSlow
	DOCK_SLOT_COUNT_EE		// KeyDockSlotCount
};

uint16_t EEMEM eeWordSettings[WordSettingsKeyCount] = {
	DOCK_LIMIT_ZERO_LO_EE,	// KeyDockLimitZeroLo
	DOCK_LIMIT_ZERO_UP_EE,	// KeyDockLimitZeroUp
	DOCK_LIMIT_SOFT_LO_EE,	// KeyDockLimitSoftLo
	DOCK_LIMIT_SOFT_UP_EE,	// KeyDockLimitSoftUp
	DOCK_LIMIT_REGULAR_LO_EE,// KeyDockLimitRegularLo
	DOCK_LIMIT_REGULAR_UP_EE	// KeyDockLimitRegularUp
};

uint8_t EEMEM eeArraySettings[ArraySettingsKeyCount][ARRAY_SIZE] = {
	DOCK_SLOTS1_EE,			// KeyDockSlots1
	DOCK_SLOTS2_EE			// KeyDockSlots2
};


void settings_set_byte(uint8_t key, uint8_t value)
{
	if (key < ByteSettingsKeyCount)
	{
		eeprom_update_byte(&eeByteSettings[key], value);
		change_flags |= SettingsByte;
	}
}

void settings_set_word(uint8_t key, uint16_t value)
{
	if (key < WordSettingsKeyCount)
	{
		eeprom_update_word(&eeWordSettings[key], value);
		change_flags |= SettingsWord;
	}
}

void settings_set_array(uint8_t key, uint8_t size, const uint8_t *value)
{
	if (key < ArraySettingsKeyCount && size <= ARRAY_SIZE)
	{
		eeprom_update_block((const void*) value, (void*) &eeArraySettings[key], size);
		change_flags |= SettingsArray;
	}
}

uint8_t settings_get_byte(uint8_t key)
{
	if (key < ByteSettingsKeyCount)
	{
		return eeprom_read_byte(&eeByteSettings[key]);
	}

	return 0;
}

uint16_t settings_get_word(uint8_t key)
{
	if (key < WordSettingsKeyCount)
	{
		return eeprom_read_word(&eeWordSettings[key]);
	}

	return 0;
}

void settings_get_array(uint8_t key, uint8_t max_size, uint8_t *value)
{
	if (key < ArraySettingsKeyCount && max_size <= ARRAY_SIZE)
	{
		eeprom_read_block((void*) value, (const void*) &eeArraySettings[key], max_size);
	}
}

uint8_t settings_changed()
{
	const uint8_t flags = change_flags;
	change_flags = 0x00;
	return flags;
}
