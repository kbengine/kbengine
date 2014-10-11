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

#ifndef KBE_BILLINGSYSTEM_HPP
#define KBE_BILLINGSYSTEM_HPP
	
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
class CreateAccountTask;
class LoginAccountTask;

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

	/* ��ʼ����ؽӿ� */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();
	
	bool initDB();
	
	void lockthread();
	void unlockthread();

	virtual void onShutdownEnd();

	/** ����ӿ�
		���󴴽��˺�
	*/
	void reqCreateAccount(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		һ�����û���¼�� ��Ҫ���Ϸ���
	*/
	void onAccountLogin(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		��������ͻ�����������
	*/
	void eraseClientReq(Mercury::Channel* pChannel, std::string& logkey);

	/** ����ӿ�
		�����ֵ
	*/
	void charge(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	typedef KBEUnordered_map<std::string, KBEShared_ptr<Orders> > ORDERS;
	BillingSystem::ORDERS& orders(){ return orders_; }

	typedef KBEUnordered_map<std::string, CreateAccountTask* /*���õ����ͷ�����, ���̳߳����*/> REQCREATE_MAP;
	typedef KBEUnordered_map<std::string, LoginAccountTask* /*���õ����ͷ�����, ���̳߳����*/> REQLOGIN_MAP;

	REQCREATE_MAP& reqCreateAccount_requests(){ return reqCreateAccount_requests_; }
	REQLOGIN_MAP& reqAccountLogin_requests(){ return reqAccountLogin_requests_; }

	void eraseOrders_s(std::string ordersid);
	
	bool hasOrders(std::string ordersid);
protected:
	TimerHandle																mainProcessTimer_;

	// ����
	ORDERS orders_;

	// ���е������¼�� ����ĳ���ظ�������
	REQCREATE_MAP															reqCreateAccount_requests_;
	REQLOGIN_MAP															reqAccountLogin_requests_;

	KBEngine::thread::ThreadMutex											mutex_;
};

}

#endif // KBE_BILLINGSYSTEM_HPP

