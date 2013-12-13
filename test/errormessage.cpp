#include "errormessage.h"

#include <QByteArray>

ErrorMessage::ErrorMessage(quint8 seq, ErrorCode errorCode, quint8 seqError) :
	AbstractMessage(seq)
{
	m_data.append(static_cast<quint8>(errorCode)).append(seqError);
}

ErrorMessage::ErrorMessage(const QByteArray &rawData) :
	AbstractMessage(rawData.size() > 4 ? static_cast<quint8>(rawData.at(2)) : 0)
{
	if (rawData.size() > 4 && rawData.at(0) == identifier() && (static_cast<quint8>(rawData.at(rawData.size() - 1)) != checksum()))
	{
		m_data.append(rawData.mid(3, rawData.size() - 4));
	}
}

bool ErrorMessage::isValid() const
{
	return m_data.size() > 0;
}

ErrorMessage::ErrorCode ErrorMessage::errorCode() const
{
	if (m_data.size())
	{
		switch (m_data.at(0))
		{
			case ErrorDisconnect: return ErrorDisconnect;
			case ErrorSequence: return ErrorSequence;
			case ErrorVersion: return ErrorVersion;
			case ErrorParser: return ErrorParser;
			case ErrorTimeout: return ErrorTimeout;
			case ErrorUnhandled: return ErrorUnhandled;
			case DockCommunicationError: return DockCommunicationError;
			case DockTimeoutError: return DockTimeoutError;
			default: return ErrorUnknown;
		}
	}

	return ErrorUnknown;
}

quint8 ErrorMessage::errorSeq() const
{
	if (m_data.size() > 1)
	{
		return static_cast<quint8>(m_data.at(1));
	}

	return 0;
}
