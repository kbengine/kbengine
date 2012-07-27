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


#ifndef __CELLAPPMGR_H__
#define __CELLAPPMGR_H__
	
// common include	
#include "cstdkbe/timer.hpp"
#include "network/endpoint.hpp"
#include "helper/debug_helper.hpp"
#include "xmlplus/xmlplus.hpp"	
#include "cstdkbe/singleton.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/timer.hpp"
#include "network/interfaces.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "server/serverconfig.hpp"

//#define NDEBUG
#include <map>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

class Bots : public Singleton<Bots>,
			public TimerHandler, 
			public Mercury::ChannelTimeOutHandler,
			public Mercury::ChannelDeregisterHandler
{
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK
	};
	
	Bots(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Bots();
	
	bool run();
	
	void handleTimeout(TimerHandle handle, void * arg);
	void handleGameTick();

	/* 初始化相关接口 */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();

	Mercury::EventDispatcher & getMainDispatcher()				{ return mainDispatcher_; }
	Mercury::NetworkInterface & getNetworkInterface()			{ return networkInterface_; }

	COMPONENT_ID componentID()const	{ return componentID_; }
	COMPONENT_TYPE componentType()const	{ return componentType_; }

	GAME_TIME time() const { return time_; }
	Timers & timers() { return timers_; }
protected:
	COMPONENT_TYPE											componentType_;
	COMPONENT_ID											componentID_;									// 本组件的ID

	Mercury::EventDispatcher& 								mainDispatcher_;	
	Mercury::NetworkInterface&								networkInterface_;
	
	GAME_TIME												time_;
	Timers													timers_;
	TimerHandle												gameTimer_;
};

}
#endif
