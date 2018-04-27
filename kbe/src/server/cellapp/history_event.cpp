// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "history_event.h"	
#include "network/message_handler.h"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
HistoryEvent::HistoryEvent(HistoryEventID id, const Network::MessageHandler& msgHandler, uint32 msglen):
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
