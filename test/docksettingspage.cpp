#include "docksettingspage.h"
#include "ui_docksettingspage.h"

#include <femtotester.h>

#include <QIcon>

DockSettingsPage::DockSettingsPage(FemtoTester *tester, QWidget *parent) :
	AbstractSettingsPage(parent),
	ui(new Ui::DockSettingsPage),
	m_tester(tester)
{
	ui->setupUi(this);

	connect(ui->vsServo, SIGNAL(valueChanged(int)), m_tester, SLOT(setServo(int)));
	connect(m_tester, SIGNAL(dockingForceChanged(quint16,quint16)), this, SLOT(setDockingForce(quint16,quint16)));
}

DockSettingsPage::~DockSettingsPage()
{
	delete ui;
}

QIcon DockSettingsPage::icon() const
{
	return QIcon(); /// @todo get proper icon
}

void DockSettingsPage::loadSettings()
{
	/// @todo implement me
}

void DockSettingsPage::saveSettings()
{
	/// @todo implement me
}

void DockSettingsPage::setDockingForce(quint16 dms, quint16 ref)
{
	qreal vRef = (3 * 2.56d * static_cast<qreal>(ref)) / 1024;
	ui->lDMS->setText(QString::number(dms));
	ui->lRef->setText(QString("%2V (%1)").arg(ref).arg(QString::number(vRef, 'f', 2)));
}

void DockSettingsPage::on_pbBottom_clicked()
{
	m_tester->setDockingLimit(0, static_cast<uint>(ui->rbRegularDocking->isChecked() ? ui->sbUpperRegular->value() : ui->sbUpperSoft->value()));
	m_tester->setServo(-40);
}

void DockSettingsPage::on_pbUndock_clicked()
{
	uint upperLimit = static_cast<uint>(ui->rbRegularDocking->isChecked() ? ui->sbUpperRegular->value() : ui->sbUpperSoft->value());
	uint lowerLimit = static_cast<uint>(ui->rbRegularDocking->isChecked() ? ui->sbLowerRegular->value() : ui->sbLowerSoft->value());
	m_tester->setDockingLimit(lowerLimit, upperLimit);
	m_tester->setServo(-40);
}

void DockSettingsPage::on_pbDock_clicked()
{
	uint upperLimit = static_cast<uint>(ui->rbRegularDocking->isChecked() ? ui->sbUpperRegular->value() : ui->sbUpperSoft->value());
	uint lowerLimit = static_cast<uint>(ui->rbRegularDocking->isChecked() ? ui->sbLowerRegular->value() : ui->sbLowerSoft->value());
	m_tester->setDockingLimit(lowerLimit, upperLimit);
	m_tester->setServo(40);
}
