#include "serverapp.hpp"
#include "server/serverconfig.hpp"
#include "server/componentbridge.hpp"

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
bool ServerApp::loadConfig()
{
	g_kbeSrvConfig.loadConfig("../../res/server/kbengine_defs.xml");
	g_kbeSrvConfig.loadConfig("../../../demo/res/server/kbengine.xml");
	g_kbeSrvConfig.updateInfos(true, componentType_, componentID_, 
			getNetworkInterface().addr(), getNetworkInterface().addr());
	
	return true;
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
	
	if(!loadConfig())
		return false;
	
	if(!initializeBegin())
		return false;
	
	if(!inInitialize())
		return false;
	
	// 广播自己的地址给网上上的所有kbemachine
	// 并且从kbemachine获取basappmgr和cellappmgr以及dbmgr地址
	this->getMainDispatcher().addFrequentTask(&Componentbridge::getSingleton());

	return initializeEnd();
}

//-------------------------------------------------------------------------------------		
void ServerApp::finalise(void)
{
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
