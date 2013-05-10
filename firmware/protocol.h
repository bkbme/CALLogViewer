/*
 * protocol.h
 *
 * Created: 4/5/2013 2:06:33 PM
 *  Author: mlohse
 */ 


#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>

typedef enum
{
	StateDisconnected = 0,
	StateConnected = 1,
	StateUnknown = 2
} ConnectionState;

typedef enum
{
	ErrorUnknown = 0,
	ErrorDisconnect = 1,
	ErrorSequence = 2,
	ErrorVersion = 3,
	ErrorParser = 4,
	ErrorTimeout = 5,
	ErrorUnhandled = 6,
	DockCommunicationError = 7,
	DockTimeoutError = 8
} ErrorCode; // keep in sync with CALLogView's "errormessage.cpp"

void process_messages();

void send_docking_force(uint16_t force, uint8_t steady);
void send_docking_state(uint8_t state);
void send_error(ErrorCode code, uint8_t seq);
void send_ack(uint8_t seq);
void send_version();

ConnectionState connection_state();


#endif /* PROTOCOL_H_ */