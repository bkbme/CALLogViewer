#ifndef FEMTOTESTER_H
#define FEMTOTESTER_H

#include <procedurefootswitch.h>
#include <dockingstatemessage.h>
#include <qextserialport.h>

#include <QByteArray>
#include <QObject>
#include <QQueue>

class AbstractMessage;
class AckMessage;
class ErrorMessage;
class VersionMessage;
class SettingsMessage;
class DockingForceMessage;
class QTimer;

class FemtoTester : public QObject
{
	Q_OBJECT
public:
	enum ByteSettingsKeys				// keep in sync with enum in setting.h (firmware)
	{
		KeyDockServoUp 			= 0x00,
		KeyDockServoDown		= 0x01,
		KeyDockServoUpSlow		= 0x02,
		KeyDockServoDownSlow	= 0x03,
		KeyDockSlotCount		= 0x04,
		ByteSettingsKeyCount	= 0x05	// keep at end!
	};

	enum KeyWordSettings				// must be kept in sync with enum in FemtoTester class
	{
		KeyDockLimitZeroLo		= 0x00,
		KeyDockLimitZeroUp		= 0x01,
		KeyDockLimitSoftLo		= 0x02,
		KeyDockLimitSoftUp		= 0x03,
		KeyDockLimitRegularLo	= 0x04,
		KeyDockLimitRegularUp	= 0x05,
		WordSettingsKeyCount	= 0x06 // keep at end!
	};

	enum ArraySettingsKeys				// keep in sync with enum in setting.h (firmware)
	{
		KeyDockSlots1			= 0x00,
		KeyDockSlots2			= 0x01,
		ArraySettingsKeyCount	= 0x02 // keep at end!
	};
	
	explicit FemtoTester(QObject *parent = 0);
	virtual ~FemtoTester();

	QString port() const;
	bool connected() const { return m_testerConnected; }
	bool dockAvailable() const { return m_dockAvailable; }

public slots:
	void tare();
	void setDockingLimit(int lowerLimit, int upperLimit);
	void setFootswitchState(ProcedureFootswitch::FootswitchState state);
	void getArraySetting(quint8 key);
	void getByteSetting(quint8 key);
	void getWordSetting(quint8 key);
	void setArraySetting(quint8 key, const QByteArray& value);
	void setByteSetting(quint8 key, quint8 value);
	void setWordSetting(quint8 key, quint16 value);
	void stopServo(int id);
	void setServo(int id, int value);
	void setPort(const QString &portName);
	void setEnabled(bool enabled);

	void loadSettings();
	void saveSettings();

protected:
	void sendMessage(AbstractMessage *msg);
	void handleAck(AckMessage *msg);
	void handleError(ErrorMessage *msg);
	void handleVersion(VersionMessage *msg);
	void handleSettings(SettingsMessage *msg);
	void handleDockingForce(DockingForceMessage *msg);
	void handleDockingState(DockingStateMessage *msg);
	void onSendMessageSuccess(AbstractMessage *msg);

signals:
	void connectedStateChanged(bool connected);
	void statusMessage(const QString &msg, int timeout = 0);
	void footswitchState(ProcedureFootswitch::FootswitchState state);
	void dockingForceChanged(qreal zForce, bool isSteady);
	void dockingStateChanged(DockingStateMessage::DockingState state);
	void settingsByteChanged(quint8 key, quint8 value);
	void settingsWordChanged(quint8 key, quint16 value);
	void settingsArrayChanged(quint8 key, const QByteArray &value);

private slots:
	void timeout();
	void readMessages();
	void sendNextMessage();

private:
	QextSerialPort* m_serialPort;
	quint8 m_seqCount;
	QByteArray m_readBuffer;
	QQueue<AbstractMessage*> m_sendBuffer;
	QTimer *m_sendTimer;
	bool m_testerConnected;
	bool m_dockAvailable;
};

#endif // FEMTOTESTER_H
