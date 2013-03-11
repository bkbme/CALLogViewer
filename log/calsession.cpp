#include "calsession.h"

CALSession::CALSession(QObject *parent) :
	QObject(parent),
	m_exitCode(-1),
	m_esdCode(-1),
	m_esdStr(),
	m_start(),
	m_end(),
	m_messages()
{
}

void CALSession::appendMessage(const LogMessage &msg)
{
	m_messages << msg;
	emit messageAppended(msg);
}

uint CALSession::durationSecs() const
{
	if (!m_start.isValid() || !m_end.isValid() || m_start > m_end)
	{
		return 0;
	}

	return m_start.secsTo(m_end);
}
