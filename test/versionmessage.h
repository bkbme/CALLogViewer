#ifndef VERSIONMESSAGE_H
#define VERSIONMESSAGE_H

#include <abstractmessage.h>

#include <QString>
#include <QDate>

class VersionMessage : public AbstractMessage
{
public:
	VersionMessage(quint8 seq);
	VersionMessage(const QByteArray &rawData);

	quint8 identifier() const { return 0x04; }
	bool isValid() const;

	QString version() const;
	QDate buildDate() const;
	int buildNo() const;
};

#endif // VERSIONMESSAGE_H
