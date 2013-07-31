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

#ifndef __BILLING_HANDLER_H__
#define __BILLING_HANDLER_H__

// common include	
// #define NDEBUG
#include "dbtasks.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "thread/threadtask.hpp"
#include "helper/debug_helper.hpp"
#include "thread/threadpool.hpp"

namespace KBEngine{ 

namespace Mercury{
	class EndPoint;
}

class DBThreadPool;

/*
	处理计费、第三方运营账号、注册登录系统等挂接
*/
class BillingHandler
{
public:
	BillingHandler(thread::ThreadPool& threadPool, DBThreadPool& dbThreadPool);
	virtual ~BillingHandler();
	
	virtual bool initialize() = 0;

	virtual void eraseClientReq(Mercury::Channel* pChannel, std::string& logkey) = 0;

	virtual bool createAccount(Mercury::Channel* pChannel, std::string& registerName, 
		std::string& password, std::string& datas, ACCOUNT_TYPE uatype) = 0;


	virtual bool loginAccount(Mercury::Channel* pChannel, std::string& loginName, 
		std::string& password, std::string& datas) = 0;

	virtual void onCreateAccountCB(KBEngine::MemoryStream& s) = 0;

	virtual void onLoginAccountCB(KBEngine::MemoryStream& s) = 0;

	virtual void charge(Mercury::Channel* pChannel, KBEngine::MemoryStream& s) = 0;
	virtual void onChargeCB(KBEngine::MemoryStream& s) = 0;
protected:
	DBThreadPool& dbThreadPool_;
	thread::ThreadPool& threadPool_;
};

class BillingHandler_Normal : public BillingHandler
{
public:
	BillingHandler_Normal(thread::ThreadPool& threadPool, DBThreadPool& dbThreadPool);
	virtual ~BillingHandler_Normal();
	
	virtual bool initialize(){ return true; }

	virtual void eraseClientReq(Mercury::Channel* pChannel, std::string& logkey);

	virtual bool createAccount(Mercury::Channel* pChannel, std::string& registerName, 
		std::string& password, std::string& datas, ACCOUNT_TYPE uatype);

	virtual bool loginAccount(Mercury::Channel* pChannel, std::string& loginName, 
		std::string& password, std::string& datas);

	virtual void onCreateAccountCB(KBEngine::MemoryStream& s);

	virtual void onLoginAccountCB(KBEngine::MemoryStream& s);

	virtual void charge(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	virtual void onChargeCB(KBEngine::MemoryStream& s);
protected:
};

class BillingHandler_ThirdParty : public BillingHandler_Normal, public thread::TPTask
{
public:
	BillingHandler_ThirdParty(thread::ThreadPool& threadPool, DBThreadPool& dbThreadPool);
	virtual ~BillingHandler_ThirdParty();
	
	virtual bool initialize();

	virtual void eraseClientReq(Mercury::Channel* pChannel, std::string& logkey);

	virtual bool createAccount(Mercury::Channel* pChannel, std::string& registerName, 
		std::string& password, std::string& datas, ACCOUNT_TYPE uatype);

	virtual bool loginAccount(Mercury::Channel* pChannel, std::string& loginName, 
		std::string& password, std::string& datas);

	virtual void onCreateAccountCB(KBEngine::MemoryStream& s);

	virtual void onLoginAccountCB(KBEngine::MemoryStream& s);

	virtual void charge(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	virtual void onChargeCB(KBEngine::MemoryStream& s);

	bool reconnect();

	virtual bool process();
protected:
	Mercury::Channel* pBillingChannel_;
};

class BillingHandlerFactory
{
public:
	static BillingHandler* create(std::string type, thread::ThreadPool& threadPool, 
		DBThreadPool& dbThreadPool);
};

}
#endif
