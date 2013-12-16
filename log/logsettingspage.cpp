#include "logsettingspage.h"
#include "ui_logsettingspage.h"

#include <QSettings>

const bool DEFAULT_AUTO_CONNECT = false;
const int DEFAULT_HISTORY_LENGTH = 10000;
const int DEFAULT_SCROLL_BUFFER_MAX = 10000;

LogSettingsPage::LogSettingsPage(QWidget *parent) :
	AbstractSettingsPage(parent),
	ui(new Ui::LogSettingsPage)
{
	ui->setupUi(this);
	loadSettings();
}

LogSettingsPage::~LogSettingsPage()
{
	delete ui;
}

QIcon LogSettingsPage::icon() const
{
	return QIcon(":/icons/logviewer.png");
}

void LogSettingsPage::accept()
{
	saveSettings();
}

void LogSettingsPage::reject()
{
	// nothing to do here
}

void LogSettingsPage::apply()
{
	saveSettings();
}

void LogSettingsPage::reset()
{
	ui->cbAutoConnect->setChecked(DEFAULT_AUTO_CONNECT);
	ui->sbASCHistoryLength->setValue(DEFAULT_HISTORY_LENGTH);
	ui->sbMaxScrollBuffer->setValue(DEFAULT_SCROLL_BUFFER_MAX);
}

void LogSettingsPage::loadSettings()
{
	QSettings settings;
	settings.beginGroup("ASC");
	ui->cbAutoConnect->setChecked(settings.value("autoConnect", DEFAULT_AUTO_CONNECT).toBool());
	ui->sbASCHistoryLength->setValue(settings.value("maxHistory", DEFAULT_HISTORY_LENGTH).toInt());
	ui->sbMaxScrollBuffer->setValue(settings.value("maxScrollBuffer", DEFAULT_SCROLL_BUFFER_MAX).toInt());
	settings.endGroup();
}

void LogSettingsPage::saveSettings()
{
	QSettings settings;
	settings.beginGroup("ASC");
	settings.setValue("autoConnect", ui->cbAutoConnect->isChecked());
	settings.setValue("maxHistory", ui->sbASCHistoryLength->value());
	settings.setValue("maxScrollBuffer", ui->sbMaxScrollBuffer->value());
	settings.endGroup();
}
