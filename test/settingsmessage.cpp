#include "settingsmessage.h"

SettingsMessage::SettingsMessage(quint8 seq, SettingsType type, quint8 key, const QVariant &value) :
	AbstractMessage(seq)
{
	m_data.append(static_cast<quint8>(type));
	m_data.append(key);
	if (value.isValid())
	{
		switch (type)
		{
			case SettingsByte:
				if (value.canConvert(QVariant::Char))
				{
					m_data.append(value.toChar().toAscii());
				}
				break;
			case SettingsWord:
				if (value.canConvert(QVariant::UInt))
				{
					m_data.append(static_cast<quint8>((value.toUInt() & 0xFF00) >> 8));
					m_data.append(static_cast<quint8>(value.toUInt() & 0x00FF));
				}
				break;
			case SettingsArray:
				if (value.canConvert(QVariant::ByteArray))
				{
					m_data.append(value.toByteArray());
				}
				break;
			default:
				break;
		}
	}
}

SettingsMessage::SettingsMessage(const QByteArray &rawData) :
	AbstractMessage(rawData)
{
	if (!isValid() || (rawData.size() > 6 && !value().isValid()) || checksum() != static_cast<quint8>(rawData.at(rawData.size() - 1)))
	{
		m_data.clear();
	}
}

bool SettingsMessage::isValid() const
{
	return (type() != SettingsTypeInvalid && key() < 0xFF);
}

quint8 SettingsMessage::key() const
{
	return (m_data.size() > 1 ? m_data.at(1) : 0xFF);
}

SettingsMessage::SettingsType SettingsMessage::type() const
{
	if (!m_data.isEmpty())
	{
		switch (m_data.at(0))
		{
			case SettingsByte: return SettingsByte;
			case SettingsWord: return SettingsWord;
			case SettingsArray: return SettingsArray;
			default: break;
		}
	}

	return SettingsTypeInvalid;
}

QVariant SettingsMessage::value() const
{
	switch (type())
	{
		case SettingsByte:
			if (m_data.size() == 3)
			{
				return QVariant(static_cast<quint8>(m_data.at(2)));
			}
			break;
		case SettingsWord:
			if (m_data.size() == 4)
			{
				return QVariant(static_cast<quint16>(m_data.at(2)) << 8 & static_cast<quint8>(m_data.at(3)));
			}
			break;
		case SettingsArray:
			if (m_data.size() > 2)
			{
				return QVariant(m_data.mid(2, m_data.size() - 2));
			}
			break;
		default:
			break;
	}

	return QVariant();
}
