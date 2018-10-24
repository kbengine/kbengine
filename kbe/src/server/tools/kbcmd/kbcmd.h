/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KBE_KBCMD_TOOL_H
#define KBE_KBCMD_TOOL_H

#include "server/kbemain.h"
#include "server/python_app.h"
#include "server/serverconfig.h"
#include "common/timer.h"
#include "network/endpoint.h"
#include "resmgr/resmgr.h"
#include "thread/threadpool.h"

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

static int creatDir(const char *pDir)
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

class KBCMD : public PythonApp,
	public Singleton<KBCMD>
{
public:
	enum TimeOutType
	{
		TIMEOUT_TICK = TIMEOUT_PYTHONAPP_MAX + 1
	};

	KBCMD(Network::EventDispatcher& dispatcher,
		Network::NetworkInterface& ninterface,
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~KBCMD();

	bool run();

	void handleTimeout(TimerHandle handle, void * arg);
	void handleMainTick();

	/* 初始化相关接口 */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();
	void onInstallPyModules();
	virtual bool installPyModules();

	bool initDB();

	virtual void onShutdownBegin();
	virtual void onShutdownEnd();

protected:
	TimerHandle																mainProcessTimer_;

};

}

#endif // KBE_KBCMD_TOOL_H

