#ifndef LOGANALYZER_H
#define LOGANALYZER_H

#include <QObject>

class LogMessage;

class LogAnalyzer : public QObject
{
	Q_OBJECT
public:
	explicit LogAnalyzer(QObject *parent = 0);

signals:
	void calStarted();
	void calStopped(int exitCode);
	void treatmentStarted();
	void treatmentFinished();
	void treatmentAborted();
    void executingTreatment();

public slots:
	void analyzeMessage(const LogMessage &msg);
};

#endif // LOGANALYZER_H
