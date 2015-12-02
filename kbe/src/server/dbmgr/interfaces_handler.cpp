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
#include "dbmgr.h"
#include "interfaces_handler.h"
#include "buffered_dbtasks.h"
#include "db_interface/db_threadpool.h"
#include "db_interface/db_interface.h"
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
InterfacesHandler* InterfacesHandlerFactory::create(std::string type)
{
	if(type.size() == 0 || type == "normal")
	{
		return new InterfacesHandler_Normal();
	}
	else if(type == "thirdparty")
	{
		return new InterfacesHandler_ThirdParty();
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
InterfacesHandler::InterfacesHandler()
{
}

//-------------------------------------------------------------------------------------
InterfacesHandler::~InterfacesHandler()
{
}

//-------------------------------------------------------------------------------------
InterfacesHandler_Normal::InterfacesHandler_Normal():
InterfacesHandler()
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
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(registerName);

	thread::ThreadPool* pThreadPool =DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Normal::createAccount: not found dbInterface({})!\n",
			dbInterfaceName));

		return false;
	}

	// 如果是email，先查询账号是否存在然后将其登记入库
	if(uatype == ACCOUNT_TYPE_MAIL)
	{
		pThreadPool->addTask(new DBTaskCreateMailAccount(pChannel->addr(),
			registerName, registerName, password, datas, datas));

		return true;
	}

	pThreadPool->addTask(new DBTaskCreateAccount(pChannel->addr(),
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
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(loginName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Normal::loginAccount: not found dbInterface({})!\n",
			dbInterfaceName));

		return false;
	}

	pThreadPool->addTask(new DBTaskAccountLogin(pChannel->addr(),
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
	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();
	std::vector<DBInterfaceInfo>::iterator dbinfo_iter = dbcfg.dbInterfaceInfos.begin();
	for (; dbinfo_iter != dbcfg.dbInterfaceInfos.end(); ++dbinfo_iter)
	{
		std::string dbInterfaceName = dbinfo_iter->name;

		thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
		if (!pThreadPool)
		{
			ERROR_MSG(fmt::format("InterfacesHandler_Normal::accountActivate: not found dbInterface({})!\n",
				dbInterfaceName));

			return;
		}

		pThreadPool->addTask(new DBTaskActivateAccount(pChannel->addr(), scode));
	}
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::accountReqResetPassword(Network::Channel* pChannel, std::string& accountName)
{
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(accountName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Normal::accountReqResetPassword: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	pThreadPool->addTask(new DBTaskReqAccountResetPassword(pChannel->addr(), accountName));
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::accountResetPassword(Network::Channel* pChannel, std::string& accountName, std::string& newpassword, std::string& scode)
{
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(accountName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Normal::accountResetPassword: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	pThreadPool->addTask(new DBTaskAccountResetPassword(pChannel->addr(), accountName, newpassword, scode));
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::accountReqBindMail(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
											   std::string& password, std::string& email)
{
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(accountName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Normal::accountReqBindMail: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	pThreadPool->addTask(new DBTaskReqAccountBindEmail(pChannel->addr(), entityID, accountName, password, email));
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::accountBindMail(Network::Channel* pChannel, std::string& username, std::string& scode)
{
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(username);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Normal::accountBindMail: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	pThreadPool->addTask(new DBTaskAccountBindEmail(pChannel->addr(), username, scode));
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Normal::accountNewPassword(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
											   std::string& password, std::string& newpassword)
{
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(accountName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Normal::accountNewPassword: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	pThreadPool->addTask(new DBTaskAccountNewPassword(pChannel->addr(), entityID, accountName, password, newpassword));
}

//-------------------------------------------------------------------------------------
InterfacesHandler_ThirdParty::InterfacesHandler_ThirdParty():
InterfacesHandler_Normal()
{
}

//-------------------------------------------------------------------------------------
InterfacesHandler_ThirdParty::~InterfacesHandler_ThirdParty()
{
	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(g_kbeSrvConfig.interfacesAddr());
	if(pInterfacesChannel)
	{
		pInterfacesChannel->condemn();
	}

	pInterfacesChannel = NULL;
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_ThirdParty::createAccount(Network::Channel* pChannel, std::string& registerName, 
											  std::string& password, std::string& datas, ACCOUNT_TYPE uatype)
{
	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(g_kbeSrvConfig.interfacesAddr());
	KBE_ASSERT(pInterfacesChannel);

	if(pInterfacesChannel->isDestroyed())
	{
		if(!this->reconnect())
		{
			return false;
		}
	}

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	
	(*pBundle).newMessage(InterfacesInterface::reqCreateAccount);
	(*pBundle) << pChannel->componentID();

	uint8 accountType = uatype;
	(*pBundle) << registerName << password << accountType;
	(*pBundle).appendBlob(datas);
	pInterfacesChannel->send(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_ThirdParty::onCreateAccountCB(KBEngine::MemoryStream& s)
{
	std::string registerName, accountName, password, postdatas, getdatas;
	COMPONENT_ID cid;
	SERVER_ERROR_CODE success = SERVER_ERR_OP_FAILED;

	s >> cid >> registerName >> accountName >> password >> success;
	s.readBlob(postdatas);
	s.readBlob(getdatas);

	if(success != SERVER_SUCCESS)
	{
		accountName = "";
	}

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(LOGINAPP_TYPE, cid);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG("InterfacesHandler_ThirdParty::onCreateAccountCB: loginapp not found!\n");
		return;
	}

	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(accountName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_ThirdParty::onCreateAccountCB: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	pThreadPool->addTask(new DBTaskCreateAccount(cinfos->pChannel->addr(),
		registerName, accountName, password, postdatas, getdatas));
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_ThirdParty::loginAccount(Network::Channel* pChannel, std::string& loginName, 
											 std::string& password, std::string& datas)
{
	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(g_kbeSrvConfig.interfacesAddr());
	KBE_ASSERT(pInterfacesChannel);

	if(pInterfacesChannel->isDestroyed())
	{
		if(!this->reconnect())
			return false;
	}

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();

	(*pBundle).newMessage(InterfacesInterface::onAccountLogin);
	(*pBundle) << pChannel->componentID();
	(*pBundle) << loginName << password;
	(*pBundle).appendBlob(datas);
	pInterfacesChannel->send(pBundle);
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

	if(success != SERVER_SUCCESS)
	{
		accountName = "";
	}

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(LOGINAPP_TYPE, cid);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG("InterfacesHandler_ThirdParty::onLoginAccountCB: loginapp not found!\n");
		return;
	}

	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(accountName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_ThirdParty::onLoginAccountCB: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	pThreadPool->addTask(new DBTaskAccountLogin(cinfos->pChannel->addr(),
		loginName, accountName, password, success, postdatas, getdatas));
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_ThirdParty::initialize()
{
	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(g_kbeSrvConfig.interfacesAddr());
	if(pInterfacesChannel)
		return true;
	return reconnect();
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_ThirdParty::reconnect()
{
	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(g_kbeSrvConfig.interfacesAddr());

	if(pInterfacesChannel)
	{
		if(!pInterfacesChannel->isDestroyed())
			Dbmgr::getSingleton().networkInterface().deregisterChannel(pInterfacesChannel);

		pInterfacesChannel->destroy();
		Network::Channel::ObjPool().reclaimObject(pInterfacesChannel);
	}

	Network::Address addr = g_kbeSrvConfig.interfacesAddr();
	Network::EndPoint* pEndPoint = Network::EndPoint::ObjPool().createObject();
	pEndPoint->addr(addr);

	pEndPoint->socket(SOCK_STREAM);
	if (!pEndPoint->good())
	{
		ERROR_MSG("InterfacesHandler_ThirdParty::initialize: couldn't create a socket\n");
		return true;
	}

	pEndPoint->setnonblocking(true);
	pEndPoint->setnodelay(true);

	pInterfacesChannel = Network::Channel::ObjPool().createObject();
	bool ret = pInterfacesChannel->initialize(Dbmgr::getSingleton().networkInterface(), pEndPoint, Network::Channel::INTERNAL);
	if(!ret)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_ThirdParty::initialize: initialize({}) is failed!\n",
			pInterfacesChannel->c_str()));

		pInterfacesChannel->destroy();
		Network::Channel::ObjPool().reclaimObject(pInterfacesChannel);
		return 0;
	}

	if(pInterfacesChannel->pEndPoint()->connect() == -1)
	{
		struct timeval tv = { 0, 300000 }; // 300ms
		fd_set	fds;
		FD_ZERO(&fds);
		FD_SET((int)(*pInterfacesChannel->pEndPoint()), &fds);

		bool connected = false;
		int selgot = select((*pInterfacesChannel->pEndPoint())+1, &fds, &fds, NULL, &tv);
		if(selgot > 0)
		{
			int error;
			socklen_t len = sizeof(error);
#if KBE_PLATFORM == PLATFORM_WIN32
			getsockopt(int(*pInterfacesChannel->pEndPoint()), SOL_SOCKET, SO_ERROR, (char*)&error, &len);
#else
			getsockopt(int(*pInterfacesChannel->pEndPoint()), SOL_SOCKET, SO_ERROR, &error, &len);
#endif
			if(0 == error)
				connected = true;
		}

		if(!connected)
		{
			ERROR_MSG(fmt::format("InterfacesHandler_ThirdParty::reconnect(): couldn't connect to:{}\n", 
				pInterfacesChannel->pEndPoint()->addr().c_str()));

			pInterfacesChannel->destroy();
			Network::Channel::ObjPool().reclaimObject(pInterfacesChannel);
			return false;
		}
	}

	// 不检查超时
	pInterfacesChannel->stopInactivityDetection();
	Dbmgr::getSingleton().networkInterface().registerChannel(pInterfacesChannel);
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
	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(g_kbeSrvConfig.interfacesAddr());
	KBE_ASSERT(pInterfacesChannel);

	if(pInterfacesChannel->isDestroyed())
	{
		if(!this->reconnect())
			return;
	}

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

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();

	(*pBundle).newMessage(InterfacesInterface::charge);
	(*pBundle) << pChannel->componentID();
	(*pBundle) << chargeID;
	(*pBundle) << dbid;
	(*pBundle).appendBlob(datas);
	(*pBundle) << cbid;
	pInterfacesChannel->send(pBundle);
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

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();

	(*pBundle).newMessage(BaseappInterface::onChargeCB);
	(*pBundle) << chargeID;
	(*pBundle) << dbid;
	(*pBundle).appendBlob(datas);
	(*pBundle) << cbid;
	(*pBundle) << retcode;

	cinfos->pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_ThirdParty::eraseClientReq(Network::Channel* pChannel, std::string& logkey)
{
	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(g_kbeSrvConfig.interfacesAddr());
	KBE_ASSERT(pInterfacesChannel);

	if(pInterfacesChannel->isDestroyed())
	{
		if(!this->reconnect())
			return;
	}

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();

	(*pBundle).newMessage(InterfacesInterface::eraseClientReq);
	(*pBundle) << logkey;
	pInterfacesChannel->send(pBundle);
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
