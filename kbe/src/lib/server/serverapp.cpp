#include "serverapp.hpp"
namespace KBEngine{
template<> ServerApp* Singleton<ServerApp>::m_singleton_ = 0;

//-------------------------------------------------------------------------------------
ServerApp::ServerApp():
m_componentType_(UNKNOWN_COMPONENT_TYPE)
{
}

//-------------------------------------------------------------------------------------
ServerApp::~ServerApp()
{
}

//-------------------------------------------------------------------------------------
bool ServerApp::loadConfig()
{
	ServerConfig sc;
	sc.loadConfig("../../res/server/KBEngineDefault.xml");
	sc.loadConfig("../../../demo/res/server/KBEngine.xml");
	return true;
}

//-------------------------------------------------------------------------------------
bool ServerApp::installPyScript()
{
	std::wstring pyPaths = L"../../../demo/res/scripts/common;";

	switch(m_componentType_)
	{
	case BASEAPP_TYPE:
		pyPaths += L"../../../demo/res/scripts/base;";
		break;
	case CELLAPP_TYPE:
		pyPaths += L"../../../demo/res/scripts/cell;";
		break;
	default:
		pyPaths += L"../../../demo/res/scripts/cell;";
		break;
	};
							

	return getScript().install(L"../../res/script/common", pyPaths, "KBEngine");
}

//-------------------------------------------------------------------------------------
bool ServerApp::installPyModules()
{
	//Entities::installScript(NULL);
	//Entity::installScript(g_script.getModule());
	return true;
}

//-------------------------------------------------------------------------------------
bool ServerApp::uninstallPyModules()
{
	//Entities::uninstallScript();
	//Entity::uninstallScript();
	return true;
}

//-------------------------------------------------------------------------------------
void ServerApp::registerScript(PyTypeObject* pto)
{
	m_scriptBaseTypes_.push_back(pto);
}

//-------------------------------------------------------------------------------------
bool ServerApp::installEntityDef()
{
	// 初始化数据类别
	if(!DataTypes::initialize("../../../demo/res/scripts/entity_defs/alias.xml"))
		return false;

	// 初始化所有扩展模块
	if(!EntityDef::initialize("../../../demo/res/scripts/", m_scriptBaseTypes_, CELLAPP_TYPE)){
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ServerApp::uninstallPyScript()
{
	return uninstallPyModules() && getScript().uninstall();
}

//-------------------------------------------------------------------------------------		
bool ServerApp::initialize(COMPONENT_TYPE componentType)
{
	m_componentType_ = componentType;
	initializeBegin(componentType);
	loadConfig();
	installPyScript();
	installPyModules();
	installEntityDef();
	return initializeEnd(componentType);
}

//-------------------------------------------------------------------------------------		
void ServerApp::finalise(void)
{
	uninstallPyScript();
}

//-------------------------------------------------------------------------------------		
bool ServerApp::run(void)
{
	return true;
}

//-------------------------------------------------------------------------------------		
}
