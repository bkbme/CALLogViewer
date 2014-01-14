#include "autodock.h"
#include "config.h"
#include <femtotester.h>
#include <syslogparser.h>
#include <servoctrlmessage.h>

#include <QSettings>
#include <QVector>
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
	connect(tester, SIGNAL(connectedStateChanged(bool)), this, SLOT(loadHwSettings()));
	connect(tester, SIGNAL(dockingForceChanged(qreal,bool)), this, SLOT(setDockingForce(qreal,bool)));
	connect(tester, SIGNAL(settingsByteChanged(quint8,quint8)), this, SLOT(onHwByteSettingChanged(quint8,quint8)));
	connect(tester, SIGNAL(settingsWordChanged(quint8,quint16)), this, SLOT(onHwWordSettingChanged(quint8,quint16)));
	connect(tester, SIGNAL(settingsArrayChanged(quint8,QByteArray)), this, SLOT(onHwArraySettingChanged(quint8,QByteArray)));
	connect(tester, SIGNAL(dockingStateChanged(DockingStateMessage::DockingState)), this, SLOT(onDockingStateChange(DockingStateMessage::DockingState)));
	connect(tester, SIGNAL(dockingStateChanged(DockingStateMessage::DockingState)), this, SIGNAL(dockingStateChanged(DockingStateMessage::DockingState)));
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
/*	m_softDockingLimits.lower = settings.value("lowerSoftDockingLimit", 35).toInt();
	m_softDockingLimits.upper = settings.value("upperSoftDockingLimit", 60).toInt();
	m_zeroDockingLimits.lower = settings.value("lowerZeroDockingLimit", -100).toInt();
	m_zeroDockingLimits.upper = settings.value("upperZeroDockingLimit", 10).toInt();
	m_regularDockingLimits.lower = settings.value("lowerRegularDockingLimit", 300).toInt();
	m_regularDockingLimits.upper = settings.value("upperRegularDockingLimit", 380).toInt();
*/
	m_autoDock = settings.value("autoDockEnabled", false).toBool();
	m_autoUndock = settings.value("autoUndockEnabled", false).toBool();
	m_autoSlotSelect = settings.value("autoSlotSelect", false).toBool();
	settings.endGroup();
}

void AutoDock::loadHwSettings()
{
	if (m_tester->connected())
	{
		QVector<quint8> keys;
		keys << FemtoTester::KeyDockServoUp
			 << FemtoTester::KeyDockServoUpSlow
			 << FemtoTester::KeyDockServoDown
			 << FemtoTester::KeyDockServoDownSlow
			 << FemtoTester::KeyDockSlotCount;
	
		foreach (const quint8 key, keys)
		{
			m_tester->getByteSetting(key);
		}

		keys.clear();
		keys << FemtoTester::KeyDockLimitZeroLo
			 << FemtoTester::KeyDockLimitZeroUp
			 << FemtoTester::KeyDockLimitSoftLo
			 << FemtoTester::KeyDockLimitSoftUp
			 << FemtoTester::KeyDockLimitRegularLo
			 << FemtoTester::KeyDockLimitRegularUp;

		foreach (const quint8 key, keys)
		{
			m_tester->getWordSetting(key);
		}
	}
}

void AutoDock::onHwByteSettingChanged(quint8 key, quint8 value)
{
	switch (key)
	{
		case FemtoTester::KeyDockServoUp:
			setServoSpeedUp(ServoCtrlMessage::posHw2PosInt(value));
			break;
		case FemtoTester::KeyDockServoUpSlow:
			setServoSpeedUpSlow(ServoCtrlMessage::posHw2PosInt(value));
			break;
		case FemtoTester::KeyDockServoDown:
			setServoSpeedDown(ServoCtrlMessage::posHw2PosInt(value));
			break;
		case FemtoTester::KeyDockServoDownSlow:
			setServoSpeedDownSlow(ServoCtrlMessage::posHw2PosInt(value));
			break;
		case FemtoTester::KeyDockSlotCount:
			setDockingSlotCount(value);
			m_tester->getArraySetting(FemtoTester::KeyDockSlots1); // request slots 0..9...
			if (value > 10)
			{
				m_tester->getArraySetting(FemtoTester::KeyDockSlots2); // and slots 10..19 if necessary
			}
			break;
		default:
			break;
	}
}

