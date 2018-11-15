// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_KBCMD_TOOL_H
#define KBE_KBCMD_TOOL_H

#include "server/kbemain.h"
#include "server/python_app.h"
#include "server/serverconfig.h"
#include "common/timer.h"
#include "network/endpoint.h"
#include "resmgr/resmgr.h"
#include "thread/threadpool.h"

namespace KBEngine{

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

	static int creatDir(const char *pDir);

protected:
	TimerHandle																mainProcessTimer_;

};

}

#endif // KBE_KBCMD_TOOL_H

