#include "entity_app.hpp"
#include "helper/debug_helper.hpp"
#include "server/script_timers.hpp"
#include "entitydef/entitydef.hpp"

namespace KBEngine{

//-------------------------------------------------------------------------------------
EntityApp::EntityApp(Mercury::EventDispatcher& dispatcher, Mercury::NetworkInterface& ninterface, COMPONENT_TYPE componentType):
ServerApp(dispatcher, ninterface, componentType)
{
	ScriptTimers::initialize(*this);
}

//-------------------------------------------------------------------------------------
EntityApp::~EntityApp()
{
	ScriptTimers::finalise(*this);
}

//-------------------------------------------------------------------------------------
bool EntityApp::inInitialize()
{
	if(!installPyScript())
		return false;

	if(!installPyModules())
		return false;
	
	return true;
}

//-------------------------------------------------------------------------------------
void EntityApp::finalise(void)
{
	uninstallPyScript();
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------
bool EntityApp::installEntityDef()
{
	// 初始化数据类别
	if(!DataTypes::initialize("../../../demo/res/scripts/entity_defs/alias.xml"))
		return false;

	// 初始化所有扩展模块
	if(!EntityDef::initialize("../../../demo/res/scripts/", scriptBaseTypes_, componentType_)){
		return false;
	}

	return EntityDef::installScript(NULL);
}

//-------------------------------------------------------------------------------------
int EntityApp::registerPyObjectToScript(const char* attrName, PyObject* pyObj)
{ 
	return script_.registerToModule(attrName, pyObj); 
}

//-------------------------------------------------------------------------------------
bool EntityApp::installPyScript()
{
	std::wstring pyPaths = L"../../../demo/res/scripts/common;";

	switch(componentType_)
	{
	case BASEAPP_TYPE:
		pyPaths += L"../../../demo/res/scripts/base;";
		break;
	case CELLAPP_TYPE:
		pyPaths += L"../../../demo/res/scripts/cell;";
		break;
	default:
		pyPaths += L"../../../demo/res/scripts/client;";
		break;
	};
	
	return getScript().install(L"../../res/script/common", pyPaths, "KBEngine", componentType_);
}

//-------------------------------------------------------------------------------------
void EntityApp::registerScript(PyTypeObject* pto)
{
	scriptBaseTypes_.push_back(pto);
}

//-------------------------------------------------------------------------------------
bool EntityApp::uninstallPyScript()
{
	return uninstallPyModules() && getScript().uninstall();
}

//-------------------------------------------------------------------------------------
bool EntityApp::installPyModules()
{
	//Entities::installScript(NULL);
	//Entity::installScript(g_script.getModule());
	return installEntityDef();
}

//-------------------------------------------------------------------------------------
bool EntityApp::uninstallPyModules()
{
	//Entities::uninstallScript();
	//Entity::uninstallScript();
	EntityDef::uninstallScript();
	return true;
}

//-------------------------------------------------------------------------------------
void EntityApp::onSignalled(int sigNum)
{
	this->ServerApp::onSignalled(sigNum);
	
	switch (sigNum)
	{
	case SIGQUIT:
		CRITICAL_MSG("Received QUIT signal. This is likely caused by the "
					"%sMgr killing this %s because it has been "
					"unresponsive for too long. Look at the callstack from "
					"the core dump to find the likely cause.\n",
				COMPONENT_NAME[componentType_], 
				COMPONENT_NAME[componentType_] );
		
		break;
	default: 
		break;
	}
}

//-------------------------------------------------------------------------------------		
}
