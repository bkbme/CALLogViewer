#ifndef ABSTRACTMESSAGE_H
#define ABSTRACTMESSAGE_H

#include <QTime>
#include <QByteArray>

class AbstractMessage
{
public:
	AbstractMessage(quint8 seq = 0);
	AbstractMessage(const QByteArray &rawData);

	virtual quint8 identifier() const = 0;
	virtual bool isValid() const = 0;
	
	quint8 size() const;
	quint8 sequence() const;
	quint8 checksum() const;

	QByteArray data() const;
	QByteArray toByteArray() const;

	void markAsSent();
	qint64 msecsSinceSent() const;

protected:
	quint8 m_seq;
	QTime m_timeSent;
	QByteArray m_data;
};

#endif // ABSTRACTMESSAGE_H
