#include "entity_app.hpp"
#include "helper/debug_helper.hpp"
#include "server/script_timers.hpp"

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
