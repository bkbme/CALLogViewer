#include "ackmessage.h"

#include <QByteArray>

AckMessage::AckMessage(quint8 seq, quint8 seqAck) :
	AbstractMessage(seq)
{
	m_data.append(seqAck);
}

AckMessage::AckMessage(const QByteArray &rawData) :
	AbstractMessage(rawData.size() == 5 ? rawData.at(2) : 0)
{
	if (rawData.size() == 5)
	{
		if (rawData.at(0) == identifier())
		{
			m_data.append(rawData.at(3));
		}

		if (checksum() != static_cast<quint8>(rawData.at(4)))
		{
			m_data.clear(); // invalidate message
		}
	}
}

bool AckMessage::isValid() const
{
	return (m_data.size() == 1);
}

quint8 AckMessage::acknowledgedSeq() const
{
	if (m_data.size() == 1)
	{
		return static_cast<quint8>(m_data.at(0));
	}

	return 0;
}
