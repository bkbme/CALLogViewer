/*
 * settings.h
 *
 * Created: 10/16/2013 2:21:17 PM
 *  Author: mlohse
 */ 


#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <stdint.h>

typedef enum					// must be kept in sync with enum in SettingsMessage class, used as flags for settings_changed()
{
	SettingsByte		= 0x10,
	SettingsWord		= 0x20,
	SettingsArray	= 0x40
} SettingsType;

typedef enum					// must be kept in sync with enum in FemtoTester class
{							// used as index for eeByteSettings!
	KeyDockServoUp 			= 0x00,
	KeyDockServoDown			= 0x01,
	KeyDockServoUpSlow		= 0x02,
	KeyDockServoDownSlow		= 0x03,
	KeyDockSlotCount			= 0x04,
	ByteSettingsKeyCount		= 0x05 // keep at end!
} ByteSettingsKeys;

typedef enum					// must be kept in sync with enum in FemtoTester class
{							// used as index for eeWordSettings!
	KeyDockLimitZeroLo		= 0x00,
	KeyDockLimitZeroUp		= 0x01,
	KeyDockLimitSoftLo		= 0x02,
	KeyDockLimitSoftUp		= 0x03,
	KeyDockLimitRegularLo	= 0x04,
	KeyDockLimitRegularUp	= 0x05,
	WordSettingsKeyCount		= 0x06 // keep at end!
} KeyWordSettings;

typedef enum					// must be kept in sync with enum in FemtoTester class
{							// used as index for eeArraySettings!
	KeyDockSlots1			= 0x00,
	KeyDockSlots2			= 0x01,
	ArraySettingsKeyCount	= 0x02 // keep at end!
} ArraySettingsKeys;


void settings_set_byte(uint8_t key, uint8_t value);
void settings_set_word(uint8_t key, uint16_t value);
void settings_set_array(uint8_t key, uint8_t size, const uint8_t *value);

uint8_t settings_get_byte(uint8_t key);
uint16_t settings_get_word(uint8_t key);
void settings_get_array(uint8_t key, uint8_t max_size, uint8_t *value);

uint8_t settings_changed();

#endif /* SETTINGS_H_ */