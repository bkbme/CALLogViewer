#ifndef DOCKINGLIMITMESSAGE_H
#define DOCKINGLIMITMESSAGE_H

#include <abstractmessage.h>

class DockingLimitMessage : public AbstractMessage
{
public:
	DockingLimitMessage(quint8 seq, int lowerLimit, int upperLimit);

	quint8 identifier() const { return 0x08; }
	bool isValid() const;

	int lowerLimit() const;
	int upperLimit() const;
};

#endif // DOCKINGLIMITMESSAGE_H
