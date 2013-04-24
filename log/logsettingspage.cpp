#include "logsettingspage.h"
#include "ui_logsettingspage.h"

#include <QSettings>

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
	ui->cbAutoConnect->setChecked(false);
	ui->sbASCHistoryLength->setValue(10000);
}

void LogSettingsPage::loadSettings()
{
	QSettings settings;
	settings.beginGroup("ASC");
	ui->cbAutoConnect->setChecked(settings.value("autoConnect", false).toBool());
	ui->sbASCHistoryLength->setValue(settings.value("maxHistory", 10000).toInt());
	settings.endGroup();
}

void LogSettingsPage::saveSettings()
{
	QSettings settings;
	settings.beginGroup("ASC");
	settings.setValue("autoConnect", ui->cbAutoConnect->isChecked());
	settings.setValue("maxHistory", ui->sbASCHistoryLength->value());
	settings.endGroup();
}
