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


#ifndef __CLIENT_APP_H__
#define __CLIENT_APP_H__
// common include
#include "cstdkbe/cstdkbe.hpp"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4996)
#endif
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <stdarg.h> 
#include "helper/debug_helper.hpp"
#include "xmlplus/xmlplus.hpp"	
#include "cstdkbe/singleton.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/timer.hpp"
#include "network/interfaces.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"

// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

namespace Mercury
{

class Channel;
}

class ClientApp : 
	public Singleton<ClientApp>,
	public TimerHandler, 
	public Mercury::ChannelTimeOutHandler,
	public Mercury::ChannelDeregisterHandler
{
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK
	};
public:
	ClientApp(Mercury::EventDispatcher& dispatcher, 
			Mercury::NetworkInterface& ninterface, 
			COMPONENT_TYPE componentType,
			COMPONENT_ID componentID);

	~ClientApp();

	virtual bool initialize();
	virtual bool initializeBegin(){return true;};
	virtual bool inInitialize(){ return true; }
	virtual bool initializeEnd(){return true;};
	virtual void finalise();
	virtual bool run();
	
	bool installSingnals();

	virtual bool loadConfig();
	const char* name(){return COMPONENT_NAME[componentType_];}
	
	virtual void handleTimeout(TimerHandle, void * pUser);

	GAME_TIME time() const { return time_; }
	Timers & timers() { return timers_; }
	double gameTimeInSeconds() const;
	void handleTimers();

	Mercury::EventDispatcher & getMainDispatcher()				{ return mainDispatcher_; }
	Mercury::NetworkInterface & getNetworkInterface()			{ return networkInterface_; }

	COMPONENT_ID componentID()const	{ return componentID_; }
	COMPONENT_TYPE componentType()const	{ return componentType_; }
		

	virtual void onChannelTimeOut(Mercury::Channel * pChannel);
	virtual void onChannelDeregister(Mercury::Channel * pChannel);

	void shutDown();

	/* 网络接口
	   登录失败回调
	   @failedcode: 失败返回码 -1=服务器没有准备好, 
		                       0=服务器负载过重, 
							   1=账号或者密码不正确
	*/
	void onLoginFailed(Mercury::Channel * pChannel, int8 failedcode);

	/* 网络接口
	   登录成功
	*/
	void onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s);
protected:
	COMPONENT_TYPE											componentType_;
	COMPONENT_ID											componentID_;									// 本组件的ID

	Mercury::EventDispatcher& 								mainDispatcher_;
	Mercury::NetworkInterface&								networkInterface_;
	
	GAME_TIME												time_;
	Timers													timers_;

};

}
#endif
