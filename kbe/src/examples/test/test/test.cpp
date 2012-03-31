/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#include "stdafx.h"
#include "Python.h"
#include "cstdkbe/cstdkbe.hpp"
#include "log/debug_helper.hpp"
#include "pyscript/Script.hpp"
#include "server/kbemain.hpp"
#include "server/serverconfig.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "pyscript/pyobject_pointer.hpp"
#include "entitydef/entitydef.hpp"
#include "entity.hpp"
#include "entities.hpp"

using namespace KBEngine;



bool test_script()
{
	KBEngine::script::Script g_script;

	std::wstring pyPaths = L"../../../demo/res/scripts/common;" \
							L"../../../demo/res/scripts/cell;" ;

	g_script.install(L"../../res/script/common", pyPaths, "KBEngine");





	// 初始化entities到脚本
	Entities::installScript(NULL);
	// 初始化Entity类型， 提供给脚本继承出Entity
	Entity::installScript(g_script.getModule());


	// 初始化数据类别
	if(!DataTypes::initialize("../../../demo/res/scripts/entity_defs/alias.xml"))
		return false;

	// 初始化所有扩展模块
	std::vector<PyTypeObject*> scriptBaseTypes;
	scriptBaseTypes.push_back(Entity::getScriptType());
	if(!EntityDef::initialize("../../../demo/res/scripts/", scriptBaseTypes, CELLAPP_TYPE)){
		getchar();
		return false;
	}

	PyRun_SimpleString("import Math;a=Math.Vector3(1,3,4);print ('kbe:python is init successfully!!!', a is a)");
	KBEngine::DEBUG_MSG("kbe:python is init successfully!!! %d\n", 88);
	KBEngine::SmartPointer<PyObject> testsmartpointer(::PyBytes_FromString("test"));
	testsmartpointer.clear();
	Py_Finalize();
	return true;
}

int KBENGINE_MAIN(int argc, char* argv[])
{
	ServerConfig sc;
	sc.loadConfig("../../res/server/KBEngineDefault.xml");
	sc.loadConfig("../../../demo/res/server/KBEngine.xml");
	test_script();
	getchar();
	return 0; 
}