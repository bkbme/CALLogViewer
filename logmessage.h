#ifndef LOGMESSAGE_H
#define LOGMESSAGE_H

#include <QString>
#include <QDateTime>

class LogMessage
{
public:
	enum LogLevel
	{
		Error,
		Warning,
		Info,
		StateMachine,
		SoapIn,
		SoapOut,
		Debug,
		NonCAL,
		Unknown
	};

    enum LogFacility
    {
        InscriptFw,
        CAL,
        Kernel,
        Other
    };

	LogMessage();
    LogMessage(LogLevel level, LogFacility facility, const QDateTime &timestamp, const QString &application, const QString &message);

public:
	unsigned int id() const { return m_id; }
	LogLevel level() const { return m_level; }
    LogFacility facility() const { return m_facility; }
	QDateTime timestamp() const { return m_timestamp; }
	QString application() const { return m_app; }
	QString message() const { return m_msg; }
	QString messageHtml() const;

private:
	unsigned int m_id;
	LogLevel m_level;
	QString m_msg;
	QString m_app;
    LogFacility m_facility;
	QDateTime m_timestamp;
};

#endif // LOGMESSAGE_H
