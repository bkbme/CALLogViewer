#ifndef DOCKINGSTATEMESSAGE_H
#define DOCKINGSTATEMESSAGE_H

#include <abstractmessage.h>

class DockingStateMessage : public AbstractMessage
{
public:
	enum DockingState
	{
		DockedManually = 0x00,
		DockedBottom = 0x01,
		DockedTop = 0x02,
		DockedLimit = 0x03,
		DockMovingUp = 0x04,
		DockMovingDown = 0x05,
		DockError = 0xFF
	};

	DockingStateMessage(quint8 seq, DockingState state);
	DockingStateMessage(const QByteArray &rawData);
	
	quint8 identifier() const { return 0x0C; }
	bool isValid() const;

	DockingState state() const;
};

#endif // DOCKINGSTATEMESSAGE_H
