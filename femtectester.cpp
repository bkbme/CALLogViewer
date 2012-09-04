#include "femtectester.h"


#include <QTimer>
//#include <QDebug>
#include <QByteArray>
#include <QDateTime>
#include <QSettings>


FemtecTester::FemtecTester(QObject *parent) :
	QObject(parent),
	m_serialPort(new QextSerialPort()),
	m_currentProcedureFSState(Intermediate),
	m_targetProcedureFSState(PedalUp),
	m_pauseState(NoPause),
	m_pauseTimer(new QTimer(this)),
	m_statusMessageTimer(new QTimer(this)),
	m_fakeTrmPause(false),
	m_enabled(false),
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
}

void FemtecTester::setEnabled(bool enabled)
{
	if (m_enabled == enabled)
	{
		return;
	}

	m_enabled = enabled;
	
	if (!m_enabled)
	{
		m_serialPort->close();
		//qDebug() << "serial port closed: " << m_serialPort->portName();
		emit statusMessage(QString("serial port closed: %1").arg(m_serialPort->portName()));
		emit enabledStateChanged(false);
		return;
	}

	if (m_serialPort->open(QIODevice::WriteOnly))
	{
		//qDebug() << "serial port opened: " << m_serialPort->portName();
		emit statusMessage(QString("serial port opened: %1").arg(m_serialPort->portName()));
		emit enabledStateChanged(true);
		setFSState(PedalUp);
		return;
	}
	
	m_enabled = false;
	emit statusMessage(QString("failed to open serial port '%1': %2").arg(m_serialPort->portName(), m_serialPort->errorString()));
	emit enabledStateChanged(false);
}

void FemtecTester::setPort(const QString &portName)
{
	if (portName.isEmpty() || portName == m_serialPort->portName())
	{
		return;
	}
	
	bool enabled = m_enabled;
	setEnabled(false);
	m_serialPort->setPortName(portName);
	setEnabled(enabled);
}

void FemtecTester::pressProcedureFootswitch()
{
	m_targetProcedureFSState = PedalDown;

	if (m_currentProcedureFSState == PedalUp)
	{
		setFSState(Intermediate);
		QTimer::singleShot(randomTimerInterval(m_intermediateLength), this, SLOT(timerElapsed()));
	}
}

void FemtecTester::pressProcedureFootswitchDelayed()
{
	m_targetProcedureFSState = PedalDown;

	if (m_currentProcedureFSState == PedalUp) // if Intermediate, fs is pressed immediately
	{
		int delay = randomTimerInterval(m_treatmentDelay);
		//qDebug() << "pressing footswitch in " << (delay / 1000) << " seconds...";
		showCountdown("pressing footswitch in %1 seconds...", delay, 100);
		QTimer::singleShot(delay, this, SLOT(timerElapsed()));
	}
}

void FemtecTester::releaseProcedureFootswitch()
{
	m_pauseState = NoPause;
	m_targetProcedureFSState = PedalUp;

	if (m_currentProcedureFSState == PedalDown) // if Intermediate timer is still active
	{
		setFSState(Intermediate);
		QTimer::singleShot(randomTimerInterval(m_intermediateLength), this, SLOT(timerElapsed()));
	}
}

