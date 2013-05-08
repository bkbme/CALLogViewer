#ifndef ERRORMESSAGE_H
#define ERRORMESSAGE_H

#include <abstractmessage.h>

class ErrorMessage : public AbstractMessage
{
public:
	enum ErrorCode
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
	};

	ErrorMessage(quint8 seq, ErrorCode errorCode, quint8 seqError = 0);
	ErrorMessage(const QByteArray &rawData);

	quint8 identifier() const { return 0x02; }
	bool isValid() const;

	ErrorCode errorCode() const;
	quint8 errorSeq() const;
};

#endif // ERRORMESSAGE_H
