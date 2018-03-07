/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

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
