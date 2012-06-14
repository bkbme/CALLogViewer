#include "callogview.h"

#include <QDebug>
#include <QUrl>
#include <QSize>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QMetaObject>
#include <QTextStream>

const QString MSG_FORMAT = "<div id='log_%6' class='%2'>%3 %4: %1%5</div>";

const QString PAGE = "<?xml version='1.0' encoding='UTF-8'?>"
					 "<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en_US' lang='en_US'>"
					 "	<head>"
					 "		<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />"
					 "		<title>CAL log view</title>"
					 "		<style type='text/css'>\n%1</style>"
					 "	</head>"
					 "	<body>"
					 "	</body>"
					 "</html>";

const QString STYLESHEET =  ".Error { color: #fe1c3c; } "
							".Warning { color: #ccff00; } "
							".Info { color: #ffffff; } "
							".State { color: #ff00ff; } "
							".SoapIn { color: #0cff00; } "
							".SoapOut { color: #3e70ff; } "
							".Debug { color: #a7a7a7; } "
							".NonCal { color: #797979; } "
							".kernel { color: #656565; }"
							".fw { color: #ff6500; }"
							".hidden { display: none; }"
							".marked { background: blue; display: inline; }"
							"body { font: 12px monospace; background: #000000; }";

CALLogView::CALLogView(QWidget *parent) :
	QWebView(parent),
	m_autoScroll(true),
	m_maxScrollBufferLength(0),
	m_logs(),
	m_logLevelVisibility(),
	m_markers()
{
	settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true); // enable web-inspector
	QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

void CALLogView::init()
{
	setHtml(PAGE.arg(STYLESHEET));
	connect(page()->mainFrame(), SIGNAL(contentsSizeChanged(QSize)), this, SLOT(onContentSizeChanged(QSize)));
}

void CALLogView::onContentSizeChanged(const QSize &size)
{
	if (m_autoScroll)
	{
                page()->mainFrame()->scroll(0, size.height());
	}
}

void CALLogView::newLogMessage(const LogMessage &log)
{
	QString html;
	QTextStream oStream(&html, QIODevice::WriteOnly);

	oStream << "<div id='log_" << log.id() << "' class='" << jsLogLevel(log.level());
	if (log.application().startsWith("fw["))
	{
		oStream << " fw";
	}
	if (log.application().startsWith("kernel"))
	{
		oStream << " kernel";
	}
	if (m_logLevelVisibility.contains(log.level()) && !m_logLevelVisibility[log.level()])
	{
		oStream << " hidden";
	}

	QString msg(log2Html(log));
	applyMarkers(msg);
	oStream << "'>" << msg << "</div>";

	if (page() && page()->mainFrame())
	{
		QWebFrame *frame = page()->mainFrame();
		QWebElement body = frame->documentElement().findFirst("body");
		if (!body.isNull())
		{
			body.appendInside(html);
		}

		m_autoScroll = frame->scrollPosition().y() == frame->scrollBarMaximum(Qt::Vertical);
		m_logs.enqueue(log);
		purgeOldLogs();
	}
}

void CALLogView::setScrollBufferMaxLength(int lines)
{
	m_maxScrollBufferLength = lines;
	purgeOldLogs();
}

void CALLogView::clear()
{
	if (page() && page()->mainFrame())
	{
		QWebFrame *frame = page()->mainFrame();
		QWebElement body = frame->documentElement().findFirst("body");
		if (!body.isNull())
		{
			body.removeAllChildren();
		}
	}

	m_logs.clear();
}

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

void CALLogView::setLogLevelVisible(LogMessage::LogLevel level, bool visible)
{
	if (!page() || !page()->mainFrame())
	{
		return;
	}

	QWebFrame *frame = page()->mainFrame();
	QWebElement body = frame->documentElement().findFirst("body");
	foreach (const LogMessage &log, m_logs)
	{
		if (log.level() == level)
		{
			QWebElement div = body.findFirst(QString("#log_%1").arg(log.id()));
			visible ? div.removeClass("hidden") : div.addClass("hidden");
		}
	}

	m_logLevelVisibility[level] = visible;
}

QString CALLogView::log2Html(const LogMessage &log) const
{
	QString htmlMsg;
	QTextStream oStream(&htmlMsg, QIODevice::WriteOnly);

	oStream << log.timestamp().toString("yyyy-MM-dd HH:mm:ss.zzz ") << log.application() << ": ";

	switch(log.level())
	{
		case LogMessage::Error:
			oStream << "E: ";
			break;
		case LogMessage::Warning:
			oStream << "W: ";
			break;
		case LogMessage::Info:
			oStream << "N: ";
			break;
		case LogMessage::StateMachine:
			oStream << "S: ";
			break;
		case LogMessage::SoapIn:
			oStream << "I: ";
			break;
		case LogMessage::SoapOut:
			oStream << "O: ";
			break;
		case LogMessage::Debug:
			oStream << "D: ";
			break;
		default:
			break;
	}

	oStream << log.messageHtml();
	return htmlMsg;
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
		case LogMessage::NonCAL: // fall
		default:
			return "NonCal";
	}
}

void CALLogView::addMarker(const QString &text)
{
	if (text.isEmpty() || m_markers.contains(text))
	{
		return;
	}

	m_markers << escapeHtml(text);

	int matchingLines = 0;
	int matchesTotal = 0;
	QWebFrame *frame = page()->mainFrame();
	QWebElement body = frame->documentElement().findFirst("body");
	foreach (const LogMessage &log, m_logs)
	{
		QString msg(log2Html(log));
		int matches = applyMarkers(msg);
		if (matches > 0)
		{
			//qDebug() << "applying marker to id #" << log.id();
			QWebElement div = body.findFirst(QString("#log_%1").arg(log.id()));
			div.setInnerXml(msg);
			matchesTotal += matches;
			++matchingLines;
		}
	}

	emit statusMessage(QString("found %1 matches (%2 log messges)").arg(matchesTotal).arg(matchingLines), 0);
}

void CALLogView::clearAllMarkers()
{
	QWebFrame *frame = page()->mainFrame();
	QWebElement body = frame->documentElement().findFirst("body");
	foreach (const LogMessage &log, m_logs)
	{
		const QString msgHtml(log2Html(log));
		foreach (const QString &marker, m_markers)
		{
			if (msgHtml.contains(marker))
			{
				//qDebug() << "removing marker from log id #" << log.id();
				QWebElement div = body.findFirst(QString("#log_%1").arg(log.id()));
				div.setInnerXml(msgHtml);
				break;
			}
		}
	}

	m_markers.clear();
}

int CALLogView::applyMarkers(QString &msg) const
{
	int matches = 0;
	foreach (const QString &marker, m_markers)
	{
		msg.replace(marker, "<div class='marked'>" + marker + "</div>");
		++matches;
	}

	return matches;
}
