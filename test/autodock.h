#ifndef AUTODOCK_H
#define AUTODOCK_H

#include <QObject>

class SysLogParser;
class FemtoTester;

class AutoDock : public QObject
{
	Q_OBJECT
public:
	struct DockingLimits
	{
		DockingLimits(quint16 up, quint16 low) : upper(up), lower(low) {}
		quint16 upper;
		quint16 lower;
	};

	enum DockingMode
	{
		RegularDocking,
		SoftDocking,
		ZeroDocking,
		ManualDocking
	};

	explicit AutoDock(SysLogParser *parser, FemtoTester *tester, QObject *parent = 0);

	void setDockingLimits(DockingMode mode, DockingLimits limits);
	DockingLimits dockingLimits(DockingMode mode) const;
	DockingMode dockingMode() const { return m_dockingMode; }

	int servoSpeedUp() const { return m_servoSpeedUp; }
	int servoSpeedDown() const { return m_servoSpeedDown; }
	bool autoDockingEnabled() const { return m_autoDock; }
	bool autoUndockingEnabled() const { return m_autoUndock; }

	qreal referenceVoltage() const;
	quint16 zForce() const;
	
	void loadSettings();
	void saveSettings();

public slots:
	void moveDown();
	void moveUp();
	void stop();
	void undock();
	void setDockingMode(AutoDock::DockingMode mode);
	void setDockingForce(quint16 dms, quint16 ref);
	void setServoSpeedUp(int speed) { m_servoSpeedUp = speed; }
	void setServoSpeedDown(int speed) { m_servoSpeedDown = speed; }
	void setAutoDockingEnabled(bool enabled) { m_autoDock = enabled; }
	void setAutoUndockingEnabled(bool enabled) { m_autoUndock = enabled; }

signals:
	void zForceChanged(quint16 force);
	void referenceVoltageChanged(qreal voltage);
	
private:
	SysLogParser *m_parser;
	FemtoTester *m_tester;
	DockingLimits m_softDockingLimits;
	DockingLimits m_regularDockingLimits;
	DockingLimits m_zeroDockingLimits;
	DockingMode m_dockingMode;
	int m_servoSpeedUp;
	int m_servoSpeedDown;
	quint16 m_dms;
	quint16 m_ref;
	bool m_autoDock;
	bool m_autoUndock;
};

#endif // AUTODOCK_H
