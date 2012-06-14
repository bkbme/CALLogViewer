#include "syslogloader.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>
#include <QDateTime>
#include <QDebug>
#include <QProcess>
#include <QFile>

const QString TAR_EXEC = "/bin/tar";

SysLogLoader::SysLogLoader(QNetworkAccessManager *netMgr, QObject *parent) :
	QObject(parent),
	m_net(netMgr),
	m_log(0)
{

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
			statusMessage(QString("Failed to extract support info archive: %1 doesn't exist!").arg(TAR_EXEC), 0);
			return;
		}

		QStringList args;
		args << "Oxf" << filename << "var/log/messages";
		QProcess *tar = new QProcess(this);
		connect(tar, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
		connect(tar, SIGNAL(finished(int)), this, SLOT(closeLog()));
		tar->start(TAR_EXEC, args, QIODevice::ReadOnly);
		m_log = tar;
		emit logOpened();
		return;
	}

	// parse regular file
	m_log = new QFile(filename, this);
	if (m_log->open(QIODevice::ReadOnly))
	{
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
	while (m_log && m_log->canReadLine())
	{
		emit newLogMessage(parseLogMessage(m_log->readLine()));
	}
}

LogMessage SysLogLoader::parseLogMessage(const QString &message) const
{
	LogMessage::LogLevel level = LogMessage::NonCAL;
	QDateTime timestamp(QDateTime::currentDateTime());
	QString host, app, msg;

	QStringList msgChunks(message.split(' '));
	// parse message
	if (msgChunks.count() > 3)
	{
		timestamp = QDateTime::fromString(msgChunks.takeFirst().mid(0, 23), "yyyy-MM-ddTHH:mm:ss.zzz");
		host = msgChunks.takeFirst();
		app = msgChunks.takeFirst().remove(":");
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

	return LogMessage(level, timestamp, app, msg);
}

bool SysLogLoader::isLogOpen() const
{
	return (m_log && m_log->isOpen());
}

bool SysLogLoader::canHandleTarArchive() const
{
	return QFile::exists(TAR_EXEC); /// @todo make this more intelligent later (and possibly support Windows as well)
}
