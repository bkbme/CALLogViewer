#ifndef SYSLOGLOADER_H
#define SYSLOGLOADER_H

#include "logmessage.h"
#include <QObject>
#include <QString>
#include <QUrl>

class QIODevice;
class QNetworkAccessManager;

class SysLogLoader : public QObject
{
	Q_OBJECT
public:
	explicit SysLogLoader(QNetworkAccessManager *netMgr, QObject *parent = 0);

	LogMessage parseLogMessage(const QString &message) const;
	bool isLogOpen() const;
	bool canHandleTarArchive() const;

signals:
	void statusMessage(const QString &msg, int timeout);
	void newLogMessage(const LogMessage &log);
	void logClosed();
	void logOpened();

public slots:
	void openLog(const QUrl &url);
	void openLog(const QString &filename);
	void closeLog();

private slots:
	void onReadyRead();

private:
	QNetworkAccessManager *m_net;
	QIODevice *m_log;
};

#endif // SYSLOGLOADER_H
