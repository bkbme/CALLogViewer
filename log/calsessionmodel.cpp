#include "calsessionmodel.h"
#include <config.h>

#include <syslogparser.h>
#include <QFontMetrics>
#include <QIcon>
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

QStringList CALSessionModel::mimeTypes() const
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
	if (index.row() < 0 || index.row() >= m_sessions.count())
	{
		return QVariant();
	}

	CALSession *session = m_sessions.at(index.row());

	if (role == Qt::DisplayRole)
	{
		const uint duration = session->durationSecs();
		switch (index.column())
		{
			//case 0: return session->exitCode() >= 0 ? QString("(%1)").arg(session->exitCode()) : QVariant();
			case 1: return session->start().toString("yyyy-MM-dd HH:mm:ss");
			case 2: return session->end().toString("yyyy-MM-dd HH:mm:ss");
			case 3: return (duration ? QString("%2min %3sec").arg(static_cast<int>(duration / 60), 3).arg(duration % 60, 2) : QVariant());
			case 4: return (session->esdErrorCode() > 0 ? session->esdErrorCode() : QVariant());
			case 5: return (session->esdErrorCode() > 0 ? session->esdErrorStr() : QVariant());
			default:
				return QVariant();
		}
	}

	if (role == Qt::DecorationRole && index.column() == 0)
	{
		switch (session->exitCode())
		{
			case CAL::APP_EXITCODE_NORMAL: return QIcon(":/icons/app_exit.png");
			case CAL::APP_EXITCODE_RESET: return QIcon(":/icons/app_reset.png");
			case CAL::APP_EXITCODE_MACHINEHALT: return QIcon(":/icons/app_halt.png");
			case CAL::APP_EXITCODE_EMERGENCY: return QIcon(":/icons/app_esd.png");
			case CAL::APP_EXITCODE_INTERNALERROR: return QIcon(":/icons/app_error.png");
			case CAL::APP_EXITCODE_DAEMONFAIL: return QIcon(":/icons/app_fail.png");
			case CAL::APP_EXITCODE_CRASHED: return QIcon(":/icons/app_crash.png");
			default: return QIcon((m_continuous && index.row() == rowCount() - 1) ? ":/icons/app_running.png" : ":/icons/app_unknown.png");
		}
	}

	if (role == Qt::ToolTipRole)
	{
		switch (index.column())
		{
			case 0:
				switch (session->exitCode())
				{
					case CAL::APP_EXITCODE_NORMAL: return QString("0 - CAL application shutdown (e.g. via init script)");
					case CAL::APP_EXITCODE_RESET: return QString("1 - CAL restarted due to GUI disconnect");
					case CAL::APP_EXITCODE_MACHINEHALT: return QString("2 - CAL application shutdown and system power off");
					case CAL::APP_EXITCODE_EMERGENCY: return QString("3 - CAL restarted due to an emergency shutdown");
					case CAL::APP_EXITCODE_INTERNALERROR: return QString("4 - CAL was terminated due to an internal application error");
					case CAL::APP_EXITCODE_DAEMONFAIL: return QString("5 - CAL failed to fork into background (daemon)");
					case CAL::APP_EXITCODE_CRASHED: return QString("6 - CAL crashed or was killed");
					default: return QString((m_continuous && index.row() == rowCount() - 1) ? "still running..." : "unknown");
				}

			case 5: return QString(session->esdErrorCode() > 0 ? session->esdErrorStr() : "");
			default: return QVariant();
		}
	}

	return QVariant();
}

QVariant CALSessionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (role)
		{
			case Qt::DisplayRole:
				switch (section)
				{
					case 0: return QString("Exit");
					case 1: return QString("Start");
					case 2: return QString("End");
					case 3: return QString("Duration");
					case 4: return QString("ESD");
					case 5: return QString("Error");
					default: return QVariant();
				}
			case Qt::ToolTipRole:
				switch (section)
				{
					case 0: return QString("CAL exit code");
					case 1: return QString("Timestamp CAL was started");
					case 2: return QString("Timestamp CAL was stopped");
					case 3: return QString("Duration CAL was running");
					case 4: return QString("Emergency shutdown error code (if any)");
					case 5: return QString("Emergency shutdown error message");
					default: return QVariant();
				}
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
