#include "autodock.h"
#include "config.h"
#include <femtotester.h>
#include <syslogparser.h>

#include <QSettings>
#include <QDebug>

AutoDock::AutoDock(SysLogParser *parser, FemtoTester *tester, QObject *parent) :
	QObject(parent),
	m_parser(parser),
	m_tester(tester),
	m_dockingState(DockingStateMessage::DockedManually),
	m_softDockingLimits(0, 0),
	m_regularDockingLimits(0, 0),
	m_zeroDockingLimits(0, 0),
	m_dockingMode(ManualDocking),
	m_slots(),
	m_currentSlot(0),
	m_targetSlot(0),
	m_servoSpeedUp(0),
	m_servoSpeedDown(0),
	m_zForce(0),
	m_forceIsSteady(false),
	m_autoDock(false),
	m_autoUndock(false),
	m_autoSlotSelect(false)
{
	loadSettings();
	connect(tester, SIGNAL(dockingForceChanged(qreal,bool)), this, SLOT(setDockingForce(qreal,bool)));
	connect(tester, SIGNAL(dockingStateChanged(DockingStateMessage::DockingState)), this, SLOT(onDockingStateChange(DockingStateMessage::DockingState)));
	connect(m_parser, SIGNAL(dockingModeSelected(AutoDock::DockingMode)), this, SLOT(setDockingMode(AutoDock::DockingMode)));
	connect(m_parser, SIGNAL(suctionRingVacuumDisabled()), this, SLOT(onTreatmentFinished()));
	connect(m_parser, SIGNAL(treatmentAborted()), this, SLOT(onTreatmentFinished()));
	connect(m_parser, SIGNAL(dmsSensorReset()), this, SLOT(undock()));
	connect(m_parser, SIGNAL(treatmentStarted()), this, SLOT(stop())); // lock when treatment starts
}

void AutoDock::loadSettings()
{
	QSettings settings;
	settings.beginGroup("AutoDock");
	m_softDockingLimits.lower = settings.value("lowerSoftDockingLimit", 35).toInt();
	m_softDockingLimits.upper = settings.value("upperSoftDockingLimit", 60).toInt();
	m_zeroDockingLimits.lower = settings.value("lowerZeroDockingLimit", -100).toInt();
	m_zeroDockingLimits.upper = settings.value("upperZeroDockingLimit", 10).toInt();
	m_regularDockingLimits.lower = settings.value("lowerRegularDockingLimit", 300).toInt();
	m_regularDockingLimits.upper = settings.value("upperRegularDockingLimit", 380).toInt();
	m_servoSpeedUp = settings.value("servoSpeedUp", 70).toInt();
	m_servoSpeedDown = settings.value("servoSpeedDown", -70).toInt();
	m_autoDock = settings.value("autoDockEnabled", false).toBool();
	m_autoUndock = settings.value("autoUndockEnabled", false).toBool();
	m_autoSlotSelect = settings.value("autoSlotSelect", false).toBool();

	m_slots.clear();
	int size = settings.beginReadArray("Slot");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex(i);
		m_slots.append(settings.value("position").toInt());
	}
	settings.endArray();
	settings.endGroup();

	if (m_slots.isEmpty())
	{
		m_slots.insert(0, 6); // initialize with default size: 6 slots
	}
}

void AutoDock::saveSettings()
{
	QSettings settings;
	settings.beginGroup("AutoDock");
	settings.setValue("lowerSoftDockingLimit", m_softDockingLimits.lower);
	settings.setValue("upperSoftDockingLimit", m_softDockingLimits.upper);
	settings.setValue("lowerZeroDockingLimit", m_zeroDockingLimits.lower);
	settings.setValue("upperZeroDockingLimit", m_zeroDockingLimits.upper);
	settings.setValue("lowerRegularDockingLimit", m_regularDockingLimits.lower);
	settings.setValue("upperRegularDockingLimit", m_regularDockingLimits.upper);
	settings.setValue("servoSpeedUp", m_servoSpeedUp);
	settings.setValue("servoSpeedDown", m_servoSpeedDown);
	settings.setValue("autoDockEnabled", m_autoDock);
	settings.setValue("autoUndockEnabled", m_autoUndock);
	settings.setValue("autoSlotSelect", m_autoSlotSelect);

	settings.beginWriteArray("Slot", m_slots.count());
	for (int i = 0; i < m_slots.count(); ++i)
	{
		settings.setArrayIndex(i);
		settings.setValue("position", m_slots.at(i));
	}
	settings.endArray();
	settings.endGroup();
}

