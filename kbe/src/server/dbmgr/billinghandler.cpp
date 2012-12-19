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
#include "dbmgr.hpp"
#include "billinghandler.hpp"
#include "buffered_dbtasks.hpp"
#include "db_threadpool.hpp"
#include "thread/threadpool.hpp"
#include "thread/threadguard.hpp"
#include "server/serverconfig.hpp"
#include "network/endpoint.hpp"
#include "network/channel.hpp"
#include "network/bundle.hpp"

#include "tools/billing_system/billingsystem_interface.hpp"

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
bool BillingHandler_Normal::createAccount(Mercury::Channel* pChannel, std::string& registerName, 
										  std::string& password, std::string& datas)
{
	dbThreadPool_.addTask(new DBTaskCreateAccount(pChannel->addr(), 
		registerName, registerName, password, datas));

	return true;
}

//-------------------------------------------------------------------------------------
void BillingHandler_Normal::onCreateAccountCB(KBEngine::MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
bool BillingHandler_Normal::loginAccount(Mercury::Channel* pChannel, std::string& loginName, 
										 std::string& password, std::string& datas)
{
	dbThreadPool_.addTask(new DBTaskAccountLogin(pChannel->addr(), 
		loginName, loginName, password, true, datas));

	return true;
}

//-------------------------------------------------------------------------------------
void BillingHandler_Normal::onLoginAccountCB(KBEngine::MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
BillingHandler_ThirdParty::BillingHandler_ThirdParty(thread::ThreadPool& threadPool, DBThreadPool& dbThreadPool):
BillingHandler_Normal(threadPool, dbThreadPool),
pBillingChannel_(NULL)
{
}

//-------------------------------------------------------------------------------------
BillingHandler_ThirdParty::~BillingHandler_ThirdParty()
{
	//pBillingChannel_->decRef(); 由networkInterface自己解决
	pBillingChannel_ = NULL;
}

//-------------------------------------------------------------------------------------
bool BillingHandler_ThirdParty::createAccount(Mercury::Channel* pChannel, std::string& registerName, 
											  std::string& password, std::string& datas)
{
	KBE_ASSERT(pBillingChannel_);

	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();
	
	(*(*bundle)).newMessage(BillingSystemInterface::reqCreateAccount);
	(*(*bundle)) << pChannel->componentID();
	(*(*bundle)) << registerName << password;
	(*(*bundle)).appendBlob(datas);

	(*(*bundle)).send(Dbmgr::getSingleton().getNetworkInterface(), pBillingChannel_);
	return true;
}

//-------------------------------------------------------------------------------------
void BillingHandler_ThirdParty::onCreateAccountCB(KBEngine::MemoryStream& s)
{
	std::string registerName, accountName, password, datas;
	COMPONENT_ID cid;
	bool success = false;

	s >> cid >> registerName >> accountName >> password >> success;
	s.readBlob(datas);

	if(!success)
	{
		accountName = "";
	}

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(LOGINAPP_TYPE, cid);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG("BillingHandler_ThirdParty::onCreateAccountCB: loginapp not found!\n");
		return;
	}

	dbThreadPool_.addTask(new DBTaskCreateAccount(cinfos->pChannel->addr(), 
		registerName, accountName, password, datas));
}

//-------------------------------------------------------------------------------------
bool BillingHandler_ThirdParty::loginAccount(Mercury::Channel* pChannel, std::string& loginName, 
											 std::string& password, std::string& datas)
{
	KBE_ASSERT(pBillingChannel_);

	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();

	(*(*bundle)).newMessage(BillingSystemInterface::onAccountLogin);
	(*(*bundle)) << pChannel->componentID();
	(*(*bundle)) << loginName << password;
	(*(*bundle)).appendBlob(datas);

	(*(*bundle)).send(Dbmgr::getSingleton().getNetworkInterface(), pBillingChannel_);
	return true;
}

//-------------------------------------------------------------------------------------
void BillingHandler_ThirdParty::onLoginAccountCB(KBEngine::MemoryStream& s)
{
	std::string loginName, accountName, password, datas;
	COMPONENT_ID cid;
	bool success = false;

	s >> cid >> loginName >> accountName >> password >> success;
	s.readBlob(datas);

	if(!success)
	{
		accountName = "";
	}

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(LOGINAPP_TYPE, cid);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG("BillingHandler_ThirdParty::onCreateAccountCB: loginapp not found!\n");
		return;
	}

	dbThreadPool_.addTask(new DBTaskAccountLogin(cinfos->pChannel->addr(), 
		loginName, accountName, password, success, datas));
}

//-------------------------------------------------------------------------------------
bool BillingHandler_ThirdParty::initialize()
{
	Mercury::Address addr = g_kbeSrvConfig.billingSystemAddr();
	Mercury::EndPoint* pEndPoint = new Mercury::EndPoint(addr);

	pEndPoint->socket(SOCK_STREAM);
	if (!pEndPoint->good())
	{
		ERROR_MSG("BillingHandler_ThirdParty::initialize: couldn't create a socket\n");
		return true;
	}

	pEndPoint->setnonblocking(true);
	pEndPoint->setnodelay(true);

	pBillingChannel_ = new Mercury::Channel(Dbmgr::getSingleton().getNetworkInterface(), pEndPoint, Mercury::Channel::INTERNAL);

	bool ret = reconnect();
	if(ret)
	{
		Dbmgr::getSingleton().getNetworkInterface().registerChannel(pBillingChannel_);
	}
	else
	{
		pBillingChannel_->decRef();
		pBillingChannel_ = NULL;
	}

	return ret;
}

//-------------------------------------------------------------------------------------
bool BillingHandler_ThirdParty::reconnect()
{
	if(pBillingChannel_ == NULL)
		return false;

	int trycount = 0;

	while(true)
	{
		fd_set	frds, fwds;
		struct timeval tv = { 0, 100000 }; // 100ms

		FD_ZERO( &frds );
		FD_ZERO( &fwds );
		FD_SET((int)(*pBillingChannel_->endpoint()), &frds);
		FD_SET((int)(*pBillingChannel_->endpoint()), &fwds);

		if(pBillingChannel_->endpoint()->connect() == -1)
		{
			int selgot = select((*pBillingChannel_->endpoint())+1, &frds, &fwds, NULL, &tv);
			if(selgot > 0)
			{
				break;
			}

			trycount++;

			if(trycount > 3)
			{
				ERROR_MSG(boost::format("BillingHandler_ThirdParty::reconnect(): couldn't connect to:%1%\n") % 
					pBillingChannel_->endpoint()->addr().c_str());

				return false;
			}
		}
	}

	// 不检查超时
	pBillingChannel_->stopInactivityDetection();
	return true;
}

//-------------------------------------------------------------------------------------
bool BillingHandler_ThirdParty::process()
{
	return true;
}

//-------------------------------------------------------------------------------------

}
