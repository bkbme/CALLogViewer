#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
	class MainWindow;
}

class SysLogLoader;
class ServiceManager;
class QCheckBox;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void onLogOpened();
	void onLogClosed();

private slots:
	void onServiceStarted(const QString &service);
	void onServiceStopped(const QString &service);
	void on_pbConnect_clicked();
	void on_actionOpen_Logfile_triggered();
	void on_pbAddMarker_clicked();
	void on_actionCALStart_triggered();
	void on_actionCALStop_triggered();
	void on_actionCALRestart_triggered();
	void on_actionCALStatus_triggered();
	void on_actionFWStart_triggered();
	void on_actionFWStop_triggered();
	void on_actionFWRestart_triggered();
	void on_actionFWStatus_triggered();

protected:
	virtual void closeEvent(QCloseEvent *event);

private:
	void loadSettings();
	QList<QCheckBox*> getLogFilter();

	Ui::MainWindow *ui;
	SysLogLoader *m_logLoader;
	ServiceManager *m_serviceMgr;

};

#endif // MAINWINDOW_H
