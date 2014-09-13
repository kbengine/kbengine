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
#include "dbmgr_lib/db_threadpool.hpp"
#include "thread/threadpool.hpp"
#include "thread/threadguard.hpp"
#include "server/serverconfig.hpp"
#include "network/endpoint.hpp"
#include "network/channel.hpp"
#include "network/bundle.hpp"

#include "baseapp/baseapp_interface.hpp"
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
										  std::string& password, std::string& datas, ACCOUNT_TYPE uatype)
{
	// 如果是email， 先查询账号是否存在然后将其登记入库
	if(uatype == ACCOUNT_TYPE_MAIL)
	{
		dbThreadPool_.addTask(new DBTaskCreateMailAccount(pChannel->addr(), 
			registerName, registerName, password, datas, datas));

		return true;
	}

	dbThreadPool_.addTask(new DBTaskCreateAccount(pChannel->addr(), 
		registerName, registerName, password, datas, datas));

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
		loginName, loginName, password, true, datas, datas));

	return true;
}

//-------------------------------------------------------------------------------------
void BillingHandler_Normal::onLoginAccountCB(KBEngine::MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void BillingHandler_Normal::charge(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	INFO_MSG("BillingHandler_Normal::charge: no implement!\n");
}

//-------------------------------------------------------------------------------------
void BillingHandler_Normal::onChargeCB(KBEngine::MemoryStream& s)
{
	INFO_MSG("BillingHandler_Normal::onChargeCB: no implement!\n");
}

//-------------------------------------------------------------------------------------
void BillingHandler_Normal::eraseClientReq(Mercury::Channel* pChannel, std::string& logkey)
{
}

//-------------------------------------------------------------------------------------
void BillingHandler_Normal::accountActivate(Mercury::Channel* pChannel, std::string& scode)
{
	dbThreadPool_.addTask(new DBTaskActivateAccount(pChannel->addr(), scode));
}

//-------------------------------------------------------------------------------------
void BillingHandler_Normal::accountReqResetPassword(Mercury::Channel* pChannel, std::string& accountName)
{
	dbThreadPool_.addTask(new DBTaskReqAccountResetPassword(pChannel->addr(), accountName));
}

//-------------------------------------------------------------------------------------
void BillingHandler_Normal::accountResetPassword(Mercury::Channel* pChannel, std::string& accountName, std::string& newpassword, std::string& scode)
{
	dbThreadPool_.addTask(new DBTaskAccountResetPassword(pChannel->addr(), accountName, newpassword, scode));
}

//-------------------------------------------------------------------------------------
void BillingHandler_Normal::accountReqBindMail(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
											   std::string& password, std::string& email)
{
	dbThreadPool_.addTask(new DBTaskReqAccountBindEmail(pChannel->addr(), entityID, accountName, password, email));
}

//-------------------------------------------------------------------------------------
void BillingHandler_Normal::accountBindMail(Mercury::Channel* pChannel, std::string& username, std::string& scode)
{
	dbThreadPool_.addTask(new DBTaskAccountBindEmail(pChannel->addr(), username, scode));
}

//-------------------------------------------------------------------------------------
void BillingHandler_Normal::accountNewPassword(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
											   std::string& password, std::string& newpassword)
{
	dbThreadPool_.addTask(new DBTaskAccountNewPassword(pChannel->addr(), entityID, accountName, password, newpassword));
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
	if(pBillingChannel_)
		pBillingChannel_->decRef();

	pBillingChannel_ = NULL;
}

//-------------------------------------------------------------------------------------
bool BillingHandler_ThirdParty::createAccount(Mercury::Channel* pChannel, std::string& registerName, 
											  std::string& password, std::string& datas, ACCOUNT_TYPE uatype)
{
	KBE_ASSERT(pBillingChannel_);

	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();
	
	(*(*bundle)).newMessage(BillingSystemInterface::reqCreateAccount);
	(*(*bundle)) << pChannel->componentID();

	uint8 accountType = uatype;
	(*(*bundle)) << registerName << password << accountType;
	(*(*bundle)).appendBlob(datas);

	if(pBillingChannel_->isDestroyed())
	{
		if(!this->reconnect())
			return false;
	}

	(*(*bundle)).send(Dbmgr::getSingleton().networkInterface(), pBillingChannel_);
	return true;
}

//-------------------------------------------------------------------------------------
void BillingHandler_ThirdParty::onCreateAccountCB(KBEngine::MemoryStream& s)
{
	std::string registerName, accountName, password, postdatas, getdatas;
	COMPONENT_ID cid;
	bool success = false;

	s >> cid >> registerName >> accountName >> password >> success;
	s.readBlob(postdatas);
	s.readBlob(getdatas);

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
		registerName, accountName, password, postdatas, getdatas));
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

	if(pBillingChannel_->isDestroyed())
	{
		if(!this->reconnect())
			return false;
	}

	(*(*bundle)).send(Dbmgr::getSingleton().networkInterface(), pBillingChannel_);
	return true;
}

//-------------------------------------------------------------------------------------
void BillingHandler_ThirdParty::onLoginAccountCB(KBEngine::MemoryStream& s)
{
	std::string loginName, accountName, password, postdatas, getdatas;
	COMPONENT_ID cid;
	bool success = false;

	s >> cid >> loginName >> accountName >> password >> success;
	s.readBlob(postdatas);
	s.readBlob(getdatas);

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
		loginName, accountName, password, success, postdatas, getdatas));
}

