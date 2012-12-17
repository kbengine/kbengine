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
#include "billinghandler.hpp"
#include "buffered_dbtasks.hpp"
#include "db_threadpool.hpp"
#include "thread/threadpool.hpp"
#include "thread/threadguard.hpp"
#include "server/serverconfig.hpp"
#include "network/endpoint.hpp"
#include "network/channel.hpp"

namespace KBEngine{

//-------------------------------------------------------------------------------------
BillingHandler* BillingHandlerFactory::create(std::string type, 
											  thread::ThreadPool& threadPool, 
											  DBThreadPool& dbThreadPool)
{
	if(type.size() == 0 || type == "normal")
	{
		return new BillingHandler_Normal(threadPool, dbThreadPool);
	}
	else if(type == "thirdparty")
	{
		return new BillingHandler_ThirdParty(threadPool, dbThreadPool);
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
BillingHandler::BillingHandler(thread::ThreadPool& threadPool, DBThreadPool& dbThreadPool):
dbThreadPool_(dbThreadPool),
threadPool_(threadPool)
{
}

//-------------------------------------------------------------------------------------
BillingHandler::~BillingHandler()
{
}

//-------------------------------------------------------------------------------------
BillingHandler_Normal::BillingHandler_Normal(thread::ThreadPool& threadPool, DBThreadPool& dbThreadPool):
BillingHandler(threadPool, dbThreadPool)
{
}

//-------------------------------------------------------------------------------------
BillingHandler_Normal::~BillingHandler_Normal()
{
}

//-------------------------------------------------------------------------------------
bool BillingHandler_Normal::createAccount(Mercury::Channel* pChannel, std::string& accountName, 
										  std::string& password, std::string& datas)
{
	dbThreadPool_.addTask(new DBTaskCreateAccount(pChannel->addr(), 
		accountName, password, datas));

	return true;
}

//-------------------------------------------------------------------------------------
bool BillingHandler_Normal::loginAccount(Mercury::Channel* pChannel, std::string& loginName, 
										 std::string& password, std::string& datas)
{
	dbThreadPool_.addTask(new DBTaskAccountLogin(pChannel->addr(), 
		loginName, password, datas));

	return true;
}

//-------------------------------------------------------------------------------------
BillingHandler_ThirdParty::BillingHandler_ThirdParty(thread::ThreadPool& threadPool, DBThreadPool& dbThreadPool):
BillingHandler(threadPool, dbThreadPool),
pEndPoint_(NULL)
{
}

//-------------------------------------------------------------------------------------
BillingHandler_ThirdParty::~BillingHandler_ThirdParty()
{
	SAFE_RELEASE(pEndPoint_);
}

//-------------------------------------------------------------------------------------
bool BillingHandler_ThirdParty::createAccount(Mercury::Channel* pChannel, std::string& accountName, 
											  std::string& password, std::string& datas)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool BillingHandler_ThirdParty::loginAccount(Mercury::Channel* pChannel, std::string& loginName, 
											 std::string& password, std::string& datas)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool BillingHandler_ThirdParty::initialize()
{
	Mercury::Address addr = g_kbeSrvConfig.billingSystemAddr();

	pEndPoint_ = new Mercury::EndPoint(addr);

	pEndPoint_->socket(SOCK_STREAM);
	if (!pEndPoint_->good())
	{
		ERROR_MSG("BillingHandler_ThirdParty::initialize: couldn't create a socket\n");
		return true;
	}

	pEndPoint_->setnonblocking(true);
	pEndPoint_->setnodelay(true);
	return reconnect();
}

//-------------------------------------------------------------------------------------
bool BillingHandler_ThirdParty::reconnect()
{
	if(pEndPoint_ == NULL)
		return false;

	int trycount = 0;

	while(true)
	{
		fd_set	frds, fwds;
		struct timeval tv = { 0, 100000 }; // 100ms

		FD_ZERO( &frds );
		FD_ZERO( &fwds );
		FD_SET((int)(*pEndPoint_), &frds);
		FD_SET((int)(*pEndPoint_), &fwds);

		if(pEndPoint_->connect() == -1)
		{
			int selgot = select((*pEndPoint_)+1, &frds, &fwds, NULL, &tv);
			if(selgot > 0)
			{
				break;
			}

			trycount++;

			if(trycount > 3)
			{
				ERROR_MSG(boost::format("BillingHandler_ThirdParty::reconnect(): couldn't connect to:%1%\n") % 
					pEndPoint_->addr().c_str());

				return false;
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------

}
