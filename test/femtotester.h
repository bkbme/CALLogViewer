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
class DockingForceMessage;
class QTimer;

class FemtoTester : public QObject
{
	Q_OBJECT
public:
	explicit FemtoTester(QObject *parent = 0);
	virtual ~FemtoTester();

	QString port() const;
	bool connected() const { return m_testerConnected; }
	bool dockAvailable() const { return m_dockAvailable; }

public slots:
	void tare();
	void setDockingLimit(int lowerLimit, int upperLimit);
	void setFootswitchState(ProcedureFootswitch::FootswitchState state);
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
	void handleDockingForce(DockingForceMessage *msg);
	void handleDockingState(DockingStateMessage *msg);
	void onSendMessageSuccess(AbstractMessage *msg);

signals:
	void connectedStateChanged(bool connected);
	void statusMessage(const QString &msg, int timeout = 0);
	void footswitchState(ProcedureFootswitch::FootswitchState state);
	void dockingForceChanged(qreal zForce, bool isSteady);
	void dockingStateChanged(DockingStateMessage::DockingState state);

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
