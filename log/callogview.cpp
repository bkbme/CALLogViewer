#include "callogview.h"
#include <calsessionmodel.h>

#include <QDebug>
#include <QUrl>
#include <QSize>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QMetaObject>
#include <QDropEvent>

const QString FORMAT_MSG = "<div id='log_%6' class='%2'>%3 %4: %1%5</div>";
const QString FORMAT_SEARCH_MATCH = "<div id='match_%1' class='match'>%2</div>";

const QString PAGE = "<?xml version='1.0' encoding='UTF-8'?>"
					 "<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en_US' lang='en_US'>"
					 "	<head>"
					 "		<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />"
					 "		<title>CAL log view</title>"
					 "		<style type='text/css'>\n%1</style>"
					 "	</head>"
					 "	<body>%2</body>"
					 "</html>";

const QString STYLESHEET =  ".Error { color: #fe1c3c; }\n"
							".Warning { color: #ccff00; }\n"
							".Info { color: #ffffff; }\n"
							".State { color: #ff00ff; }\n"
							".SoapIn { color: #0cff00; }\n"
							".SoapOut { color: #3e70ff; }\n"
							".Debug { color: #a7a7a7; }\n"
							".NonCal { color: #797979; }\n"
							".kernel { color: #656565; }\n"
							".fw { color: #ff6500; }\n"
							".hidden { display: none; }\n"
							".match { background: #ffff00; display: inline; color: #000000; font-weight:bold; }\n"
							".currentMatch { background: #ffff00; display: inline; color: #000000; font-weight:bold; border: dashed 2px red; }\n"
							"body { font: 12px monospace; background: #000000; }\n";

struct CALLogView::SearchData
{
	SearchData() :
		term(),
		cs(Qt::CaseInsensitive),
		currentPosition(0),
		matchCount(0)
	{}

	QString term;
	Qt::CaseSensitivity cs;
	int currentPosition;
	int matchCount;
};

CALLogView::CALLogView(QWidget *parent) :
	QWebView(parent),
	m_autoScroll(true),
	m_currentSession(0),
	m_logLevelVisibility(),
	m_logFacilityVisibility(),
	m_search(new SearchData())
{
	settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true); // enable web-inspector
	QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
	setAcceptDrops(true);
}

CALLogView::~CALLogView()
{
	delete m_search;
}

void CALLogView::init()
{
	setHtml(PAGE.arg(STYLESHEET, ""));
	connect(page()->mainFrame(), SIGNAL(contentsSizeChanged(QSize)), this, SLOT(onContentSizeChanged(QSize)));
}

void CALLogView::onContentSizeChanged(const QSize &size)
{
	if (m_autoScroll)
	{
		page()->mainFrame()->scroll(0, size.height());
	}
}

void CALLogView::appendLogMessage(const LogMessage &log)
{
	QString html;
	QTextStream oStream(&html, QIODevice::WriteOnly);

	log2Html(oStream, log);

	if (page() && page()->mainFrame())
	{
		QWebFrame *frame = page()->mainFrame();
		QWebElement body = frame->documentElement().findFirst("body");
		if (!body.isNull())
		{
			body.appendInside(html);
		}

		m_autoScroll = frame->scrollPosition().y() == frame->scrollBarMaximum(Qt::Vertical);
	}
}

void CALLogView::setSession(CALSession *session)
{
	if (m_currentSession == session)
	{
		return;
	}

	clear();
	if (!session)
	{
		return;
	}

	QString body;
	QTextStream oStream(&body, QIODevice::WriteOnly);
	m_currentSession = session;
	foreach (const LogMessage &msg, session->messages())
	{
		log2Html(oStream, msg);
	}

	setHtml(PAGE.arg(STYLESHEET, body));
	connect(m_currentSession, SIGNAL(messageAppended(LogMessage)), this, SLOT(appendLogMessage(LogMessage)));
	connect(m_currentSession, SIGNAL(destroyed()), this, SLOT(clear()));
}

void CALLogView::clear()
{
	if (m_currentSession)
	{
		m_currentSession->disconnect();
		m_currentSession = 0;
	}

	if (page() && page()->mainFrame())
	{
		QWebFrame *frame = page()->mainFrame();
		QWebElement body = frame->documentElement().findFirst("body");
		if (!body.isNull())
		{
			body.removeAllChildren();
		}
	}

	m_search->matchCount = 0;
	m_search->currentPosition = 0;
}

