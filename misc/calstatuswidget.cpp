#include "calstatuswidget.h"
#include <syslogparser.h>

CALStatusWidget::CALStatusWidget(SysLogParser *parser, QWidget *parent) :
	QLabel(parent)
{
	setText("<img height='14' src=':/icons/icons/status_unknown.png'>");
	hide();

	connect(parser, SIGNAL(logOpened(bool)), this, SLOT(setShown(bool))); // only show while connected
	connect(parser, SIGNAL(calStopped(int)), this, SLOT(onCALStopped(int)));
	connect(parser, SIGNAL(calStarted()), this, SLOT(onCALStarted()));
	connect(parser, SIGNAL(logClosed()), this, SLOT(onLogClosed()));
	connect(parser, SIGNAL(logClosed()), this, SLOT(hide()));
}

void CALStatusWidget::onCALStarted()
{
	setText("<img height='14' src=':/icons/icons/status_running.png'>");
	setToolTip("CAL is running");
}

void CALStatusWidget::onCALStopped(int exitCode)
{
	setText("<img height='14' src=':/icons/icons/status_stopped.png'>");
	setToolTip(QString("CAL stopped (exit code: %1)").arg(exitCode));
}

void CALStatusWidget::onLogClosed()
{
	setText("<img height='14' src=':/icons/icons/status_unknown.png'>");
	setToolTip("");
}
