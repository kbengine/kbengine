/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __SERVER_APP_H__
#define __SERVER_APP_H__
// common include
#include "cstdkbe/cstdkbe.hpp"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4996)
#endif
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <stdarg.h> 
#include "Python.h"
#include "helper/debug_helper.hpp"
#include "pyscript/Script.hpp"
#include "xmlplus/xmlplus.hpp"	
#include "cstdkbe/singleton.hpp"
#include "server/serverconfig.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "pyscript/pyobject_pointer.hpp"
#include "entitydef/entitydef.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

class ServerApp
{
public:
	ServerApp(Mercury::EventDispatcher& dispatcher, Mercury::NetworkInterface& ninterface, COMPONENT_TYPE componentType);
	~ServerApp();
	
	KBEngine::script::Script& getScript(){ return script_; }

	void registerScript(PyTypeObject*);
	int registerPyObjectToScript(const char* attrName, PyObject* pyObj){ return script_.registerToModule(attrName, pyObj); };

	bool initialize();
	virtual bool initializeBegin(){return true;};
	virtual bool initializeEnd(){return true;};
	virtual void finalise();
	virtual bool run();
	
	bool installSingnal(int sigNum);
	bool installPyScript();
	bool installEntityDef();
	virtual bool installPyModules();
	virtual bool uninstallPyModules();
	bool uninstallPyScript();

	virtual bool loadConfig();
	const char* name(){return COMPONENT_NAME[componentType_];}
	
	GAME_TIME time() const { return time_; }
	double gameTimeInSeconds() const;
		
	Mercury::EventDispatcher & getMainDispatcher()				{ return mainDispatcher_; }
	Mercury::NetworkInterface & getNetworkInterface()			{ return networkInterface_; }

	COMPONENT_ID componentID()const	{ return componentID_; }
	COMPONENT_TYPE componentType()const	{ return componentType_; }
		
	virtual void onSignalled(int sigNum);
	
	void shutDown();
protected:
	COMPONENT_TYPE											componentType_;
	COMPONENT_ID											componentID_;									// 本组件的ID
	KBEngine::script::Script								script_;
	std::vector<PyTypeObject*>								scriptBaseTypes_;
	Mercury::EventDispatcher& 								mainDispatcher_;	
	Mercury::NetworkInterface&								networkInterface_;
	GAME_TIME												time_;
};

}
#endif
