#include "dockingforcemessage.h"
#include <endian.h>

DockingForceMessage::DockingForceMessage(quint8 seq, quint16 voltageDMS, quint16 voltageRef) :
	AbstractMessage(seq)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	m_data[0] = quint8((voltageDMS & 0xFF00) >> 8);
	m_data[1] = quint8(voltageDMS & 0x00FF);
	m_data[2] = quint8((voltageRef & 0xFF00) >> 8);
	m_data[3] = quint8(voltageRef & 0x00FF);
#else
	m_data[1] = quint8((voltageDMS & 0xFF00) >> 8);
	m_data[0] = quint8(voltageDMS & 0x00FF);
	m_data[3] = quint8((voltageRef & 0xFF00) >> 8);
	m_data[0] = quint8(voltageRef & 0x00FF);
#endif
}

DockingForceMessage::DockingForceMessage(const QByteArray &rawData) :
	AbstractMessage((rawData.size() == 8) ? static_cast<quint8>(rawData.at(2)) : 0)
{
	if (rawData.size() == 8 && static_cast<quint8>(rawData.at(0)) == identifier())
	{
		m_data.append(rawData.mid(3, 4));

		if (checksum() != static_cast<quint8>(rawData.at(7)))
		{
			m_data.clear(); // checksum missmatch -> invalidate message
		}
	}
}

bool DockingForceMessage::isValid() const
{
	return (m_data.size() == 4);
}

quint16 DockingForceMessage::voltageDMS() const
{
	if (isValid())
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		quint16 dms = ((static_cast<quint8>(m_data.at(0)) << 8) | (static_cast<quint8>(m_data.at(1))));
#else
		quint16 dms = ((static_cast<quint8>(m_data.at(0))) | (static_cast<quint8>(m_data.at(1)) << 8));
#endif
		return dms;
	}

	return 0;
}

quint16 DockingForceMessage::voltageRef() const
{
	if (isValid())
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		quint16 ref = ((static_cast<quint8>(m_data.at(2)) << 8) | (static_cast<quint8>(m_data.at(3))));
#else
		quint16 ref = ((static_cast<quint8>(m_data.at(2))) | (static_cast<quint8>(m_data.at(3)) << 8));
#endif
		return ref;
	}

	return 0;
}
