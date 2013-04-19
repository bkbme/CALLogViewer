#include "servoctrlmessage.h"

const quint8 SERVO_POS_MIN = 40;
const quint8 SERVO_POS_MAX = 200;
const quint8 SERVO_POS_STOP = 116;

ServoCtrlMessage::ServoCtrlMessage(quint8 seq, int position) :
	AbstractMessage(seq)
{
	quint8 pos = ~(static_cast<quint8>(position + static_cast<quint8>(~SERVO_POS_STOP)));
	if (pos >= SERVO_POS_MIN && pos <= SERVO_POS_MAX)
	{
		m_data.append(pos);
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
	return (~(static_cast<quint8>(isValid() ? m_data.at(1) : SERVO_POS_STOP)) - static_cast<quint8>(~SERVO_POS_STOP));
}
