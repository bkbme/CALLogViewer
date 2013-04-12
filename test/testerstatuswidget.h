#ifndef TESTERSTATUSWIDGET_H
#define TESTERSTATUSWIDGET_H

#include <QWidget>
#include <procedurefootswitch.h>

class QLabel;

class TesterStatusWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TesterStatusWidget(QWidget *parent = 0);

public slots:
	void setFootswitchState(ProcedureFootswitch::FootswitchState state);
	void setDockingPressure(double dms);

	void setDockingStatusEnabled(bool enabled);

private:
	QLabel *m_lFS;
	QLabel *m_lDMS;
};

#endif // TESTERSTATUSWIDGET_H
