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

#include "client_lib/entity_aspect.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "Python.h"

namespace KBEngine{

typedef int32 EventID;

#define CLIENT_EVENT_UNKNOWN	0
#define CLIENT_EVENT_ENTERWORLD 1
#define CLIENT_EVENT_LEAVEWORLD 2
#define CLIENT_EVENT_ENTERSPACE 3
#define CLIENT_EVENT_LEAVESPACE 4
#define CLIENT_EVENT_CREATEDENTITY 5
#define CLIENT_EVENT_LOGIN_SUCCESS 6
#define CLIENT_EVENT_LOGIN_FAILED 7
#define CLIENT_EVENT_LOGIN_GATEWAY_SUCCESS 8
#define CLIENT_EVENT_LOGIN_GATEWAY_FAILED 9
#define CLIENT_EVENT_SCRIPT 10

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

	void fire(const EventData* lpEventData);
protected:
	EVENT_HANDLES eventHandles_;
};

struct EventData_LoginSuccess : public EventData
{
	EventData_LoginSuccess():
	EventData(CLIENT_EVENT_LOGIN_SUCCESS)
	{
	}
};

struct EventData_LoginFailed : public EventData
{
	EventData_LoginFailed():
	EventData(CLIENT_EVENT_LOGIN_FAILED)
	{
	}
};

struct EventData_LoginGatewaySuccess : public EventData
{
	EventData_LoginGatewaySuccess():
	EventData(CLIENT_EVENT_LOGIN_GATEWAY_SUCCESS)
	{
	}
};

struct EventData_LoginGatewayFailed : public EventData
{
	EventData_LoginGatewayFailed():
	EventData(CLIENT_EVENT_LOGIN_GATEWAY_SUCCESS)
	{
	}
};

struct EventData_CreatedEntity : public EventData
{
	EventData_CreatedEntity():
	EventData(CLIENT_EVENT_CREATEDENTITY),
	pEntity(NULL),
	res()
	{
	}

	const EntityAspect* pEntity;
	std::string res;
};

struct EventData_EnterWorld : public EventData
{
	EventData_EnterWorld():
	EventData(CLIENT_EVENT_ENTERWORLD),
	spaceID(0),
	pEntity(NULL),
	res()
	{
	}

	SPACE_ID spaceID;
	const EntityAspect* pEntity;
	std::string res;
	float x, y, z;
	float pitch, roll, yaw;
};

struct EventData_LeaveWorld : public EventData
{
	EventData_LeaveWorld():
	EventData(CLIENT_EVENT_LEAVEWORLD),
	spaceID(0),
	pEntity(NULL)
	{
	}

	SPACE_ID spaceID;
	const EntityAspect* pEntity;
};

struct EventData_EnterSpace : public EventData
{
	EventData_EnterSpace():
	EventData(CLIENT_EVENT_ENTERSPACE),
	spaceID(0),
	pEntity(NULL)
	{
	}

	SPACE_ID spaceID;
	const EntityAspect* pEntity;
};

struct EventData_LeaveSpace : public EventData
{
	EventData_LeaveSpace():
	EventData(CLIENT_EVENT_LEAVESPACE),
	spaceID(0),
	pEntity(NULL)
	{
	}

	SPACE_ID spaceID;
	const EntityAspect* pEntity;
};

struct EventData_Script : public EventData
{
	EventData_Script():
	EventData(CLIENT_EVENT_SCRIPT),
	argsSize(0),
	name(),
	pyDatas(NULL)
	{
	}
	
	~EventData_Script()
	{
		if(pyDatas != NULL)
		{
			Py_DECREF(pyDatas);
		}
	}
	
	uint8 argsSize;
	std::string name;
	PyObject* pyDatas;
};

