// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CLIENT_EVENT_H
#define KBE_CLIENT_EVENT_H

#include "client_lib/entity_aspect.h"
#include "common/common.h"
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
#define CLIENT_EVENT_LOGIN_BASEAPP_SUCCESS 8
#define CLIENT_EVENT_LOGIN_BASEAPP_FAILED 9
#define CLIENT_EVENT_SCRIPT 10
#define CLIENT_EVENT_POSITION_CHANGED 11
#define CLIENT_EVENT_DIRECTION_CHANGED 12
#define CLIENT_EVENT_MOVESPEED_CHANGED 13
#define CLIENT_EVENT_SERVER_CLOSED 14
#define CLIENT_EVENT_POSITION_FORCE 15
#define CLIENT_EVENT_DIRECTION_FORCE 16
#define CLIENT_EVENT_ADDSPACEGEOMAPPING 17
#define CLIENT_EVENT_VERSION_NOT_MATCH 18
#define CLIENT_EVENT_ON_KICKED 19
#define CLIENT_EVENT_LAST_ACCOUNT_INFO 20
#define CLIENT_EVENT_SCRIPT_VERSION_NOT_MATCH 21

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

struct EventData_LastAccountInfo : public EventData
{
	EventData_LastAccountInfo():
	EventData(CLIENT_EVENT_LAST_ACCOUNT_INFO),
	name(),
	password()
	{
	}

	std::string name, password;
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
	EventData(CLIENT_EVENT_LOGIN_FAILED),
	failedcode(0)
	{
	}

	int failedcode;
};

struct EventData_LoginBaseappSuccess : public EventData
{
	EventData_LoginBaseappSuccess():
	EventData(CLIENT_EVENT_LOGIN_BASEAPP_SUCCESS)
	{
	}
};

struct EventData_LoginBaseappFailed : public EventData
{
	EventData_LoginBaseappFailed():
	EventData(CLIENT_EVENT_LOGIN_BASEAPP_SUCCESS),
	failedcode(0),
	relogin(false)
	{
	}

	int failedcode;
	bool relogin;
};

struct EventData_CreatedEntity : public EventData
{
	EventData_CreatedEntity():
	EventData(CLIENT_EVENT_CREATEDENTITY),
	entityID(0),
//	modelres(),
	modelScale(0.f)
	{
	}

	ENTITY_ID entityID;
	//std::string modelres;
	float modelScale;	
};

struct EventData_EnterWorld : public EventData
{
	EventData_EnterWorld():
	EventData(CLIENT_EVENT_ENTERWORLD),
	spaceID(0),
	entityID(0),
	res()
	{
	}

	SPACE_ID spaceID;
	ENTITY_ID entityID;
	std::string res;
	float x, y, z;
	float pitch, roll, yaw;
	float speed;
	bool isOnGround;
};

struct EventData_LeaveWorld : public EventData
{
	EventData_LeaveWorld():
	EventData(CLIENT_EVENT_LEAVEWORLD),
	entityID(0),
	spaceID(0)
	{
	}

	ENTITY_ID entityID;
	SPACE_ID spaceID;
};

struct EventData_EnterSpace : public EventData
{
	EventData_EnterSpace():
	EventData(CLIENT_EVENT_ENTERSPACE),
	spaceID(0),
	entityID(0),
	res()
	{
	}

	SPACE_ID spaceID;
	ENTITY_ID entityID;
	std::string res;
	float x, y, z;
	float pitch, roll, yaw;
	float speed;
	bool isOnGround;
};

struct EventData_LeaveSpace : public EventData
{
	EventData_LeaveSpace():
	EventData(CLIENT_EVENT_LEAVESPACE),
	spaceID(0),
	entityID(0)
	{
	}

	SPACE_ID spaceID;
	ENTITY_ID entityID;
};

struct EventData_Script : public EventData
{
	EventData_Script():
	EventData(CLIENT_EVENT_SCRIPT),
	name(),
	datas()
	{
	}
	
	~EventData_Script()
	{
	}
	
	std::string name;
	std::string datas;
};

struct EventData_PositionChanged : public EventData
{
	EventData_PositionChanged():
	EventData(CLIENT_EVENT_POSITION_CHANGED),
	x(0.f),
	y(0.f),
	z(0.f),
	speed(0.f),
	entityID(0)
	{
	}

