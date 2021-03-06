#ifndef FOOTSWITCHMESSAGE_H
#define FOOTSWITCHMESSAGE_H

#include <abstractmessage.h>
#include <procedurefootswitch.h>
#include <messageparser.h>

class FootswitchMessage : public AbstractMessage
{
public:
	FootswitchMessage(quint8 seq, ProcedureFootswitch::FootswitchState state);

	quint8 identifier() const { return MessageParser::IdFootSwitchMessage; }
	bool isValid() const;

	ProcedureFootswitch::FootswitchState state() const;
};

#endif // FOOTSWITCHMESSAGE_H
