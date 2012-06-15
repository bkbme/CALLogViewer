#include "syslogloader.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>
#include <QDateTime>
#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

#ifdef Q_WS_WIN
QString TAR_EXEC = "tar.exe";
QString GZIP_EXEC = "gzip.exe";
#else
	const QString TAR_EXEC = "/bin/tar";
#endif

SysLogLoader::SysLogLoader(QNetworkAccessManager *netMgr, QObject *parent) :
	QObject(parent),
	m_net(netMgr),
    m_log(0)
{
#ifdef Q_WS_WIN
        TAR_EXEC = QCoreApplication::applicationDirPath() + QDir::separator() + TAR_EXEC;
        GZIP_EXEC = QCoreApplication::applicationDirPath() + QDir::separator() + GZIP_EXEC;
#endif
}

void SysLogLoader::openLog(const QUrl &url)
{
	if (m_log)
	{
		qDebug() << "Log already open - call closeLog() first!";
		return;
	}

	QNetworkReply *reply = m_net->get(QNetworkRequest(url));
	connect(reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(reply, SIGNAL(finished()), this, SLOT(closeLog()));
	m_log = reply;

	emit logOpened();
	emit statusMessage(QString("Connected to %1").arg(url.host()), 5000);
}

void SysLogLoader::openLog(const QString &filename)
{
	if (m_log)
	{
		qDebug() << "Log already open - call closeLog() first!";
		return;
	}

	emit statusMessage(QString("Reading file %1...").arg(filename), 0);
	if (filename.endsWith(".tar.gz"))
	{
		// extract and parse 'var/log/messages' from support info archive
		if (!canHandleTarArchive())
		{
			qDebug() << "Failed to extract support info archive: '" << TAR_EXEC << "'' doesn't exist";
			if (!QFile::exists(TAR_EXEC))
			{
				statusMessage(QString("Failed to extract support info archive: %1 doesn't exist!").arg(TAR_EXEC), 0);
			}
#ifdef Q_WS_WIN
			if (!QFile::exists(GZIP_EXEC))
			{
				statusMessage(QString("Failed to extract support info archive: %1 doesn't exist!").arg(GZIP_EXEC), 0);
			}
#endif
			return;
		}

		QStringList args;
		QProcess *tar = new QProcess(this);
		connect(tar, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
		connect(tar, SIGNAL(finished(int)), this, SLOT(closeLog()));

#ifdef Q_WS_WIN
		args << "Ox";

		QStringList gzip_args;
		gzip_args << "-dc" << filename;
		QProcess *gzip = new QProcess(tar);
		gzip->setStandardOutputProcess(tar); // tar.exe doesn't support on-the-fly gzip extraction :(
		gzip->start(GZIP_EXEC, gzip_args);
#else
		args << "Oxf" << filename;
#endif
		args << "var/log/messages";
		tar->start(TAR_EXEC, args);
		m_log = tar;
		emit logOpened();
		return;
	}

	// parse regular file
	QFile *logFile = new QFile(filename, this);
	m_log = logFile;
	if (logFile->open(QIODevice::ReadOnly))
	{
		qDebug() << "file opened: " << filename;
		emit logOpened();
		onReadyRead();
	}
	closeLog();
}

void SysLogLoader::closeLog()
{
	if (!m_log)
	{
		qDebug() << "Log not open yet - nothing to do";
		return;
	}

	QNetworkReply *reply = qobject_cast<QNetworkReply*>(m_log);
	if (reply)
	{
		statusMessage(QString("Disconnected from %1").arg(reply->request().url().host())
					  .append((reply->error() == QNetworkReply::NoError ? "" : ": " + reply->errorString())), 0);
		reply->disconnect();
		reply->abort();
	}

	m_log->close();
	m_log->deleteLater();
	m_log = 0;

	emit logClosed();
	if (!reply)
	{
		emit statusMessage("done", 3000);
	}
}

void SysLogLoader::onReadyRead()
{
	while (m_log && !m_log->atEnd() && (!m_log->isSequential() || m_log->canReadLine()))
	{
		emit newLogMessage(parseLogMessage(m_log->readLine()));
	}
}

LogMessage SysLogLoader::parseLogMessage(const QString &message) const
{
	LogMessage::LogLevel level = LogMessage::NonCAL;
    LogMessage::LogFacility facility = LogMessage::Other;
	QDateTime timestamp(QDateTime::currentDateTime());
	QString host, app, msg;

	QStringList msgChunks(message.split(' '));
	// parse message
	if (msgChunks.count() > 3)
	{
		timestamp = QDateTime::fromString(msgChunks.takeFirst().mid(0, 23), "yyyy-MM-ddTHH:mm:ss.zzz");
		host = msgChunks.takeFirst();
		app = msgChunks.takeFirst().remove(":");

        if (app.contains("fw[")) {
            facility = LogMessage::InscriptFw;
        } else if(app.contains("CAL")) {
            facility = LogMessage::CAL;
        } else if(app.contains("kernel")) {
            facility = LogMessage::Kernel;
        }

		if (app == "CAL")
		{
			// parse loglevel
			switch (msgChunks.takeFirst().at(0).toAscii())
			{
				case 'E':
					level = LogMessage::Error;
					break;
				case 'W':
					level = LogMessage::Warning;
					break;
				case 'N':
					level = LogMessage::Info;
					break;
				case 'S':
					level = LogMessage::StateMachine;
					break;
				case 'I':
					level = LogMessage::SoapIn;
					break;
				case 'O':
					level = LogMessage::SoapOut;
					break;
				case 'D':
					level = LogMessage::Debug;
					break;
			}
		}
		msg = msgChunks.join(" ");
	} else
	{
		qDebug() << "failed to parse message: " << message;
	}

    return LogMessage(level, facility, timestamp, app, msg);
}

bool SysLogLoader::isLogOpen() const
{
	return (m_log && m_log->isOpen());
}

bool SysLogLoader::canHandleTarArchive() const
{
#ifdef Q_WS_WIN
	return (QFile::exists(TAR_EXEC) && QFile::exists(GZIP_EXEC));
#else
	return QFile::exists(TAR_EXEC);
#endif
}
