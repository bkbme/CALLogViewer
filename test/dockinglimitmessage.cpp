#include "dockinglimitmessage.h"

DockingLimitMessage::DockingLimitMessage(quint8 seq, int lowerLimit, int upperLimit) :
	AbstractMessage(seq)
{
	qint16 lower = static_cast<qint16>(lowerLimit * 10);
	qint16 upper = static_cast<qint16>(upperLimit * 10);
	m_data.append(static_cast<quint8>((lower & 0xFF00) >> 8));
	m_data.append(static_cast<quint8>(lower & 0x00FF));
	m_data.append(static_cast<quint8>((upper & 0xFF00) >> 8));
	m_data.append(static_cast<quint8>(upper & 0x00FF));
}


bool DockingLimitMessage::isValid() const
{
	return (m_data.size() == 4);
}

int DockingLimitMessage::lowerLimit() const
{
	if (m_data.size() == 4)
	{
		int limit = (static_cast<quint8>(m_data.at(0) << 8) | static_cast<quint8>(m_data.at(1))) * 10;
		return limit;
	}

	return 0;
}

int DockingLimitMessage::upperLimit() const
{
	if (m_data.size() == 4)
	{
		int limit = (static_cast<quint8>(m_data.at(2) << 8) | static_cast<quint8>(m_data.at(3))) * 10;
		return limit;
	}

	return 0;
}
