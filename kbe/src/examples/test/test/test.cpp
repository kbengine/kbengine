/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#include "stdafx.h"
#include "server/kbemain.hpp"
#include "entity.hpp"
#include "entities.hpp"

using namespace KBEngine;

class TestApp: public ServerApp
{
public:
	TestApp()
	{
	}

	~TestApp()
	{
	}

	bool installPyModules()
	{
		Entities::installScript(NULL);
		Entity::installScript(getScript().getModule());

		registerScript(Entity::getScriptType());
		return true;
	}

	//-------------------------------------------------------------------------------------
	bool uninstallPyModules()
	{
		Entities::uninstallScript();
		Entity::uninstallScript();
		return true;
	}

	bool run()
	{
		PyRun_SimpleString("import Math;a=Math.Vector3(1,3,4);b=Math.Vector3(1,2,4);print ('kbe:python is init successfully!!!', a == b)");
		DEBUG_MSG("kbe:python is init successfully!!! %d\n", 88);
		SmartPointer<PyObject> testsmartpointer(::PyBytes_FromString("test"));
		testsmartpointer.clear();
		return true;
	}
};

template<> TestApp* Singleton<TestApp>::m_singleton_ = 0;

int KBENGINE_MAIN(int argc, char* argv[])
{
	int ret= kbeMainT<TestApp>(argc, argv, CELLAPP_TYPE);
	getchar();
	return 0; 
}
