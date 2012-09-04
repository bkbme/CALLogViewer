#include "loganalyzer.h"
#include "logmessage.h"

LogAnalyzer::LogAnalyzer(QObject *parent) :
	QObject(parent)
{
}

void LogAnalyzer::analyzeMessage(const LogMessage &msg)
{
	switch (msg.level())
	{
		case LogMessage::Info:
			if (msg.message().startsWith("Starting the CAL application"))
			{
				emit calStarted();
				return;
			}
			if (msg.message().startsWith("CAL exited with exit code:"))
			{
				emit calStopped(msg.message().mid(28).toInt());
				return;
			}
			if (msg.message().startsWith("Exiting CAL wrapper"))
			{
				emit calStopped(0);
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
			break;
		default:
			break;
	}
}
