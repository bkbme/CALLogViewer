#ifndef MESSAGEPARSER_H
#define MESSAGEPARSER_H

#include <abstractmessage.h>

class QByteArray;

class MessageParser
{
public:
	enum MessageIdentifier
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
	};

	explicit MessageParser(QByteArray *readBuffer);

	AbstractMessage* nextMessage();

private:
	QByteArray *m_buffer;
};

#endif // MESSAGEPARSER_H
