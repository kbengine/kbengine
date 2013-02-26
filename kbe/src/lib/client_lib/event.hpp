/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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

#ifndef __CLIENT_EVENT_HPP__
#define __CLIENT_EVENT_HPP__

//#include "client_lib/entity.hpp"
#include "cstdkbe/cstdkbe.hpp"


namespace KBEngine{

typedef int32 EventID;

#define CLIENT_EVENT_UNKNOWN	0
#define CLIENT_EVENT_ENTERWORLD 1
#define CLIENT_EVENT_LEAVEWORLD 2
#define CLIENT_EVENT_ENTERSPACE 3
#define CLIENT_EVENT_LEAVESPACE 4

struct EventData
{
	EventData(EventID v):id(v)
	{
	}

	EventID id;
};

class EventHandle
{
public:
	virtual void kbengine_onEvent(const EventData* lpEventData) = 0;
};

class EventHandler
{
public:
	EventHandler();
	virtual ~EventHandler();
	typedef std::vector<EventHandle*> EVENT_HANDLES;

	bool registerHandle(EventHandle* pHhandle);
	bool deregisterHandle(EventHandle* pHhandle);

	void trigger(const EventData* lpEventData);
protected:
	EVENT_HANDLES eventHandles_;
};


struct EventData_EnterWorld : public EventData
{
	EventData_EnterWorld():
	EventData(CLIENT_EVENT_ENTERWORLD),
	spaceID(0)
	//pEntity(NULL)
	{
	}

	SPACE_ID spaceID;
	//client::Entity* pEntity;
};

struct EventData_LeaveWorld : public EventData
{
	EventData_LeaveWorld():
	EventData(CLIENT_EVENT_LEAVEWORLD),
	spaceID(0)
	//pEntity(NULL)
	{
	}

	SPACE_ID spaceID;
	//client::Entity* pEntity;
};

struct EventData_EnterSpace : public EventData
{
	EventData_EnterSpace():
	EventData(CLIENT_EVENT_ENTERSPACE),
	spaceID(0)
	//pEntity(NULL)
	{
	}

	SPACE_ID spaceID;
	//client::Entity* pEntity;
};

struct EventData_LeaveSpace : public EventData
{
	EventData_LeaveSpace():
	EventData(CLIENT_EVENT_LEAVESPACE),
	spaceID(0)
	//pEntity(NULL)
	{
	}

	SPACE_ID spaceID;
	//client::Entity* pEntity;
};

}
#endif
