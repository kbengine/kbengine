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
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

class ServerApp : public Singleton<ServerApp>
{
protected:
	COMPONENT_TYPE											m_componentType_;
	COMPONENT_ID											m_componentID_;									// 本组件的ID
	KBEngine::script::Script								m_script_;
	std::vector<PyTypeObject*>								m_scriptBaseTypes_;
	
public:
	ServerApp();
	~ServerApp();
	
	KBEngine::script::Script& getScript(){ return m_script_; }

	void registerScript(PyTypeObject*);
	int registerPyObjectToScript(const char* attrName, PyObject* pyObj){ return m_script_.registerToModule(attrName, pyObj); };

	bool initialize(COMPONENT_TYPE componentType);
	virtual bool initializeBegin(COMPONENT_TYPE componentType){return true;};
	virtual bool initializeEnd(COMPONENT_TYPE componentType){return true;};
	void finalise();
	virtual bool run();

	bool installPyScript();
	bool installEntityDef();
	virtual bool installPyModules();
	virtual bool uninstallPyModules();
	bool uninstallPyScript();

	virtual bool loadConfig();
};

}
#endif
