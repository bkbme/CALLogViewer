#ifndef SYSLOGLOADER_H
#define SYSLOGLOADER_H

#include <logmessage.h>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QStringList>

class QIODevice;
class QNetworkAccessManager;

class SysLogParser : public QObject
{
	Q_OBJECT
public:
	explicit SysLogParser(QNetworkAccessManager *netMgr, QObject *parent = 0);

	LogMessage parseLogMessage(const QString &message);
	bool isLogOpen() const;
	bool isLogContinuous() const;
	static bool canHandleTarArchive();
	void openSupportInfo(const QString &supportInfoFile, const QStringList &selectedLogFiles);

signals:
	void statusMessage(const QString &msg, int timeout);
	void newLogMessage(const LogMessage &log);
	void logClosed();
	void logOpened(bool continuous);

	void calStarted();
	void calStopped(int exitCode);
	void treatmentStarted();
	void treatmentFinished();
	void treatmentAborted();
	void executingTreatment();
	void powerCheckStarted();
	void powerCheckStopped();
//	void procShutterOpened();
	void emergencyShutdown(int code, const QString &msg);

public slots:
	void openLog(const QUrl &url);
	void openLog(const QString &filename);
	void closeLog();

private slots:
	void onReadyRead();
	void loadNextLog();
	void analyzeMessage(const LogMessage &msg);

private:
	QNetworkAccessManager *m_net;
	QIODevice *m_log;
	QString m_supportInfoFile;
	QStringList m_filesToLoadList;
};

#endif // SYSLOGLOADER_H
