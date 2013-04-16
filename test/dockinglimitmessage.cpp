#include "dockinglimitmessage.h"

const quint16 DOCKING_RANGE_MIN = 0;
const quint16 DOCKING_RANGE_MAX = 1023;

DockingLimitMessage::DockingLimitMessage(quint8 seq, uint lowerLimit, uint upperLimit) :
	AbstractMessage(seq)
{
	if (lowerLimit >= DOCKING_RANGE_MIN && lowerLimit <= DOCKING_RANGE_MAX &&
		upperLimit >= DOCKING_RANGE_MIN && upperLimit <= DOCKING_RANGE_MAX)
	{
		m_data.append(static_cast<quint8>((lowerLimit & 0xFF00) >> 8));
		m_data.append(static_cast<quint8>(lowerLimit & 0x00FF));
		m_data.append(static_cast<quint8>((upperLimit & 0xFF00) >> 8));
		m_data.append(static_cast<quint8>(upperLimit & 0x00FF));
	}
}


bool DockingLimitMessage::isValid() const
{
	quint16 upper = upperLimit();
	quint16 lower = lowerLimit();

	return (upper <= DOCKING_RANGE_MAX && upper >= DOCKING_RANGE_MIN &&
			lower <= DOCKING_RANGE_MAX && lower >= DOCKING_RANGE_MIN);
}

uint DockingLimitMessage::lowerLimit() const
{
	if (m_data.size() == 4)
	{
		return (static_cast<quint8>(m_data.at(0) << 8) | static_cast<quint8>(m_data.at(1)));
	}

	return 0;
}

uint DockingLimitMessage::upperLimit() const
{
	if (m_data.size() == 4)
	{
		return (static_cast<quint8>(m_data.at(2) << 8) | static_cast<quint8>(m_data.at(3)));
	}

	return 0;
}
