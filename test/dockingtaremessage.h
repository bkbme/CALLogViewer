#ifndef DOCKINGTAREMESSAGE_H
#define DOCKINGTAREMESSAGE_H

#include <abstractmessage.h>
#include <messageparser.h>

class DockingTareMessage : public AbstractMessage
{
public:
	DockingTareMessage(quint8 seq = 0);

	quint8 identifier() const { return MessageParser::IdDockingTareMessage; }
	bool isValid() const { return true; }
};

#endif // DOCKINGTAREMESSAGE_H
