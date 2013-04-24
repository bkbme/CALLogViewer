#include "docksettingspage.h"
#include "ui_docksettingspage.h"

#include <QPixmap>
#include <QPushButton>
#include <QIcon>

DockSettingsPage::DockSettingsPage(AutoDock *dock, QWidget *parent) :
	AbstractSettingsPage(parent),
	ui(new Ui::DockSettingsPage),
	m_dock(dock),
	m_zeroLimits(0, 0),
	m_softLimits(0, 0),
	m_regularLimits(0, 0),
	m_dockingMode(AutoDock::ManualDocking)
{
	ui->setupUi(this);

	if (m_dock)
	{
		connect(m_dock, SIGNAL(zForceChanged(quint16)), this, SLOT(setZForce(quint16)));
		connect(m_dock, SIGNAL(referenceVoltageChanged(qreal)), this, SLOT(setReferenceVoltage(qreal)));
		connect(ui->pbUp, SIGNAL(pressed()), m_dock, SLOT(moveUp()));
		connect(ui->pbDown, SIGNAL(pressed()), m_dock, SLOT(moveDown()));
		connect(ui->pbUp, SIGNAL(released()), m_dock, SLOT(stop()));
		connect(ui->pbDown, SIGNAL(released()), m_dock, SLOT(stop()));
		//connect(ui->cbAutoDock, SIGNAL(toggled(bool)), m_dock, SLOT(setAutoDockingEnabled(bool)));
		//connect(ui->cbAutoUndock, SIGNAL(toggled(bool)), m_dock, SLOT(setAutoUndockingEnabled(bool)));

		loadSettings();
	}
}

DockSettingsPage::~DockSettingsPage()
{
	delete ui;
}