void FemtecTester::setFSState(FootswitchState state)
{
	if (m_currentProcedureFSState == state || !m_serialPort->isWritable())
	{
		return;
	}

	switch (state)
	{
		case PedalUp:
			m_serialPort->setDtr(false);
			m_serialPort->setRts(true);
			//qDebug() << "footswitch released";
			emit statusMessage("footswitch released");
			break;
		case PedalDown:
			m_serialPort->setRts(false);
			m_serialPort->setDtr(true);
			//qDebug() << "footswitch pressed";
			emit statusMessage("footswitch pressed");
			break;
		case Intermediate:
			m_statusMessageTimer->stop();
			m_serialPort->setRts(false);
			m_serialPort->setDtr(false);
			//qDebug() << "footswitch in state 'Intermediate'";
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

void FemtecTester::pauseTreatment()
{
	if (m_fakeTrmPause && m_targetProcedureFSState == PedalDown && m_pauseState == NoPause)
	{
		int pauseDelay = randomTimerInterval(m_pauseDelay);
		//qDebug() << "pausing treatment in " << (pauseDelay / 1000) << "seconds...";
		showCountdown("pausing treatment in %1 seconds...", pauseDelay, 100);
		m_pauseState = PreparingPause;
		m_pauseTimer->setInterval(pauseDelay); // 1 - 5 sec
		m_pauseTimer->start();
	}
}

void FemtecTester::timerElapsed()
{
	switch (m_currentProcedureFSState)
	{
		case Intermediate:
			setFSState(m_targetProcedureFSState);
			break;
		default:
			if (m_targetProcedureFSState != m_currentProcedureFSState)
			{
				setFSState(Intermediate);
				QTimer::singleShot(randomTimerInterval(m_intermediateLength), this, SLOT(timerElapsed()));
			}
			break;
	}

}

void FemtecTester::pauseTimerElapsed()
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

void FemtecTester::loadSettings()
{
	QSettings settings("TPV-ARGES", "LogView");
	settings.beginGroup("FemtecTester");

	bool enabled = settings.value("enabled", false).toBool();
	m_fakeTrmPause = settings.value("fakeTrmPause", false).toBool();
	m_treatmentDelay.first = settings.value("treatmentDelayMin", 0).toDouble();
	m_treatmentDelay.second = settings.value("treatmentDelayMax", 30).toDouble();
	m_pauseDelay.first = settings.value("pauseDelayMin", 0).toDouble();
	m_pauseDelay.second = settings.value("pauseDelayMax", 5).toDouble();
	m_pauseLength.first = settings.value("pauseLengthMin", 1).toDouble();
	m_pauseLength.second = settings.value("pauseLengthMax", 30).toDouble();
	m_intermediateLength.first = settings.value("intermediateLengthMin", 0.1).toDouble();
	m_intermediateLength.second = settings.value("intermediateLengthMax", 0.5).toDouble();

	setPort(settings.value("ComPort", "").toString());

	settings.endGroup();

	setEnabled(enabled);
}

void FemtecTester::saveSettings()
{
	QSettings settings("TPV-ARGES", "LogView");
	settings.beginGroup("FemtecTester");

	settings.setValue("enabled", m_enabled);
	settings.setValue("fakeTrmPause", m_fakeTrmPause);
	settings.setValue("treatmentDelayMin", m_treatmentDelay.first);
	settings.setValue("treatmentDelayMax", m_treatmentDelay.second);
	settings.setValue("pauseDelayMin", m_pauseDelay.first);
	settings.setValue("pauseDelayMax", m_pauseDelay.second);
	settings.setValue("pauseLengthMin", m_pauseLength.first);
	settings.setValue("pauseLengthMax", m_pauseLength.second);
	settings.setValue("intermediateLengthMin", m_intermediateLength.first);
	settings.setValue("intermediateLengthMax", m_intermediateLength.second);
	settings.setValue("ComPort", m_serialPort->portName());

	settings.endGroup();
}

int FemtecTester::randomTimerInterval(const TimingLimits &limit) const
{
	if (limit.first >= limit.second || limit.second == 0)
	{
		return (limit.first);
	}

	return ((qrand() + static_cast<int>(limit.first * 1000)) % static_cast<int>(limit.second * 1000));
}

void FemtecTester::showCountdown(const QString& msgFormat, int length, int interval)
{
	m_statusMessageFormat = msgFormat;
	m_statusMessageCountdown = length;
	m_statusMessageTimer->start(interval);
}

void FemtecTester::updateStatusMessage()
{
	if ((m_statusMessageCountdown -= m_statusMessageTimer->interval()) < 0)
	{
		m_statusMessageTimer->stop();
		return;
	}

	emit statusMessage(m_statusMessageFormat.arg(static_cast<double>(m_statusMessageCountdown) / 1000));
}

