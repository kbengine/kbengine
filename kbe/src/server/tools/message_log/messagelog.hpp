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

#ifndef KBE_MESSAGELOG_HPP
#define KBE_MESSAGELOG_HPP
	
// common include	
#include "server/kbemain.hpp"
#include "server/serverapp.hpp"
#include "server/idallocate.hpp"
#include "server/serverconfig.hpp"
#include "cstdkbe/timer.hpp"
#include "network/endpoint.hpp"
#include "network/udp_packet_receiver.hpp"
#include "network/common.hpp"
#include "network/address.hpp"
#include "logwatcher.hpp"

//#define NDEBUG
#include <map>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

class Messagelog:	public ServerApp, 
				public Singleton<Messagelog>
{
public:
	typedef std::map<Network::Address, LogWatcher> LOG_WATCHERS;
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK = TIMEOUT_SERVERAPP_MAX + 1
	};
	
	Messagelog(Network::EventDispatcher& dispatcher, 
		Network::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Messagelog();
	
	bool run();
	

	void handleTimeout(TimerHandle handle, void * arg);

	/* 初始化相关接口 */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();

	virtual bool canShutdown();

	/** 网络接口
		写日志
	*/
	void writeLog(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		注册log监听者
	*/
	void registerLogWatcher(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	LOG_WATCHERS& logWatchers(){ return logWatchers_; }
protected:
	LOG_WATCHERS logWatchers_;
};

}

#endif // KBE_MESSAGELOG_HPP
