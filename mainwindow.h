#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>
#include <QDebug>

namespace Ui {
	class MainWindow;
}

class AutoDock;
class ProcedureFootswitch;
class FemtoTester;
class LogAnalyzer;
class SysLogParser;
class ServiceManager;
class QCheckBox;
class QLabel;
class QStringListModel;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void onLogOpened(bool continuous);
	void onLogClosed();

private slots:
	void onServiceStarted(const QString &service);
	void onServiceStopped(const QString &service);
	void on_pbConnect_clicked();
	void on_actionOpen_Logfile_triggered();
	void on_actionOpen_SupportInfo_triggered();
	void onSupportInfoDropped(const QUrl& url);
	void onServiceActionTriggered();
	void on_actionSettings_triggered();

	void on_action_About_triggered();
	
protected:
	virtual void closeEvent(QCloseEvent *event);

private:
	void loadSettings();
	void toggleSessionView();
	QList<QCheckBox*> getLogFilter();

	Ui::MainWindow *ui;
	SysLogParser *m_logParser;
	ServiceManager *m_serviceMgr;
	FemtoTester *m_test;
	AutoDock *m_dock;
	ProcedureFootswitch *m_fs;
	QStringListModel *m_ascHistoryModel;
};

#endif // MAINWINDOW_H
