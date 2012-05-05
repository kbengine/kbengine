#include "serverapp.hpp"
#include "server/serverconfig.hpp"

namespace KBEngine{
COMPONENT_TYPE g_componentType;

//-------------------------------------------------------------------------------------
ServerApp::ServerApp(Mercury::EventDispatcher& dispatcher, 
					 Mercury::NetworkInterface& ninterface, COMPONENT_TYPE componentType):
SignalHandler(),
componentType_(componentType),
componentID_(0),
mainDispatcher_(dispatcher),
networkInterface_(ninterface),
time_(0)
{
	g_componentType = componentType;
	networkInterface_.pExtensionData(this);
}

//-------------------------------------------------------------------------------------
ServerApp::~ServerApp()
{
}

//-------------------------------------------------------------------------------------
int ServerApp::registerPyObjectToScript(const char* attrName, PyObject* pyObj)
{ 
	return script_.registerToModule(attrName, pyObj); 
}

//-------------------------------------------------------------------------------------
bool ServerApp::loadConfig()
{
	g_kbeSrvConfig.loadConfig("../../res/server/kbengine_defs.xml");
	g_kbeSrvConfig.loadConfig("../../../demo/res/server/kbengine.xml");
	return true;
}

//-------------------------------------------------------------------------------------
bool ServerApp::installPyScript()
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
	
	wchar_t pyhomePath[] = L"../../res/script/common";
	return getScript().install(pyhomePath, pyPaths, "KBEngine", componentType_);
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
	scriptBaseTypes_.push_back(pto);
}

//-------------------------------------------------------------------------------------
bool ServerApp::uninstallPyScript()
{
	return uninstallPyModules() && getScript().uninstall();
}

//-------------------------------------------------------------------------------------		
bool ServerApp::installSingnals()
{
	g_kbeSignalHandlers.attachApp(this);
	g_kbeSignalHandlers.addSignal(SIGINT, this);
	g_kbeSignalHandlers.addSignal(SIGHUP, this);
	return true;
}

//-------------------------------------------------------------------------------------		
bool ServerApp::initialize()
{
	if(!installSingnals())
		return false;

	if(!initializeBegin())
		return false;

	if(!loadConfig())
		return false;

	if(!installPyScript())
		return false;

	if(!installPyModules())
		return false;
	
	g_kbeSrvConfig.updateInfos(true, componentType_, componentID_, getNetworkInterface().addr(), getNetworkInterface().addr());
	return initializeEnd();
}

//-------------------------------------------------------------------------------------		
void ServerApp::finalise(void)
{
	uninstallPyScript();
}

//-------------------------------------------------------------------------------------		
double ServerApp::gameTimeInSeconds() const
{
	return double(time_) / g_kbeSrvConfig.gameUpdateHertz();
}

//-------------------------------------------------------------------------------------		
bool ServerApp::run(void)
{
	mainDispatcher_.processUntilBreak();
	return true;
}

//-------------------------------------------------------------------------------------	
void ServerApp::shutDown()
{
	INFO_MSG( "ServerApp::shutDown: shutting down\n" );
	mainDispatcher_.breakProcessing();
}

//-------------------------------------------------------------------------------------	
void ServerApp::onSignalled(int sigNum)
{
	switch (sigNum)
	{
	case SIGINT:
	case SIGHUP:
		this->shutDown();
	default:
		break;
	}
}

//-------------------------------------------------------------------------------------		
}
