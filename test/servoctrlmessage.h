#ifndef SERVOCTRLMESSAGE_H
#define SERVOCTRLMESSAGE_H

#include <abstractmessage.h>

class ServoCtrlMessage : public AbstractMessage
{
public:
	ServoCtrlMessage(quint8 seq, int position);

	quint8 identifier() const { return 0x0A; }
	bool isValid() const;

	int position() const;
};

#endif // SERVOCTRLMESSAGE_H
