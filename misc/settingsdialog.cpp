#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <abstractsettingspage.h>
#include <QListWidgetItem>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);
	connect(ui->lwContent, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::addPage(AbstractSettingsPage *page)
{
	QListWidgetItem *item = new QListWidgetItem(page->icon(), page->windowTitle());
	item->setTextAlignment(Qt::AlignHCenter);

	ui->lwContent->addItem(item);
	ui->swPages->addWidget(page); // takes ownership
	page->loadSettings();
}

void SettingsDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
	Q_UNUSED(previous);

	if (current)
	{
		ui->swPages->setCurrentIndex(ui->lwContent->row(current));
	}
}

void SettingsDialog::accept()
{
	for (int i = 0; i < ui->swPages->count(); ++i)
	{
		AbstractSettingsPage *page = qobject_cast<AbstractSettingsPage*>(ui->swPages->widget(i));
		if (page)
		{
			page->saveSettings();
		}
	}

	QDialog::accept();
}
