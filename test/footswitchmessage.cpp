#include "footswitchmessage.h"

FootswitchMessage::FootswitchMessage(quint8 seq, ProcedureFootswitch::FootswitchState state) :
	AbstractMessage(seq)
{
	m_data.append(static_cast<unsigned char>(state));
}

bool FootswitchMessage::isValid() const
{
	return m_data.size() == 1;
}

ProcedureFootswitch::FootswitchState FootswitchMessage::state() const
{
	if (isValid())
	{
		switch (m_data.at(0))
		{
			case ProcedureFootswitch::FSIntermediate: return ProcedureFootswitch::FSIntermediate;
			case ProcedureFootswitch::FSPedalDown: return ProcedureFootswitch::FSPedalDown;
			case ProcedureFootswitch::FSPedalUp: return ProcedureFootswitch::FSPedalUp;
			default:
				break;
		}
	}

	return ProcedureFootswitch::FSError;
}
