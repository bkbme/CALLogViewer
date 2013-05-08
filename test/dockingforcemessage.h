#ifndef DOCKINGFORCEMESSAGE_H
#define DOCKINGFORCEMESSAGE_H

#include <abstractmessage.h>

class DockingForceMessage : public AbstractMessage
{
public:
	DockingForceMessage(quint8 seq, qreal zForce, bool isSteady);
	DockingForceMessage(const QByteArray &rawData);

	quint8 identifier() const { return 0x09; }
	bool isValid() const;

	qreal zForce() const;
	bool isSteady() const;
};

#endif // DOCKINGFORCEMESSAGE_H
