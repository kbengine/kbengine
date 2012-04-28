/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __ENGINE_COMPONENT_MGR_H__
#define __ENGINE_COMPONENT_MGR_H__
	
// common include
//#define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/singleton.hpp"
#include "thread/threadmutex.hpp"
#include "thread/threadguard.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

namespace Mercury
{
class Channel;
}

class EngineComponentMgr : public Singleton<EngineComponentMgr>
{
public:
	KBEngine::thread::ThreadMutex myMutex;
	typedef std::map<COMPONENT_ID, Mercury::Channel*> COMPONENT_MAP;

public:
	EngineComponentMgr();
	~EngineComponentMgr();

	uint32 allocComponentID(void);

	void addComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID, Mercury::Channel* lpChannel);
	void delComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID);

	EngineComponentMgr::COMPONENT_MAP& getComponents(COMPONENT_TYPE componentType);

	Mercury::Channel* findComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID);

private:
	COMPONENT_MAP			_baseapps;
	COMPONENT_MAP			_cellapps;
	COMPONENT_MAP			_dbmgrs;
	COMPONENT_MAP			_loginapps;
	COMPONENT_MAP			_cellappmgrs;
	COMPONENT_MAP			_baseappmgrs;
	COMPONENT_MAP			_machines;
	COMPONENT_MAP			_centers;
};

}
#endif
