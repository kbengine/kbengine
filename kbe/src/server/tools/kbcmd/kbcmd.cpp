// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "profile.h"
#include "kbcmd.h"
#include "kbcmd_interface.h"
#include "client_sdk_unity.h"
#include "client_sdk_ue4.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "thread/threadpool.h"
#include "server/components.h"
#include "server/telnet_server.h"
#include "entitydef/entitydef.h"

#include "baseapp/baseapp_interface.h"
#include "cellapp/cellapp_interface.h"
#include "baseappmgr/baseappmgr_interface.h"
#include "cellappmgr/cellappmgr_interface.h"
#include "loginapp/loginapp_interface.h"
#include "dbmgr/dbmgr_interface.h"	

#ifdef _WIN32  
#include <direct.h>  
#include <io.h>  
#elif _LINUX  
#include <stdarg.h>  
#include <sys/stat.h>  
#endif  

#if KBE_PLATFORM == PLATFORM_WIN32
#define KBE_ACCESS _access  
#define KBE_MKDIR(a) _mkdir((a))  
#else
#define KBE_ACCESS access  
#define KBE_MKDIR(a) KBE_UNIX_MKDIR((a))  

int KBE_UNIX_MKDIR(const char* a)
{
	umask(0);
	return mkdir((a), 0755);
}
#endif  

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(KBCMD);

//-------------------------------------------------------------------------------------
KBCMD::KBCMD(Network::EventDispatcher& dispatcher,
	Network::NetworkInterface& ninterface,
	COMPONENT_TYPE componentType,
	COMPONENT_ID componentID) :
	PythonApp(dispatcher, ninterface, componentType, componentID),
	mainProcessTimer_()
{
}

//-------------------------------------------------------------------------------------
KBCMD::~KBCMD()
{
	mainProcessTimer_.cancel();
}

//-------------------------------------------------------------------------------------	
void KBCMD::onShutdownBegin()
{
	PythonApp::onShutdownBegin();
}

//-------------------------------------------------------------------------------------	
void KBCMD::onShutdownEnd()
{
	PythonApp::onShutdownEnd();
}

//-------------------------------------------------------------------------------------
bool KBCMD::run()
{
	return PythonApp::run();
}

//-------------------------------------------------------------------------------------
void KBCMD::handleTimeout(TimerHandle handle, void * arg)
{
	PythonApp::handleTimeout(handle, arg);

	switch (reinterpret_cast<uintptr>(arg))
	{
	case TIMEOUT_TICK:
		this->handleMainTick();
		break;
	default:
		break;
	}
}

//-------------------------------------------------------------------------------------
void KBCMD::handleMainTick()
{
	threadPool_.onMainThreadTick();
}

//-------------------------------------------------------------------------------------
bool KBCMD::initializeBegin()
{
	EntityDef::entityAliasID(ServerConfig::getSingleton().getCellApp().aliasEntityID);
	EntityDef::entitydefAliasID(ServerConfig::getSingleton().getCellApp().entitydefAliasID);
	return true;
}

//-------------------------------------------------------------------------------------
bool KBCMD::inInitialize()
{
	PythonApp::inInitialize();
	// 广播自己的地址给网上上的所有kbemachine
	Components::getSingleton().pHandler(this);
	return true;
}

//-------------------------------------------------------------------------------------
bool KBCMD::initializeEnd()
{
	PythonApp::initializeEnd();

	mainProcessTimer_ = this->dispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
		reinterpret_cast<void *>(TIMEOUT_TICK));


	return true;
}

//-------------------------------------------------------------------------------------		
bool KBCMD::installPyModules()
{
	onInstallPyModules();
	return true;
}

//-------------------------------------------------------------------------------------		
void KBCMD::onInstallPyModules()
{
}

//-------------------------------------------------------------------------------------		
bool KBCMD::initDB()
{
	return true;
}

//-------------------------------------------------------------------------------------
void KBCMD::finalise()
{
	PythonApp::finalise();
}

//-------------------------------------------------------------------------------------
int KBCMD::creatDir(const char *pDir)
{
	int i = 0;
	int iRet = -1;
	int iLen = 0;
	char* pszDir = NULL;

	if (NULL == pDir)
	{
		return 0;
	}

	pszDir = strdup(pDir);
	iLen = strlen(pszDir);

	// 创建中间目录  
	for (i = 0; i < iLen; i++)
	{
		if (pszDir[i] == '\\' || pszDir[i] == '/')
		{
			if (i == 0)
				continue;

			pszDir[i] = '\0';

			//如果不存在,创建  
			iRet = KBE_ACCESS(pszDir, 0);
			if (iRet != 0)
			{
				iRet = KBE_MKDIR(pszDir);
				if (iRet != 0)
				{
					ERROR_MSG(fmt::format("creatDir(): KBE_MKDIR [{}] error! iRet={}\n",
						pszDir, iRet));

					free(pszDir);
					return -1;
				}
			}

			//支持linux,将所有\换成/  
			pszDir[i] = '/';
		}
	}

	if (iLen > 0 && KBE_ACCESS(pszDir, 0) != 0)
	{
		iRet = KBE_MKDIR(pszDir);

		if (iRet != 0)
		{
			ERROR_MSG(fmt::format("creatDir(): KBE_MKDIR [{}] error! iRet={}\n",
				pszDir, iRet));
		}
	}

	free(pszDir);
	return iRet;
}

//-------------------------------------------------------------------------------------
}
