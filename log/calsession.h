#ifndef CALSESSION_H
#define CALSESSION_H

#include <logmessage.h>

#include <QList>
#include <QString>
#include <QObject>
#include <QDateTime>

class CALSession : public QObject
{
	Q_OBJECT

public:
	explicit CALSession(QObject *parent = 0);

	void setExitCode(int code) { m_exitCode = code; }
	void setStart(const QDateTime &start) { m_start = start; }
	void setEnd(const QDateTime &end) { m_end = end; }
	void setEsdErrorCode(int errorCode) { m_esdCode =  errorCode; }
	void setEsdErrorStr(const QString & errorStr) { m_esdStr = errorStr; }
	void setMessages(const QList<LogMessage> &messages) { m_messages = messages; }

	int exitCode() const { return m_exitCode; }
	QDateTime start() const { return m_start; }
	QDateTime end() const { return m_end; }
	int esdErrorCode() const { return m_esdCode; }
	QString esdErrorStr() const { return m_esdStr; }

	const QList<LogMessage>& messages() const { return m_messages; }
	QList<LogMessage>& messages() { return m_messages; }

	uint durationSecs() const;

public slots:
	void appendMessage(const LogMessage &msg);

signals:
	void messageAppended(const LogMessage &msg);

private:
	int m_exitCode;
	int m_esdCode;
	QString m_esdStr;
	QDateTime m_start;
	QDateTime m_end;
	QList<LogMessage> m_messages;
};

#endif // CALSESSION_H
