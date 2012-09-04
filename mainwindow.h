#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>
#include <QDebug>

namespace Ui {
	class MainWindow;
}

class FemtecTester;
class LogAnalyzer;
class SysLogLoader;
class ServiceManager;
class QCheckBox;
class QLabel;

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
    void onSupportInfoDropped(const QUrl& url);
	void on_pbAddMarker_clicked();
	void on_actionCALStart_triggered();
	void on_actionCALStop_triggered();
	void on_actionCALRestart_triggered();
	void on_actionCALStatus_triggered();
	void on_actionFWStart_triggered();
	void on_actionFWStop_triggered();
	void on_actionFWRestart_triggered();
	void on_actionFWStatus_triggered();
	void on_actionFemtecTesterSettings_triggered();
//	void on_actionFemtecTesterEnabled_toggled(bool enabled);
	
	void onCALStarted();
	void onCALStoped(int exitCode);

protected:
	virtual void closeEvent(QCloseEvent *event);

private:
	void loadSettings();
	QList<QCheckBox*> getLogFilter();

	Ui::MainWindow *ui;
	LogAnalyzer *m_analyzer;
	SysLogLoader *m_logLoader;
	ServiceManager *m_serviceMgr;
	QLabel *m_lStatus;
	FemtecTester *m_test;
};

#endif // MAINWINDOW_H
