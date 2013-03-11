#ifndef CALSTATUSWIDGET_H
#define CALSTATUSWIDGET_H

#include <QLabel>

class SysLogParser;

class CALStatusWidget : public QLabel
{
	Q_OBJECT
public:
	explicit CALStatusWidget(SysLogParser *parser, QWidget *parent = 0);

public slots:
	void onLogClosed();
	void onCALStarted();
	void onCALStopped(int exitCode);
};

#endif // CALSTATUSWIDGET_H
