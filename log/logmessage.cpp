#include "logmessage.h"

#include <QTextStream>

static unsigned int s_id = 0;

LogMessage::LogMessage() :
	m_level(Unknown),
	m_msg(),
	m_app(),
	m_timestamp()
{
}

LogMessage::LogMessage(LogLevel level, LogFacility facility, const QDateTime &timestamp, const QString &application, const QString &message) :
	m_id(s_id++),
	m_level(level),
	m_msg(message),
	m_app(application),
	m_timestamp(timestamp),
	m_facility(facility)
{
}

QString LogMessage::toString() const
{
	QString str;
	QTextStream oStream(&str, QIODevice::WriteOnly);

	oStream << m_timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz ") << m_app << ": ";

	switch(m_level)
	{
		case LogMessage::Error:
			oStream << "E: ";
			break;
		case LogMessage::Warning:
			oStream << "W: ";
			break;
		case LogMessage::Info:
			oStream << "N: ";
			break;
		case LogMessage::StateMachine:
			oStream << "S: ";
			break;
		case LogMessage::SoapIn:
			oStream << "I: ";
			break;
		case LogMessage::SoapOut:
			oStream << "O: ";
			break;
		case LogMessage::Debug:
			oStream << "D: ";
			break;
		default:
			break;
	}

	oStream << m_msg;
	return str;
}

