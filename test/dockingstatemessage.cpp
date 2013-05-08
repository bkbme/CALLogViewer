#include "dockingstatemessage.h"

#include <QByteArray>

DockingStateMessage::DockingStateMessage(quint8 seq, DockingState state) :
	AbstractMessage(seq)
{
	m_data.append(state);
}

DockingStateMessage::DockingStateMessage(const QByteArray &rawData) :
	AbstractMessage((rawData.size() == 5) ? static_cast<quint8>(rawData.at(2)) : 0)
{
	if (rawData.size() == 5 && static_cast<quint8>(rawData.at(0)) == identifier())
	{
		m_data.append(rawData.at(3));

		if (checksum() != static_cast<quint8>(rawData.at(4)))
		{
			m_data.clear(); // checksum missmatch -> invalidate message
		}
	}
}

bool DockingStateMessage::isValid() const
{
	return (m_data.size() == 1);
}

DockingStateMessage::DockingState DockingStateMessage::state() const
{
	if (m_data.size() > 0)
	{
		switch (static_cast<quint8>(m_data.at(0)))
		{
			case DockedManually: return DockedManually;
			case DockedBottom: return DockedBottom;
			case DockedTop: return DockedTop;
			case DockedLimit: return DockedLimit;
			case DockMovingUp: return DockMovingUp;
			case DockMovingDown: return DockMovingDown;
			default: return DockError;
		}
	}

	return DockError;
}