/*
void CALLogView::purgeOldLogs()
{
	if (!page() || !page()->mainFrame() || m_maxScrollBufferLength <= 0 || !m_autoScroll)
	{
		return; // keep all logs if m_maxScrollBufferLength is set to 0 or user is scrolling
	}

	QWebFrame *frame = page()->mainFrame();
	QWebElement body = frame->documentElement().findFirst("body");
	while (!body.isNull() && m_logs.count() > m_maxScrollBufferLength)
	{
		LogMessage log(m_logs.dequeue());
		body.findFirst(QString("#log_%1").arg(log.id())).removeFromDocument();
	}
}
*/

void CALLogView::setLogLevelVisible(LogMessage::LogLevel level, bool visible)
{
	if (!page() || !page()->mainFrame() || !m_currentSession)
	{
		return;
	}

	QWebFrame *frame = page()->mainFrame();
	QWebElement body = frame->documentElement().findFirst("body");
	foreach (const LogMessage &msg, m_currentSession->messages())
	{
		if (msg.level() == level)
		{
			QWebElement div = body.findFirst(QString("#log_%1").arg(msg.id()));
			visible ? div.removeClass("hidden") : div.addClass("hidden");
		}
	}

	m_logLevelVisibility[level] = visible;
}

void CALLogView::setLogFacilityVisible(LogMessage::LogFacility facility, bool visible)
{
	if (!page() || !page()->mainFrame() || !m_currentSession)
	{
		return;
	}

	QWebFrame *frame = page()->mainFrame();
	QWebElement body = frame->documentElement().findFirst("body");
	foreach (const LogMessage &msg, m_currentSession->messages())
	{
		if (msg.facility() == facility)
		{
			QWebElement div = body.findFirst(QString("#log_%1").arg(msg.id()));
			visible ? div.removeClass("hidden") : div.addClass("hidden");
		}
	}
	m_logFacilityVisibility[facility] = visible;
}


void CALLogView::log2Html(QTextStream &oStream, const LogMessage &log) const
{
	oStream << "<div id='log_" << log.id() << "' class='" << jsLogLevel(log.level());

	if ((m_logLevelVisibility.contains(log.level()) && !m_logLevelVisibility[log.level()]) ||
		(m_logFacilityVisibility.contains(log.facility()) && !m_logFacilityVisibility[log.facility()]))
	{
		oStream << " hidden";
	}

	if (log.application().startsWith("fw["))
	{
		oStream << " fw";
	} else if (log.application().startsWith("kernel"))
	{
		oStream << " kernel";
	}

	QString msg(escapeHtml(log.toString()));
	if (!m_search->term.isEmpty())
	{
		int searchIndex = 0;
		const QString escapedSearchTerm = escapeHtml(m_search->term);
		while ((searchIndex = msg.indexOf(escapedSearchTerm, searchIndex, m_search->cs)) > 0)
		{
			msg.replace(searchIndex, escapedSearchTerm.count(), QString(FORMAT_SEARCH_MATCH).arg(m_search->matchCount++).arg(escapedSearchTerm));
			searchIndex += escapedSearchTerm.count() + FORMAT_SEARCH_MATCH.count();
		}
	}
	oStream << "'>" << msg << "</div>";
}

QString CALLogView::escapeHtml(const QString &text) const
{
	return QString(text).replace("&","&amp;").replace(">", "&gt;").replace("<", "&lt;");
}

QString CALLogView::jsLogLevel(LogMessage::LogLevel level) const
{
	switch (level)
	{
		case LogMessage::Error:
			return "Error";
		case LogMessage::Warning:
			return "Warning";
		case LogMessage::Info:
			return "Info";
		case LogMessage::StateMachine:
			return "State";
		case LogMessage::SoapIn:
			return "SoapIn";
		case LogMessage::SoapOut:
			return "SoapOut";
		case LogMessage::Debug:
			return "Debug";
//		case LogMessage::NonCAL: // fall
		default:
			return "NonCal";
	}
}

