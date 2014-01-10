#include "mainwindow.h"

#include <config.h>
#include <aboutdialog.h>
#include <searchwidget.h>
#include <ui_mainwindow.h>
#include <servicemanager.h>
#include <syslogparser.h>
#include <femtotester.h>
#include <procedurefootswitch.h>
#include <calsessionmodel.h>
#include <calstatuswidget.h>
#include <settingsdialog.h>
#include <logsettingspage.h>
#include <docksettingspage.h>
#include <testersettingspage.h>
#include <supportinfoopendialog.h>
#include <testerstatuswidget.h>
#include <autodock.h>

#include <QNetworkAccessManager>
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QLineEdit>
#include <QApplication>
#include <QStringListModel>
#include <QMetaObject>
#include <stdlib.h>

const int MAX_HISTORY_ASC = 10;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_logParser(0),
	m_serviceMgr(0),
	m_test(new FemtoTester(this)),
	m_dock(0),
	m_fs(0),
	m_ascHistoryModel(new QStringListModel(this))
{
	QNetworkAccessManager *netMgr = new QNetworkAccessManager(this);
	m_logParser = new SysLogParser(netMgr, this);
	m_serviceMgr = new ServiceManager(netMgr, this);
	m_fs = new ProcedureFootswitch(m_logParser, m_test, this);
	m_dock = new AutoDock(m_logParser, m_test, this);

	ui->setupUi(this);
	statusBar()->addPermanentWidget(new TesterStatusWidget(m_test, m_dock));
	statusBar()->addPermanentWidget(new CALStatusWidget(m_logParser, this));
	setWindowTitle(QString("%1 %2.%3%4").arg(APPLICATION_NAME).arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(DEVELOPER_BUILD ? " beta" : ""));
	ui->actionOpen_SupportInfo->setEnabled(SysLogParser::canHandleTarArchive());

	//setup actions
	ui->tbCALStart->setDefaultAction(ui->actionService_cal_start);
	ui->tbCALStop->setDefaultAction(ui->actionService_cal_stop);
	ui->tbCALRestart->setDefaultAction(ui->actionService_cal_restart);
	ui->tbFWStart->setDefaultAction(ui->actionService_argesfw_start);
	ui->tbFWStop->setDefaultAction(ui->actionService_argesfw_stop);
	ui->tbFWRestart->setDefaultAction(ui->actionService_argesfw_restart);

	// setup signal/slots
	connect(m_logParser, SIGNAL(logClosed()), this, SLOT(onLogClosed()));
	connect(m_logParser, SIGNAL(logOpened(bool)), this, SLOT(onLogOpened(bool)));
	connect(m_logParser, SIGNAL(logOpened(bool)), ui->wvLogView, SLOT(clear()));
	connect(m_logParser, SIGNAL(statusMessage(QString,int)), ui->statusBar, SLOT(showMessage(QString,int)));

	connect(m_serviceMgr, SIGNAL(serviceStarted(QString)), this, SLOT(onServiceStarted(QString)));
	connect(m_serviceMgr, SIGNAL(serviceStopped(QString)), this, SLOT(onServiceStopped(QString)));

	connect(ui->wvLogView, SIGNAL(supportInfoDropped(QUrl)), this, SLOT(onSupportInfoDropped(QUrl)));
	connect(ui->tvSessionView, SIGNAL(supportInfoDropped(QUrl)), this, SLOT(onSupportInfoDropped(QUrl)));
	connect(ui->cbHost->lineEdit(), SIGNAL(returnPressed()), ui->pbConnect, SIGNAL(clicked()));

	connect(ui->pbUnDock, SIGNAL(clicked()), m_dock, SLOT(undock()));
	connect(ui->pbDockUp, SIGNAL(pressed()), m_dock, SLOT(moveUp()));
	connect(ui->pbDockUp, SIGNAL(released()), m_dock, SLOT(stop()));
	connect(ui->pbDockDown, SIGNAL(pressed()), m_dock, SLOT(moveDown()));
	connect(ui->pbDockDown, SIGNAL(released()), m_dock, SLOT(stop()));
	connect(ui->pbProcedureFS, SIGNAL(pressed()), m_fs, SLOT(pressProcedureFootswitch()));
	connect(ui->pbProcedureFS, SIGNAL(released()), m_fs, SLOT(releaseProcedureFootswitch()));
	connect(ui->actionFemtoTesterEnabled, SIGNAL(toggled(bool)), m_test, SLOT(setEnabled(bool)));

	connect(ui->wSearch, SIGNAL(newSearch(QString,Qt::CaseSensitivity)), ui->wvLogView, SLOT(search(QString,Qt::CaseSensitivity)));
	connect(ui->wSearch, SIGNAL(continueSearch(SearchDirection)), ui->wvLogView, SLOT(continueSearch(SearchDirection)));
	connect(ui->wSearch, SIGNAL(closed()), ui->wvLogView, SLOT(clearSearch()));

	connect(m_test, SIGNAL(connectedStateChanged(bool)), ui->actionFemtoTesterEnabled, SLOT(setChecked(bool)));
	connect(m_test, SIGNAL(statusMessage(QString,int)), statusBar(), SLOT(showMessage(QString, int)));
	connect(m_fs, SIGNAL(statusMessage(QString,int)), statusBar(), SLOT(showMessage(QString,int)));

	CALSessionModel *sessionModel = new CALSessionModel(m_logParser, this);
	ui->tvSessionView->setModel(sessionModel);

	ui->cbHost->setModel(m_ascHistoryModel);

	loadSettings();

	if(QApplication::arguments().size() > 1) // load support-info/log file on startup if given as an argument on cmd-line...
	{
		const QString path = QApplication::arguments().at(1);
		bool isSupportInfo = path.endsWith(".tar.gz");
		QMetaObject::invokeMethod((isSupportInfo ? static_cast<QObject*>(this) : static_cast<QObject*>(m_logParser)),
								  (isSupportInfo ? "onSupportInfoDropped" : "openLog"),
								  Qt::QueuedConnection, Q_ARG(QUrl, QUrl(path)));
		return;
	}

	if (QSettings().value("ASC/autoConnect", false).toBool())
	{
		QMetaObject::invokeMethod(this, "on_pbConnect_clicked", Qt::QueuedConnection);
	}
}

