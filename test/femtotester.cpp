#include "femtotester.h"
#include <messageparser.h>
#include <abstractmessage.h>
#include <errormessage.h>
#include <initmessage.h>
#include <ackmessage.h>
#include <versionmessage.h>
#include <settingsmessage.h>
#include <servoctrlmessage.h>
#include <footswitchmessage.h>
#include <dockinglimitmessage.h>
#include <dockingforcemessage.h>
#include <dockingtaremessage.h>
#include <dockingstatemessage.h>
#include <config.h>

#include <QDebug>
#include <QTimer>
#include <QSettings>

const int SEND_TIMER_INTERVAL = 500; // ms
const qint64 SEND_TIMEOUT = 2500; // ms

FemtoTester::FemtoTester(QObject *parent) :
	QObject(parent),
	m_serialPort(new QextSerialPort(QextSerialPort::EventDriven, this)),
	m_seqCount(0),
	m_readBuffer(),
	m_sendBuffer(),
	m_sendTimer(new QTimer(this)),
	m_testerConnected(false),
	m_dockAvailable(false)
{
	connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(readMessages()));
	connect(m_sendTimer, SIGNAL(timeout()), this, SLOT(timeout()));

	loadSettings();
}

FemtoTester::~FemtoTester()
{
	setEnabled(false);
}

void FemtoTester::loadSettings()
{
	QSettings settings;
	settings.beginGroup("FemtoTester");
	setPort(settings.value("port", "").toString());
	settings.endGroup();
}

void FemtoTester::saveSettings()
{
	QSettings settings;
	settings.beginGroup("FemtoTester");
	settings.setValue("port", port());
	settings.endGroup();
}

void FemtoTester::setFootswitchState(ProcedureFootswitch::FootswitchState state)
{
	if (m_testerConnected)
	{
		qDebug() << "FemtoTester: setFootswitchState(" << state << ")";
		sendMessage(new FootswitchMessage(++m_seqCount, state));
	}
}

void FemtoTester::getArraySetting(quint8 key)
{
	if (m_testerConnected)
	{
		qDebug() << "FemtoTester::getArraySetting(): key=" << key;
		sendMessage(new SettingsMessage(++m_seqCount, SettingsMessage::SettingsArray, key));
	}
}

void FemtoTester::getByteSetting(quint8 key)
{
	if (m_testerConnected)
	{
		qDebug() << "FemtoTester::getByteSetting(): key=" << key;
		sendMessage(new SettingsMessage(++m_seqCount, SettingsMessage::SettingsByte, key));
	}
}

void FemtoTester::getWordSetting(quint8 key)
{
	if (m_testerConnected)
	{
		qDebug() << "FemtoTester::getWordSetting(): key=" << key;
		sendMessage(new SettingsMessage(++m_seqCount, SettingsMessage::SettingsWord, key));
	}
}

void FemtoTester::setArraySetting(quint8 key, const QByteArray& value)
{
	if (m_testerConnected)
	{
		qDebug() << "FemtoTester::setArraySetting(): key=" << key << " value=" << value.toHex();
		sendMessage(new SettingsMessage(++m_seqCount, SettingsMessage::SettingsArray, key, value));
	}
}

void FemtoTester::setByteSetting(quint8 key, quint8 value)
{
	if (m_testerConnected)
	{
		qDebug() << "FemtoTester::setByteSetting(): key=" << key << " value=" << value;
		sendMessage(new SettingsMessage(++m_seqCount, SettingsMessage::SettingsByte, key, value));
	}
}

void FemtoTester::setWordSetting(quint8 key, quint16 value)
{
	if (m_testerConnected)
	{
		qDebug() << "FemtoTester::setWordSetting(): key=" << key << " value=" << value;
		sendMessage(new SettingsMessage(++m_seqCount, SettingsMessage::SettingsWord, key, value));
	}
}

void FemtoTester::tare()
{
	if (m_testerConnected)
	{
		qDebug() << "FemtoTester: tare()";
		sendMessage(new DockingTareMessage(++m_seqCount));
	}
}

void FemtoTester::setDockingLimit(int lowerLimit, int upperLimit)
{
	if (m_testerConnected)
	{
		qDebug() << "FemtoTester: setDockingLimit(" << lowerLimit << ", " << upperLimit << ")";
		sendMessage(new DockingLimitMessage(++m_seqCount, lowerLimit, upperLimit));
	}
}

void FemtoTester::stopServo(int id)
{
	if (m_testerConnected)
	{
		qDebug() << "FemtoTester: stopServo(" << id << ")";
		sendMessage(new ServoCtrlMessage(++m_seqCount, id));
	}
}

