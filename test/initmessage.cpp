#include "initmessage.h"

InitMessage::InitMessage(quint8 seq) :
	AbstractMessage(seq)
{
}

bool InitMessage::isValid() const
{
	return m_data.isEmpty();
}
