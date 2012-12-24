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

#ifndef __BILLINGSYSTEM_H__
#define __BILLINGSYSTEM_H__
	
// common include	
#include "server/kbemain.hpp"
#include "server/serverapp.hpp"
#include "server/serverconfig.hpp"
#include "cstdkbe/timer.hpp"
#include "network/endpoint.hpp"
#include "resmgr/resmgr.hpp"
#include "thread/threadpool.hpp"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

class DBInterface;
class Orders;

class BillingSystem : public ServerApp, 
				public Singleton<BillingSystem>
{
public:
	enum TimeOutType
	{
		TIMEOUT_TICK = TIMEOUT_SERVERAPP_MAX + 1
	};

	BillingSystem(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~BillingSystem();
	
	bool run();
	
	void handleTimeout(TimerHandle handle, void * arg);
	void handleMainTick();

	/* 初始化相关接口 */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();
	
	bool initDB();
	
	/** 网络接口
		请求创建账号
	*/
	void reqCreateAccount(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		一个新用户登录， 需要检查合法性
	*/
	void onAccountLogin(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		请求充值
	*/
	void charge(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
protected:
	TimerHandle																mainProcessTimer_;

	// 订单
	typedef std::tr1::unordered_map<std::string, std::tr1::shared_ptr<Orders> > ORDERS;
	ORDERS orders_;
};

}

#endif

