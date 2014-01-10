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
	ui->gbSlots->hide();
	ui->gbAdvOptions->hide();
	ui->lAdvOptionWarning->hide();

	if (m_dock)
	{
		connect(m_dock, SIGNAL(zForceChanged(qreal)), this, SLOT(setZForce(qreal)));
		connect(m_dock, SIGNAL(dockingStateChanged(DockingStateMessage::DockingState)), this, SLOT(setDockingState(DockingStateMessage::DockingState)));

		connect(ui->pbUp, SIGNAL(pressed()), m_dock, SLOT(moveUp()));
		connect(ui->pbDown, SIGNAL(pressed()), m_dock, SLOT(moveDown()));
		connect(ui->pbUp, SIGNAL(released()), m_dock, SLOT(stop()));
		connect(ui->pbDown, SIGNAL(released()), m_dock, SLOT(stop()));
		//connect(ui->cbAutoDock, SIGNAL(toggled(bool)), m_dock, SLOT(setAutoDockingEnabled(bool)));
		//connect(ui->cbAutoUndock, SIGNAL(toggled(bool)), m_dock, SLOT(setAutoUndockingEnabled(bool)));

		connect(ui->sbServoUp, SIGNAL(valueChanged(int)), m_dock, SLOT(setServoSpeedUp(int)));
		connect(ui->sbServoDown, SIGNAL(valueChanged(int)), m_dock, SLOT(setServoSpeedDown(int)));
		connect(ui->sbServoUpSlow, SIGNAL(valueChanged(int)), m_dock, SLOT(setServoSpeedUpSlow(int)));
		connect(ui->sbServoDownSlow, SIGNAL(valueChanged(int)), m_dock, SLOT(setServoSpeedDownSlow(int)));

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

	m_dock->loadHwSettings();
}

void DockSettingsPage::apply()
{
	saveSettings();
}

void DockSettingsPage::reset()
{
	ui->sbLowerZero->setValue(-100);
	ui->sbUpperZero->setValue(10);
	ui->sbLowerSoft->setValue(35);
	ui->sbUpperSoft->setValue(60);
	ui->sbLowerRegular->setValue(300);
	ui->sbUpperRegular->setValue(380);
	ui->rbRegularDocking->setChecked(true);

	m_dock->loadHwSettings();
}

void DockSettingsPage::loadSettings()
{
	if (!m_dock)
	{
		return;
	}

	setDockingState(m_dock->dockingState());
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
	ui->cbAutoSlotSelect->setChecked(m_dock->autoSlotSelectEnabled());
	ui->sbSlotCount->setValue(m_dock->dockingSlotCount());
	ui->sbServoUp->setValue(m_dock->servoSpeedUp());
	ui->sbServoUpSlow->setValue(m_dock->servoSpeedUpSlow());
	ui->sbServoDown->setValue(m_dock->servoSpeedDown());
	ui->sbServoDownSlow->setValue(m_dock->servoSpeedDownSlow());

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

	m_zeroLimits = AutoDock::DockingLimits(ui->sbUpperZero->value(), ui->sbLowerZero->value());
	m_softLimits = AutoDock::DockingLimits(ui->sbUpperSoft->value(), ui->sbLowerSoft->value());
	m_regularLimits = AutoDock::DockingLimits(ui->sbUpperRegular->value(), ui->sbLowerRegular->value());
	
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
	m_dock->setAutoSlotSelectEnabled(ui->cbAutoSlotSelect->isChecked());

	m_dock->saveHwSettings();
	m_dock->saveSettings();
}

void DockSettingsPage::setZForce(qreal force)
{
	ui->lZForce->setText(QString::number(force, 'f', 1).append(m_dock->forceIsSteady() ? 'g' : ' '));
}

void DockSettingsPage::setDockingState(DockingStateMessage::DockingState state)
{
	switch (state)
	{
		case DockingStateMessage::DockedBottom:
			ui->lDockState->setText("DockedBottom");
			break;
		case DockingStateMessage::DockedTop:
			ui->lDockState->setText("DockedTop");
			break;
		case DockingStateMessage::DockedLimit:
			ui->lDockState->setText("DockedLimit");
			break;
		case DockingStateMessage::DockedManually:
			ui->lDockState->setText("DockedManually");
			break;
		case DockingStateMessage::DockMovingUp:
			ui->lDockState->setText("DockMovingUp");
			break;
		case DockingStateMessage::DockMovingDown:
			ui->lDockState->setText("DockMovingDown");
			break;
		default:
			ui->lDockState->setText("Error");
			break;
	}
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
		m_dock->setDockingLimits(AutoDock::ZeroDocking, AutoDock::DockingLimits(ui->sbUpperZero->value(), ui->sbLowerZero->value()));
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
		AutoDock::DockingLimits limits(ui->sbUpperRegular->value(), ui->sbLowerRegular->value());
		m_dock->setDockingLimits(AutoDock::RegularDocking, limits);
		m_dock->setDockingMode(AutoDock::RegularDocking);
	}
	if (ui->rbSoftDocking->isChecked())
	{
		AutoDock::DockingLimits limits(ui->sbUpperSoft->value(), ui->sbLowerSoft->value());
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

	int force = static_cast<int>(m_dock->zForce());
	int lower = qMax(0, force - 50);
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

	int force = static_cast<int>(m_dock->zForce());
	int lower = qMax(0, force - 50);
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

	m_dock->tare();
}


void DockSettingsPage::on_cbSlot_currentIndexChanged(int index)
{
	if (index >= 0 && ui->cbSlot->isVisible())
	{
		m_dock->setDockingSlot(index);
		ui->sbSlotPos->setValue(m_dock->dockingSlotPosition(index));
	}
}

void DockSettingsPage::on_sbSlotPos_valueChanged(int pos)
{
	m_dock->setDockingSlotPosition(ui->cbSlot->currentIndex(), pos);
}

void DockSettingsPage::on_sbSlotCount_valueChanged(int count)
{
	if (count > ui->cbSlot->count())
	{
		while (count != ui->cbSlot->count())
		{
			ui->cbSlot->addItem(QString("Slot %1").arg(ui->cbSlot->count()));
		}
	}
	if (count < ui->cbSlot->count())
	{
		while (count != ui->cbSlot->count())
		{
			ui->cbSlot->removeItem(ui->cbSlot->count() - 1);
		}
	}

	m_dock->setDockingSlotCount(count);
}

void DockSettingsPage::on_btnAdvOpt_toggled(bool checked)
{
	ui->btnAdvOpt->setText(checked ? "Hide advanced options" : "Show advanced options");
}
