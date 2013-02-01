#define KBE_DLL_API extern "C" _declspec(dllexport)
#include "kbengine_dll.h"

#include "cstdkbe/cstdkbe.hpp"
#include "client_lib/kbemain.hpp"
#include "server/serverconfig.hpp"

#undef DEFINE_IN_INTERFACE
#include "client_lib/client_interface.hpp"
#define DEFINE_IN_INTERFACE
#define CLIENT
#include "client_lib/client_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.hpp"

#include "machine/machine_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "machine/machine_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "tools/message_log/messagelog_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "tools/message_log/messagelog_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "tools/billing_system/billingsystem_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "tools/billing_system/billingsystem_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "tools/bots/bots_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "tools/bots/bots_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "resourcemgr/resourcemgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "resourcemgr/resourcemgr_interface.hpp"

using namespace KBEngine;

ClientApp* g_pApp = NULL;
KBEngine::script::Script g_script;

BOOL APIENTRY DllMain( HANDLE hModule,
					  DWORD ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		printf("\nprocess attach of dll");
		break;
	case DLL_THREAD_ATTACH:
		printf("\nthread attach of dll");
		break;
	case DLL_THREAD_DETACH:
		printf("\nthread detach of dll");
		break;
	case DLL_PROCESS_DETACH:
		printf("\nprocess detach of dll");
		break;
	}

	return TRUE;
}


bool kbengine_init()
{
	g_componentID = genUUID64();
	g_componentType = CLIENT_TYPE;

	ServerConfig* pconfig = new ServerConfig;

	if(!loadConfig())
	{
		ERROR_MSG("loadConfig() is error!\n");
		return false;
	}

	DebugHelper::initHelper(g_componentType);

	INFO_MSG( "-----------------------------------------------------------------------------------------\n\n\n");

	Mercury::EventDispatcher dispatcher;
	Mercury::NetworkInterface networkInterface(&dispatcher, 
		0, 0, "", 0, 0,
		0, "", 0, 0);
	
	if(!installPyScript(g_script, g_componentType))
	{
		ERROR_MSG("installPyScript() is error!\n");
		return false;
	}

	g_pApp = new ClientApp(dispatcher, networkInterface, g_componentType, g_componentID);
	g_pApp->setScript(&g_script);

	START_MSG(COMPONENT_NAME_EX(g_componentType), g_componentID);
	if(!g_pApp->initialize()){
		ERROR_MSG("app::initialize is error!\n");
		g_pApp->finalise();
		Py_DECREF(g_pApp);
		g_pApp = NULL;
		uninstallPyScript(g_script);
		return false;
	}

	INFO_MSG(boost::format("---- %1% is running ----\n") % COMPONENT_NAME_EX(g_componentType));
	return true;
}

bool kbengine_destroy()
{
	if(g_pApp)
	{
		g_pApp->finalise();
		Py_DECREF(g_pApp);
		g_pApp = NULL;
	}
	
	bool ret = uninstallPyScript(g_script);

	INFO_MSG(boost::format("%1% has shut down.\n") % COMPONENT_NAME_EX(g_componentType));
	return ret;
}
