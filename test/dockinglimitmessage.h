#ifndef DOCKINGLIMITMESSAGE_H
#define DOCKINGLIMITMESSAGE_H

#include <abstractmessage.h>

class DockingLimitMessage : public AbstractMessage
{
public:
	DockingLimitMessage(quint8 seq, uint lowerLimit, uint upperLimit);

	quint8 identifier() const { return 0x08; }
	bool isValid() const;

	uint lowerLimit() const;
	uint upperLimit() const;
};

#endif // DOCKINGLIMITMESSAGE_H
