#include "servoctrlmessage.h"

const quint8 SERVO_POS_CENTER = 127;

ServoCtrlMessage::ServoCtrlMessage(quint8 seq, int id, int position) :
	AbstractMessage(seq)
{
	quint8 pos = static_cast<quint8>(position + SERVO_POS_CENTER);
	m_data.append(static_cast<quint8>(id));
	m_data.append(pos);
}

bool ServoCtrlMessage::isValid() const
{
	if (m_data.size() == 2)
	{
		quint8 id = static_cast<quint8>(m_data.at(0));
		return (id == 0 || id == 1);
	}

	return false;
}

int ServoCtrlMessage::position() const
{
	return (~(static_cast<quint8>(isValid() ? m_data.at(1) : SERVO_POS_CENTER)) - static_cast<quint8>(~SERVO_POS_CENTER));
}
