#include "versionmessage.h"

#include <QByteArray>

VersionMessage::VersionMessage(quint8 seq) :
	AbstractMessage(seq)
{
}

VersionMessage::VersionMessage(const QByteArray &rawData) :
	AbstractMessage(rawData.size() > 11 ? static_cast<quint8>(rawData.at(2)) : 0)
{
	if (rawData.size() == 12 || rawData.size() == 13)
	{
		if (static_cast<quint8>(rawData.at(0)) == identifier())
		{
			m_data.append(rawData.mid(3, rawData.size() - 4));
		}

		if (checksum() != static_cast<quint8>(rawData.at(rawData.size() == 13 ? 12 : 11)))
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
		return QString(m_data.left(8).data());
	}

	return QString();
}

QDate VersionMessage::buildDate() const
{
	if (m_data.size() >= 8)
	{
		return QDate::fromString(QString(m_data).left(6), "yyMMdd");
	}

	return QDate();
}

int VersionMessage::buildNo() const
{
	if (m_data.size() >= 8)
	{
		bool isInteger = false;
		int build = m_data.mid(6, 2).toInt(&isInteger);
		return (isInteger ? build : -1);
	}

	return -1;
}

VersionMessage::BuildTarget VersionMessage::buildTarget() const
{
	if (m_data.size() > 8)
	{
		switch (m_data.at(8))
		{
			case TargetVictusFS: return TargetVictusFS;
			case TargetExcimerFS: return TargetExcimerFS;
			case TargetVictusDock: return TargetVictusDock;
			case TargetEitechDock: return TargetEitechDock;
			default:
				break;
		}
	}

	return TargetVictusFS; // assume victus footswitch by default
}
