#ifndef INITMESSAGE_H
#define INITMESSAGE_H

#include <abstractmessage.h>
#include <messageparser.h>

class InitMessage : public AbstractMessage
{
public:
	InitMessage(quint8 seq);

	quint8 identifier() const { return MessageParser::IdInitMessage; }
	bool isValid() const;
};

#endif // INITMESSAGE_H
