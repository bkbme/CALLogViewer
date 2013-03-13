#ifndef CALLOGVIEW_H
#define CALLOGVIEW_H

#include <logmessage.h>
#include <calsession.h>
#include <calsessionview.h>
#include <searchwidget.h>

#include <QTextStream>
#include <QWebView>
#include <QQueue>
#include <QPair>
#include <QMap>

class CALLogView : public QWebView
{
	Q_OBJECT
public:
	explicit CALLogView(QWidget *parent = 0);
	~CALLogView();

	void setLogLevelVisible(LogMessage::LogLevel level, bool visible);
	void setLogFacilityVisible(LogMessage::LogFacility facility, bool visible);

public slots:
	void clearSearch();
	void search(const QString& term, Qt::CaseSensitivity cs);
	void continueSearch(SearchDirection direction);
	void setSession(CALSession *session);
	void clear();

signals:
	void statusMessage(const QString &msg, int timeout);
	void supportInfoDropped(const QUrl& url);

private slots:
	void init();
	void appendLogMessage(const LogMessage &msg);
	void onContentSizeChanged(const QSize &size);
	void setErrorLevelVisible(bool visible) { setLogLevelVisible(LogMessage::Error, visible); }
	void setWarningLevelVisible(bool visible) { setLogLevelVisible(LogMessage::Warning, visible); }
	void setInfoLevelVisible(bool visible) { setLogLevelVisible(LogMessage::Info, visible); }
	void setStateMachineLevelVisible(bool visible) { setLogLevelVisible(LogMessage::StateMachine, visible); }
	void setSoapInLevelVisible(bool visible) { setLogLevelVisible(LogMessage::SoapIn, visible); }
	void setSoapOutLevelVisible(bool visible) { setLogLevelVisible(LogMessage::SoapOut, visible); }
	void setDebugLevelVisible(bool visible) { setLogLevelVisible(LogMessage::Debug, visible); }
	void setInscriptFwFacilityVisible(bool visible) { setLogFacilityVisible(LogMessage::InscriptFw, visible); }
	void setCalFacilityVisible(bool visible) { setLogFacilityVisible(LogMessage::CAL, visible); }
	void setKernelFacilityVisible(bool visible) { setLogFacilityVisible(LogMessage::Kernel, visible); }
	void setOtherFacilityVisible(bool visible) { setLogFacilityVisible(LogMessage::Other, visible); }

protected:
	virtual void dropEvent(QDropEvent *);

private:
	struct SearchData;

	QString jsLogLevel(LogMessage::LogLevel level) const;
	void log2Html(QTextStream &oStream, const LogMessage &log) const;
	QString escapeHtml(const QString &text) const;

	bool m_autoScroll;
	CALSession *m_currentSession;
	QMap<LogMessage::LogLevel, bool> m_logLevelVisibility;
	QMap<LogMessage::LogFacility, bool> m_logFacilityVisibility;
	SearchData *m_search;
};

#endif // CALLOGVIEW_H
