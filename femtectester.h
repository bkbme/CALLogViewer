#ifndef FEMTECTESTER_H
#define FEMTECTESTER_H

#include <QObject>
#include <QPair>
#include "qextserialport.h"

class QTimer;

typedef QPair<double, double> TimingLimits;

class FemtecTester : public QObject
{
	Q_OBJECT

	enum FootswitchState
	{
		PedalDown,
		PedalUp,
		Intermediate
	};

	enum PauseState
	{
		NoPause,
		PreparingPause,
		Pausing
	};

public:
	explicit FemtecTester(QObject *parent = 0);

	bool enabled() const { return m_enabled; }
	bool fakeTreatmentPause() const { return m_fakeTrmPause; }
	TimingLimits treatmentDelay() const { return m_treatmentDelay; }
	TimingLimits pauseDelay() const { return m_pauseDelay; }
	TimingLimits pauseLength() const { return m_pauseLength; }
	TimingLimits intermediateFsStateLength() const { return m_intermediateLength; }
	QString port() const { return m_serialPort->portName(); }

signals:
	void procedureFootswitchStateChanged(FootswitchState state);
	void statusMessage(const QString &msg, int timeout = 0);
	void enabledStateChanged(bool);

public slots:
	void pressProcedureFootswitch();
	void pressProcedureFootswitchDelayed();
	void releaseProcedureFootswitch();
	void pauseTreatment();
	void setEnabled(bool enabled);
	void setFakeTreatmentPause(bool enabled) { m_fakeTrmPause = enabled; }

	void setTreatmentDelay(const TimingLimits &value) { m_treatmentDelay = value; }
	void setPauseDelay(const TimingLimits &value) { m_pauseDelay = value; }
	void setPauseLength(const TimingLimits &value) { m_pauseLength = value; }
	void setIntermediateFsStateLength(const TimingLimits &value) { m_intermediateLength = value; }

	void setPort(const QString &portName);

	void loadSettings();
	void saveSettings();

private slots:
	void timerElapsed();
	void pauseTimerElapsed();
	void updateStatusMessage();

private:
	void setFSState(FootswitchState state);
	int randomTimerInterval(const TimingLimits &limit) const;
	void showCountdown(const QString& msgFormat, int length, int interval = 1000);

	QextSerialPort* m_serialPort;
	FootswitchState m_currentProcedureFSState;
	FootswitchState m_targetProcedureFSState;
	PauseState m_pauseState;
	QTimer *m_pauseTimer;
	QTimer *m_statusMessageTimer;
	bool m_fakeTrmPause;
	bool m_enabled;
	TimingLimits m_treatmentDelay;
	TimingLimits m_pauseDelay;
	TimingLimits m_pauseLength;
	TimingLimits m_intermediateLength;
	QString m_statusMessageFormat;
	int m_statusMessageCountdown;
};

#endif // FEMTECTESTER_H
