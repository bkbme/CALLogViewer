#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "syslogloader.h"
#include "servicemanager.h"

#include <QNetworkAccessManager>
//#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_logLoader(0),
	m_serviceMgr(0)
{
	ui->setupUi(this);

	//setup actions
	ui->tbCALStart->setDefaultAction(ui->actionCALStart);
	ui->tbCALStop->setDefaultAction(ui->actionCALStop);
	ui->tbCALRestart->setDefaultAction(ui->actionCALRestart);
	ui->tbFWStart->setDefaultAction(ui->actionFWStart);
	ui->tbFWStop->setDefaultAction(ui->actionFWStop);
	ui->tbFWRestart->setDefaultAction(ui->actionFWRestart);

	QNetworkAccessManager *netMgr = new QNetworkAccessManager(this);
	m_logLoader = new SysLogLoader(netMgr, this);
	m_serviceMgr = new ServiceManager(netMgr, this);

	connect(m_logLoader, SIGNAL(logClosed()), this, SLOT(onLogClosed()));
	connect(m_logLoader, SIGNAL(logOpened()), this, SLOT(onLogOpened()));
	connect(m_logLoader, SIGNAL(logOpened()), ui->wvLogView, SLOT(clear()));
	connect(m_logLoader, SIGNAL(newLogMessage(LogMessage)), ui->wvLogView, SLOT(newLogMessage(LogMessage)));
	connect(m_logLoader, SIGNAL(statusMessage(QString,int)), ui->statusBar, SLOT(showMessage(QString,int)));

	connect(m_serviceMgr, SIGNAL(serviceStarted(QString)), this, SLOT(onServiceStarted(QString)));
	connect(m_serviceMgr, SIGNAL(serviceStopped(QString)), this, SLOT(onServiceStopped(QString)));

    connect(ui->wvLogView, SIGNAL(supportInfoDropped(QUrl)), this, SLOT(onSupportInfoDropped(QUrl)));

	ui->wvLogView->setScrollBufferMaxLength(ui->sbScrollBuffer->value());

	loadSettings();

    if(QApplication::arguments().size() > 1) {
        m_logLoader->openLog(QApplication::arguments().at(1));
    }
}

MainWindow::~MainWindow()
{
	m_serviceMgr->disconnect();
	m_logLoader->disconnect();

	delete ui;
}

void MainWindow::onLogOpened()
{
	ui->pbConnect->setText("Disconnect");
	ui->pbConnect->setEnabled(true);
	ui->sbScrollBuffer->setEnabled(true);
	//ui->leHost->setEnabled(true);
	m_serviceMgr->setHost(ui->leHost->text());
	ui->menuServices->setEnabled(true);
	ui->gbServices->setEnabled(true);
}

void MainWindow::onLogClosed()
{
	ui->pbConnect->setText("Connect");
	ui->pbConnect->setEnabled(true);
	ui->sbScrollBuffer->setEnabled(true);
	ui->leHost->setEnabled(true);
	ui->menuServices->setEnabled(false);
	ui->gbServices->setEnabled(false);
}

void MainWindow::on_pbConnect_clicked()
{
	ui->pbConnect->setEnabled(false);
	ui->sbScrollBuffer->setEnabled(false);
	ui->leHost->setEnabled(false);

	if (m_logLoader->isLogOpen())
	{
		m_logLoader->closeLog();
	} else
	{
		QUrl url(QString("http://%1:81/syslog.sh").arg(ui->leHost->text()));
		int maxHistoryLength = ui->sbScrollBuffer->value();
		if (maxHistoryLength > 0)
		{
			url.addQueryItem("maxLength", QString::number(maxHistoryLength));
		}

		m_logLoader->openLog(QUrl(url));
	}
}

