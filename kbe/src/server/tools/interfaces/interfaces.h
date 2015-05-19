/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

#ifndef KBE_INTERFACES_TOOL_H
#define KBE_INTERFACES_TOOL_H
	
// common include	
#include "server/kbemain.h"
#include "server/serverapp.h"
#include "server/serverconfig.h"
#include "common/timer.h"
#include "network/endpoint.h"
#include "resmgr/resmgr.h"
#include "thread/threadpool.h"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

class DBInterface;
class Orders;
class CreateAccountTask;
class LoginAccountTask;

class Interfaces : public ServerApp, 
				public Singleton<Interfaces>
{
public:
	enum TimeOutType
	{
		TIMEOUT_TICK = TIMEOUT_SERVERAPP_MAX + 1
	};

	Interfaces(Network::EventDispatcher& dispatcher, 
		Network::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Interfaces();
	
	bool run();
	
	void handleTimeout(TimerHandle handle, void * arg);
	void handleMainTick();

	/* 初始化相关接口 */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();
	
	bool initDB();
	
	void lockthread();
	void unlockthread();

	virtual void onShutdownEnd();

	/** 网络接口
		请求创建账号
	*/
	void reqCreateAccount(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		一个新用户登录， 需要检查合法性
	*/
	void onAccountLogin(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		请求擦除客户端请求任务
	*/
	void eraseClientReq(Network::Channel* pChannel, std::string& logkey);

	/** 网络接口
		请求充值
	*/
	void charge(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	typedef KBEUnordered_map<std::string, KBEShared_ptr<Orders> > ORDERS;
	Interfaces::ORDERS& orders(){ return orders_; }

	typedef KBEUnordered_map<std::string, CreateAccountTask* /*不用担心释放问题, 由线程池完成*/> REQCREATE_MAP;
	typedef KBEUnordered_map<std::string, LoginAccountTask* /*不用担心释放问题, 由线程池完成*/> REQLOGIN_MAP;

	REQCREATE_MAP& reqCreateAccount_requests(){ return reqCreateAccount_requests_; }
	REQLOGIN_MAP& reqAccountLogin_requests(){ return reqAccountLogin_requests_; }

	void eraseOrders_s(std::string ordersid);
	
	bool hasOrders(std::string ordersid);
protected:
	TimerHandle																mainProcessTimer_;

	// 订单
	ORDERS orders_;

	// 所有的请求记录， 避免某类重复性请求。
	REQCREATE_MAP															reqCreateAccount_requests_;
	REQLOGIN_MAP															reqAccountLogin_requests_;

	KBEngine::thread::ThreadMutex											mutex_;
};

}

#endif // KBE_INTERFACES_TOOL_H

