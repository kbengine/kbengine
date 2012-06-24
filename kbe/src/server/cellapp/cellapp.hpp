/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __CELLAPP_H__
#define __CELLAPP_H__
	
// common include	
#include "server/kbemain.hpp"
#include "server/entity_app.hpp"
#include "server/idallocate.hpp"
#include "server/serverconfig.hpp"
#include "entitydef/entities.hpp"
#include "cstdkbe/timer.hpp"

//#define NDEBUG
#include <map>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{
class Entity;

class CellApp:	public EntityApp, 
				public TimerHandler, 
				public Singleton<CellApp>
{
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK,
		TIMEOUT_LOADING_TICK
	};
	
	CellApp(Mercury::EventDispatcher& dispatcher, Mercury::NetworkInterface& ninterface, COMPONENT_TYPE componentType);
	~CellApp();

	bool installPyModules();
	bool uninstallPyModules();
	
	bool run();
	
	void handleTimeout(TimerHandle handle, void * arg);
	void handleGameTick();
	void handleTimers();

	bool initializeBegin();
	bool initializeEnd();
	void finalise();
	
	Entity* createEntity(const char* entityType, PyObject* params, bool isInitializeScript = true, ENTITY_ID eid = 0);

	Entity* findEntity(ENTITY_ID eid);

	PyObject* tryGetEntityByMailbox(COMPONENT_ID componentID, ENTITY_ID eid);

	virtual bool destroyEntity(ENTITY_ID entityID);
	
protected:
	IDClient<ENTITY_ID>*		idClient_;
	Entities<Entity>*			pEntities_;									// 存储所有的entity的容器
	TimerHandle					gameTimer_;
};

}
#endif
