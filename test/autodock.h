#ifndef AUTODOCK_H
#define AUTODOCK_H

#include <QObject>
#include <QVector>

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
	bool autoSlotSelectEnabled() const { return m_autoSlotSelect; }

	qreal zForce() const { return m_zForce; }
	bool forceIsSteady() const { return m_forceIsSteady; }
	
	void loadSettings();
	void saveSettings();

	FemtoTester* tester() const { return m_tester; }
	
	int dockingSlotCount() const { return m_slots.count(); }
	int dockingSlotPosition(int slot) const;
	void setDockingSlotCount(int count);
	void setDockingSlotPosition(int slot, int position);
	int currentDockingSlot() const { return m_currentSlot; }

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
	void setAutoSlotSelectEnabled(bool enabled) { m_autoSlotSelect = enabled; }
	void nextDockingSlot();
	void setDockingSlot(int slot);

signals:
	void zForceChanged(qreal force);

private slots:
	void onDockingStateChange(DockingStateMessage::DockingState state);
	void onTreatmentFinished();

private:
	SysLogParser *m_parser;
	FemtoTester *m_tester;
	DockingStateMessage::DockingState m_dockingState;
	DockingLimits m_softDockingLimits;
	DockingLimits m_regularDockingLimits;
	DockingLimits m_zeroDockingLimits;
	DockingMode m_dockingMode;
	QVector<int> m_slots;
	int m_currentSlot;
	int m_targetSlot;
	int m_servoSpeedUp;
	int m_servoSpeedDown;
	qreal m_zForce;
	bool m_forceIsSteady;
	bool m_autoDock;
	bool m_autoUndock;
	bool m_autoSlotSelect;
};

#endif // AUTODOCK_H
