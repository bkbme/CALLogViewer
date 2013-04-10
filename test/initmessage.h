#ifndef INITMESSAGE_H
#define INITMESSAGE_H

#include <abstractmessage.h>

class InitMessage : public AbstractMessage
{
public:
	InitMessage(quint8 seq);

	quint8 identifier() const { return 0x03; }
	bool isValid() const;
};

#endif // INITMESSAGE_H
