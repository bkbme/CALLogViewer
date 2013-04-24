#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <abstractsettingspage.h>
#include <QDialogButtonBox>
#include <QListWidgetItem>
#include <QAbstractButton>
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
			page->accept();
		}
	}

	QDialog::accept();
}

void SettingsDialog::reject()
{
	for (int i = 0; i < ui->swPages->count(); ++i)
	{
		AbstractSettingsPage *page = qobject_cast<AbstractSettingsPage*>(ui->swPages->widget(i));
		if (page)
		{
			page->reject();
		}
	}
	QDialog::reject();
}

void SettingsDialog::apply()
{
	for (int i = 0; i < ui->swPages->count(); ++i)
	{
		AbstractSettingsPage *page = qobject_cast<AbstractSettingsPage*>(ui->swPages->widget(i));
		if (page)
		{
			page->apply();
		}
	}
}

void SettingsDialog::reset()
{
	AbstractSettingsPage *page = qobject_cast<AbstractSettingsPage*>(ui->swPages->currentWidget());
	if (page)
	{
		page->reset();
	}
}

void SettingsDialog::buttonClicked(QAbstractButton *button)
{
	switch (ui->buttonBox->buttonRole(button))
	{
		case QDialogButtonBox::ResetRole:
			reset();
			break;
		case QDialogButtonBox::ApplyRole:
			apply();
			break;
		default:
			break;
	}
}

