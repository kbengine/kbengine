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
#include "dbmgr.h"
#include "interfaces_handler.h"
#include "buffered_dbtasks.h"
#include "db_interface/db_threadpool.h"
#include "thread/threadpool.h"
#include "thread/threadguard.h"
#include "server/serverconfig.h"
#include "network/endpoint.h"
#include "network/channel.h"
#include "network/bundle.h"

#include "baseapp/baseapp_interface.h"
#include "tools/interfaces/interfaces_interface.h"

namespace KBEngine{

//-------------------------------------------------------------------------------------
InterfacesHandler* InterfacesHandlerFactory::create(std::string type, 
											  thread::ThreadPool& threadPool, 
											  DBThreadPool& dbThreadPool)
{
	if(type.size() == 0 || type == "normal")
	{
		return new InterfacesHandler_Normal(threadPool, dbThreadPool);
	}
	else if(type == "thirdparty")
	{
		return new InterfacesHandler_ThirdParty(threadPool, dbThreadPool);
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
InterfacesHandler::InterfacesHandler(thread::ThreadPool& threadPool, DBThreadPool& dbThreadPool):
dbThreadPool_(dbThreadPool),
threadPool_(threadPool)
{
}

//-------------------------------------------------------------------------------------
InterfacesHandler::~InterfacesHandler()
{
}

//-------------------------------------------------------------------------------------
InterfacesHandler_Normal::InterfacesHandler_Normal(thread::ThreadPool& threadPool, DBThreadPool& dbThreadPool):
InterfacesHandler(threadPool, dbThreadPool)
{
}

//-------------------------------------------------------------------------------------
InterfacesHandler_Normal::~InterfacesHandler_Normal()
{
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_Normal::createAccount(Network::Channel* pChannel, std::string& registerName, 
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
void InterfacesHandler_Normal::onCreateAccountCB(KBEngine::MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_Normal::loginAccount(Network::Channel* pChannel, std::string& loginName, 
										 std::string& password, std::string& datas)
{
	dbThreadPool_.addTask(new DBTaskAccountLogin(pChannel->addr(), 
		loginName, loginName, password, SERVER_SUCCESS, datas, datas));

	return true;
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::onLoginAccountCB(KBEngine::MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::charge(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	INFO_MSG("InterfacesHandler_Normal::charge: no implement!\n");
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::onChargeCB(KBEngine::MemoryStream& s)
{
	INFO_MSG("InterfacesHandler_Normal::onChargeCB: no implement!\n");
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::eraseClientReq(Network::Channel* pChannel, std::string& logkey)
{
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::accountActivate(Network::Channel* pChannel, std::string& scode)
{
	dbThreadPool_.addTask(new DBTaskActivateAccount(pChannel->addr(), scode));
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::accountReqResetPassword(Network::Channel* pChannel, std::string& accountName)
{
	dbThreadPool_.addTask(new DBTaskReqAccountResetPassword(pChannel->addr(), accountName));
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::accountResetPassword(Network::Channel* pChannel, std::string& accountName, std::string& newpassword, std::string& scode)
{
	dbThreadPool_.addTask(new DBTaskAccountResetPassword(pChannel->addr(), accountName, newpassword, scode));
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::accountReqBindMail(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
											   std::string& password, std::string& email)
{
	dbThreadPool_.addTask(new DBTaskReqAccountBindEmail(pChannel->addr(), entityID, accountName, password, email));
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::accountBindMail(Network::Channel* pChannel, std::string& username, std::string& scode)
{
	dbThreadPool_.addTask(new DBTaskAccountBindEmail(pChannel->addr(), username, scode));
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::accountNewPassword(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
											   std::string& password, std::string& newpassword)
{
	dbThreadPool_.addTask(new DBTaskAccountNewPassword(pChannel->addr(), entityID, accountName, password, newpassword));
}

//-------------------------------------------------------------------------------------
InterfacesHandler_ThirdParty::InterfacesHandler_ThirdParty(thread::ThreadPool& threadPool, DBThreadPool& dbThreadPool):
InterfacesHandler_Normal(threadPool, dbThreadPool),
pInterfacesChannel_(NULL)
{
}

//-------------------------------------------------------------------------------------
InterfacesHandler_ThirdParty::~InterfacesHandler_ThirdParty()
{
	if(pInterfacesChannel_)
		pInterfacesChannel_->decRef();

	pInterfacesChannel_ = NULL;
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_ThirdParty::createAccount(Network::Channel* pChannel, std::string& registerName, 
											  std::string& password, std::string& datas, ACCOUNT_TYPE uatype)
{
	KBE_ASSERT(pInterfacesChannel_);

	Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();
	
	(*(*bundle)).newMessage(InterfacesInterface::reqCreateAccount);
	(*(*bundle)) << pChannel->componentID();

	uint8 accountType = uatype;
	(*(*bundle)) << registerName << password << accountType;
	(*(*bundle)).appendBlob(datas);

	if(pInterfacesChannel_->isDestroyed())
	{
		if(!this->reconnect())
			return false;
	}

	(*(*bundle)).send(Dbmgr::getSingleton().networkInterface(), pInterfacesChannel_);
	return true;
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_ThirdParty::onCreateAccountCB(KBEngine::MemoryStream& s)
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
		ERROR_MSG("InterfacesHandler_ThirdParty::onCreateAccountCB: loginapp not found!\n");
		return;
	}

	dbThreadPool_.addTask(new DBTaskCreateAccount(cinfos->pChannel->addr(), 
		registerName, accountName, password, postdatas, getdatas));
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_ThirdParty::loginAccount(Network::Channel* pChannel, std::string& loginName, 
											 std::string& password, std::string& datas)
{
	KBE_ASSERT(pInterfacesChannel_);

	Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();

	(*(*bundle)).newMessage(InterfacesInterface::onAccountLogin);
	(*(*bundle)) << pChannel->componentID();
	(*(*bundle)) << loginName << password;
	(*(*bundle)).appendBlob(datas);

	if(pInterfacesChannel_->isDestroyed())
	{
		if(!this->reconnect())
			return false;
	}

	(*(*bundle)).send(Dbmgr::getSingleton().networkInterface(), pInterfacesChannel_);
	return true;
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_ThirdParty::onLoginAccountCB(KBEngine::MemoryStream& s)
{
	std::string loginName, accountName, password, postdatas, getdatas;
	COMPONENT_ID cid;
	SERVER_ERROR_CODE success = SERVER_ERR_OP_FAILED;

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
		ERROR_MSG("InterfacesHandler_ThirdParty::onCreateAccountCB: loginapp not found!\n");
		return;
	}

	dbThreadPool_.addTask(new DBTaskAccountLogin(cinfos->pChannel->addr(), 
		loginName, accountName, password, success, postdatas, getdatas));
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_ThirdParty::initialize()
{
	return reconnect();
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_ThirdParty::reconnect()
{
	if(pInterfacesChannel_)
	{
		if(!pInterfacesChannel_->isDestroyed())
			Dbmgr::getSingleton().networkInterface().deregisterChannel(pInterfacesChannel_);

		pInterfacesChannel_->decRef();
	}

	Network::Address addr = g_kbeSrvConfig.interfacesAddr();
	Network::EndPoint* pEndPoint = new Network::EndPoint(addr);

	pEndPoint->socket(SOCK_STREAM);
	if (!pEndPoint->good())
	{
		ERROR_MSG("InterfacesHandler_ThirdParty::initialize: couldn't create a socket\n");
		return true;
	}

	pEndPoint->setnonblocking(true);
	pEndPoint->setnodelay(true);

	pInterfacesChannel_ = new Network::Channel(Dbmgr::getSingleton().networkInterface(), pEndPoint, Network::Channel::INTERNAL);
	pInterfacesChannel_->incRef();

	int trycount = 0;

	while(true)
	{
		fd_set	frds, fwds;
		struct timeval tv = { 0, 100000 }; // 100ms

		FD_ZERO( &frds );
		FD_ZERO( &fwds );
		FD_SET((int)(*pInterfacesChannel_->endpoint()), &frds);
		FD_SET((int)(*pInterfacesChannel_->endpoint()), &fwds);

		if(pInterfacesChannel_->endpoint()->connect() == -1)
		{
			int selgot = select((*pInterfacesChannel_->endpoint())+1, &frds, &fwds, NULL, &tv);
			if(selgot > 0)
			{
				int error;

				if(FD_ISSET(int(*pInterfacesChannel_->endpoint()), &frds) || FD_ISSET(int(*pInterfacesChannel_->endpoint()), &fwds) )
				{
					socklen_t len = sizeof(error);

#if KBE_PLATFORM == PLATFORM_WIN32
					if( getsockopt(int(*pInterfacesChannel_->endpoint()), SOL_SOCKET, SO_ERROR, (char*)&error, &len) == 0)
						break;
#else
					if( getsockopt(int(*pInterfacesChannel_->endpoint()), SOL_SOCKET, SO_ERROR, &error, &len) < 0)
						break;
#endif
				}
			}

			trycount++;

			if(trycount > 3)
			{
				ERROR_MSG(fmt::format("InterfacesHandler_ThirdParty::reconnect(): couldn't connect to:{}\n", 
					pInterfacesChannel_->endpoint()->addr().c_str()));
				
				pInterfacesChannel_->destroy();
				return false;
			}
		}
	}

	// 不检查超时
	pInterfacesChannel_->stopInactivityDetection();
	Dbmgr::getSingleton().networkInterface().registerChannel(pInterfacesChannel_);
	return true;
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_ThirdParty::process()
{
	return true;
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_ThirdParty::charge(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string chargeID;
	std::string datas;
	CALLBACK_ID cbid;
	DBID dbid;

	s >> chargeID;
	s >> dbid;
	s.readBlob(datas);
	s >> cbid;

	INFO_MSG(fmt::format("InterfacesHandler_ThirdParty::charge: chargeID={0}, dbid={3}, cbid={1}, datas={2}!\n",
		chargeID, cbid, datas, dbid));

	KBE_ASSERT(pInterfacesChannel_);

	Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();

	(*(*bundle)).newMessage(InterfacesInterface::charge);
	(*(*bundle)) << pChannel->componentID();
	(*(*bundle)) << chargeID;
	(*(*bundle)) << dbid;
	(*(*bundle)).appendBlob(datas);
	(*(*bundle)) << cbid;

	if(pInterfacesChannel_->isDestroyed())
	{
		if(!this->reconnect())
			return;
	}

	(*(*bundle)).send(Dbmgr::getSingleton().networkInterface(), pInterfacesChannel_);
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_ThirdParty::onChargeCB(KBEngine::MemoryStream& s)
{
	std::string chargeID;
	std::string datas;
	CALLBACK_ID cbid;
	COMPONENT_ID cid;
	DBID dbid;
	SERVER_ERROR_CODE retcode;

	s >> cid;
	s >> chargeID;
	s >> dbid;
	s.readBlob(datas);
	s >> cbid;
	s >> retcode;

	INFO_MSG(fmt::format("InterfacesHandler_ThirdParty::onChargeCB: chargeID={0}, dbid={3}, cbid={1}, cid={4}, datas={2}!\n",
		chargeID, cbid, datas, dbid, cid));

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, cid);
	if(cinfos == NULL || cinfos->pChannel == NULL || cinfos->pChannel->isDestroyed())
	{
		ERROR_MSG(fmt::format("InterfacesHandler_ThirdParty::onChargeCB: baseapp not found!, chargeID={}, cid={}.\n", 
			chargeID, cid));

		return;
	}

	Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();

	(*(*bundle)).newMessage(BaseappInterface::onChargeCB);
	(*(*bundle)) << chargeID;
	(*(*bundle)) << dbid;
	(*(*bundle)).appendBlob(datas);
	(*(*bundle)) << cbid;
	(*(*bundle)) << retcode;

	(*(*bundle)).send(Dbmgr::getSingleton().networkInterface(), cinfos->pChannel);
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_ThirdParty::eraseClientReq(Network::Channel* pChannel, std::string& logkey)
{
	KBE_ASSERT(pInterfacesChannel_);

	Network::Bundle::SmartPoolObjectPtr bundle = Network::Bundle::createSmartPoolObj();

	(*(*bundle)).newMessage(InterfacesInterface::eraseClientReq);
	(*(*bundle)) << logkey;

	if(pInterfacesChannel_->isDestroyed())
	{
		if(!this->reconnect())
			return;
	}

	(*(*bundle)).send(Dbmgr::getSingleton().networkInterface(), pInterfacesChannel_);
}


//-------------------------------------------------------------------------------------
void InterfacesHandler_ThirdParty::accountActivate(Network::Channel* pChannel, std::string& scode)
{
}


//-------------------------------------------------------------------------------------
void InterfacesHandler_ThirdParty::accountReqResetPassword(Network::Channel* pChannel, std::string& accountName)
{
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_ThirdParty::accountResetPassword(Network::Channel* pChannel, std::string& accountName, std::string& newpassword, std::string& scode)
{
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_ThirdParty::accountReqBindMail(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
												   std::string& password, std::string& email)
{
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_ThirdParty::accountBindMail(Network::Channel* pChannel, std::string& username, std::string& scode)
{
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_ThirdParty::accountNewPassword(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
												   std::string& password, std::string& newpassword)
{
}

//-------------------------------------------------------------------------------------

}