void MainWindow::on_actionOpen_Logfile_triggered()
{
	const QString filter = (m_logLoader->canHandleTarArchive() ?
								"Syslog (Support Info (support_info_*.tar.gz);; messages messages.* syslog);; All Files (*)" :
								"Syslog (messages messages.* syslog);; All Files (*)");
	QString logFile = QFileDialog::getOpenFileName(this, "Open Logfile", QDir::homePath(), filter);

	if (logFile.isNull())
	{
		return; // user pressed cancel button
	}

	if (m_logLoader->isLogOpen())
	{
		m_logLoader->closeLog();
	}

    m_logLoader->openLog(logFile);
}

void MainWindow::onSupportInfoDropped(const QUrl &url)
{
    if(url.isLocalFile()) {
        qDebug() << url.toLocalFile();
        m_logLoader->openLog(url.toLocalFile());
    }
}

void MainWindow::on_pbAddMarker_clicked()
{
	ui->wvLogView->addMarker(ui->leMarker->text());
	ui->leMarker->clear();
}

void MainWindow::onServiceStarted(const QString &service)
{
	//QMessageBox::information(this, QString("%1 status").arg(service), QString("%1 is running.").arg(service));
	this->statusBar()->showMessage(QString("%1 is running.").arg(service));
}

void MainWindow::onServiceStopped(const QString &service)
{
	//QMessageBox::information(this, QString("%1 status").arg(service), QString("%1 is stopped.").arg(service));
	this->statusBar()->showMessage(QString("%1 is stopped.").arg(service));
}

void MainWindow::on_actionCALStart_triggered()
{
	m_serviceMgr->startService("cal");
}

void MainWindow::on_actionCALStop_triggered()
{
	m_serviceMgr->stopService("cal");
}

void MainWindow::on_actionCALRestart_triggered()
{
	m_serviceMgr->restartService("cal");
}

void MainWindow::on_actionCALStatus_triggered()
{
	m_serviceMgr->queryService("cal");
}

void MainWindow::on_actionFWStart_triggered()
{
	m_serviceMgr->startService("argesfw");
}

void MainWindow::on_actionFWStop_triggered()
{
	m_serviceMgr->stopService("argesfw");
}

void MainWindow::on_actionFWRestart_triggered()
{
	m_serviceMgr->restartService("argesfw");
}

void MainWindow::on_actionFWStatus_triggered()
{
	m_serviceMgr->queryService("argesfw");
}

void MainWindow::loadSettings()
{
	QSettings settings("TPV-ARGES", "LogView");
	ui->leHost->setText(settings.value("ip", "calservice").toString());
	ui->sbScrollBuffer->setValue(settings.value("scrollBuffer", 2500).toInt());

	QList<QCheckBox*> logFilter = getLogFilter();

	settings.beginReadArray("filter");
	QCheckBox* cb;
	for(int i=0; i<logFilter.size(); i++) {
		cb = logFilter.at(i);
		settings.setArrayIndex(i);
		cb->setChecked(settings.value(cb->objectName(), true).toBool());
	}
	settings.endArray();
}

QList<QCheckBox*> MainWindow::getLogFilter()
{
	QList<QCheckBox*> logFilter;
	logFilter.append(ui->cbFilterError);
	logFilter.append(ui->cbFilterWarning);
	logFilter.append(ui->cbFilterInfo);
	logFilter.append(ui->cbFilterState);
	logFilter.append(ui->cbFilterSoapIn);
	logFilter.append(ui->cbFilterSoapOut);
	logFilter.append(ui->cbFilterDebug);
	logFilter.append(ui->cbFilterNonCAL); //Only append to list!

	return logFilter;
}

void MainWindow::closeEvent(QCloseEvent * /*event*/)
{
	QSettings settings("TPV-ARGES", "LogView");
	settings.setValue("ip", ui->leHost->text());
	settings.setValue("scrollBuffer", ui->sbScrollBuffer->value());

	QList<QCheckBox*> logFilter = getLogFilter();

	settings.beginWriteArray("filter");
	QCheckBox* cb;
	for(int i=0; i<logFilter.size(); i++) {
		cb = logFilter.at(i);
		settings.setArrayIndex(i);
		settings.setValue(cb->objectName(), cb->isChecked());
	}
	settings.endArray();
}
