#ifndef PROCEDUREFOOTSWITCH_H
#define PROCEDUREFOOTSWITCH_H

#include <QObject>
#include <QPair>

typedef QPair<double, double> TimingLimits;

class QTimer;
class LogMessage;
class FemtoTester;
class SysLogParser;

class ProcedureFootswitch : public QObject
{
	Q_OBJECT
public:
	enum FootswitchState
	{
		FSIntermediate = 0x00,
		FSPedalDown = 0x01,
		FSPedalUp = 0x02,
		FSError = 0x03
	};

	enum PauseState
	{
		NoPause,
		PreparingPause,
		Pausing
	};

	explicit ProcedureFootswitch(SysLogParser *parser, FemtoTester *tester, QObject *parent = 0);

	bool fakeTreatmentPause() const { return m_fakeTrmPause; }
	TimingLimits treatmentDelay() const { return m_treatmentDelay; }
	TimingLimits pauseDelay() const { return m_pauseDelay; }
	TimingLimits pauseLength() const { return m_pauseLength; }
	TimingLimits intermediateFsStateLength() const { return m_intermediateLength; }
	
signals:
	void procedureFootswitchStateChanged(FootswitchState state);
	void statusMessage(const QString &msg, int timeout = 0);
	void logMessage(const LogMessage& msg);

public slots:
	void onProcShutterOpen();
	void pressProcedureFootswitch();
	void pressProcedureFootswitchDelayed();
	void releaseProcedureFootswitch();
	void pauseTreatment();
	void setFakeTreatmentPause(bool enabled) { m_fakeTrmPause = enabled; }

	void setTreatmentDelay(const TimingLimits &value) { m_treatmentDelay = value; }
	void setPauseDelay(const TimingLimits &value) { m_pauseDelay = value; }
	void setPauseLength(const TimingLimits &value) { m_pauseLength = value; }
	void setIntermediateFsStateLength(const TimingLimits &value) { m_intermediateLength = value; }

	void loadSettings();
	void saveSettings();
	void reset();

private slots:
	void timerElapsed();
	void pauseTimerElapsed();
	void updateStatusMessage();

private:
	void setFSState(FootswitchState state);
	int randomTimerInterval(const TimingLimits &limit) const;
	void showCountdown(const QString& msgFormat, int length, int interval = 1000);
	
	SysLogParser* m_logParser;
	FemtoTester* m_tester;
	FootswitchState m_currentProcedureFSState;
	FootswitchState m_targetProcedureFSState;
	PauseState m_pauseState;
	QTimer *m_pauseTimer;
	QTimer *m_statusMessageTimer;
	bool m_fakeTrmPause;
	TimingLimits m_treatmentDelay;
	TimingLimits m_pauseDelay;
	TimingLimits m_pauseLength;
	TimingLimits m_intermediateLength;
	QString m_statusMessageFormat;
	int m_statusMessageCountdown;
};

#endif // PROCEDUREFOOTSWITCH_H
