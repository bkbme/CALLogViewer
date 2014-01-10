#include "abstractmessage.h"

AbstractMessage::AbstractMessage(quint8 seq) :
	m_seq(seq),
	m_timeSent(),
	m_data()
{
}

AbstractMessage::AbstractMessage(const QByteArray &rawData) :
	m_seq(rawData.size() > 3 ? rawData.at(2) : 0),
	m_timeSent(),
	m_data()
{
	if (rawData.size() > 4 && rawData.size() == rawData.at(1))
	{
		m_data.append(rawData.mid(3, rawData.size() - 4));
	}
}

quint8 AbstractMessage::size() const
{
	return (m_data.size() + 4); // 4Byte = identifier + size + sequence + checksum
}

quint8 AbstractMessage::sequence() const
{
	return m_seq;
}

quint8 AbstractMessage::checksum() const
{
	quint8 sum = identifier();
	sum += size();
	sum += m_seq;

	for (int i = 0; i < m_data.size(); ++i)
	{
		sum += static_cast<quint8>(m_data.at(i));
	}

	return (0xFF - sum);
}

QByteArray AbstractMessage::data() const
{
	return m_data;
}

QByteArray AbstractMessage::toByteArray() const
{
	QByteArray a;
	a.reserve(size());
	a.append(identifier());
	a.append(size());
	a.append(m_seq);
	a.append(m_data);
	a.append(checksum());
	return a;
}

void AbstractMessage::markAsSent()
{
	m_timeSent = QTime::currentTime();
}

qint64 AbstractMessage::msecsSinceSent() const
{
	if (m_timeSent.isValid())
	{
		qint64 msecs = m_timeSent.msecsTo(QTime::currentTime());
		return ((msecs < 0) ? (86400000 + msecs) : msecs);
	}

	return -1;
}
