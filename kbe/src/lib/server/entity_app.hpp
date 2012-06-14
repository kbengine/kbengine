/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __ENTITY_APP_H__
#define __ENTITY_APP_H__
// common include
#include "Python.h"
#include "pyscript/script.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "pyscript/pyobject_pointer.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4996)
#endif
//#define NDEBUG
#include "server/serverapp.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

class EntityApp : public ServerApp
{
public:
	EntityApp(Mercury::EventDispatcher& dispatcher, Mercury::NetworkInterface& ninterface, COMPONENT_TYPE componentType);
	~EntityApp();
	
	virtual bool destroyEntity(ENTITY_ID entityID) = 0;

	KBEngine::script::Script& getScript(){ return script_; }
	
	void registerScript(PyTypeObject*);
	int registerPyObjectToScript(const char* attrName, PyObject* pyObj);
	int unregisterPyObjectToScript(const char* attrName);

	bool installPyScript();
	virtual bool installPyModules();
	virtual bool uninstallPyModules();
	bool uninstallPyScript();
	bool installEntityDef();
	
	virtual void finalise();
	
	virtual bool inInitialize();
	
	Timers & timers() { return timers_; }
		
	virtual void onSignalled(int sigNum);
protected:
	Timers timers_;

	KBEngine::script::Script								script_;
	std::vector<PyTypeObject*>								scriptBaseTypes_;
	
};

}
#endif
