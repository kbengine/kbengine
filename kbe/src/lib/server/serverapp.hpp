/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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


#ifndef KBE_SERVER_APP_HPP
#define KBE_SERVER_APP_HPP

// common include
#include "cstdkbe/cstdkbe.hpp"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4996)
#endif
//#define NDEBUG
#include <stdarg.h> 
#include "helper/debug_helper.hpp"
#include "helper/watcher.hpp"
#include "helper/profile.hpp"
#include "helper/profiler.hpp"
#include "helper/profile_handler.hpp"
#include "xmlplus/xmlplus.hpp"	
#include "server/common.hpp"
#include "server/components.hpp"
#include "server/serverconfig.hpp"
#include "server/signal_handler.hpp"
#include "server/shutdown_handler.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/timer.hpp"
#include "cstdkbe/singleton.hpp"
#include "network/interfaces.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "thread/threadpool.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

namespace Mercury
{

class Channel;
}

class Shutdowner;
class ComponentActiveReportHandler;

class ServerApp : 
	public SignalHandler, 
	public TimerHandler, 
	public ShutdownHandler,
	public Mercury::ChannelTimeOutHandler,
	public Mercury::ChannelDeregisterHandler,
	public Components::ComponentsNotificationHandler
{
public:
	enum TimeOutType
	{
		TIMEOUT_SERVERAPP_MAX
	};
public:
	ServerApp(Mercury::EventDispatcher& dispatcher, 
			Mercury::NetworkInterface& ninterface, 
			COMPONENT_TYPE componentType,
			COMPONENT_ID componentID);

	~ServerApp();

	virtual bool initialize();
	virtual bool initializeBegin(){return true;};
	virtual bool inInitialize(){ return true; }
	virtual bool initializeEnd(){return true;};
	virtual void finalise();
	virtual bool run();
	
	virtual bool initThreadPool();

	bool installSingnals();

	virtual bool initializeWatcher();

	virtual bool loadConfig();
	const char* name(){return COMPONENT_NAME_EX(componentType_);}
	
	virtual void handleTimeout(TimerHandle, void * pUser);

	GAME_TIME time() const { return g_kbetime; }
	Timers & timers() { return timers_; }
	double gameTimeInSeconds() const;
	void handleTimers();

	thread::ThreadPool& threadPool(){ return threadPool_; }

	Mercury::EventDispatcher & mainDispatcher()				{ return mainDispatcher_; }
	Mercury::NetworkInterface & networkInterface()			{ return networkInterface_; }

	COMPONENT_ID componentID()const	{ return componentID_; }
	COMPONENT_TYPE componentType()const	{ return componentType_; }
		
	virtual void onSignalled(int sigNum);
	virtual void onChannelTimeOut(Mercury::Channel * pChannel);
	virtual void onChannelDeregister(Mercury::Channel * pChannel);
	virtual void onAddComponent(const Components::ComponentInfos* pInfos);
	virtual void onRemoveComponent(const Components::ComponentInfos* pInfos);

	virtual void onShutdownBegin();
	virtual void onShutdown(bool first);
	virtual void onShutdownEnd();

	/** 网络接口
		请求查看watcher
	*/
	void queryWatcher(Mercury::Channel* pChannel, MemoryStream& s);

	void shutDown(float shutdowntime = -FLT_MAX);

	int32 globalOrder()const{ return startGlobalOrder_; }
	int32 groupOrder()const{ return startGroupOrder_; }

	/** 网络接口
		注册一个新激活的baseapp或者cellapp或者dbmgr
		通常是一个新的app被启动了， 它需要向某些组件注册自己。
	*/
	virtual void onRegisterNewApp(Mercury::Channel* pChannel, 
							int32 uid, 
							std::string& username, 
							int8 componentType, uint64 componentID, int8 globalorderID, int8 grouporderID,
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx);

	/** 网络接口
		某个app向本app告知处于活动状态。
	*/
	void onAppActiveTick(Mercury::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID);
	
	/** 网络接口
		请求断开服务器的连接
	*/
	virtual void reqClose(Mercury::Channel* pChannel);

	/** 网络接口
		某个app请求查看该app
	*/
	virtual void lookApp(Mercury::Channel* pChannel);

	/** 网络接口
		请求关闭服务器
	*/
	virtual void reqCloseServer(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		某个app请求查看该app负载状态， 通常是console请求查看
	*/
	virtual void queryLoad(Mercury::Channel* pChannel);

	/** 网络接口
		请求关闭服务器
	*/
	void reqKillServer(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		客户端与服务端第一次建立交互, 客户端发送自己的版本号与通讯密钥等信息
		给服务端， 服务端返回是否握手成功
	*/
	virtual void hello(Mercury::Channel* pChannel, MemoryStream& s);
	virtual void onHello(Mercury::Channel* pChannel, 
		const std::string& verInfo, 
		const std::string& scriptVerInfo, 
		const std::string& encryptedKey);

	// 引擎版本不匹配
	virtual void onVersionNotMatch(Mercury::Channel* pChannel);

	// 引擎脚本层版本不匹配
	virtual void onScriptVersionNotMatch(Mercury::Channel* pChannel);

	/** 网络接口
		console请求开始profile
	*/
	void startProfile(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	virtual void startProfile_(Mercury::Channel* pChannel, std::string profileName, int8 profileType, uint32 timelen);
protected:
	COMPONENT_TYPE											componentType_;
	COMPONENT_ID											componentID_;									// 本组件的ID

	Mercury::EventDispatcher& 								mainDispatcher_;	
	Mercury::NetworkInterface&								networkInterface_;
	
	Timers													timers_;

	// app启动顺序， global为全局(如dbmgr，cellapp的顺序)启动顺序， 
	// group为组启动顺序(如:所有baseapp为一组)
	int32													startGlobalOrder_;
	int32													startGroupOrder_;

	Shutdowner*												pShutdowner_;
	ComponentActiveReportHandler*							pActiveTimerHandle_;

	// 线程池
	thread::ThreadPool										threadPool_;	
};

}

#endif // KBE_SERVER_APP_HPP
