#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <config.h>

AboutDialog::AboutDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AboutDialog)
{
	ui->setupUi(this);
	ui->lAboutText->setText(ui->lAboutText->text()
							.arg(APPLICATION_NAME)
							.arg(VERSION_MAJOR)
							.arg(VERSION_MINOR)
							.arg(DEVELOPER_BUILD ? "beta" : "")
							.arg(qVersion())
							.arg(FEMTO_TEST_VERSION));
}

AboutDialog::~AboutDialog()
{
	delete ui;
}
