#ifndef AUTODOCK_H
#define AUTODOCK_H

#include <QObject>

#include <dockingstatemessage.h>

class SysLogParser;
class FemtoTester;

class AutoDock : public QObject
{
	Q_OBJECT
public:
	struct DockingLimits
	{
		DockingLimits(int up, int low) : upper(up), lower(low) {}
		int upper;
		int lower;
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

	qreal zForce() const { return m_zForce; }
	bool forceIsSteady() const { return m_forceIsSteady; }
	
	void loadSettings();
	void saveSettings();

public slots:
	void moveDown();
	void moveUp();
	void stop();
	void undock();
	void tare();
	void setDockingMode(AutoDock::DockingMode mode);
	void setDockingForce(qreal zForce, bool isSteady);
	void setServoSpeedUp(int speed) { m_servoSpeedUp = speed; }
	void setServoSpeedDown(int speed) { m_servoSpeedDown = speed; }
	void setAutoDockingEnabled(bool enabled) { m_autoDock = enabled; }
	void setAutoUndockingEnabled(bool enabled) { m_autoUndock = enabled; }

signals:
	void zForceChanged(qreal force);

private slots:
	void onDockingStateChange(DockingStateMessage::DockingState state);

private:
	SysLogParser *m_parser;
	FemtoTester *m_tester;
	DockingLimits m_softDockingLimits;
	DockingLimits m_regularDockingLimits;
	DockingLimits m_zeroDockingLimits;
	DockingMode m_dockingMode;
	int m_servoSpeedUp;
	int m_servoSpeedDown;
	qreal m_zForce;
	bool m_forceIsSteady;
	bool m_autoDock;
	bool m_autoUndock;
};

#endif // AUTODOCK_H