MainWindow::~MainWindow()
{
	m_serviceMgr->disconnect();
	m_logParser->disconnect();
	m_dock->disconnect();
	m_fs->disconnect();

	delete ui;
	delete m_dock;
	delete m_fs;
	delete m_test;
}

void MainWindow::onLogOpened(bool continuous)
{
	ui->pbConnect->setText(continuous ? "Disconnect" : "Cancel loading...");
	ui->pbConnect->setEnabled(true);
	ui->cbHost->setEnabled(false);
	m_serviceMgr->setHost(ui->cbHost->currentText());
	ui->menuServices->setEnabled(true);
	ui->gbServices->setEnabled(true);
}

void MainWindow::onLogClosed()
{
	ui->pbConnect->setText("Connect");
	ui->pbConnect->setEnabled(true);
	ui->cbHost->setEnabled(true);
	ui->menuServices->setEnabled(false);
	ui->gbServices->setEnabled(false);

	if (m_logParser->isLogContinuous())
	{
		setWindowTitle(windowTitle() + " (Disconnected)");
	}
}

void MainWindow::on_pbConnect_clicked()
{
	ui->pbConnect->setEnabled(false);
	ui->cbHost->setEnabled(false);

	if (m_logParser->isLogOpen())
	{
		m_logParser->closeLog();
	} else
	{
		const QString host = ui->cbHost->currentText().toLower().trimmed();
		if (!m_ascHistoryModel->stringList().contains(host))
		{
			m_ascHistoryModel->insertRows(0, 1);
			m_ascHistoryModel->setData(m_ascHistoryModel->index(0), host);
			if (m_ascHistoryModel->rowCount() > MAX_HISTORY_ASC)
			{
				m_ascHistoryModel->removeRows(m_ascHistoryModel->rowCount() - 1, 1);
			}
			ui->cbHost->setCurrentIndex(0);
		}

		QUrl url(QString("http://%1:81/syslog.sh").arg(host));
		int maxHistoryLength = QSettings().value("ASC/maxHistory", 10000).toInt();
		if (maxHistoryLength > 0)
		{
			url.addQueryItem("maxLength", QString::number(maxHistoryLength));
		}
		//url.addQueryItem("maxLength", QString::number(20000)); // todo: add to settings dialog

		m_logParser->openLog(QUrl(url));
		setWindowTitle(QString("%1 - %2").arg(APPLICATION_NAME, url.host()));
	}
}

void MainWindow::on_actionOpen_Logfile_triggered()
{
	const QString filter = ("Syslog (messages messages.* syslog);; All Files (*)");
	QString logFile = QFileDialog::getOpenFileName(this, "Open Logfile", QDir::homePath(), filter);

	if (logFile.isNull())
	{
		return; // user pressed cancel button
	}

	if (m_logParser->isLogOpen())
	{
		m_logParser->closeLog();
	}

	m_logParser->openLog(logFile);
	setWindowTitle(QString("%1 - %2").arg(APPLICATION_NAME, QFileInfo(logFile).fileName()));
}

