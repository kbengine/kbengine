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
#pragma warning(disable: 4996)
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <stdarg.h> 
#include "Python.h"
#include "cstdkbe/cstdkbe.hpp"
#include "log/debug_helper.hpp"
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
	COMPONENT_TYPE m_componentType_;
	KBEngine::script::Script m_script_;
	std::vector<PyTypeObject*> m_scriptBaseTypes_;
public:
	ServerApp();
	~ServerApp();
	
	KBEngine::script::Script& getScript(){ return m_script_; }
	void registerScript(PyTypeObject*);

	bool initialize(COMPONENT_TYPE componentType);
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
