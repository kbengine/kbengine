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
	EntityApp(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);
	~EntityApp();
	
	virtual bool destroyEntity(ENTITY_ID entityID) = 0;

	KBEngine::script::Script& getScript(){ return script_; }
	
	PyObjectPtr getEntryScript(){ return entryScript_; }

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

	PyObjectPtr												entryScript_;
	
};

}
#endif