	float x, y, z;
	float speed;
	ENTITY_ID entityID;
};

struct EventData_PositionForce : public EventData
{
	EventData_PositionForce():
	EventData(CLIENT_EVENT_POSITION_FORCE),
	x(0.f),
	y(0.f),
	z(0.f),
	speed(0.f),
	entityID(0)
	{
	}

	float x, y, z;
	float speed;
	ENTITY_ID entityID;
};

struct EventData_DirectionForce : public EventData
{
	EventData_DirectionForce():
	EventData(CLIENT_EVENT_DIRECTION_FORCE),
	yaw(0.f),
	pitch(0.f),
	roll(0.f),
	entityID(0)
	{
	}

	float yaw, pitch, roll;
	ENTITY_ID entityID;
};

struct EventData_DirectionChanged : public EventData
{
	EventData_DirectionChanged():
	EventData(CLIENT_EVENT_DIRECTION_CHANGED),
	yaw(0.f),
	pitch(0.f),
	roll(0.f),
	entityID(0)
	{
	}

	float yaw, pitch, roll;
	ENTITY_ID entityID;
};

struct EventData_MoveSpeedChanged : public EventData
{
	EventData_MoveSpeedChanged():
	EventData(CLIENT_EVENT_MOVESPEED_CHANGED),
	speed(0.f),
	entityID(0)
	{
	}

	float speed;
	ENTITY_ID entityID;
};

struct EventData_ServerCloased : public EventData
{
	EventData_ServerCloased():
	EventData(CLIENT_EVENT_SERVER_CLOSED)
	{
	}
};

struct EventData_AddSpaceGEOMapping : public EventData
{
	EventData_AddSpaceGEOMapping():
	EventData(CLIENT_EVENT_ADDSPACEGEOMAPPING)
	{
	}

	SPACE_ID spaceID;
	std::string respath;
};

struct EventData_VersionNotMatch : public EventData
{
	EventData_VersionNotMatch():
	EventData(CLIENT_EVENT_VERSION_NOT_MATCH)
	{
	}

	std::string verInfo;
	std::string serVerInfo;
};

struct EventData_ScriptVersionNotMatch : public EventData
{
	EventData_ScriptVersionNotMatch():
	EventData(CLIENT_EVENT_SCRIPT_VERSION_NOT_MATCH)
	{
	}

	std::string verInfo;
	std::string serVerInfo;
};

struct EventData_onKicked : public EventData
{
	EventData_onKicked():
	EventData(CLIENT_EVENT_ON_KICKED),
	failedcode(0)
	{
	}

