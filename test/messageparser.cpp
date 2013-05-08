#include "messageparser.h"
#include <ackmessage.h>
#include <errormessage.h>
#include <initmessage.h>
#include <versionmessage.h>
#include <footswitchmessage.h>
#include <dockingforcemessage.h>
#include <dockingstatemessage.h>

#include <QDebug>
#include <QByteArray>

MessageParser::MessageParser(QByteArray *readBuffer) :
	m_buffer(readBuffer)
{
}

AbstractMessage* MessageParser::nextMessage()
{
	if (m_buffer && m_buffer->size() > 2)
	{
		QByteArray msgHeader = m_buffer->left(2);
		if (msgHeader.size() != 2)
		{
			qDebug() << "MessageParser: unexpected error while parsing message";
			return 0 ;
		}

		quint8 identifier = msgHeader.at(0);
		quint8 size = msgHeader.at(1);
		if (m_buffer->size() < size)
		{
			return 0; // not enough data to parse next message
		}

		QByteArray msgData = m_buffer->left(size);
		m_buffer->remove(0, size);
		switch (identifier)
		{
			case IdAckMessage:
				return new AckMessage(msgData);
			case IdErrorMessage:
				return new ErrorMessage(msgData);
			case IdVersionMessage:
				return new VersionMessage(msgData);
			case IdDockingForceMessage:
				return new DockingForceMessage(msgData);
			case IdDockingStateMessage:
				return new DockingStateMessage(msgData);
			default:
				qDebug() << "Failed to parse message (identifier: " << identifier << ")";
				break;
		}
	}

	return 0;
}
