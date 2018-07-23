// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_SERVER_APP_H
#define KBE_SERVER_APP_H

#include "common/common.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4996)
#endif

#include <stdarg.h> 
#include "helper/debug_helper.h"
#include "helper/watcher.h"
#include "helper/profile.h"
#include "helper/profiler.h"
#include "helper/profile_handler.h"
#include "xml/xml.h"	
#include "server/common.h"
#include "server/components.h"
#include "server/serverconfig.h"
#include "server/signal_handler.h"
#include "server/shutdown_handler.h"
#include "common/smartpointer.h"
#include "common/timer.h"
#include "common/singleton.h"
#include "network/interfaces.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "thread/threadpool.h"

	
namespace KBEngine{

namespace Network
{

class Channel;
}

class Shutdowner;
class ComponentActiveReportHandler;

class ServerApp : 
	public SignalHandler, 
	public TimerHandler, 
	public ShutdownHandler,
	public Network::ChannelTimeOutHandler,
	public Network::ChannelDeregisterHandler,
	public Components::ComponentsNotificationHandler
{
public:
	enum TimeOutType
	{
		TIMEOUT_SERVERAPP_MAX
	};

public:
	ServerApp(Network::EventDispatcher& dispatcher, 
			Network::NetworkInterface& ninterface, 
			COMPONENT_TYPE componentType,
			COMPONENT_ID componentID);

	~ServerApp();

	virtual bool initialize();
	virtual bool initializeBegin() { return true; }
	virtual bool inInitialize(){ return true; }
	virtual bool initializeEnd() {return true; }
	virtual void finalise();
	virtual bool run();
	
	virtual bool initThreadPool();

	virtual bool installSignals();

	virtual bool initializeWatcher();

	virtual bool loadConfig();
	const char* name(){return COMPONENT_NAME_EX(componentType_);}
	
	virtual void handleTimeout(TimerHandle, void * pUser);

	GAME_TIME time() const { return g_kbetime; }
	Timers & timers() { return timers_; }
	double gameTimeInSeconds() const;
	void handleTimers();

	thread::ThreadPool& threadPool() { return threadPool_; }

	Network::EventDispatcher & dispatcher()				{ return dispatcher_; }
	Network::NetworkInterface & networkInterface()			{ return networkInterface_; }

	COMPONENT_ID componentID() const	{ return componentID_; }
	COMPONENT_TYPE componentType() const	{ return componentType_; }
		
	virtual void onSignalled(int sigNum);
	virtual void onChannelTimeOut(Network::Channel * pChannel);
	virtual void onChannelDeregister(Network::Channel * pChannel);
	virtual void onAddComponent(const Components::ComponentInfos* pInfos);
	virtual void onRemoveComponent(const Components::ComponentInfos* pInfos);
	virtual void onIdentityillegal(COMPONENT_TYPE componentType, COMPONENT_ID componentID, uint32 pid, const char* pAddr);

	virtual void onShutdownBegin();
	virtual void onShutdown(bool first);
	virtual void onShutdownEnd();

	/** 网络接口
		请求查看watcher
	*/
	void queryWatcher(Network::Channel* pChannel, MemoryStream& s);

	void shutDown(float shutdowntime = -FLT_MAX);

	COMPONENT_ORDER globalOrder() const { return startGlobalOrder_; }
	COMPONENT_ORDER groupOrder() const { return startGroupOrder_; }

	/** 网络接口
		注册一个新激活的baseapp或者cellapp或者dbmgr
		通常是一个新的app被启动了， 它需要向某些组件注册自己。
	*/
	virtual void onRegisterNewApp(Network::Channel* pChannel, 
							int32 uid, 
							std::string& username, 
							COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER globalorderID, COMPONENT_ORDER grouporderID,
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx);

	/** 网络接口
		某个app向本app告知处于活动状态。
	*/
	void onAppActiveTick(Network::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID);
	
	/** 网络接口
		请求断开服务器的连接
	*/
	virtual void reqClose(Network::Channel* pChannel);

	/** 网络接口
		某个app请求查看该app
	*/
	virtual void lookApp(Network::Channel* pChannel);

	/** 网络接口
		请求关闭服务器
	*/
	virtual void reqCloseServer(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		某个app请求查看该app负载状态， 通常是console请求查看
	*/
	virtual void queryLoad(Network::Channel* pChannel);

	/** 网络接口
		请求关闭服务器
	*/
	void reqKillServer(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		客户端与服务端第一次建立交互, 客户端发送自己的版本号与通讯密钥等信息
		给服务端， 服务端返回是否握手成功
	*/
	virtual void hello(Network::Channel* pChannel, MemoryStream& s);
	virtual void onHello(Network::Channel* pChannel, 
		const std::string& verInfo, 
		const std::string& scriptVerInfo, 
		const std::string& encryptedKey);

	// 引擎版本不匹配
	virtual void onVersionNotMatch(Network::Channel* pChannel);

	// 引擎脚本层版本不匹配
	virtual void onScriptVersionNotMatch(Network::Channel* pChannel);

	/** 网络接口
		console请求开始profile
	*/
	void startProfile(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	virtual void startProfile_(Network::Channel* pChannel, std::string profileName, int8 profileType, uint32 timelen);
		
protected:
	COMPONENT_TYPE											componentType_;
	COMPONENT_ID											componentID_;									// 本组件的ID

	Network::EventDispatcher& 								dispatcher_;	
	Network::NetworkInterface&								networkInterface_;
	
	Timers													timers_;

	// app启动顺序， global为全局(如dbmgr，cellapp的顺序)启动顺序， 
	// group为组启动顺序(如:所有baseapp为一组)
	COMPONENT_ORDER											startGlobalOrder_;
	COMPONENT_ORDER											startGroupOrder_;

	Shutdowner*												pShutdowner_;
	ComponentActiveReportHandler*							pActiveTimerHandle_;

	// 线程池
	thread::ThreadPool										threadPool_;	
};

}

#endif // KBE_SERVER_APP_H
