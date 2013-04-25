#include "autodock.h"
#include <femtotester.h>
#include <syslogparser.h>

#include <QSettings>
#include <QDebug>

AutoDock::AutoDock(SysLogParser *parser, FemtoTester *tester, QObject *parent) :
	QObject(parent),
	m_parser(parser),
	m_tester(tester),
	m_softDockingLimits(0, 0),
	m_regularDockingLimits(0, 0),
	m_zeroDockingLimits(0, 0),
	m_dockingMode(ManualDocking),
	m_servoSpeedUp(40),
	m_servoSpeedDown(-40),
	m_dms(0),
	m_ref(0)
{
	loadSettings();
	connect(tester, SIGNAL(dockingForceChanged(quint16,quint16)), this, SLOT(setDockingForce(quint16,quint16)));
	connect(m_parser, SIGNAL(dockingModeSelected(AutoDock::DockingMode)), this, SLOT(setDockingMode(AutoDock::DockingMode)));
	//connect(m_parser, SIGNAL(treatmentFinished()), this, SLOT(undock()));
	connect(m_parser, SIGNAL(suctionRingVacuumDisabled()), this, SLOT(undock()));
	connect(m_parser, SIGNAL(treatmentAborted()), this, SLOT(undock()));
	connect(m_parser, SIGNAL(dmsSensorReset()), this, SLOT(undock()));
	connect(m_parser, SIGNAL(treatmentStarted()), this, SLOT(stop())); // lock when treatment starts
}

void AutoDock::loadSettings()
{
	QSettings settings;
	settings.beginGroup("AutoDock");
	m_softDockingLimits.lower = static_cast<quint16>(settings.value("lowerSoftDockingLimit", 150).toUInt());
	m_softDockingLimits.upper = static_cast<quint16>(settings.value("upperSoftDockingLimit", 300).toUInt());
	m_zeroDockingLimits.lower = static_cast<quint16>(settings.value("lowerZeroDockingLimit", 700).toUInt());
	m_zeroDockingLimits.upper = static_cast<quint16>(settings.value("upperZeroDockingLimit", 800).toUInt());
	m_regularDockingLimits.lower = static_cast<quint16>(settings.value("lowerRegularDockingLimit", 0).toUInt());
	m_regularDockingLimits.upper = static_cast<quint16>(settings.value("upperRegularDockingLimit", 120).toUInt());
	m_servoSpeedUp = settings.value("servoSpeedUp", 25).toInt();
	m_servoSpeedDown = settings.value("servoSpeedDown", -25).toInt();
	m_autoDock = settings.value("autoDockEnabled", false).toBool();
	m_autoUndock = settings.value("autoUndockEnabled", false).toBool();
	settings.endGroup();
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
	settings.endGroup();
}

void AutoDock::moveDown()
{
	m_tester->setServo(m_servoSpeedDown);
}

void AutoDock::moveUp()
{
	m_tester->setServo(m_servoSpeedUp);
}

void AutoDock::stop()
{
	m_tester->setServo(0);
}

void AutoDock::undock()
{
	if (!m_autoUndock && sender() == m_parser)
	{
		return;
	}

	setDockingMode(ZeroDocking);
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
			m_tester->setServo(0); // disables autodocking
			break;
		default:
			qDebug() << "AutoDock::setDockingMode(...): invalid docking mode";
			break;
	}
}

qreal AutoDock::referenceVoltage() const
{
	return (3 * 2.56 * static_cast<qreal>(m_ref)) / 1024;
}

quint16 AutoDock::zForce() const
{
	return m_dms;
}

void AutoDock::setDockingForce(quint16 dms, quint16 ref)
{
	if (dms != m_dms)
	{
		m_dms = dms;
		emit zForceChanged(m_dms);
	}

	if (ref != m_ref)
	{
		m_ref = ref;
		emit referenceVoltageChanged(referenceVoltage());
	}
}