void FemtoTester::setServo(int id, int value)
{
	if (m_testerConnected)
	{
		qDebug() << "FemtoTester: setServo(" << id << ", " << value << ")";
		sendMessage(new ServoCtrlMessage(++m_seqCount, id, value));
	}
}

void FemtoTester::setPort(const QString &portName)
{
	if (portName.isEmpty() || portName == m_serialPort->portName())
	{
		return;
	}
	
	bool enabled = m_serialPort->isOpen();
	setEnabled(false);
	m_serialPort->setPortName(portName);
	m_serialPort->setBaudRate(BAUD9600);
	m_serialPort->setDataBits(DATA_8);
	m_serialPort->setStopBits(STOP_1);
	m_serialPort->setFlowControl(FLOW_OFF);
	setEnabled(enabled);
}

QString FemtoTester::port() const
{
	return m_serialPort->portName();
}

void FemtoTester::setEnabled(bool enabled)
{
	if (m_serialPort->isOpen() == enabled)
	{
		return;
	}

	while (!m_sendBuffer.isEmpty()) // drop all messages from queue
	{
		delete m_sendBuffer.dequeue();
	}

	if (!enabled)
	{
		if (m_testerConnected)
		{
			m_dockAvailable = false;
			m_testerConnected = false;
			sendMessage(new ErrorMessage(++m_seqCount, ErrorMessage::ErrorDisconnect)); // notify test tool about disconnect (will be send immmediately as send queue is empty)
			emit statusMessage(QString("FemtoTester disconnected"));
			return;
		}

		m_sendTimer->stop();
		m_serialPort->close();

		qDebug() << "FemtoTester: serial port closed: " << m_serialPort->portName();
		emit connectedStateChanged(false);
		return;
	}

	m_sendBuffer.clear();
	m_readBuffer.clear();

	if (m_serialPort->open(QIODevice::ReadWrite))
	{
		/// @todo 2nd generation (rev. B) test tool reports 'ConnectedState' via DCD control line...
		/// if (m_serialPort->lineStatus() & LS_DCD)
		/// {
			qDebug() << "FemtoTester: serial port opened: " << m_serialPort->portName();
			emit statusMessage(QString("establishing connection with FemtoTester on serial port: %1").arg(m_serialPort->portName()));
			sendMessage(new InitMessage(++m_seqCount));
			m_sendTimer->start(SEND_TIMER_INTERVAL);
			return;
		/// }
		/// qDebug() << "Test tool already in use!";
		/// emit statusMessage("Test tool already in use!");
		/// setEnabled(false);
		/// return;
	}

	emit connectedStateChanged(false); // update ui-elements in main-window
	emit statusMessage(QString("FemtoTester: failed to open serial port '%1': %2").arg(m_serialPort->portName(), m_serialPort->errorString()));
}

void FemtoTester::sendMessage(AbstractMessage *msg)
{
	if (msg->identifier() == MessageParser::IdAckMessage) // send ack's...
	{
		if (m_serialPort->write(msg->toByteArray()) != msg->size())
		{
			qDebug() << "FemtoTester: failed to write ack message";
		} /*else
		{
			qDebug() << "FemtoTester: wrote ack: " << msg->toByteArray().toHex();
		} */

		delete msg;
		return;
	}
	
	if (msg->identifier() == MessageParser::IdErrorMessage) // ...and error messages right away...
	{
		if (m_serialPort->write(msg->toByteArray()) != msg->size())
		{
			qDebug() << "FemtoTester: failed to write error message";
		} else
		{
			qDebug() << "FemtoTester: wrote error message: " << msg->toByteArray().toHex();
		}

		delete msg;
		m_testerConnected = false;
		setEnabled(false);
		return;
	}
	
	m_sendBuffer.enqueue(msg); // ...but queue all other messages
	if (m_sendBuffer.size() == 1)
	{
		sendNextMessage();
	}
}

void FemtoTester::sendNextMessage()
{
	if (m_sendBuffer.isEmpty())
	{
		return;
	}

	if (!m_serialPort->isWritable())
	{
		qDebug() << "FemtoTester: failed to send message: serial port is not writable";
		m_testerConnected = false;
		setEnabled(false);
		return;
	}
	
	AbstractMessage *msg = m_sendBuffer.head(); // will be dequeued and deleted when acknowledged by test-tool
	if (m_serialPort->write(msg->toByteArray()) != msg->size())
	{
		qDebug() << "FemtoTester: failed to send message: error writing message";
		m_testerConnected = false;
		setEnabled(false);
		return;
	}

	msg->markAsSent();
	qDebug() << "FemtoTester: wrote message" << msg->toByteArray().toHex();
}

