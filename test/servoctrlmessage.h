#ifndef SERVOCTRLMESSAGE_H
#define SERVOCTRLMESSAGE_H

#include <abstractmessage.h>
#include <messageparser.h>

class ServoCtrlMessage : public AbstractMessage
{
public:
	ServoCtrlMessage(quint8 seq, int id, int position = -127);

	quint8 identifier() const { return MessageParser::IdServoCtrlMessage; }
	bool isValid() const;

	int position() const;

	static int posHw2PosInt(quint8 hwPos);
	static quint8 posInt2PosHw(int pos);
};

#endif // SERVOCTRLMESSAGE_H
