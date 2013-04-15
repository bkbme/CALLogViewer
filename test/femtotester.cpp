#include "femtotester.h"
#include <messageparser.h>
#include <abstractmessage.h>
#include <errormessage.h>
#include <initmessage.h>
#include <ackmessage.h>
#include <versionmessage.h>
#include <footswitchmessage.h>
#include <dockingforcemessage.h>
#include <testerstatuswidget.h>
#include <config.h>

#include <QDebug>
#include <QTimer>
#include <QSettings>

const int SEND_TIMER_INTERVAL = 1000; // ms
const qint64 SEND_TIMEOUT = 1500; // ms

FemtoTester::FemtoTester(QObject *parent) :
	QObject(parent),
	m_serialPort(new QextSerialPort(QextSerialPort::EventDriven, this)),
	m_seqCount(0),
	m_readBuffer(),
	m_sendBuffer(),
	m_sendTimer(new QTimer(this)),
	m_statusWidget(0),
	m_testerConnected(false)
{
	connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(readMessages()));
	connect(m_sendTimer, SIGNAL(timeout()), this, SLOT(timeout()));

	loadSettings();
}

FemtoTester::~FemtoTester()
{
	setEnabled(false);

	// delete status widget unless it has been reparented to another widget
	if (m_statusWidget && !m_statusWidget->parent())
	{
		delete m_statusWidget;
	}
}

TesterStatusWidget *FemtoTester::statusWidget()
{
	if (!m_statusWidget)
	{
		m_statusWidget = new TesterStatusWidget();
		connect(this, SIGNAL(connectedStateChanged(bool)), m_statusWidget, SLOT(setVisible(bool)));
		connect(this, SIGNAL(footswitchState(ProcedureFootswitch::FootswitchState)), m_statusWidget, SLOT(setFootswitchState(ProcedureFootswitch::FootswitchState)));
		m_statusWidget->setVisible(m_testerConnected);
	}

	return m_statusWidget;
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
		qDebug() << "FemtoTester: serial port opened: " << m_serialPort->portName();
		emit statusMessage(QString("establishing connection with FemtoTester on serial port: %1").arg(m_serialPort->portName()));
		sendMessage(new InitMessage(++m_seqCount));
		m_sendTimer->start(SEND_TIMER_INTERVAL);
		return;
	}

	//emit connectedStateChanged(false);
	emit statusMessage(QString("FemtoTester: failed to open serial port '%1': %2").arg(m_serialPort->portName(), m_serialPort->errorString()));
}

void FemtoTester::sendMessage(AbstractMessage *msg)
{
	if (msg->identifier() == MessageParser::IdAckMessage) // send ack's...
	{
		if (m_serialPort->write(msg->toByteArray()) != msg->size())
		{
			qDebug() << "FemtoTester: failed to write ack message";
		} else
		{
			qDebug() << "FemtoTester: wrote ack: " << msg->toByteArray().toHex();
		}

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
			default:
				qDebug() << "FemtoTester: received unknown message (identifier: " << msg->identifier() << ")";
		}
		qDebug() << "FemtoTester: read message" << msg->toByteArray().toHex();
		delete msg;
	}
}

void FemtoTester::onSendMessageSuccess(AbstractMessage *msg)
{
	if (!msg)
	{
		return;
	}

	FootswitchMessage *fsMsg;
	switch (msg->identifier())
	{
		case MessageParser::IdFootSwitchMessage:
			fsMsg = dynamic_cast<FootswitchMessage*>(msg);
			if (fsMsg && fsMsg->isValid())
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
		qDebug() << "FemtoTester: received ack for message #" << msg->sequence();
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
		emit connectedStateChanged(true);
		emit statusMessage("FemtoTester connected");
		qDebug() << "FemtoTester connected";
		return;
	}

	emit statusMessage(QString("FemtoTester found, but FirmwareVersion '%1' is not supported (expected: '%2xx')").arg(msg->version(), FEMTO_TEST_VERSION));
	qDebug() << QString("FemtoTester found, but FirmwareVersion '%1' is not supported (expected: '%2xx')").arg(msg->version(), FEMTO_TEST_VERSION);
	sendMessage(new ErrorMessage(++m_seqCount, ErrorMessage::ErrorVersion));
}

void FemtoTester::handleDockingForce(DockingForceMessage* msg)
{
	if (!msg || !msg->isValid())
	{
		qDebug() << "FemtoTester: invalid DockingForceMessage passed to handleDockingForce()";
		sendMessage(new ErrorMessage(++m_seqCount, ErrorMessage::ErrorParser));
		return;
	}

	/// @todo implement signal for dms/ref voltage

	qDebug() << "FemtoTester: Docking force: vDMS=" << msg->voltageDMS() << "vREF=" << msg->voltageRef();
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

