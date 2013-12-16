#include "config.h"
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
#include <QSettings>

const QString FORMAT_MSG = "<div id='log_%6' class='%2'>%3 %4: %1%5</div>";
const QString FORMAT_SEARCH_MATCH = "<div id='match_%1' class='match'>%2</div>";

const QString PAGE = "<?xml version='1.0' encoding='UTF-8'?>"
					 "<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en_US' lang='en_US'>"
					 "	<head>"
					 "		<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />"
					 "		<title>CAL log view</title>"
					 "		<style type='text/css'>\n%1</style>"
					 "	</head>"
					 "	<body>"
					 "		<div id='show_all_button' class='button hidden' onclick='logView.showAllMessages()'>&lt;&lt;&lt; Show all (be patient... this may take a while!) &gt;&gt;&gt;</div>"
					 "		<div id='messages'>%2</div>"
					 "	</body>"
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
							".button { background: darkBlue; color: white; font-weight: bold; line-height: 20px; text-align: center; border-radius: 5px; }\n"
							".button:hover { border: 1px; cursor: pointer; }"
							".button:active { border: 1px; background: blue; }"
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
	m_maxScrollBufferLength(0),
	m_scrollBufferLength(0),
	m_currentSession(0),
	m_logLevelVisibility(),
	m_logFacilityVisibility(),
	m_search(new SearchData())
{
	if (DEVELOPER_BUILD)
	{
		settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true); // enable web-inspector in development builds
	}

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
	QWebFrame *frame = page()->mainFrame();
	frame->addToJavaScriptWindowObject("logView", this);
	connect(frame, SIGNAL(contentsSizeChanged(QSize)), this, SLOT(onContentSizeChanged(QSize)));
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
		QWebElement messages = body.findFirst("#messages");
		if (!messages.isNull())
		{
			messages.appendInside(html);
			if (m_maxScrollBufferLength > 0 && ++m_scrollBufferLength > static_cast<unsigned int>(m_maxScrollBufferLength))
			{
				QWebElement oldestMessage = messages.firstChild();
				oldestMessage.removeFromDocument();
				if (m_currentSession->messages().count() == static_cast<unsigned int>(m_maxScrollBufferLength + 1))
				{
					QWebElement showAllButton = frame->documentElement().findFirst("#show_all_button");
					if (!showAllButton.isNull())
					{
						showAllButton.removeClass("hidden");
					}
				}
				--m_scrollBufferLength;
			}
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
	if (!session || !page() || !page()->mainFrame())
	{
		return;
	}

	QString html;
	QSettings settings;
	QTextStream oStream(&html, QIODevice::WriteOnly);
	QWebFrame *frame = page()->mainFrame();
	QWebElement body = frame->documentElement().findFirst("body");
	QWebElement messages = body.findFirst("#messages");
	QWebElement showAllButton = body.findFirst("#show_all_button");

	m_currentSession = session;
	settings.beginGroup("ASC");
	m_maxScrollBufferLength = settings.value("maxScrollBuffer", 10000).toInt();
	
	int i = (m_maxScrollBufferLength && session->messages().count() > m_maxScrollBufferLength) ? (session->messages().count() - m_maxScrollBufferLength) : 0;
	if  (i > 0)
	{
		showAllButton.removeClass("hidden");
	}

	while (i < session->messages().count())
	{
		log2Html(oStream, session->messages().at(i++));
		++m_scrollBufferLength;
	}
	messages.appendInside(html);
	frame->scroll(0, frame->contentsSize().height());

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
		QWebElement showAllButton = body.findFirst("#show_all_button");
		QWebElement messages = body.findFirst("#messages");
		messages.removeAllChildren();
		showAllButton.addClass("hidden");
	}

	m_search->matchCount = 0;
	m_search->currentPosition = 0;
	m_scrollBufferLength = 0;
}

void CALLogView::setLogLevelVisible(LogMessage::LogLevel level, bool visible)
{
	if (!page() || !page()->mainFrame() || !m_currentSession)
	{
		return;
	}

	QWebFrame *frame = page()->mainFrame();
	QWebElement body = frame->documentElement().findFirst("body");
	QWebElement messages = body.findFirst("#messages");
	foreach (const LogMessage &msg, m_currentSession->messages())
	{
		if (msg.level() == level)
		{
			QWebElement div = messages.findFirst(QString("#log_%1").arg(msg.id()));
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
	QWebElement messages = body.findFirst("#messages");
	foreach (const LogMessage &msg, m_currentSession->messages())
	{
		if (msg.facility() == facility)
		{
			QWebElement div = messages.findFirst(QString("#log_%1").arg(msg.id()));
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
	QWebElement messages = body.findFirst("#messages");
	foreach (const LogMessage &msg, m_currentSession->messages())
	{
		if (msg.toString().contains(m_search->term, m_search->cs))
		{
			QWebElement div = messages.findFirst(QString("#log_%1").arg(msg.id()));
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
	QWebElement messages = body.findFirst("#messages");

	foreach (const LogMessage &msg, m_currentSession->messages())
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
			QWebElement divLog = messages.findFirst(QString("#log_%1").arg(msg.id()));
			if (!divLog.isNull())
			{
				divLog.setInnerXml(msgHtml);
			}
			matchingMessages++;
		}
	}

	QWebElement divMatch = messages.findFirst(QString("#match_0"));
	if (m_search->matchCount && !divMatch.isNull())
	{
		divMatch.addClass("currentMatch");
		frame->setScrollPosition(QPoint(0, divMatch.geometry().y()));
	}

	emit statusMessage(QString("found %1 matches in %2 of %3 log messges").arg(m_search->matchCount).arg(matchingMessages).arg(m_currentSession->messages().count()), 0);
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
	QWebElement messages = body.findFirst("#messages");
	QWebElement divPreviousMatch = messages.findFirst(QString("#match_%1").arg(previousSearchPosition));
	QWebElement divCurrentMatch = messages.findFirst(QString("#match_%1").arg(m_search->currentPosition));
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

void CALLogView::showAllMessages()
{
	qDebug() << "CALLogView::showAllMessages() called";
	
	if (page() && page()->mainFrame() && m_currentSession)
	{
		QString html;
		QTextStream oStream(&html, QIODevice::WriteOnly);
		QWebFrame *frame = page()->mainFrame();
		QWebElement messages = frame->documentElement().findFirst("#messages");
		QWebElement showAllButton = frame->documentElement().findFirst("#show_all_button");

		if (!messages.isNull())
		{
			for (int i = 0; i < (m_currentSession->messages().count() - m_maxScrollBufferLength); ++i)
			{
				log2Html(oStream, m_currentSession->messages().at(i));
				++m_scrollBufferLength;
			}
			messages.prependInside(html);
		}

		showAllButton.addClass("hidden");
		m_maxScrollBufferLength = 0;
	}
}