void FemtoTester::readMessages()
{
	AbstractMessage *msg;
	MessageParser parser(&m_readBuffer);
	
	if (!m_serialPort->isReadable())
	{
		qDebug() << "FemtoTester: failed to read message: serial port is not readable";
		sendMessage(new ErrorMessage(++m_seqCount, ErrorMessage::ErrorUnknown));
		return;
	}

	m_readBuffer.append(m_serialPort->readAll());
	//qDebug() << "FemtoTester: read buffer: " << m_readBuffer.toHex();

	while ((msg = parser.nextMessage()) != 0)
	{
		switch (msg->identifier())
		{
			case MessageParser::IdAckMessage:
				handleAck(dynamic_cast<AckMessage*>(msg));
				break;
			case MessageParser::IdErrorMessage:
				handleError(dynamic_cast<ErrorMessage*>(msg));
				break;
			case MessageParser::IdVersionMessage:
				handleVersion(dynamic_cast<VersionMessage*>(msg));
				break;
			case MessageParser::IdDockingForceMessage:
				handleDockingForce(dynamic_cast<DockingForceMessage*>(msg));
				break;
			case MessageParser::IdDockingStateMessage:
				handleDockingState(dynamic_cast<DockingStateMessage*>(msg));
				break;
			case MessageParser::IdSettingsMessage:
				handleSettings(dynamic_cast<SettingsMessage*>(msg));
				break;
			default:
				qDebug() << "FemtoTester: received unknown message (identifier: " << msg->identifier() << ")";
		}
		//qDebug() << "FemtoTester: read message" << msg->toByteArray().toHex();
		delete msg;
	}
}

void FemtoTester::onSendMessageSuccess(AbstractMessage *msg)
{
	if (!msg || !msg->isValid())
	{
		return;
	}

	SettingsMessage *sMsg;
	FootswitchMessage *fsMsg;
	switch (msg->identifier())
	{
		case MessageParser::IdSettingsMessage:
			sMsg = dynamic_cast<SettingsMessage*>(msg);
			if (sMsg && sMsg->value().isValid())
			{
				switch (sMsg->type())
				{
					case SettingsMessage::SettingsByte:
						emit settingsByteChanged(sMsg->key(), static_cast<quint8>(sMsg->value().toUInt()));
						break;
					case SettingsMessage::SettingsWord:
						emit settingsWordChanged(sMsg->key(), static_cast<quint16>(sMsg->value().toUInt()));
						break;
					default:
						/// @todo implement string type
						break;
				}
			}
			break;
		case MessageParser::IdFootSwitchMessage:
			fsMsg = dynamic_cast<FootswitchMessage*>(msg);
			if (fsMsg)
			{
				emit footswitchState(fsMsg->state());
			}
			break;
		default:
			break;
	}
}

void FemtoTester::handleAck(AckMessage *msg)
{
	if (!msg || !msg->isValid())
	{
		qDebug() << "FemtoTester: received invalid AckMessage";
		sendMessage(new ErrorMessage(++m_seqCount, ErrorMessage::ErrorParser));
		return;
	}

	if (!m_sendBuffer.isEmpty() && m_sendBuffer.head()->sequence() == msg->acknowledgedSeq())
	{
		AbstractMessage *msgSent = m_sendBuffer.dequeue();
		onSendMessageSuccess(msgSent);
		delete msgSent;
		sendNextMessage();
		//qDebug() << "FemtoTester: received ack for message #" << msg->sequence();
		return;
	}

	qDebug() << "FemtoTester: received unexpected AckMessage for seq #" << msg->acknowledgedSeq();
	sendMessage(new ErrorMessage(++m_seqCount, ErrorMessage::ErrorSequence, msg->sequence()));
}

void FemtoTester::handleError(ErrorMessage *msg)
{
	if (!msg || !msg->isValid())
	{
		qDebug() << "FemtoTester: received invalid ErrorMessage";
		sendMessage(new ErrorMessage(++m_seqCount, ErrorMessage::ErrorParser));
		return;
	}

	m_testerConnected = false;
	setEnabled(false);

	qDebug() << "FemtoTester: received ErrorMessage (error code: " << msg->errorCode() << " seq #" << msg->errorSeq() << ")";
}

