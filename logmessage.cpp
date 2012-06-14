#include "logmessage.h"

static unsigned int s_id = 0;

LogMessage::LogMessage() :
	m_level(Unknown),
	m_msg(),
	m_app(),
	m_timestamp()
{
}

LogMessage::LogMessage(LogLevel level, const QDateTime &timestamp, const QString &application, const QString &message) :
	m_id(s_id++),
	m_level(level),
	m_msg(message),
	m_app(application),
	m_timestamp(timestamp)
{
}

QString LogMessage::messageHtml() const
{
	return QString(m_msg).replace("&","&amp;").replace(">", "&gt;").replace("<", "&lt;");
}