void AutoDock::moveDown()
{
	m_tester->setServo(SERVO_ZAXIS_ID, m_servoSpeedDown);
}

void AutoDock::moveUp()
{
	m_tester->setServo(SERVO_ZAXIS_ID, m_servoSpeedUp);
}

void AutoDock::stop()
{
	m_tester->stopServo(SERVO_ZAXIS_ID);
}

void AutoDock::undock()
{
	setDockingMode(ZeroDocking);
}

void AutoDock::tare()
{
	m_tester->tare();
}

void AutoDock::setDockingLimits(DockingMode mode, DockingLimits limits)
{
	switch (mode)
	{
		case RegularDocking:
			m_regularDockingLimits = limits;
			break;
		case SoftDocking:
			m_softDockingLimits = limits;
			break;
		case ZeroDocking:
			m_zeroDockingLimits = limits;
			break;
		default:
			qDebug() << "AutoDock::setDockingLimits(...): invalid docking mode";
			break;
	}
}

AutoDock::DockingLimits AutoDock::dockingLimits(DockingMode mode) const
{
	switch (mode)
	{
		case RegularDocking:
			return m_regularDockingLimits;
		case SoftDocking:
			return m_softDockingLimits;
		case ZeroDocking:
			return m_zeroDockingLimits;
		default:
			qDebug() << "AutoDock::dockingLimits(...): invalid docking mode";
			break;
	}

	return DockingLimits(0, 0);
}

void AutoDock::setDockingMode(AutoDock::DockingMode mode)
{
	if (!m_autoDock && sender() == m_parser)
	{
		return;
	}

	switch (mode)
	{
		case RegularDocking:
			m_tester->setDockingLimit(m_regularDockingLimits.lower, m_regularDockingLimits.upper);
			break;
		case SoftDocking:
			m_tester->setDockingLimit(m_softDockingLimits.lower, m_softDockingLimits.upper);
			break;
		case ZeroDocking:
			m_tester->setDockingLimit(m_zeroDockingLimits.lower, m_zeroDockingLimits.upper);
			break;
		case ManualDocking:
			m_tester->stopServo(SERVO_ZAXIS_ID);
			break;
		default:
			qDebug() << "AutoDock::setDockingMode(...): invalid docking mode";
			break;
	}
}

void AutoDock::setDockingForce(qreal zForce, bool isSteady)
{
	if (zForce != m_zForce || isSteady != m_forceIsSteady)
	{
		m_zForce = zForce;
		m_forceIsSteady = isSteady;
		emit zForceChanged(m_zForce);
	}
}

void AutoDock::onDockingStateChange(DockingStateMessage::DockingState state)
{
	m_dockingState = state;
	
	if (state == DockingStateMessage::DockedBottom)
	{
		tare();
		setDockingSlot(m_targetSlot);
	}
}

void AutoDock::onTreatmentFinished()
{
	if (m_autoSlotSelect)
	{
		nextDockingSlot();
		return;
	}

	if (m_autoUndock)
	{
		undock();
	}
}

int AutoDock::dockingSlotPosition(int slot) const
{
	if (slot >= 0 && slot < m_slots.count())
	{
		return m_slots.at(slot);
	}

	return 0;
}


void AutoDock::setDockingSlotCount(int count)
{
	if (count < 1) // min size = 1
	{
		return;
	}

	if (m_slots.count() < count)
	{
		m_slots.insert(0, count - m_slots.count());
	}
	if (m_slots.count() > count)
	{
		m_slots.remove(m_slots.count() - 1, m_slots.count() - count);
		if (m_currentSlot > (count - 1))
		{
			setDockingSlot(count - 1);
		}
	}
}

void AutoDock::setDockingSlotPosition(int slot, int position)
{
	if (slot >= 0 && slot < m_slots.count())
	{
		m_slots[slot] = position;
		if (slot == m_currentSlot)
		{
			m_currentSlot = -1;
			setDockingSlot(slot);
		}
	}
}

void AutoDock::nextDockingSlot()
{
	setDockingSlot((m_currentSlot < (m_slots.count() - 1)) ? m_currentSlot + 1 : 0);
}

void AutoDock::setDockingSlot(int slot)
{
	//qDebug() << "selecting docking slot " << slot;
	if (m_dockingState != DockingStateMessage::DockedBottom) // undock first!
	{
		m_targetSlot = slot;
		moveDown();
		return;
	}

	if (slot != m_currentSlot && slot < m_slots.count() && slot >= 0)
	{
		m_currentSlot = slot;
		m_tester->setServo(SERVO_XAXIS_ID, m_slots.at(slot));
	}
}