inline EventData* newKBEngineEvent(EventID v)
{
	switch(v)
	{
		case CLIENT_EVENT_ENTERWORLD:
			return new EventData_EnterWorld();
			break;
		case CLIENT_EVENT_LEAVEWORLD:
			return new EventData_LeaveWorld();
			break;
		case CLIENT_EVENT_ENTERSPACE:
			return new EventData_EnterSpace();
			break;
		case CLIENT_EVENT_LEAVESPACE:
			return new EventData_LeaveSpace();
			break;
		case CLIENT_EVENT_CREATEDENTITY:
			return new EventData_CreatedEntity();
			break;
		case CLIENT_EVENT_LOGIN_SUCCESS:
			return new EventData_LoginSuccess();
			break;
		case CLIENT_EVENT_LOGIN_FAILED:
			return new EventData_LoginFailed();
			break;
		case CLIENT_EVENT_LOGIN_GATEWAY_SUCCESS:
			return new EventData_LoginGatewaySuccess();
			break;
		case CLIENT_EVENT_LOGIN_GATEWAY_FAILED:
			return new EventData_LoginGatewayFailed();
			break;
		case CLIENT_EVENT_SCRIPT:
			return new EventData_Script();
			break;
		default:
			break;
	}

	return NULL;
}

inline EventData* copyKBEngineEvent(const KBEngine::EventData* lpEventData)
{
	KBEngine::EventData* peventdata = NULL;
	switch(lpEventData->id)
	{
		case CLIENT_EVENT_ENTERWORLD:
			peventdata = new EventData_EnterWorld();
			(*static_cast<EventData_EnterWorld*>(peventdata)) = (*static_cast<const EventData_EnterWorld*>(lpEventData));
			break;
		case CLIENT_EVENT_LEAVEWORLD:
			peventdata = new EventData_LeaveWorld();
			(*static_cast<EventData_LeaveWorld*>(peventdata)) = (*static_cast<const EventData_LeaveWorld*>(lpEventData));
			break;
		case CLIENT_EVENT_ENTERSPACE:
			peventdata = new EventData_EnterSpace();
			(*static_cast<EventData_EnterSpace*>(peventdata)) = (*static_cast<const EventData_EnterSpace*>(lpEventData));
			break;
		case CLIENT_EVENT_LEAVESPACE:
			peventdata = new EventData_LeaveSpace();
			(*static_cast<EventData_LeaveSpace*>(peventdata)) = (*static_cast<const EventData_LeaveSpace*>(lpEventData));
			break;
		case CLIENT_EVENT_CREATEDENTITY:
			peventdata = new EventData_CreatedEntity();
			(*static_cast<EventData_CreatedEntity*>(peventdata)) = (*static_cast<const EventData_CreatedEntity*>(lpEventData));
			break;
		case CLIENT_EVENT_LOGIN_SUCCESS:
			peventdata = new EventData_LoginSuccess();
			(*static_cast<EventData_LoginSuccess*>(peventdata)) = (*static_cast<const EventData_LoginSuccess*>(lpEventData));
			break;
		case CLIENT_EVENT_LOGIN_FAILED:
			peventdata = new EventData_LoginFailed();
			(*static_cast<EventData_LoginFailed*>(peventdata)) = (*static_cast<const EventData_LoginFailed*>(lpEventData));
			break;
		case CLIENT_EVENT_LOGIN_GATEWAY_SUCCESS:
			peventdata = new EventData_LoginGatewaySuccess();
			(*static_cast<EventData_LoginGatewaySuccess*>(peventdata)) = (*static_cast<const EventData_LoginGatewaySuccess*>(lpEventData));
			break;
		case CLIENT_EVENT_LOGIN_GATEWAY_FAILED:
			peventdata = new EventData_LoginGatewayFailed();
			(*static_cast<EventData_LoginGatewayFailed*>(peventdata)) = (*static_cast<const EventData_LoginGatewayFailed*>(lpEventData));
			break;
		case CLIENT_EVENT_SCRIPT:
			peventdata = new EventData_Script();
			(*static_cast<EventData_Script*>(peventdata)) = (*static_cast<const EventData_Script*>(lpEventData));
			Py_INCREF(static_cast<EventData_Script*>(peventdata)->pyDatas);
			break;
		default:
			break;
	}

	return peventdata;
}

}
#endif
