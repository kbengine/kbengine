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


#ifndef __SERVER_APP_H__
#define __SERVER_APP_H__
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
#include "server/serverconfig.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "network/interfaces.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "server/signal_handler.hpp"

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

class ServerApp : 
	public SignalHandler, 
	public Mercury::ChannelTimeOutHandler
{
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
	
	bool installSingnals();

	virtual bool loadConfig();
	const char* name(){return COMPONENT_NAME[componentType_];}
	
	GAME_TIME time() const { return time_; }
	double gameTimeInSeconds() const;
		
	Mercury::EventDispatcher & getMainDispatcher()				{ return mainDispatcher_; }
	Mercury::NetworkInterface & getNetworkInterface()			{ return networkInterface_; }

	COMPONENT_ID componentID()const	{ return componentID_; }
	COMPONENT_TYPE componentType()const	{ return componentType_; }
		
	virtual void onSignalled(int sigNum);
	virtual void onChannelTimeOut(Mercury::Channel * pChannel);

	void shutDown();

	int32 globalOrder()const{ return startGlobalOrder_; }
	int32 groupOrder()const{ return startGroupOrder_; }

	/* 网络接口
		注册一个新激活的baseapp或者cellapp或者dbmgr
		通常是一个新的app被启动了， 它需要向某些组件注册自己。
	*/
	virtual void onRegisterNewApp(Mercury::Channel* pChannel, 
							int32 uid, 
							std::string& username, 
							int8 componentType, uint64 componentID, 
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport);
protected:
	COMPONENT_TYPE											componentType_;
	COMPONENT_ID											componentID_;									// 本组件的ID

	Mercury::EventDispatcher& 								mainDispatcher_;	
	Mercury::NetworkInterface&								networkInterface_;
	
	GAME_TIME												time_;
	
	// app启动顺序， global为全局(如dbmgr，cellapp的顺序)启动顺序， 
	// group为组启动顺序(如:所有baseapp为一组)
	int32													startGlobalOrder_;
	int32													startGroupOrder_;
};

}
#endif
