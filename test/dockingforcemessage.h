#ifndef DOCKINGFORCEMESSAGE_H
#define DOCKINGFORCEMESSAGE_H

#include <abstractmessage.h>

class DockingForceMessage : public AbstractMessage
{
public:
	DockingForceMessage(quint8 seq, quint16 voltageDMS, quint16 voltageRef);
	DockingForceMessage(const QByteArray &rawData);

	quint8 identifier() const { return 0x09; }
	bool isValid() const;

	quint16 dms() const;
	quint16 ref() const;
};

#endif // DOCKINGFORCEMESSAGE_H
