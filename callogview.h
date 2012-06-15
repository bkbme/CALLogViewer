#ifndef CALLOGVIEW_H
#define CALLOGVIEW_H

#include "logmessage.h"

#include <QWebView>
#include <QVector>
#include <QQueue>
#include <QMap>

class CALLogView : public QWebView
{
	Q_OBJECT
public:
	explicit CALLogView(QWidget *parent = 0);
	int scrollBufferMaxLength() const { return m_maxScrollBufferLength; }
	void setLogLevelVisible(LogMessage::LogLevel level, bool visible);
    void setLogFacilityVisible(LogMessage::LogFacility facility, bool visible);

public slots:
	void newLogMessage(const LogMessage &msg);
	void setScrollBufferMaxLength(int lines);
	void addMarker(const QString &text);
	void clearAllMarkers();

signals:
	void statusMessage(const QString &msg, int timeout);
    void supportInfoDropped(const QUrl& url);

private slots:
	void init();
	void clear();
	void purgeOldLogs();
	void onContentSizeChanged(const QSize &size);
	void setErrorLevelVisible(bool visible) { setLogLevelVisible(LogMessage::Error, visible); }
	void setWarningLevelVisible(bool visible) { setLogLevelVisible(LogMessage::Warning, visible); }
	void setInfoLevelVisible(bool visible) { setLogLevelVisible(LogMessage::Info, visible); }
	void setStateMachineLevelVisible(bool visible) { setLogLevelVisible(LogMessage::StateMachine, visible); }
	void setSoapInLevelVisible(bool visible) { setLogLevelVisible(LogMessage::SoapIn, visible); }
	void setSoapOutLevelVisible(bool visible) { setLogLevelVisible(LogMessage::SoapOut, visible); }
	void setDebugLevelVisible(bool visible) { setLogLevelVisible(LogMessage::Debug, visible); }
	void setNonCalLevelVisible(bool visible) { setLogLevelVisible(LogMessage::NonCAL, visible); }
    void setInscriptFwFacilityVisible(bool visible) { setLogFacilityVisible(LogMessage::InscriptFw, visible); }
    void setCalFacilityVisible(bool visible) { setLogFacilityVisible(LogMessage::CAL, visible); }
    void setKernelFacilityVisible(bool visible) { setLogFacilityVisible(LogMessage::Kernel, visible); }
    void setOtherFacilityVisible(bool visible) { setLogFacilityVisible(LogMessage::Other, visible); }

protected:
    virtual void dropEvent(QDropEvent *);

private:
	QString jsLogLevel(LogMessage::LogLevel level) const;
	int applyMarkers(QString &msg) const;
	QString log2Html(const LogMessage &log) const;
	QString escapeHtml(const QString &text) const;

	bool m_autoScroll;
	int m_maxScrollBufferLength;
	QQueue<LogMessage> m_logs;
	QMap<LogMessage::LogLevel, bool> m_logLevelVisibility;
    QMap<LogMessage::LogFacility, bool> m_logFacilityVisibility;
	QVector<QString> m_markers;
};

#endif // CALLOGVIEW_H
