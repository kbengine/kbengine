#pragma warning(disable : 4049)
#pragma warning(disable : 4217)
#define KBE_DLL_API extern "C" _declspec(dllexport)
#include "kbengine_dll.h"

#include "cstdkbe/cstdkbe.hpp"
#include "client_lib/kbemain.hpp"
#include "server/serverconfig.hpp"
#include "server/telnet_server.hpp"
#include "cstdkbe/memorystream.hpp"
#include "cstdkbe/kbekey.hpp"
#include "thread/threadtask.hpp"
#include "thread/concurrency.hpp"
#include "helper/debug_helper.hpp"
#include "network/address.hpp"
#include "client_lib/event.hpp"
#include "client_lib/config.hpp"
#include "pyscript/pythread_lock.hpp"

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
Mercury::NetworkInterface* g_pNetworkInterface = NULL;
thread::ThreadPool* g_pThreadPool = NULL;
ServerConfig* pserverconfig = NULL;
Config* pconfig = NULL;
KBEngine::script::PyThreadStateLock* g_pLock = NULL;
KBEngine::script::PyThreadStateLock* g_pNewLock = NULL;
TelnetServer* g_pTelnetServer = NULL;

volatile bool g_inProcess = false;
volatile bool g_break = false;
volatile int targetID = -1;

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
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
		while(g_pApp && !g_break)
		{
			g_pLock = new KBEngine::script::PyThreadStateLock;
			
			if(targetID >= 0)
			{
				g_pApp->setTargetID(targetID);
				targetID = -1;
			}

			g_inProcess = true;
			g_pApp->processOnce(true);
			g_inProcess = false;
			SAFE_RELEASE(g_pLock);
		}
	
		g_break = true;
		return false;
	}
	
	virtual thread::TPTask::TPTaskState presentMainThread()
	{
		return thread::TPTask::TPTASK_STATE_COMPLETED;
	}
};

//-------------------------------------------------------------------------------------
void acquireLock()
{
#ifndef KBE_SINGLE_THREADED
	if(g_pLock == NULL)
		g_pLock = new KBEngine::script::PyThreadStateLock;
#endif
}

//-------------------------------------------------------------------------------------
void releaseLock()
{
#ifndef KBE_SINGLE_THREADED
	SAFE_RELEASE(g_pLock);
#endif
}

//-------------------------------------------------------------------------------------
void kbe_lock()
{
	//g_pApp->getScript().acquireLock();
	g_pNewLock = new KBEngine::script::PyThreadStateLock;
}

void kbe_unlock()
{
	//g_pApp->getScript().releaseLock();
	SAFE_RELEASE(g_pNewLock);
}

//-------------------------------------------------------------------------------------
bool kbe_inProcess()
{
	return g_inProcess;
}

