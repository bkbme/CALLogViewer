#ifndef SETTINGSMESSAGE_H
#define SETTINGSMESSAGE_H

#include <abstractmessage.h>
#include <messageparser.h>

#include <QVariant>

class SettingsMessage : public AbstractMessage
{
public:
	enum SettingsType			// keep in sync with enum in setting.h (firmware)
	{
		SettingsByte = 0x10,
		SettingsWord = 0x20,
		SettingsArray = 0x40,
		SettingsTypeInvalid = 0xFF
	};
	
	SettingsMessage(quint8 seq, SettingsType type, quint8 key, const QVariant &value = QVariant());
	SettingsMessage(const QByteArray &rawData);

	quint8 identifier() const { return MessageParser::IdSettingsMessage; }
	bool isValid() const;

	quint8 key() const;
	SettingsType type() const;
	QVariant value() const;
};

#endif // SETTINGSMESSAGE_H