	uint16 failedcode;
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
		case CLIENT_EVENT_LOGIN_BASEAPP_SUCCESS:
			return new EventData_LoginBaseappSuccess();
			break;
		case CLIENT_EVENT_LOGIN_BASEAPP_FAILED:
			return new EventData_LoginBaseappFailed();
			break;
		case CLIENT_EVENT_SCRIPT:
			return new EventData_Script();
			break;
		case CLIENT_EVENT_POSITION_CHANGED:
			return new EventData_PositionChanged();
			break;
		case CLIENT_EVENT_DIRECTION_CHANGED:
			return new EventData_DirectionChanged();
			break;
		case CLIENT_EVENT_MOVESPEED_CHANGED:
			return new EventData_MoveSpeedChanged();
			break;
		case CLIENT_EVENT_SERVER_CLOSED:
			return new EventData_ServerCloased();
			break;
		case CLIENT_EVENT_POSITION_FORCE:
			return new EventData_PositionForce();
			break;
		case CLIENT_EVENT_DIRECTION_FORCE:
			return new EventData_DirectionForce();
			break;
		case CLIENT_EVENT_ADDSPACEGEOMAPPING:
			return new EventData_AddSpaceGEOMapping();
			break;
		case CLIENT_EVENT_VERSION_NOT_MATCH:
			return new EventData_VersionNotMatch();
			break;
		case CLIENT_EVENT_SCRIPT_VERSION_NOT_MATCH:
			return new EventData_ScriptVersionNotMatch();
			break;
		case CLIENT_EVENT_ON_KICKED:
			return new EventData_onKicked();
			break;
		case CLIENT_EVENT_LAST_ACCOUNT_INFO:
			return new EventData_LastAccountInfo();
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
		case CLIENT_EVENT_LOGIN_BASEAPP_SUCCESS:
			peventdata = new EventData_LoginBaseappSuccess();
			(*static_cast<EventData_LoginBaseappSuccess*>(peventdata)) = (*static_cast<const EventData_LoginBaseappSuccess*>(lpEventData));
			break;
		case CLIENT_EVENT_LOGIN_BASEAPP_FAILED:
			peventdata = new EventData_LoginBaseappFailed();
			(*static_cast<EventData_LoginBaseappFailed*>(peventdata)) = (*static_cast<const EventData_LoginBaseappFailed*>(lpEventData));
			break;
		case CLIENT_EVENT_SCRIPT:
			peventdata = new EventData_Script();
			(*static_cast<EventData_Script*>(peventdata)) = (*static_cast<const EventData_Script*>(lpEventData));
			break;
		case CLIENT_EVENT_POSITION_CHANGED:
			peventdata = new EventData_PositionChanged();
			(*static_cast<EventData_PositionChanged*>(peventdata)) = (*static_cast<const EventData_PositionChanged*>(lpEventData));
			break;
		case CLIENT_EVENT_DIRECTION_CHANGED:
			peventdata = new EventData_DirectionChanged();
			(*static_cast<EventData_DirectionChanged*>(peventdata)) = (*static_cast<const EventData_DirectionChanged*>(lpEventData));
			break;
		case CLIENT_EVENT_MOVESPEED_CHANGED:
			peventdata = new EventData_MoveSpeedChanged();
			(*static_cast<EventData_MoveSpeedChanged*>(peventdata)) = (*static_cast<const EventData_MoveSpeedChanged*>(lpEventData));
			break;
		case CLIENT_EVENT_SERVER_CLOSED:
			peventdata = new EventData_ServerCloased();
			(*static_cast<EventData_ServerCloased*>(peventdata)) = (*static_cast<const EventData_ServerCloased*>(lpEventData));
			break;
		case CLIENT_EVENT_POSITION_FORCE:
			peventdata = new EventData_PositionForce();
			(*static_cast<EventData_PositionForce*>(peventdata)) = (*static_cast<const EventData_PositionForce*>(lpEventData));
			break;
		case CLIENT_EVENT_DIRECTION_FORCE:
			peventdata = new EventData_DirectionForce();
			(*static_cast<EventData_DirectionForce*>(peventdata)) = (*static_cast<const EventData_DirectionForce*>(lpEventData));
			break;
		case CLIENT_EVENT_ADDSPACEGEOMAPPING:
			peventdata = new EventData_AddSpaceGEOMapping();
			(*static_cast<EventData_AddSpaceGEOMapping*>(peventdata)) = (*static_cast<const EventData_AddSpaceGEOMapping*>(lpEventData));
			break;
		case CLIENT_EVENT_VERSION_NOT_MATCH:
			peventdata = new EventData_VersionNotMatch();
			(*static_cast<EventData_VersionNotMatch*>(peventdata)) = (*static_cast<const EventData_VersionNotMatch*>(lpEventData));
			break;
		case CLIENT_EVENT_SCRIPT_VERSION_NOT_MATCH:
			peventdata = new EventData_ScriptVersionNotMatch();
			(*static_cast<EventData_ScriptVersionNotMatch*>(peventdata)) = (*static_cast<const EventData_ScriptVersionNotMatch*>(lpEventData));
			break;
		case CLIENT_EVENT_ON_KICKED:
			peventdata = new EventData_onKicked();
			(*static_cast<EventData_onKicked*>(peventdata)) = (*static_cast<const EventData_onKicked*>(lpEventData));
			break;
		case CLIENT_EVENT_LAST_ACCOUNT_INFO:
			peventdata = new EventData_LastAccountInfo();
			(*static_cast<EventData_LastAccountInfo*>(peventdata)) = (*static_cast<const EventData_LastAccountInfo*>(lpEventData));
			break;
		default:
			break;
	}

	return peventdata;
}

}
#endif