void AutoDock::onHwWordSettingChanged(quint8 key, quint16 value)
{
	switch (key)
	{
		case FemtoTester::KeyDockLimitZeroLo:
			m_zeroDockingLimits.lower = static_cast<qint16>(value);
			break;
		case FemtoTester::KeyDockLimitZeroUp:
			m_zeroDockingLimits.upper = static_cast<qint16>(value);
			break;
		case FemtoTester::KeyDockLimitSoftLo:
			m_softDockingLimits.lower = static_cast<qint16>(value);
			break;
		case FemtoTester::KeyDockLimitSoftUp:
			m_softDockingLimits.upper = static_cast<qint16>(value);
			break;
		case FemtoTester::KeyDockLimitRegularLo:
			m_regularDockingLimits.lower = static_cast<qint16>(value);
			break;
		case FemtoTester::KeyDockLimitRegularUp:
			m_regularDockingLimits.upper = static_cast<qint16>(value);
			break;
		default:
			break;
	}
}

void AutoDock::onHwArraySettingChanged(quint8 key, const QByteArray &value)
{
	int i;
	int j = 0;

	switch (key)
	{
		case FemtoTester::KeyDockSlots2:
			j += 10;					// fall
		case FemtoTester::KeyDockSlots1:
			for (i = 0; i < qMin(value.count(), m_slots.count()); ++i)
			{
				setDockingSlotPosition(j + i, static_cast<int>(value.at(i)));
			}
			break;
		default:
			break;
	}
}

void AutoDock::saveSettings()
{
	QSettings settings;
	settings.beginGroup("AutoDock");
/*	settings.setValue("lowerSoftDockingLimit", m_softDockingLimits.lower);
	settings.setValue("upperSoftDockingLimit", m_softDockingLimits.upper);
	settings.setValue("lowerZeroDockingLimit", m_zeroDockingLimits.lower);
	settings.setValue("upperZeroDockingLimit", m_zeroDockingLimits.upper);
	settings.setValue("lowerRegularDockingLimit", m_regularDockingLimits.lower);
	settings.setValue("upperRegularDockingLimit", m_regularDockingLimits.upper);
*/
	settings.setValue("autoDockEnabled", m_autoDock);
	settings.setValue("autoUndockEnabled", m_autoUndock);
	settings.setValue("autoSlotSelect", m_autoSlotSelect);

	settings.endGroup();
}

void AutoDock::saveHwSettings()
{
	if (m_tester->connected())
	{
		m_tester->setWordSetting(FemtoTester::KeyDockLimitZeroLo, static_cast<quint16>(m_zeroDockingLimits.lower));
		m_tester->setWordSetting(FemtoTester::KeyDockLimitZeroUp, static_cast<quint16>(m_zeroDockingLimits.upper));
		m_tester->setWordSetting(FemtoTester::KeyDockLimitSoftLo, static_cast<quint16>(m_softDockingLimits.lower));
		m_tester->setWordSetting(FemtoTester::KeyDockLimitSoftUp, static_cast<quint16>(m_softDockingLimits.upper));
		m_tester->setWordSetting(FemtoTester::KeyDockLimitRegularLo, static_cast<quint16>(m_regularDockingLimits.lower));
		m_tester->setWordSetting(FemtoTester::KeyDockLimitRegularUp, static_cast<quint16>(m_regularDockingLimits.upper));

		m_tester->setByteSetting(FemtoTester::KeyDockServoUp, ServoCtrlMessage::posInt2PosHw(servoSpeedUp()));
		m_tester->setByteSetting(FemtoTester::KeyDockServoDown, ServoCtrlMessage::posInt2PosHw(servoSpeedDown()));
		m_tester->setByteSetting(FemtoTester::KeyDockServoUpSlow, ServoCtrlMessage::posInt2PosHw(servoSpeedUpSlow()));
		m_tester->setByteSetting(FemtoTester::KeyDockServoDownSlow, ServoCtrlMessage::posInt2PosHw(servoSpeedDownSlow()));
		m_tester->setByteSetting(FemtoTester::KeyDockSlotCount, m_slots.count());

		QByteArray buffer;
		foreach (const int value, m_slots)
		{
			buffer.append(static_cast<char>(value));
		}

		m_tester->setArraySetting(FemtoTester::KeyDockSlots1, buffer.left(10));
		if (m_slots.count() > 10)
		{
			m_tester->setArraySetting(FemtoTester::KeyDockSlots2, buffer.right(buffer.count() - 10));
		}
	}
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
	if (count < 1 || count > 20) // min size = 1, max size = 20
	{
		return;
	}

	if (m_slots.count() < count)
	{
		m_slots.insert(m_slots.count(), count - m_slots.count(), 0);
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
	m_targetSlot = slot;
	if (m_dockingState != DockingStateMessage::DockedBottom) // undock first!
	{
		moveDown();
		return;
	}

	if (slot != m_currentSlot && slot < m_slots.count() && slot >= 0)
	{
		m_currentSlot = slot;
		m_tester->setServo(SERVO_XAXIS_ID, m_slots.at(slot));
	}
}

