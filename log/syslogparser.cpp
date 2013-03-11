#include "config.h"
#include "syslogparser.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>
#include <QDateTime>
#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

SysLogParser::SysLogParser(QNetworkAccessManager *netMgr, QObject *parent) :
	QObject(parent),
	m_net(netMgr),
    m_log(0)
{
#ifdef Q_WS_WIN
        TAR_EXEC = QCoreApplication::applicationDirPath() + QDir::separator() + TAR_EXEC;
        GZIP_EXEC = QCoreApplication::applicationDirPath() + QDir::separator() + GZIP_EXEC;
#endif
}

void SysLogParser::openLog(const QUrl &url)
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

	emit logOpened(true);
	emit statusMessage(QString("Connected to %1").arg(url.host()), 5000);
}

void SysLogParser::openLog(const QString &filename)
{
	if (m_log || !m_filesToLoadList.isEmpty())
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
		emit logOpened(false);
		return;
	}

	// parse regular file
	QFile *logFile = new QFile(filename, this);
	m_log = logFile;
	if (logFile->open(QIODevice::ReadOnly))
	{
		qDebug() << "file opened: " << filename;
		emit logOpened(false);
		onReadyRead();
	}
	closeLog();
}

void SysLogParser::openSupportInfo(const QString &supportInfoFile, const QStringList &selectedLogFiles)
{
	if (m_log || !m_filesToLoadList.isEmpty())
	{
		qDebug() << "Log already open - call closeLog() first!";
		return;
	}

	m_supportInfoFile = supportInfoFile;
	m_filesToLoadList = selectedLogFiles;

	emit logOpened(false);
	loadNextLog();
}

void SysLogParser::loadNextLog()
{
	if (m_supportInfoFile.isEmpty() || m_filesToLoadList.isEmpty())
	{
		qDebug() << "finished reading support info file" << m_supportInfoFile;
		closeLog();
		return;
	}

	if (m_log)
	{
		qDebug() << "cleaning up finished Process";
		m_log->close();
		m_log->deleteLater();
		m_log = 0;
	}

	QStringList args;
	QProcess *tar = new QProcess(this);
	connect(tar, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(tar, SIGNAL(finished(int)), this, SLOT(loadNextLog()));

#ifdef Q_WS_WIN
	args << "Ox";

	QStringList gzip_args;
	gzip_args << "-dc" << m_supportInfoFile;
	QProcess *gzip = new QProcess(tar);
	gzip->setStandardOutputProcess(tar); // tar.exe doesn't support on-the-fly gzip extraction :(
	gzip->start(GZIP_EXEC, gzip_args);
#else
	args << "Oxf" << m_supportInfoFile;
#endif
	args << m_filesToLoadList.takeLast().trimmed();

	m_log = tar;
	tar->start(TAR_EXEC, args);
	qDebug() << "process started:" << TAR_EXEC << "args: " << args.join(" ");
	return;
}

void SysLogParser::closeLog()
{
	if (!m_log)
	{
		qDebug() << "Log not open yet - nothing to do";
		return;
	}

	QNetworkReply *reply = qobject_cast<QNetworkReply*>(m_log);
	QProcess *process = qobject_cast<QProcess*>(m_log);
	if (reply)
	{
		statusMessage(QString("Disconnected from %1").arg(reply->request().url().host())
					  .append((reply->error() == QNetworkReply::NoError ? "" : ": " + reply->errorString())), 0);
		reply->disconnect();
		reply->abort();
	}
	if(process)
	{
		process->disconnect();
		process->terminate();
	}

	m_log->close();
	
	emit logClosed();

	if (!reply)
	{
		emit statusMessage("done", 3000);
	}

	m_supportInfoFile.clear();
	m_filesToLoadList.clear();
	m_log->deleteLater();
	m_log = 0;
}

void SysLogParser::onReadyRead()
{
	while (m_log && !m_log->atEnd() && (!m_log->isSequential() || m_log->canReadLine()))
	{
		LogMessage msg(parseLogMessage(m_log->readLine()));
		emit newLogMessage(msg);
		analyzeMessage(msg);
	}
}

LogMessage SysLogParser::parseLogMessage(const QString &message)
{
	LogMessage::LogLevel level = LogMessage::Unknown;
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

		if (app.contains("fw["))
		{
			facility = LogMessage::InscriptFw;
		} else if(app.contains("CAL"))
		{
			facility = LogMessage::CAL;
		} else if (app.contains("kernel"))
		{
			facility = LogMessage::Kernel;
		}

		if (facility == LogMessage::CAL)
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
		msg = msgChunks.join(" ").trimmed();
	} else
	{
		qDebug() << "failed to parse message: " << message;
	}

	return LogMessage(level, facility, timestamp, app, msg);
}

void SysLogParser::analyzeMessage(const LogMessage &msg)
{
	switch (msg.level())
	{
/*		case LogMessage::Debug:
			if (msg.message().startsWith("Ignoring procedure shutter intermediate state: opening"))
			{
				emit procShutterOpened();
				return;
			}
			break;
*/		case LogMessage::Info:
			if(msg.message().startsWith("Starting new CAL..."))
			{
				emit calStarted();
				return;
			}
			if (msg.message().startsWith("CAL exited with exit code:"))
			{
				emit calStopped(msg.message().mid(27).toInt());
				return;
			}
//			if (msg.message().startsWith("Exiting CAL wrapper..."))
//			{
//				emit calStopped(0);
//				return;
//			}
			if (msg.message().endsWith("EXIT main"))
			{
				emit calStopped(0);
			}
			if (msg.message().startsWith("CAL was terminated by signal:"))
			{
				//emit calStopped(-(msg.message().mid(30).toInt()));
				emit calStopped(6);
				return;
			}
			if (msg.message().startsWith("~~~ STARTING MAINTENANCE PROCEDURE") ||
				msg.message().startsWith("~~~ STARTING PROCEDURE"))
			{
				emit treatmentStarted();
				return;
			}
			if (msg.message().contains("SUCCESSFULLY EXECUTED ~~~"))
			{
				emit treatmentFinished();
				return;
			}
			if (msg.message().contains("ABORTED ~~~"))
			{
				emit treatmentAborted();
				return;
			}
			break;
		case LogMessage::StateMachine:
			if (msg.message().contains("state change 'StartingTrm' --> 'ExecutingTrm'"))
			{
				emit executingTreatment();
				return;
			}
			if (msg.message().contains("state change 'Waiting_For_Startup_Check' --> 'FiringInPwrChk'"))
			{
				emit powerCheckStarted();
				return;
			}
			break;
		case LogMessage::Error:
			if (msg.message().contains("(ERR_"))
			{
				int errorStrIndex = msg.message().indexOf("(ERR_");
				int errorCode = QString(msg.message().left(errorStrIndex - 1)).toInt();
				emit emergencyShutdown(errorCode, msg.message().mid(errorStrIndex));
			}
		default:
			break;
	}
}

bool SysLogParser::isLogOpen() const
{
	return (m_log && m_log->isOpen());
}

bool SysLogParser::isLogContinuous() const
{
	return (m_log && qobject_cast<QNetworkReply*>(m_log));
}

bool SysLogParser::canHandleTarArchive()
{
#ifdef Q_WS_WIN
	return (QFile::exists(TAR_EXEC) && QFile::exists(GZIP_EXEC));
#else
	return QFile::exists(TAR_EXEC);
#endif
}
