#include "serverapp.hpp"
#include "server/serverconfig.hpp"
#include "server/signal_handler.hpp"

namespace KBEngine{
COMPONENT_TYPE g_componentType;
SignalHandlers g_signalHandlers;

void signalHandler(int signum)
{
	DEBUG_MSG("SignalHandlers: receive sigNum %d.\n", signum);
	g_signalHandlers.onSignalled(signum);
};

class ServerSignalHandler : public SignalHandler
{
	virtual void onHandle(int sigNum)
	{
		switch(sigNum)
		{
		case SIGINT:
			exit( 1 );
			break;
		default:
			break;
		};
	};
};

ServerSignalHandler g_pServerSignalHandler;

//-------------------------------------------------------------------------------------
ServerApp::ServerApp(Mercury::EventDispatcher& dispatcher, 
					 Mercury::NetworkInterface& ninterface, COMPONENT_TYPE componentType):
componentType_(componentType),
componentID_(0),
mainDispatcher_(dispatcher),
networkInterface_(ninterface),
time_(0)
{
	g_componentType = componentType;
	networkInterface_.pExtensionData(this);
	
	g_signalHandlers.addSignal(SIGINT, &g_pServerSignalHandler);
	mainDispatcher_.addFrequentTask(&g_signalHandlers);
}

//-------------------------------------------------------------------------------------
ServerApp::~ServerApp()
{
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
	EntityDef::installScript(NULL);
	return true;
}

//-------------------------------------------------------------------------------------
bool ServerApp::uninstallPyModules()
{
	//Entities::uninstallScript();
	//Entity::uninstallScript();
	EntityDef::uninstallScript();
	return true;
}

//-------------------------------------------------------------------------------------
void ServerApp::registerScript(PyTypeObject* pto)
{
	scriptBaseTypes_.push_back(pto);
}

//-------------------------------------------------------------------------------------
bool ServerApp::installEntityDef()
{
	// 初始化数据类别
	if(!DataTypes::initialize("../../../demo/res/scripts/entity_defs/alias.xml"))
		return false;

	// 初始化所有扩展模块
	if(!EntityDef::initialize("../../../demo/res/scripts/", scriptBaseTypes_, CELLAPP_TYPE)){
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
bool ServerApp::installSingnal(int sigNum)
{
#if KBE_PLATFORM != PLATFORM_WIN32
	struct sigaction act;
	
	act.sa_handler = signalHandler;
	sigemptyset( &act.sa_mask ); 
	if( sigaction(sigNum, &act, NULL) < 0 )
	{
		ERROR_MSG("install sigal SIGINT error!\n");
		return false;
	}	
#endif
	return true;
}

//-------------------------------------------------------------------------------------		
bool ServerApp::initialize()
{
	if(!initializeBegin())
		return false;

	if(!loadConfig())
		return false;

	if(!installPyScript())
		return false;

	if(!installPyModules())
		return false;

	if(!installEntityDef())
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
