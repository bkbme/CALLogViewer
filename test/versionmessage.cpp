#include "versionmessage.h"

#include <QByteArray>

VersionMessage::VersionMessage(quint8 seq) :
	AbstractMessage(seq)
{
}

VersionMessage::VersionMessage(const QByteArray &rawData) :
	AbstractMessage(rawData.size() == 12 ? static_cast<quint8>(rawData.at(2)) : 0)
{
	if (rawData.size() == 12)
	{
		if (static_cast<quint8>(rawData.at(0)) == identifier())
		{
			m_data.append(rawData.mid(3, 8));
		}
	
		if (checksum() != static_cast<quint8>(rawData.at(11)))
		{
			m_data.clear(); //invalidate message
		}
	}
}

bool VersionMessage::isValid() const
{
	return (buildDate().isValid() && buildNo() >= 0);
}

QString VersionMessage::version() const
{
	if (isValid())
	{
		return QString(m_data.data());
	}

	return QString();
}

QDate VersionMessage::buildDate() const
{
	if (m_data.size() == 8)
	{
		return QDate::fromString(QString(m_data).left(6), "yyMMdd");
	}

	return QDate();
}

int VersionMessage::buildNo() const
{
	if (m_data.size() == 8)
	{
		bool isInteger = false;
		int build = m_data.right(2).toInt(&isInteger);
		return (isInteger ? build : -1);
	}

	return -1;
}
