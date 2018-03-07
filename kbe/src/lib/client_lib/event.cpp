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

#include "event.h"
#include "helper/debug_helper.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
EventHandler::EventHandler():
eventHandles_()
{
}

//-------------------------------------------------------------------------------------
EventHandler::~EventHandler()
{
}

//-------------------------------------------------------------------------------------
bool EventHandler::registerHandle(EventHandle* pHhandle)
{
	if(std::binary_search(eventHandles_.begin(), eventHandles_.end(), pHhandle))
	{
		return false;
	}
	
	eventHandles_.push_back(pHhandle);
	return true;
}

//-------------------------------------------------------------------------------------
bool EventHandler::deregisterHandle(EventHandle* pHhandle)
{
	EVENT_HANDLES::iterator iter = eventHandles_.begin();
	for(; iter != eventHandles_.end(); ++iter)
	{
		if((*iter) == pHhandle)
		{
			eventHandles_.erase(iter);
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------
void EventHandler::fire(const EventData* lpEventData)
{
	EVENT_HANDLES::iterator iter = eventHandles_.begin();
	for(; iter != eventHandles_.end(); ++iter)
	{
		(*iter)->kbengine_onEvent(lpEventData);
	}
}

//-------------------------------------------------------------------------------------

}
