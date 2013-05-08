#ifndef TESTERSTATUSWIDGET_H
#define TESTERSTATUSWIDGET_H

#include <QWidget>
#include <procedurefootswitch.h>

class QLabel;
class AutoDock;
class FemtoTester;

class TesterStatusWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TesterStatusWidget(FemtoTester *tester, AutoDock *dock, QWidget *parent = 0);

public slots:
	void setFootswitchState(ProcedureFootswitch::FootswitchState state);
	void setZForce(qreal force);

	//void setDockingStatusEnabled(bool enabled);

private:
	FemtoTester *m_tester;
	AutoDock *m_dock;
	QLabel *m_lFS;
	QLabel *m_lForce;
};

#endif // TESTERSTATUSWIDGET_H