QIcon DockSettingsPage::icon() const
{
	return QIcon(QPixmap(":/icons/dock.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void DockSettingsPage::accept()
{
	saveSettings();
}

void DockSettingsPage::reject()
{
	if (!m_dock)
	{
		return;
	}

	m_dock->setDockingLimits(AutoDock::ZeroDocking, m_zeroLimits);
	m_dock->setDockingLimits(AutoDock::SoftDocking, m_softLimits);
	m_dock->setDockingLimits(AutoDock::RegularDocking, m_regularLimits);
}

void DockSettingsPage::apply()
{
	saveSettings();
}

void DockSettingsPage::reset()
{
	ui->sbLowerZero->setValue(0);
	ui->sbUpperZero->setValue(130);
	ui->sbLowerSoft->setValue(150);
	ui->sbUpperSoft->setValue(300);
	ui->sbLowerRegular->setValue(600);
	ui->sbUpperRegular->setValue(700);
	ui->rbRegularDocking->setChecked(true);
}

void DockSettingsPage::loadSettings()
{
	if (!m_dock)
	{
		return;
	}

	m_zeroLimits = m_dock->dockingLimits(AutoDock::ZeroDocking);
	m_softLimits = m_dock->dockingLimits(AutoDock::SoftDocking);
	m_regularLimits = m_dock->dockingLimits(AutoDock::RegularDocking);
	m_dockingMode = m_dock->dockingMode();

	ui->sbLowerZero->setValue(m_zeroLimits.lower);
	ui->sbUpperZero->setValue(m_zeroLimits.upper);
	ui->sbLowerSoft->setValue(m_softLimits.lower);
	ui->sbUpperSoft->setValue(m_softLimits.upper);
	ui->sbLowerRegular->setValue(m_regularLimits.lower);
	ui->sbUpperRegular->setValue(m_regularLimits.upper);
	ui->cbAutoDock->setChecked(m_dock->autoDockingEnabled());
	ui->cbAutoUndock->setChecked(m_dock->autoUndockingEnabled());

	switch (m_dockingMode)
	{
		case AutoDock::RegularDocking:
			ui->rbRegularDocking->setChecked(true);
			break;
		case AutoDock::SoftDocking:
			ui->rbSoftDocking->setChecked(true);
			break;
		default:
			ui->rbSoftDocking->setChecked(false);
			ui->rbRegularDocking->setChecked(false);
			break;
	}
}

void DockSettingsPage::saveSettings()
{
	if (!m_dock)
	{
		return;
	}

	m_zeroLimits = AutoDock::DockingLimits(static_cast<quint16>(ui->sbUpperZero->value()), 0);
	m_softLimits = AutoDock::DockingLimits(static_cast<quint16>(ui->sbUpperSoft->value()), static_cast<quint16>(ui->sbLowerSoft->value()));
	m_regularLimits = AutoDock::DockingLimits(static_cast<quint16>(ui->sbUpperRegular->value()), static_cast<quint16>(ui->sbLowerRegular->value()));
	
	if (ui->rbRegularDocking->isChecked())
	{
		m_dockingMode = AutoDock::RegularDocking;
	}
	if (ui->rbSoftDocking->isChecked())
	{
		m_dockingMode = AutoDock::SoftDocking;
	}

	m_dock->setDockingLimits(AutoDock::ZeroDocking, m_zeroLimits);
	m_dock->setDockingLimits(AutoDock::SoftDocking, m_softLimits);
	m_dock->setDockingLimits(AutoDock::RegularDocking, m_regularLimits);
	m_dock->setAutoDockingEnabled(ui->cbAutoDock->isChecked());
	m_dock->setAutoUndockingEnabled(ui->cbAutoUndock->isChecked());
	m_dock->saveSettings();
}

void DockSettingsPage::setZForce(quint16 force)
{
	ui->lDMS->setText(QString::number(force));
}

void DockSettingsPage::setReferenceVoltage(qreal voltage)
{
	ui->lRef->setText(QString("%1V").arg(QString::number(voltage, 'f', 2)));
}

void DockSettingsPage::on_pbBottom_clicked()
{
	if (m_dock)
	{
		m_dock->moveDown();
	}
}

void DockSettingsPage::on_pbUndock_clicked()
{
	if (m_dock)
	{
		m_dock->setDockingLimits(AutoDock::ZeroDocking, AutoDock::DockingLimits(static_cast<quint16>(ui->sbUpperZero->value()), 0));
		m_dock->setDockingMode(AutoDock::ZeroDocking);
	}
}

void DockSettingsPage::on_pbDock_clicked()
{
	if (!m_dock)
	{
		return;
	}

	if (ui->rbRegularDocking->isChecked())
	{
		AutoDock::DockingLimits limits(static_cast<quint16>(ui->sbUpperRegular->value()), static_cast<quint16>(ui->sbLowerRegular->value()));
		m_dock->setDockingLimits(AutoDock::RegularDocking, limits);
		m_dock->setDockingMode(AutoDock::RegularDocking);
	}
	if (ui->rbSoftDocking->isChecked())
	{
		AutoDock::DockingLimits limits(static_cast<quint16>(ui->sbUpperSoft->value()), static_cast<quint16>(ui->sbLowerSoft->value()));
		m_dock->setDockingLimits(AutoDock::SoftDocking, limits);
		m_dock->setDockingMode(AutoDock::SoftDocking);
	}
}

void DockSettingsPage::on_pbGrabRegular_clicked()
{
	if (!m_dock)
	{
		return;
	}

	quint16 force = m_dock->zForce();
	int lower = qMax(0, (force - 50));
	int upper = qMin(900, force + 50);
	ui->sbLowerRegular->setValue(lower);
	ui->sbUpperRegular->setValue(upper);
}

void DockSettingsPage::on_pbGrabSoft_clicked()
{
	if (!m_dock)
	{
		return;
	}

	quint16 force = m_dock->zForce();
	int lower = qMax(0, (force - 50));
	int upper = qMin(900, force + 75);
	ui->sbLowerSoft->setValue(lower);
	ui->sbUpperSoft->setValue(upper);
}

void DockSettingsPage::on_pbTare_clicked()
{
	if (!m_dock)
	{
		return;
	}

	int upper = qMin(400, m_dock->zForce() + 10); // lower limit is fixed to 0
	ui->sbUpperZero->setValue(upper);
}

