#define KBE_DLL_API extern "C" _declspec(dllexport)
#include "kbengine_dll.h"

#include "cstdkbe/cstdkbe.hpp"
#include "client_lib/kbemain.hpp"
#include "server/serverconfig.hpp"
#include "cstdkbe/memorystream.hpp"
#include "thread/threadtask.hpp"
#include "helper/debug_helper.hpp"
#include "network/address.hpp"

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
KBEngine::script::Script* g_pScript = NULL;
Mercury::EventDispatcher* g_pDispatcher = NULL;
Mercury::NetworkInterface* pNetworkInterface = NULL;
thread::ThreadPool* g_pThreadPool = NULL;

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

class KBEMainTask : public thread::TPTask
{
public:
	KBEMainTask()
	{
	}

	virtual ~KBEMainTask()
	{
	}
	
	virtual bool process()
	{
		g_pApp->run();
		return false;
	}
	
	virtual thread::TPTask::TPTaskState presentMainThread()
	{
		return thread::TPTask::TPTASK_STATE_COMPLETED;
	}
};

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

	if(g_pScript == NULL)
		g_pScript = new KBEngine::script::Script();

	if(g_pDispatcher == NULL)
		g_pDispatcher = new Mercury::EventDispatcher();

	if(pNetworkInterface == NULL)
	{
		pNetworkInterface = new Mercury::NetworkInterface(g_pDispatcher, 
			0, 0, "", 0, 0,
			0, "", 0, 0);
	}

	if(!installPyScript(*g_pScript, g_componentType))
	{
		ERROR_MSG("installPyScript() is error!\n");
		return false;
	}

	g_pApp = new ClientApp(*g_pDispatcher, *pNetworkInterface, g_componentType, g_componentID);
	g_pApp->setScript(g_pScript);

	START_MSG(COMPONENT_NAME_EX(g_componentType), g_componentID);
	if(!g_pApp->initialize())
	{
		ERROR_MSG("app::initialize is error!\n");
		g_pApp->finalise();
		Py_DECREF(g_pApp);
		g_pApp = NULL;

		uninstallPyScript(*g_pScript);

		SAFE_RELEASE(pNetworkInterface);
		SAFE_RELEASE(g_pScript);
		SAFE_RELEASE(g_pDispatcher);
		return false;
	}

	if(g_pThreadPool == NULL)
	{
		g_pThreadPool = new thread::ThreadPool();
		if(!g_pThreadPool->isInitialize())
		{
			g_pThreadPool->createThreadPool(1, 1, 4);
		}
		else
		{
			ERROR_MSG("g_threadPool.isInitialize() is error!\n");
			return false;
		}
	}

	INFO_MSG(boost::format("---- %1% is running ----\n") % COMPONENT_NAME_EX(g_componentType));

	g_pThreadPool->addTask(new KBEMainTask());
	return true;
}

bool kbengine_destroy()
{
	g_pApp->getMainDispatcher().breakProcessing();
	g_pThreadPool->finalise();
	KBEngine::sleep(100);
	
	delete g_pThreadPool;

	if(g_pApp)
	{
		g_pApp->finalise();
		Py_DECREF(g_pApp);
		g_pApp = NULL;
	}
	
	bool ret = uninstallPyScript(*g_pScript);

	SAFE_RELEASE(pNetworkInterface);
	SAFE_RELEASE(g_pScript);
	SAFE_RELEASE(g_pDispatcher);

	INFO_MSG(boost::format("%1% has shut down.\n") % COMPONENT_NAME_EX(g_componentType));
	return ret;
}
