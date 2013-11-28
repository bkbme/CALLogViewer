#ifndef VERSIONMESSAGE_H
#define VERSIONMESSAGE_H

#include <abstractmessage.h>

#include <QString>
#include <QDate>

class VersionMessage : public AbstractMessage
{
public:
	enum BuildTarget			// keep consistent with firmware/config.h (TARGET)
	{
		TargetVictusFS = 0x01,	// VICTUS_FS_TARGET
		TargetExcimerFS = 0x02,	// EXCIMER_FS_TARGET
		TargetVictusDock = 0x03,// VICTUS_DOCK_TARGET
		TargetEitechDock = 0x04	// EITECH_DOCK_TARGET
	};
	
	VersionMessage(quint8 seq);
	VersionMessage(const QByteArray &rawData);

	quint8 identifier() const { return 0x04; }
	bool isValid() const;

	QString version() const;
	QDate buildDate() const;
	int buildNo() const;
	BuildTarget buildTarget() const;
};

#endif // VERSIONMESSAGE_H
