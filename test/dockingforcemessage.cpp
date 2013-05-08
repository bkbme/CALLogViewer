#include "dockingforcemessage.h"

DockingForceMessage::DockingForceMessage(quint8 seq, qreal zForce, bool isSteady) :
	AbstractMessage(seq)
{
	qint16 force = static_cast<quint16>(zForce * 10);
	m_data[0] = quint8((force & 0xFF00) >> 8);
	m_data[1] = quint8(force & 0x00FF);
	m_data[2] = (isSteady ? 1 : 0);
}

DockingForceMessage::DockingForceMessage(const QByteArray &rawData) :
	AbstractMessage((rawData.size() == 7) ? static_cast<quint8>(rawData.at(2)) : 0)
{
	if (rawData.size() == 7 && static_cast<quint8>(rawData.at(0)) == identifier())
	{
		m_data.append(rawData.mid(3, 3));

		if (checksum() != static_cast<quint8>(rawData.at(6)))
		{
			m_data.clear(); // checksum missmatch -> invalidate message
		}
	}
}

bool DockingForceMessage::isValid() const
{
	return (m_data.size() == 3);
}

qreal DockingForceMessage::zForce() const
{
	if (isValid())
	{
		qint16 force = (static_cast<quint8>(m_data.at(0)) << 8) | (static_cast<quint8>(m_data.at(1)));
		return (static_cast<qreal>(force) / 10);
	}

	return 0;
}

bool DockingForceMessage::isSteady() const
{
	if (isValid())
	{
		return (static_cast<quint8>(m_data.at(2)) == 1);
	}

	return false;
}
