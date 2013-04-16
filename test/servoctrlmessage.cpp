#include "servoctrlmessage.h"

const quint8 SERVO_POS_MIN = 60;
const quint8 SERVO_POS_MAX = 140;

ServoCtrlMessage::ServoCtrlMessage(quint8 seq, int position) :
	AbstractMessage(seq)
{
	int pos = position + 127;
	if (pos >= SERVO_POS_MIN && pos <= SERVO_POS_MAX)
	{
		m_data.append(static_cast<quint8>(pos));
	}
}

bool ServoCtrlMessage::isValid() const
{
	if (m_data.size() == 1)
	{
		quint8 pos = static_cast<quint8>(m_data.at(1));
		return (pos <= SERVO_POS_MAX && pos >= SERVO_POS_MIN);
	}

	return false;
}

int ServoCtrlMessage::position() const
{
	return (isValid() ? static_cast<quint8>(m_data.at(1)) - 127 : 0);
}