void CALLogView::clearSearch()
{
	if (!m_currentSession || !page() || !page()->mainFrame() || !m_search->matchCount)
	{
		return;
	}
	
	QWebFrame *frame = page()->mainFrame();
	QWebElement body = frame->documentElement().findFirst("body");
	foreach (const LogMessage &msg, m_currentSession->messages())
	{
		if (msg.toString().contains(m_search->term, m_search->cs))
		{
			QWebElement div = body.findFirst(QString("#log_%1").arg(msg.id()));
			div.setInnerXml(escapeHtml(msg.toString()));
		}
	}

	m_search->term = "";
	m_search->cs = Qt::CaseInsensitive;
	m_search->matchCount = 0;
	m_search->currentPosition = 0;
}

void CALLogView::search(const QString& term, Qt::CaseSensitivity cs)
{
	if (!m_currentSession || !page() || !page()->mainFrame() || term.isEmpty())
	{
		return;
	}

	clearSearch();
	m_search->currentPosition = 0;
	m_search->term = term;
	m_search->cs = cs;

	int matchingMessages = 0;
	const QString escapedSearchTerm = escapeHtml(term);
	QWebFrame *frame = page()->mainFrame();
	QWebElement body = frame->documentElement().findFirst("body");
	const QList<LogMessage> &messages = m_currentSession->messages();

	foreach (const LogMessage &msg, messages)
	{
		int searchIndex = 0;
		bool foundMatch = false;
		QString msgHtml = escapeHtml(msg.toString());

		while ((searchIndex = msgHtml.indexOf(escapedSearchTerm, searchIndex, m_search->cs)) > 0)
		{
			msgHtml.replace(searchIndex, escapedSearchTerm.count(), QString(FORMAT_SEARCH_MATCH).arg(m_search->matchCount++).arg(escapedSearchTerm));
			searchIndex += escapedSearchTerm.count() + FORMAT_SEARCH_MATCH.count();
			foundMatch = true;
		}

		if (foundMatch)
		{
			QWebElement divLog = body.findFirst(QString("#log_%1").arg(msg.id()));
			if (!divLog.isNull())
			{
				divLog.setInnerXml(msgHtml);
			}
			matchingMessages++;
		}
	}

	QWebElement divMatch = body.findFirst(QString("#match_0"));
	if (m_search->matchCount && !divMatch.isNull())
	{
		divMatch.addClass("currentMatch");
		frame->setScrollPosition(QPoint(0, divMatch.geometry().y()));
	}

	emit statusMessage(QString("found %1 matches in %2 of %3 log messges").arg(m_search->matchCount).arg(matchingMessages).arg(messages.count()), 0);
}

void CALLogView::continueSearch(SearchDirection direction)
{
	if (! m_currentSession || !page() || !page()->mainFrame() || m_search->term.isEmpty() || !m_search->matchCount)
	{
		return;
	}

	int previousSearchPosition = m_search->currentPosition;

	switch (direction)
	{
		case SearchNext:
			if (m_search->currentPosition < (m_search->matchCount - 1))
			{
				m_search->currentPosition++;
			}
			break;
		case SearchPrevious:
			if (m_search->currentPosition > 0)
			{
				m_search->currentPosition--;
			}
			break;
	}

	QWebFrame *frame = page()->mainFrame();
	QWebElement body = frame->documentElement().findFirst("body");
	QWebElement divPreviousMatch = body.findFirst(QString("#match_%1").arg(previousSearchPosition));
	QWebElement divCurrentMatch = body.findFirst(QString("#match_%1").arg(m_search->currentPosition));
	if (m_search->matchCount && !divPreviousMatch.isNull() && !divCurrentMatch.isNull())
	{
		divPreviousMatch.removeClass("currentMatch");
		divCurrentMatch.addClass("currentMatch");

		// scroll to current match if not in visible range already...
		int yPos = divCurrentMatch.geometry().y();
		int visibleVerticalRegionMin = frame->scrollPosition().y();
		int visibleVerticalRegionMax = frame->scrollPosition().y() + frame->geometry().height();
		if (yPos < visibleVerticalRegionMin || yPos > visibleVerticalRegionMax)
		{
			frame->setScrollPosition(QPoint(0, yPos));
		}
	}
}

void CALLogView::dropEvent(QDropEvent* ev)
{
	if(ev && ev->mimeData()->hasUrls()) {
		qDebug() << "file dropped: " << ev->mimeData()->urls().at(0).toString();
		emit supportInfoDropped(ev->mimeData()->urls().at(0));
	}
}

