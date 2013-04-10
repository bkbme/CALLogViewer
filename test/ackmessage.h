#ifndef ACKMESSAGE_H
#define ACKMESSAGE_H

#include <abstractmessage.h>

class AckMessage : public AbstractMessage
{
public:
	AckMessage(quint8 seq, quint8 seqAck);
	AckMessage(const QByteArray &rawData);

	quint8 identifier() const { return 0x01; }
	bool isValid() const;

	quint8 acknowledgedSeq() const;
};

#endif // ACKMESSAGE_H