//-------------------------------------------------------------------------------------
bool kbe_init()
{
	g_componentID = genUUID64();
	g_componentType = CLIENT_TYPE;

	//pserverconfig = new ServerConfig;
	pconfig = new Config;

	if(!loadConfig())
	{
		ERROR_MSG("loadConfig() is error!\n");
		return false;
	}

	DebugHelper::initHelper(g_componentType);

	INFO_MSG( "-----------------------------------------------------------------------------------------\n\n\n");

#ifdef USE_OPENSSL	
	if(KBEngine::KBEKey::getSingletonPtr() == NULL)
		KBEngine::KBEKey kbekey(KBEngine::Resmgr::getSingleton().matchPath("key/") + "kbengine_public.key", "");
#endif

	if(g_pScript == NULL)
		g_pScript = new KBEngine::script::Script();

	if(g_pDispatcher == NULL)
		g_pDispatcher = new Mercury::EventDispatcher();

	if(g_pNetworkInterface == NULL)
	{
		g_pNetworkInterface = new Mercury::NetworkInterface(g_pDispatcher, 
			0, 0, "", 0, 0,
			0, "", 0, 0);
	}

	if(!installPyScript(*g_pScript, g_componentType))
	{
		ERROR_MSG("installPyScript() is error!\n");
		return false;
	}

	g_pApp = new ClientApp(*g_pDispatcher, *g_pNetworkInterface, g_componentType, g_componentID);
	g_pApp->setScript(g_pScript);

	START_MSG(COMPONENT_NAME_EX(g_componentType), g_componentID);
	if(!g_pApp->initialize())
	{
		ERROR_MSG("app::initialize is error!\n");
		g_pApp->finalise();
		Py_DECREF(g_pApp);
		g_pApp = NULL;

		uninstallPyScript(*g_pScript);

		SAFE_RELEASE(g_pNetworkInterface);
		SAFE_RELEASE(g_pScript);
		SAFE_RELEASE(g_pDispatcher);
		SAFE_RELEASE(pserverconfig);
		SAFE_RELEASE(pconfig);
		return false;
	}

	if(g_pTelnetServer == NULL)
	{
		g_pTelnetServer = new TelnetServer(g_pDispatcher, g_pNetworkInterface);
		g_pTelnetServer->pScript(g_pScript);
		if(!g_pTelnetServer->start(pconfig->telnet_passwd, 
			pconfig->telnet_deflayer, 
			pconfig->telnet_port))
		{
			ERROR_MSG("app::initialize: TelnetServer is error!\n");
			return false;
		}
	}
	else
	{
		g_pTelnetServer->pScript(g_pScript);
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

	PyEval_ReleaseThread(PyThreadState_Get());
	KBEConcurrency::setMainThreadIdleFunctions(&releaseLock, &acquireLock);

	g_pThreadPool->addTask(new KBEMainTask());
	return true;
}

//-------------------------------------------------------------------------------------
bool kbe_destroy()
{
	g_pTelnetServer->stop();
	SAFE_RELEASE(g_pTelnetServer);

	g_break = true;
	g_pApp->getMainDispatcher().breakProcessing();
	g_pThreadPool->finalise();
	KBEngine::sleep(100);
	
	delete g_pThreadPool;

	if(g_pApp)
	{
		PyGILState_Ensure();
		g_pApp->finalise();
		Py_DECREF(g_pApp);
		g_pApp = NULL;
	}
	
	bool ret = uninstallPyScript(*g_pScript);

	SAFE_RELEASE(g_pNetworkInterface);
	SAFE_RELEASE(g_pScript);
	SAFE_RELEASE(g_pDispatcher);
	SAFE_RELEASE(pserverconfig);
	SAFE_RELEASE(pconfig);

	INFO_MSG(boost::format("%1% has shut down.\n") % COMPONENT_NAME_EX(g_componentType));
	return ret;
}

//-------------------------------------------------------------------------------------
KBEngine::uint64 kbe_genUUID64()
{
	return KBEngine::genUUID64();
}

//-------------------------------------------------------------------------------------
void kbe_sleep(KBEngine::uint32 ms)
{
	return KBEngine::sleep(ms);
}

//-------------------------------------------------------------------------------------
KBEngine::uint32 kbe_getSystemTime()
{
	return KBEngine::getSystemTime();
}

//-------------------------------------------------------------------------------------
const char* kbe_getPyUserResPath()
{
	static std::string s = KBEngine::Resmgr::getSingleton().getPyUserResPath();
	return s.c_str();
}

//-------------------------------------------------------------------------------------
bool kbe_login(const char* accountName, const char* passwd, const char* ip, KBEngine::uint32 port)
{
	if(ip == NULL || port == 0)
	{
		ip = pconfig->ip();
		port = pconfig->port();
	}

	return g_pApp->login(accountName, passwd, ip, port);
}

//-------------------------------------------------------------------------------------
void kbe_update()
{
//	if(g_pClientObject)
	{
	//	g_pClientObject->update();
	}
}

//-------------------------------------------------------------------------------------
const char* kbe_getLastAccountName()
{
	return pconfig->accountName();
}

//-------------------------------------------------------------------------------------
KBEngine::ENTITY_ID kbe_playerID()
{
	return g_pApp->entityID();
}

//-------------------------------------------------------------------------------------
KBEngine::DBID kbe_playerDBID()
{
	return g_pApp->dbid();
}

//-------------------------------------------------------------------------------------
bool kbe_registerEventHandle(KBEngine::EventHandle* pHandle)
{
	return g_pApp->registerEventHandle(pHandle);
}

//-------------------------------------------------------------------------------------
bool kbe_deregisterEventHandle(KBEngine::EventHandle* pHandle)
{
	return g_pApp->deregisterEventHandle(pHandle);
}

//-------------------------------------------------------------------------------------
PyObject* kbe_callEntityMethod(KBEngine::ENTITY_ID entityID, const char *method, 
							   PyObject *args, PyObject *kw)
{
	// KBEngine::script::PyThreadStateLock lock;
	client::Entity* pEntity = g_pApp->pEntities()->find(entityID);
	if(!pEntity)
	{
		ERROR_MSG(boost::format("kbe_callEntityMethod::entity %1% not found!\n") % entityID);
		return NULL;
	}

	PyObject* pyfunc = PyObject_GetAttrString(pEntity, method);
	if(pyfunc == NULL)
	{
		ERROR_MSG(boost::format("kbe_callEntityMethod::entity %1% method(%2%) not found!\n") % 
			entityID % method);

		return NULL;
	}

	PyObject* ret = PyObject_Call(pyfunc, args, kw); 
	Py_DECREF(pyfunc);

	if(ret == NULL)
	{
		SCRIPT_ERROR_CHECK();
	}

	return ret;
}

void kbe_fireEvent(const char *eventID, PyObject *args)
{
	KBEngine::script::PyThreadStateLock lock;
	
	KBE_ASSERT(eventID != NULL);

	// DEBUG_MSG(boost::format("kbe_fireEvent: %1%!\n") % 
	//		eventID);

	PyObject* pyEID = PyUnicode_FromString(eventID);

	// 所有脚本都加载完毕
	PyObject* pyResult = PyObject_CallMethod(g_pApp->getEntryScript().get(), 
										const_cast<char*>("kbengine_onEvent"), 
										const_cast<char*>("OO"), 
										pyEID,
										args == NULL ? Py_None : args);

	Py_DECREF(pyEID);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void kbe_updateVolatile(KBEngine::ENTITY_ID eid, float x, float y, float z, float yaw, float pitch, float roll)
{
	//client::Entity* pEntity = g_pApp->pPlayer();
	//if(pEntity == NULL)
	//	return;

	g_pApp->setPlayerPosition(x, y, z);
	g_pApp->setPlayerDirection(roll, pitch, yaw);

	if(eid >= 0)
		targetID = eid;
	//g_pApp->setTargetID(eid);
}

//-------------------------------------------------------------------------------------