void MainWindow::on_actionOpen_SupportInfo_triggered()
{
	onSupportInfoDropped(QUrl());
}

void MainWindow::onSupportInfoDropped(const QUrl &url)
{
	SupportInfoOpenDialog dialog(m_logParser, url);
	if (dialog.exec() == QDialog::Accepted)
	{
		if (m_logParser->isLogOpen())
		{
			m_logParser->closeLog();
		}

		m_logParser->openSupportInfo(dialog.selectedSupportInfoFile(), dialog.selectedLogfiles());
		setWindowTitle(QString("%1 - %2").arg(APPLICATION_NAME, QFileInfo(dialog.selectedSupportInfoFile()).fileName()));
	}
}

void MainWindow::onServiceStarted(const QString &service)
{
	this->statusBar()->showMessage(QString("%1 is running.").arg(service));
}

void MainWindow::onServiceStopped(const QString &service)
{
	this->statusBar()->showMessage(QString("%1 is stopped.").arg(service));
}

void MainWindow::onServiceActionTriggered()
{
	QStringList senderInfo(sender() ? sender()->objectName().split('_', QString::SkipEmptyParts) : QStringList());
	if (senderInfo.count() == 3 && senderInfo.at(0) == "actionService")
	{
		m_serviceMgr->invoke(senderInfo.at(1), senderInfo.at(2));
	}
}

void MainWindow::loadSettings()
{
	QSettings settings;
	QStringList ascHistory;

	settings.beginGroup("ASC");
	int size = settings.beginReadArray("History");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex(i);
		ascHistory << settings.value("host").toString();
	}
	settings.endArray();

	if (ascHistory.isEmpty())
	{
		ascHistory << "calservice";
	}
	m_ascHistoryModel->setStringList(ascHistory);
	ui->cbHost->setCurrentIndex(settings.value("selectedIndex", 0).toInt());
	settings.endGroup();

	QList<QCheckBox*> logFilter = getLogFilter();

	settings.beginReadArray("filter");
	QCheckBox* cb;
	for(int i=0; i<logFilter.size(); i++) {
		cb = logFilter.at(i);
		settings.setArrayIndex(i);
		cb->setChecked(settings.value(cb->objectName(), true).toBool());
	}
	settings.endArray();
	settings.beginGroup("FemtoTester");
	if(settings.value("connected").toBool() && getenv("AUTOCONNECT_FEMTOTESTER") != NULL)
	{
		ui->cbTesterEnabled->setChecked(true);
		ui->actionFemtoTesterEnabled->trigger();
	}
	settings.endGroup();

	if(!settings.value("showAutoDockUI", "false").toBool())
	{
		ui->pbUnDock->hide();
		ui->pbDockUp->hide();
		ui->pbDockDown->hide();
	}
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
	logFilter.append(ui->cbFilterDebug); //Only append to list!

	return logFilter;
}

void MainWindow::closeEvent(QCloseEvent * /*event*/)
{
	QSettings settings;
	const QStringList ascHistory = m_ascHistoryModel->stringList();

	settings.beginGroup("ASC");
	settings.setValue("selectedIndex", qMax(ui->cbHost->currentIndex(), 0));
	settings.beginWriteArray("History", ascHistory.count());
	for (int i = 0; i < ascHistory.count(); ++i)
	{
		settings.setArrayIndex(i);
		settings.setValue("host", ascHistory.at(i));
	}
	settings.endArray();
	settings.endGroup();

	QList<QCheckBox*> logFilter = getLogFilter();
	settings.beginWriteArray("filter");
	QCheckBox* cb;
	for(int i=0; i<logFilter.size(); i++) {
		cb = logFilter.at(i);
		settings.setArrayIndex(i);
		settings.setValue(cb->objectName(), cb->isChecked());
	}
	settings.endArray();
    settings.beginGroup("FemtoTester");
    if(ui->cbTesterEnabled->isChecked())
    {
        settings.setValue("connected", QVariant(true));
    }
    settings.endGroup();
}

void MainWindow::on_actionSettings_triggered()
{
	QSettings settings;
	SettingsDialog dialog(this);

	dialog.addPage(new LogSettingsPage(this));
	dialog.addPage(new TesterSettingsPage(m_test, m_fs));
	if (m_test->dockAvailable())
	{
		dialog.addPage(new DockSettingsPage(m_dock));
	}

	dialog.exec();
}

void MainWindow::on_action_About_triggered()
{
	AboutDialog().exec();
}
