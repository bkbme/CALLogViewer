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
	if (rawData.size() == 5 || rawData.size() == 6)
	{
		m_data.append(rawData.mid(4, rawData.size() - 4));
	}

	if (rawData.at(0) != identifier() || static_cast<quint8>(rawData.at(rawData.size() - 1)) != checksum())
	{
		m_data.clear(); // invalidate message
	}
}

bool ErrorMessage::isValid() const
{
	return m_data.size() > 0;
}

quint8 ErrorMessage::errorCode() const
{
	if (m_data.size())
	{
		return static_cast<quint8>(m_data.at(0));
	}

	return 0;
}

quint8 ErrorMessage::errorSeq() const
{
	if (m_data.size() > 1)
	{
		return static_cast<quint8>(m_data.at(1));
	}

	return 0;
}