void FemtoTester::handleVersion(VersionMessage *msg)
{
	if (!msg || !msg->isValid())
	{
		qDebug() << "FemtoTester: invalid VersionMessage passed to handleVersion()";
		sendMessage(new ErrorMessage(++m_seqCount, ErrorMessage::ErrorParser));
		return;
	}

	if (msg->version().startsWith(FEMTO_TEST_VERSION)) // check build-date only (first 6 byte)
	{
		sendMessage(new AckMessage(++m_seqCount, msg->sequence()));
		m_testerConnected = true;
		m_dockAvailable = msg->buildTarget() == VersionMessage::TargetVictusDock || msg->buildTarget() == VersionMessage::TargetEitechDock;
		qDebug() << "FemtoTester connected. Firmware: " << msg->version() << " build target: " << msg->buildTarget();
		emit statusMessage("FemtoTester connected");
		emit connectedStateChanged(true);
		return;
	}

	emit statusMessage(QString("FemtoTester found, but FirmwareVersion '%1' is not supported (expected: '%2xx')").arg(msg->version(), FEMTO_TEST_VERSION));
	qDebug() << QString("FemtoTester found, but FirmwareVersion '%1' is not supported (expected: '%2xx')").arg(msg->version(), FEMTO_TEST_VERSION);
	sendMessage(new ErrorMessage(++m_seqCount, ErrorMessage::ErrorVersion));
}

void FemtoTester::handleSettings(SettingsMessage *msg)
{
	if (!msg || !msg->isValid())
	{
		qDebug() << "FemtoTester: invalid SettingsMessage passed to handleSettings()";
		sendMessage(new ErrorMessage(++m_seqCount, ErrorMessage::ErrorParser));
		return;
	}

	if (!msg->value().isValid())
	{
		qDebug() << "FemtoTester: empty (request) SettingsMessage passed to handleSettings()";
		return;
	}

	switch (msg->type())
	{
		case SettingsMessage::SettingsByte:
			qDebug() << QString("FemtoTester received SettingsMessage: type=Byte key=%1 value=%2").arg(msg->key()).arg(msg->value().toUInt());
			emit settingsByteChanged(msg->key(), static_cast<quint8>(msg->value().toUInt()));
			break;
		case SettingsMessage::SettingsWord:
			qDebug() << QString("FemtoTester received SettingsMessage: type=Word key=%1 value=%2").arg(msg->key()).arg(msg->value().toUInt());
			emit settingsWordChanged(msg->key(), static_cast<quint16>(msg->value().toUInt()));
			break;
		case SettingsMessage::SettingsArray:
			qDebug() << QString("FemtoTester received SettingsMessage: type=Array key=%1 value=%2").arg(msg->key()).arg(QString(msg->value().toByteArray().toHex()));
			emit settingsArrayChanged(msg->key(), msg->value().toByteArray());
			break;
		default:
			qDebug() << QString("FemtoTester::handleSettings(): cannot handle SettingsMessage of unknown type %1").arg(msg->type());
			break;
	}

	sendMessage(new AckMessage(++m_seqCount, msg->sequence()));
}

void FemtoTester::handleDockingForce(DockingForceMessage* msg)
{
	if (!msg || !msg->isValid())
	{
		qDebug() << "FemtoTester: invalid DockingForceMessage passed to handleDockingForce()";
		sendMessage(new ErrorMessage(++m_seqCount, ErrorMessage::ErrorParser));
		return;
	}

	m_dockAvailable = true;
	sendMessage(new AckMessage(++m_seqCount, msg->sequence()));
	emit dockingForceChanged(msg->zForce(), msg->isSteady());
}

void FemtoTester::handleDockingState(DockingStateMessage *msg)
{
	if (!msg || !msg->isValid())
	{
		qDebug() << "FemtoTester: invalid DockingStateMessage passed to handleDockingState()";
		sendMessage(new ErrorMessage(++m_seqCount, ErrorMessage::ErrorParser));
		return;
	}

	qDebug() << "Docking State: " << msg->state();
	sendMessage(new AckMessage(++m_seqCount, msg->sequence()));
	emit dockingStateChanged(msg->state());
}

void FemtoTester::timeout()
{
	if (!m_sendBuffer.isEmpty())
	{
		if (m_sendBuffer.head()->msecsSinceSent() > SEND_TIMEOUT)
		{
			emit statusMessage(QString(m_sendBuffer.head()->identifier() == MessageParser::IdInitMessage ?
										   "FemtoTester not found" : "FemtoTester communication timed out"));
			sendMessage(new ErrorMessage(++m_seqCount, ErrorMessage::ErrorTimeout));
		}
	}
}

