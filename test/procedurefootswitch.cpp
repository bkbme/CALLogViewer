#include "procedurefootswitch.h"

#include <syslogparser.h>
#include <femtotester.h>
#include <logmessage.h>

#include <QDebug>
#include <QTimer>
#include <QSettings>
#include <QDateTime>

ProcedureFootswitch::ProcedureFootswitch(SysLogParser *parser, FemtoTester *tester, QObject *parent) :
	QObject(parent),
	m_logParser(parser),
	m_tester(tester),
	m_currentProcedureFSState(FSPedalUp),
	m_targetProcedureFSState(FSPedalUp),
	m_pauseState(NoPause),
	m_pauseTimer(new QTimer(this)),
	m_statusMessageTimer(new QTimer(this)),
	m_fakeTrmPause(false),
	m_treatmentDelay(0, 30),
	m_pauseDelay(0, 5),
	m_pauseLength(1, 30),
	m_intermediateLength(0.1, 0.5),
	m_statusMessageFormat("%1"),
	m_statusMessageCountdown(0)
{
	connect(m_pauseTimer, SIGNAL(timeout()), this, SLOT(pauseTimerElapsed()));
	connect(m_statusMessageTimer, SIGNAL(timeout()), this, SLOT(updateStatusMessage()));
	qsrand(QDateTime::currentDateTime().toTime_t());

	loadSettings();

	connect(m_logParser, SIGNAL(treatmentStarted()), this, SLOT(pressProcedureFootswitchDelayed()));
	connect(m_logParser, SIGNAL(treatmentFinished()), this, SLOT(releaseProcedureFootswitch()));
	connect(m_logParser, SIGNAL(treatmentAborted()), this, SLOT(releaseProcedureFootswitch()));
	connect(m_logParser, SIGNAL(calStopped(int)), this, SLOT(releaseProcedureFootswitch()));
	connect(m_logParser, SIGNAL(executingTreatment()), this, SLOT(pauseTreatment()));
	// power check
	connect(m_logParser, SIGNAL(powerCheckStarted()), this, SLOT(pressProcedureFootswitch()));
	//connect(m_logParser, SIGNAL(procShutterOpened()), this, SLOT(onProcShutterOpen()));
}

void ProcedureFootswitch::pressProcedureFootswitch()
{
	m_targetProcedureFSState = FSPedalDown;

	if (m_currentProcedureFSState == FSPedalUp)
	{
		setFSState(FSIntermediate);
		QTimer::singleShot(randomTimerInterval(m_intermediateLength), this, SLOT(timerElapsed()));
	}
}

void ProcedureFootswitch::pressProcedureFootswitchDelayed()
{
	m_targetProcedureFSState = FSPedalDown;

	if (m_currentProcedureFSState == FSPedalUp) // if Intermediate, fs is pressed immediately
	{
		int delay = randomTimerInterval(m_treatmentDelay);
		//qDebug() << "pressing footswitch in " << (delay / 1000) << " seconds...";
		showCountdown("pressing footswitch in %1 seconds...", delay, 100);
		QTimer::singleShot(delay, this, SLOT(timerElapsed()));
	}
}

void ProcedureFootswitch::releaseProcedureFootswitch()
{
	m_pauseState = NoPause;
	m_targetProcedureFSState = FSPedalUp;

	if (m_currentProcedureFSState == FSPedalDown) // if Intermediate timer is still active
	{
		setFSState(FSIntermediate);
		QTimer::singleShot(randomTimerInterval(m_intermediateLength), this, SLOT(timerElapsed()));
	}
}

void ProcedureFootswitch::setFSState(FootswitchState state)
{
	if (m_currentProcedureFSState == state || !m_tester)
	{
		return;
	}

	switch (state)
	{
		case FSPedalUp:
			m_tester->setFootswitchState(FSPedalUp);
			//qDebug() << "footswitch released";
			emit logMessage(LogMessage(LogMessage::Info, LogMessage::Other, QDateTime::currentDateTimeUtc(), "CALLogView", "footswitch released"));
			emit statusMessage("footswitch released");
			break;
		case FSPedalDown:
			m_tester->setFootswitchState(FSPedalDown);
			//qDebug() << "footswitch pressed";
			emit logMessage(LogMessage(LogMessage::Info, LogMessage::Other, QDateTime::currentDateTimeUtc(), "CALLogView", "footswitch pressed"));
			emit statusMessage("footswitch pressed");
			break;
		case FSIntermediate:
			m_statusMessageTimer->stop();
			m_tester->setFootswitchState(FSIntermediate);
			//qDebug() << "footswitch in state 'Intermediate'";
			emit logMessage(LogMessage(LogMessage::Info, LogMessage::Other, QDateTime::currentDateTimeUtc(), "CALLogView", "footswitch intermediate"));
			emit statusMessage("footswitch in state 'Intermediate'");
			break;
		default:
			//qDebug() << "footswitch in (invalid) state: " << state;
			emit statusMessage(QString("footswitch in (invalid) state: %1").arg(state));
			break;
	}

	m_currentProcedureFSState = state;
	emit procedureFootswitchStateChanged(state);
}