//-------------------------------------------------------------------------------------
bool BillingHandler_ThirdParty::initialize()
{
	return reconnect();
}

//-------------------------------------------------------------------------------------
bool BillingHandler_ThirdParty::reconnect()
{
	if(pBillingChannel_)
	{
		if(!pBillingChannel_->isDestroyed())
			Dbmgr::getSingleton().networkInterface().deregisterChannel(pBillingChannel_);

		pBillingChannel_->decRef();
	}

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

	pBillingChannel_ = new Mercury::Channel(Dbmgr::getSingleton().networkInterface(), pEndPoint, Mercury::Channel::INTERNAL);
	pBillingChannel_->incRef();

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
				
				pBillingChannel_->destroy();
				return false;
			}
		}
	}

	// 不检查超时
	pBillingChannel_->stopInactivityDetection();
	Dbmgr::getSingleton().networkInterface().registerChannel(pBillingChannel_);
	return true;
}

//-------------------------------------------------------------------------------------
bool BillingHandler_ThirdParty::process()
{
	return true;
}

//-------------------------------------------------------------------------------------
void BillingHandler_ThirdParty::charge(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string chargeID;
	std::string datas;
	CALLBACK_ID cbid;
	DBID dbid;

	s >> chargeID;
	s >> dbid;
	s.readBlob(datas);
	s >> cbid;

	INFO_MSG(boost::format("BillingHandler_ThirdParty::charge: chargeID=%1%, dbid=%4%, cbid=%2%, datas=%3%!\n") %
		chargeID % cbid % datas % dbid);

	KBE_ASSERT(pBillingChannel_);

	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();

	(*(*bundle)).newMessage(BillingSystemInterface::charge);
	(*(*bundle)) << pChannel->componentID();
	(*(*bundle)) << chargeID;
	(*(*bundle)) << dbid;
	(*(*bundle)).appendBlob(datas);
	(*(*bundle)) << cbid;

	if(pBillingChannel_->isDestroyed())
	{
		if(!this->reconnect())
			return;
	}

	(*(*bundle)).send(Dbmgr::getSingleton().networkInterface(), pBillingChannel_);
}

//-------------------------------------------------------------------------------------
void BillingHandler_ThirdParty::onChargeCB(KBEngine::MemoryStream& s)
{
	std::string chargeID;
	std::string datas;
	CALLBACK_ID cbid;
	COMPONENT_ID cid;
	DBID dbid;
	bool success;

	s >> cid;
	s >> chargeID;
	s >> dbid;
	s.readBlob(datas);
	s >> cbid;
	s >> success;

	INFO_MSG(boost::format("BillingHandler_ThirdParty::onChargeCB: chargeID=%1%, dbid=%4%, cbid=%2%, cid=%5%, datas=%3%!\n") %
		chargeID % cbid % datas % dbid % cid);

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, cid);
	if(cinfos == NULL || cinfos->pChannel == NULL || cinfos->pChannel->isDestroyed())
	{
		ERROR_MSG(boost::format("BillingHandler_ThirdParty::onChargeCB: baseapp not found!, chargeID=%1%, cid=%2%.\n") 
			% chargeID % cid);

		return;
	}

	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();

	(*(*bundle)).newMessage(BaseappInterface::onChargeCB);
	(*(*bundle)) << chargeID;
	(*(*bundle)) << dbid;
	(*(*bundle)).appendBlob(datas);
	(*(*bundle)) << cbid;
	(*(*bundle)) << success;

	(*(*bundle)).send(Dbmgr::getSingleton().networkInterface(), cinfos->pChannel);
}

//-------------------------------------------------------------------------------------
void BillingHandler_ThirdParty::eraseClientReq(Mercury::Channel* pChannel, std::string& logkey)
{
	KBE_ASSERT(pBillingChannel_);

	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();

	(*(*bundle)).newMessage(BillingSystemInterface::eraseClientReq);
	(*(*bundle)) << logkey;

	if(pBillingChannel_->isDestroyed())
	{
		if(!this->reconnect())
			return;
	}

	(*(*bundle)).send(Dbmgr::getSingleton().networkInterface(), pBillingChannel_);
}


//-------------------------------------------------------------------------------------
void BillingHandler_ThirdParty::accountActivate(Mercury::Channel* pChannel, std::string& scode)
{
}


//-------------------------------------------------------------------------------------
void BillingHandler_ThirdParty::accountReqResetPassword(Mercury::Channel* pChannel, std::string& accountName)
{
}

//-------------------------------------------------------------------------------------
void BillingHandler_ThirdParty::accountResetPassword(Mercury::Channel* pChannel, std::string& accountName, std::string& newpassword, std::string& scode)
{
}

//-------------------------------------------------------------------------------------
void BillingHandler_ThirdParty::accountReqBindMail(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
												   std::string& password, std::string& email)
{
}

//-------------------------------------------------------------------------------------
void BillingHandler_ThirdParty::accountBindMail(Mercury::Channel* pChannel, std::string& username, std::string& scode)
{
}

//-------------------------------------------------------------------------------------
void BillingHandler_ThirdParty::accountNewPassword(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
												   std::string& password, std::string& newpassword)
{
}

//-------------------------------------------------------------------------------------

}
