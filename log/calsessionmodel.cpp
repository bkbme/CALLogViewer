#include "calsessionmodel.h"

#include <syslogparser.h>
#include <QFontMetrics>
//#include <QDebug>

const int COLUMN_COUNT = 6;

CALSessionModel::CALSessionModel(SysLogParser *parser, QObject *parent) :
	QAbstractTableModel(parent),
	m_sessions(),
	m_continuous(false)
{
	connect(parser, SIGNAL(calStarted()), this, SLOT(startSession()));
	connect(parser, SIGNAL(calStopped(int)), this, SLOT(endSession(int)));
	connect(parser, SIGNAL(newLogMessage(LogMessage)), this, SLOT(appendLogMessage(LogMessage)));
	connect(parser, SIGNAL(emergencyShutdown(int,QString)), this, SLOT(emergencyShutdown(int,QString)));
	connect(parser, SIGNAL(logOpened(bool)), this, SLOT(setIsContinuous(bool)));
	connect(parser, SIGNAL(logOpened(bool)), this, SLOT(clear()));
	connect(parser, SIGNAL(logClosed()), this, SLOT(onLogClose()));
}

CALSessionModel::~CALSessionModel()
{
	foreach (CALSession *session, m_sessions)
	{
		if (session)
		{
			session->deleteLater();
		}
	}
	m_sessions.clear();
}

QStringList CALSessionModel::mimeTypes () const
{
	QStringList supportedTypes;
	supportedTypes << "text/uri-list";
	return supportedTypes;
}

Qt::ItemFlags CALSessionModel::flags(const QModelIndex &index ) const
{
	return QAbstractTableModel::flags(index) | Qt::ItemIsDropEnabled;
}

int CALSessionModel::rowCount(const QModelIndex & /*parent*/) const
{
	return m_sessions.count();
}

int CALSessionModel::columnCount(const QModelIndex & /*parent*/) const
{
	return COLUMN_COUNT;
}

QVariant CALSessionModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole && index.row() < m_sessions.count())
	{
		CALSession *session = m_sessions.at(index.row());
		if (!session)
		{
			return QVariant();
		}

		const uint duration = session->durationSecs();
		switch (index.column())
		{
			case 0: return session->exitCode();
			case 1: return session->start().toString("yyyy-MM-dd HH:mm:ss");
			case 2: return session->end().toString("yyyy-MM-dd HH:mm:ss");
			case 3: return (duration ? QString("%2min %3sec").arg(static_cast<int>(duration / 60), 3).arg(duration % 60, 2) : QVariant());
			case 4: return (session->esdErrorCode() > 0 ? session->esdErrorCode() : QVariant());
			case 5: return (session->esdErrorCode() > 0 ? session->esdErrorStr() : QVariant());
			default:
				return QVariant();
		}
	}
	return QVariant();
}

QVariant CALSessionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (section)
		{
			case 0: return QString("ExitCode");
			case 1: return QString("Start");
			case 2: return QString("End");
			case 3: return QString("Duration");
			case 4: return QString("ESD");
			case 5: return QString("Error");
			default: return QVariant();
		}
	}

	return QVariant();
}

int CALSessionModel::columnMinSize(int column, const QFontMetrics &metrics) const
{
	switch (column)
	{
		case 0:
			return metrics.width(headerData(column, Qt::Horizontal, Qt::DisplayRole).toString());
		case 1: // fall
		case 2:
			return metrics.width("0000-00-00 00:00:00");
		case 3:
			return metrics.width("0000min 00sec");
		case 4:
			return metrics.width("00000");
		case 5: // fall
		default:
			return -1;
	}
}

void CALSessionModel::appendSession(CALSession *session)
{
	beginInsertRows(QModelIndex(), m_sessions.count(), m_sessions.count() - 1);
	m_sessions << session;
	endInsertRows();
}

void CALSessionModel::setSessions(const QList<CALSession *> &sessions)
{
	foreach (CALSession * session, m_sessions)
	{
		if (session)
		{
			session->deleteLater();
		}
	}

	m_sessions = sessions;
	reset();
}

void CALSessionModel::clear()
{
	beginResetModel();
	foreach (CALSession *session, m_sessions)
	{
		if (session)
		{
			session->deleteLater();
		}
	}
	m_sessions.clear();
	endResetModel();
}

void CALSessionModel::startSession()
{
	CALSession *session = new CALSession();
	if (!m_sessions.isEmpty() && m_sessions.back() && m_sessions.back()->messages().count())
	{
		LogMessage msg = m_sessions.back()->messages().takeLast(); // move last message to new session
		session->appendMessage(msg);
		session->setStart(msg.timestamp());
	}
	appendSession(session);
}

void CALSessionModel::endSession(int exitCode)
{
	CALSession *session = (m_sessions.isEmpty() ? 0 : m_sessions.back());
	if (session && (session->exitCode() < 0) && session->messages().count())
	{
		LogMessage msg = session->messages().back();
		session->setExitCode(exitCode);
		session->setEnd(msg.timestamp());
		emit dataChanged(createIndex(m_sessions.count() - 1, 0), createIndex(m_sessions.count() - 1, COLUMN_COUNT));
	}
}

void CALSessionModel::appendLogMessage(const LogMessage &log)
{
	if(m_sessions.isEmpty())
	{
		appendSession(new CALSession());
	}

	CALSession *session = m_sessions.back();
	session->appendMessage(log);
}

void CALSessionModel::emergencyShutdown(int errorCode, const QString &message)
{
	if (m_sessions.isEmpty())
	{
		return;
	}

	CALSession *currentSession = m_sessions.back();
	if (currentSession && currentSession->esdErrorCode() < 0) // accept first ESD only
	{
		currentSession->setEsdErrorCode(errorCode);
		currentSession->setEsdErrorStr(message);
		emit dataChanged(createIndex(m_sessions.count() - 1, 0), createIndex(m_sessions.count() - 1, COLUMN_COUNT));
	}
}

void CALSessionModel::onLogClose()
{
	if (m_continuous)
	{
		emit finishedLoading();
	}

	m_continuous = false;
}

CALSession *CALSessionModel::sessionAtRow(int row) const
{
	if (row >= 0 && row < m_sessions.count())
	{
		return m_sessions.at(row);
	}

	return 0;
}