void ProcedureFootswitch::pauseTreatment()
{
	if (m_fakeTrmPause && m_targetProcedureFSState == FSPedalDown && m_pauseState == NoPause)
	{
		int pauseDelay = randomTimerInterval(m_pauseDelay);
		//qDebug() << "pausing treatment in " << (pauseDelay / 1000) << "seconds...";
		showCountdown("pausing treatment in %1 seconds...", pauseDelay, 100);
		m_pauseState = PreparingPause;
		m_pauseTimer->setInterval(pauseDelay); // 1 - 5 sec
		m_pauseTimer->start();
	}
}

void ProcedureFootswitch::timerElapsed()
{
	switch (m_currentProcedureFSState)
	{
		case FSIntermediate:
			setFSState(m_targetProcedureFSState);
			break;
		default:
			if (m_targetProcedureFSState != m_currentProcedureFSState)
			{
				setFSState(FSIntermediate);
				QTimer::singleShot(randomTimerInterval(m_intermediateLength), this, SLOT(timerElapsed()));
			}
			break;
	}

}

void ProcedureFootswitch::pauseTimerElapsed()
{
	int interval;
	m_pauseTimer->stop();

	switch (m_pauseState)
	{
		case NoPause:
			m_statusMessageTimer->stop();
			//qDebug() << "abort pause (footswitch released)";
			emit statusMessage("abort pause (footswitch released)");
			break;
		case PreparingPause:
			interval = randomTimerInterval(m_pauseLength);
			releaseProcedureFootswitch();
			//qDebug() << "pausing treatment for " << (interval / 1000) << " seconds...";
			showCountdown("pausing treatment for %1 seconds...", interval, 100);
			m_pauseTimer->setInterval(interval);
			m_pauseTimer->start();
			m_pauseState = Pausing;
			break;
		case Pausing:
			//qDebug() << "resuming treatment...";
			emit statusMessage("resuming treatment...");
			pressProcedureFootswitch();
			m_pauseState = NoPause;
			break;
	}
}

void ProcedureFootswitch::loadSettings()
{
	QSettings settings;
	settings.beginGroup("ProcedureFootswitch");

	m_fakeTrmPause = settings.value("fakeTrmPause", false).toBool();
	m_treatmentDelay.first = settings.value("treatmentDelayMin", 0).toDouble();
	m_treatmentDelay.second = settings.value("treatmentDelayMax", 30).toDouble();
	m_pauseDelay.first = settings.value("pauseDelayMin", 0).toDouble();
	m_pauseDelay.second = settings.value("pauseDelayMax", 5).toDouble();
	m_pauseLength.first = settings.value("pauseLengthMin", 1).toDouble();
	m_pauseLength.second = settings.value("pauseLengthMax", 30).toDouble();
	m_intermediateLength.first = settings.value("intermediateLengthMin", 0.1).toDouble();
	m_intermediateLength.second = settings.value("intermediateLengthMax", 0.5).toDouble();

	settings.endGroup();
}

void ProcedureFootswitch::saveSettings()
{
	QSettings settings;
	settings.beginGroup("ProcedureFootswitch");

	settings.setValue("fakeTrmPause", m_fakeTrmPause);
	settings.setValue("treatmentDelayMin", m_treatmentDelay.first);
	settings.setValue("treatmentDelayMax", m_treatmentDelay.second);
	settings.setValue("pauseDelayMin", m_pauseDelay.first);
	settings.setValue("pauseDelayMax", m_pauseDelay.second);
	settings.setValue("pauseLengthMin", m_pauseLength.first);
	settings.setValue("pauseLengthMax", m_pauseLength.second);
	settings.setValue("intermediateLengthMin", m_intermediateLength.first);
	settings.setValue("intermediateLengthMax", m_intermediateLength.second);

	settings.endGroup();
}

int ProcedureFootswitch::randomTimerInterval(const TimingLimits &limit) const
{
	if (limit.first >= limit.second)
	{
		return (limit.first * 1000);
	}

	return (static_cast<int>(limit.first * 1000) + (qrand() % static_cast<int>((limit.second - limit.first) * 1000)));
}

void ProcedureFootswitch::showCountdown(const QString& msgFormat, int length, int interval)
{
	m_statusMessageFormat = msgFormat;
	m_statusMessageCountdown = length;
	m_statusMessageTimer->start(interval);
}

void ProcedureFootswitch::updateStatusMessage()
{
	if ((m_statusMessageCountdown -= m_statusMessageTimer->interval()) < 0)
	{
		m_statusMessageTimer->stop();
		return;
	}

	emit statusMessage(m_statusMessageFormat.arg(static_cast<double>(m_statusMessageCountdown) / 1000));
}

void ProcedureFootswitch::onProcShutterOpen()
{
	int timeout = randomTimerInterval(TimingLimits(2.5, 4));
	showCountdown("aborting powercheck in %1 seconds...", timeout, 100);
	emit logMessage(LogMessage(LogMessage::Info, LogMessage::Other, QDateTime::currentDateTimeUtc(), "CALLogView", QString("aborting PwrCheck in %1 msec...").arg(timeout)));
	QTimer::singleShot(timeout, this, SLOT(releaseProcedureFootswitch()));
}

