#include "history_event.hpp"	
#include "network/message_handler.hpp"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
HistoryEvent::HistoryEvent(HistoryEventID id, const Mercury::MessageHandler& msgHandler, uint32 msglen):
id_(id),
msglen_(msglen),
msgHandler_(msgHandler)
{
}

//-------------------------------------------------------------------------------------
HistoryEvent::~HistoryEvent()
{
}

//-------------------------------------------------------------------------------------
EventHistory::EventHistory()
{
}

//-------------------------------------------------------------------------------------
EventHistory::~EventHistory()
{
}

//-------------------------------------------------------------------------------------
bool EventHistory::add(HistoryEvent* phe)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EventHistory::remove(HistoryEvent* phe)
{
	return true;
}

//-------------------------------------------------------------------------------------
}
