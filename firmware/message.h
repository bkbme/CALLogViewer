/*
 * MessageParser.h
 *
 * Created: 4/4/2013 1:24:32 PM
 *  Author: mlohse
 */ 

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <stdint.h>

#define MSG_DATA_SIZE_MAX 8

typedef enum
{
	IdAckMessage = 0x01,
	IdErrorMessage = 0x02,
	IdInitMessage = 0x03,
	IdVersionMessage = 0x04,
	IdFootSwitchMessage = 0x05,
	IdInterlockOpenMessage = 0x06,
	IdInterlockResetMessage = 0x07,
	IdDockingLimitMessage = 0x08,
	IdDockingForceMessage = 0x09,
	IdServoCtrlMessage = 0x0A,
	IdDockingTareMessage = 0x0B,
	IdDockingStateMessage = 0x0C,
	IdInvalidIdentifier = 0xFF // must remain at end for parsing!!!
} MessageIdentifier;

typedef enum
{
	ParseOk = 0,
	ParseNotEnoughData,
	ParseInvalidSizeError,
	ParseInvalidIdentifier,
	ParseChecksumError
} MessageParseResult;

typedef struct
{
	MessageIdentifier identifier;
	uint8_t data_size;
	uint8_t sequence;
	uint8_t data[MSG_DATA_SIZE_MAX];
} Message;

Message message;

MessageParseResult message_parse();

#endif /* MESSAGE_H_ */