// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
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
	if(type.size() == 0 || type == "dbmgr")
	{
		return new InterfacesHandler_Dbmgr();
	}
	else if(type == "interfaces")
	{
		return new InterfacesHandler_Interfaces();
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
InterfacesHandler_Dbmgr::InterfacesHandler_Dbmgr() :
InterfacesHandler()
{
}

//-------------------------------------------------------------------------------------
InterfacesHandler_Dbmgr::~InterfacesHandler_Dbmgr()
{
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_Dbmgr::createAccount(Network::Channel* pChannel, std::string& registerName,
										  std::string& password, std::string& datas, ACCOUNT_TYPE uatype)
{
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(registerName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Dbmgr::createAccount: not found dbInterface({})!\n",
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
void InterfacesHandler_Dbmgr::onCreateAccountCB(KBEngine::MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_Dbmgr::loginAccount(Network::Channel* pChannel, std::string& loginName,
										 std::string& password, std::string& datas)
{
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(loginName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Dbmgr::loginAccount: not found dbInterface({})!\n",
			dbInterfaceName));

		return false;
	}

	pThreadPool->addTask(new DBTaskAccountLogin(pChannel->addr(),
		loginName, loginName, password, SERVER_SUCCESS, datas, datas, true));

	return true;
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Dbmgr::onLoginAccountCB(KBEngine::MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Dbmgr::charge(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	INFO_MSG("InterfacesHandler_Dbmgr::charge: no implement!\n");
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Dbmgr::onChargeCB(KBEngine::MemoryStream& s)
{
	INFO_MSG("InterfacesHandler_Dbmgr::onChargeCB: no implement!\n");
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Dbmgr::eraseClientReq(Network::Channel* pChannel, std::string& logkey)
{
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Dbmgr::accountActivate(Network::Channel* pChannel, std::string& scode)
{
	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();
	std::vector<DBInterfaceInfo>::iterator dbinfo_iter = dbcfg.dbInterfaceInfos.begin();
	for (; dbinfo_iter != dbcfg.dbInterfaceInfos.end(); ++dbinfo_iter)
	{
		std::string dbInterfaceName = dbinfo_iter->name;

		thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
		if (!pThreadPool)
		{
			ERROR_MSG(fmt::format("InterfacesHandler_Dbmgr::accountActivate: not found dbInterface({})!\n",
				dbInterfaceName));

			return;
		}

		pThreadPool->addTask(new DBTaskActivateAccount(pChannel->addr(), scode));
	}
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Dbmgr::accountReqResetPassword(Network::Channel* pChannel, std::string& accountName)
{
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(accountName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Dbmgr::accountReqResetPassword: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	pThreadPool->addTask(new DBTaskReqAccountResetPassword(pChannel->addr(), accountName));
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Dbmgr::accountResetPassword(Network::Channel* pChannel, std::string& accountName, std::string& newpassword, std::string& scode)
{
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(accountName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Dbmgr::accountResetPassword: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	pThreadPool->addTask(new DBTaskAccountResetPassword(pChannel->addr(), accountName, newpassword, scode));
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Dbmgr::accountReqBindMail(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName,
											   std::string& password, std::string& email)
{
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(accountName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Dbmgr::accountReqBindMail: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	pThreadPool->addTask(new DBTaskReqAccountBindEmail(pChannel->addr(), entityID, accountName, password, email));
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Dbmgr::accountBindMail(Network::Channel* pChannel, std::string& username, std::string& scode)
{
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(username);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Dbmgr::accountBindMail: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	pThreadPool->addTask(new DBTaskAccountBindEmail(pChannel->addr(), username, scode));
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Dbmgr::accountNewPassword(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName,
											   std::string& password, std::string& newpassword)
{
	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(accountName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Dbmgr::accountNewPassword: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	pThreadPool->addTask(new DBTaskAccountNewPassword(pChannel->addr(), entityID, accountName, password, newpassword));
}

//-------------------------------------------------------------------------------------
InterfacesHandler_Interfaces::InterfacesHandler_Interfaces() :
InterfacesHandler_Dbmgr(),
addr_()
{
}

//-------------------------------------------------------------------------------------
InterfacesHandler_Interfaces::~InterfacesHandler_Interfaces()
{
	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(addr_);
	if(pInterfacesChannel)
	{
		pInterfacesChannel->condemn("");
	}

	pInterfacesChannel = NULL;
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_Interfaces::createAccount(Network::Channel* pChannel, std::string& registerName,
											  std::string& password, std::string& datas, ACCOUNT_TYPE uatype)
{
	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(addr_);

	if (!pInterfacesChannel || pInterfacesChannel->isDestroyed())
	{
		if (!this->reconnect())
		{
			return false;
		}

		pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(addr_);
	}

	KBE_ASSERT(pInterfacesChannel);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	
	(*pBundle).newMessage(InterfacesInterface::reqCreateAccount);
	(*pBundle) << pChannel->componentID();

	uint8 accountType = uatype;
	(*pBundle) << registerName << password << accountType;
	(*pBundle).appendBlob(datas);
	pInterfacesChannel->send(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Interfaces::onCreateAccountCB(KBEngine::MemoryStream& s)
{
	std::string registerName, accountName, password, postdatas, getdatas;
	COMPONENT_ID cid;
	SERVER_ERROR_CODE success = SERVER_ERR_OP_FAILED;

	s >> cid >> registerName >> accountName >> password >> success;
	s.readBlob(postdatas);
	s.readBlob(getdatas);

	if (success != SERVER_SUCCESS && success != SERVER_ERR_LOCAL_PROCESSING)
		accountName = "";

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(LOGINAPP_TYPE, cid);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG("InterfacesHandler_Interfaces::onCreateAccountCB: loginapp not found!\n");
		return;
	}

	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(accountName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Interfaces::onCreateAccountCB: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	if (success == SERVER_ERR_LOCAL_PROCESSING)
	{
		ACCOUNT_TYPE type = ACCOUNT_TYPE(g_kbeSrvConfig.getLoginApp().account_type);
		if (type == ACCOUNT_TYPE_SMART)
		{
			if (email_isvalid(accountName.c_str()))
			{
				pThreadPool->addTask(new DBTaskCreateMailAccount(cinfos->pChannel->addr(),
					registerName, accountName, password, postdatas, getdatas));

				return;
			}
			
		}
		else if (type == ACCOUNT_TYPE_MAIL)
		{
			if (!email_isvalid(accountName.c_str()))
			{
				WARNING_MSG(fmt::format("InterfacesHandler_Interfaces::onCreateAccountCB: invalid email={}\n",
					accountName));

				accountName = "";
			}
			else
			{
				pThreadPool->addTask(new DBTaskCreateMailAccount(cinfos->pChannel->addr(),
					registerName, accountName, password, postdatas, getdatas));

				return;
			}
		}
	}

	pThreadPool->addTask(new DBTaskCreateAccount(cinfos->pChannel->addr(),
		registerName, accountName, password, postdatas, getdatas));
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_Interfaces::loginAccount(Network::Channel* pChannel, std::string& loginName,
											 std::string& password, std::string& datas)
{
	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(addr_);

	if (!pInterfacesChannel || pInterfacesChannel->isDestroyed())
	{
		if (!this->reconnect())
		{
			return false;
		}

		pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(addr_);
	}

	KBE_ASSERT(pInterfacesChannel);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	(*pBundle).newMessage(InterfacesInterface::onAccountLogin);
	(*pBundle) << pChannel->componentID();
	(*pBundle) << loginName << password;
	(*pBundle).appendBlob(datas);
	pInterfacesChannel->send(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Interfaces::onLoginAccountCB(KBEngine::MemoryStream& s)
{
	std::string loginName, accountName, password, postdatas, getdatas;
	COMPONENT_ID cid;
	SERVER_ERROR_CODE success = SERVER_ERR_OP_FAILED;

	s >> cid >> loginName >> accountName >> password >> success;
	s.readBlob(postdatas);
	s.readBlob(getdatas);

	bool needCheckPassword = (success == SERVER_ERR_LOCAL_PROCESSING);

	if (success != SERVER_SUCCESS && success != SERVER_ERR_LOCAL_PROCESSING)
		accountName = "";
	else
		success = SERVER_SUCCESS;

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(LOGINAPP_TYPE, cid);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		ERROR_MSG("InterfacesHandler_Interfaces::onLoginAccountCB: loginapp not found!\n");
		return;
	}

	std::string dbInterfaceName = Dbmgr::getSingleton().selectAccountDBInterfaceName(accountName);

	thread::ThreadPool* pThreadPool = DBUtil::pThreadPool(dbInterfaceName);
	if (!pThreadPool)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Interfaces::onLoginAccountCB: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	pThreadPool->addTask(new DBTaskAccountLogin(cinfos->pChannel->addr(),
		loginName, accountName, password, success, postdatas, getdatas, needCheckPassword));
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_Interfaces::initialize()
{
	KBE_ASSERT(addr_ != Network::Address::NONE);

	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(addr_);
	if(pInterfacesChannel)
		return true;

	return reconnect();
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_Interfaces::reconnect()
{
	KBE_ASSERT(addr_ != Network::Address::NONE);

	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(addr_);

	if(pInterfacesChannel)
	{
		if(!pInterfacesChannel->isDestroyed())
			Dbmgr::getSingleton().networkInterface().deregisterChannel(pInterfacesChannel);

		pInterfacesChannel->destroy();
		Network::Channel::reclaimPoolObject(pInterfacesChannel);
	}

	Network::Address addr = addr_;
	Network::EndPoint* pEndPoint = Network::EndPoint::createPoolObject(OBJECTPOOL_POINT);
	pEndPoint->addr(addr);

	pEndPoint->socket(SOCK_STREAM);
	if (!pEndPoint->good())
	{
		ERROR_MSG("InterfacesHandler_Interfaces::initialize: couldn't create a socket\n");
		return true;
	}

	pEndPoint->setnonblocking(true);
	pEndPoint->setnodelay(true);

	pInterfacesChannel = Network::Channel::createPoolObject(OBJECTPOOL_POINT);
	bool ret = pInterfacesChannel->initialize(Dbmgr::getSingleton().networkInterface(), pEndPoint, Network::Channel::INTERNAL);
	if(!ret)
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Interfaces::initialize: initialize({}) is failed!\n",
			pInterfacesChannel->c_str()));

		pInterfacesChannel->destroy();
		Network::Channel::reclaimPoolObject(pInterfacesChannel);
		return 0;
	}

	if(pInterfacesChannel->pEndPoint()->connect() == -1)
	{
		struct timeval tv = { 0, 2000000 }; // 1000ms
		fd_set frds, fwds;
		FD_ZERO( &frds );
		FD_ZERO( &fwds );
		FD_SET((int)(*pInterfacesChannel->pEndPoint()), &frds);
		FD_SET((int)(*pInterfacesChannel->pEndPoint()), &fwds);
		
		bool connected = false;
		int selgot = select((*pInterfacesChannel->pEndPoint())+1, &frds, &fwds, NULL, &tv);
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
			ERROR_MSG(fmt::format("InterfacesHandler_Interfaces::reconnect(): couldn't connect to(interfaces server): {}! Check kbengine[_defs].xml->interfaces->host and interfaces.*.log\n", 
				pInterfacesChannel->pEndPoint()->addr().c_str()));

			pInterfacesChannel->destroy();
			Network::Channel::reclaimPoolObject(pInterfacesChannel);
			return false;
		}
	}

	// 不检查超时
	pInterfacesChannel->stopInactivityDetection();

	if (!Dbmgr::getSingleton().networkInterface().registerChannel(pInterfacesChannel))
		return false;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(InterfacesInterface::onRegisterNewApp);

	InterfacesInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(),
		g_componentType, g_componentID,
		g_componentGlobalOrder, g_componentGroupOrder,
		Dbmgr::getSingleton().networkInterface().intTcpAddr().ip, Dbmgr::getSingleton().networkInterface().intTcpAddr().port,
		Dbmgr::getSingleton().networkInterface().extTcpAddr().ip, Dbmgr::getSingleton().networkInterface().extTcpAddr().port, g_kbeSrvConfig.getConfig().externalAddress);

	pInterfacesChannel->send(pBundle);

	return true;
}

//-------------------------------------------------------------------------------------
bool InterfacesHandler_Interfaces::process()
{
	return true;
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Interfaces::charge(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(addr_);

	if (!pInterfacesChannel || pInterfacesChannel->isDestroyed())
	{
		if (!this->reconnect())
		{
			return;
		}

		pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(addr_);
	}

	KBE_ASSERT(pInterfacesChannel);

	std::string chargeID;
	std::string datas;
	CALLBACK_ID cbid;
	DBID dbid;

	s >> chargeID;
	s >> dbid;
	s.readBlob(datas);
	s >> cbid;

	INFO_MSG(fmt::format("InterfacesHandler_Interfaces::charge: chargeID={0}, dbid={3}, cbid={1}, datas={2}!\n",
		chargeID, cbid, datas, dbid));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	(*pBundle).newMessage(InterfacesInterface::charge);
	(*pBundle) << pChannel->componentID();
	(*pBundle) << chargeID;
	(*pBundle) << dbid;
	(*pBundle).appendBlob(datas);
	(*pBundle) << cbid;
	pInterfacesChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Interfaces::onChargeCB(KBEngine::MemoryStream& s)
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

	INFO_MSG(fmt::format("InterfacesHandler_Interfaces::onChargeCB: chargeID={0}, dbid={3}, cbid={1}, cid={4}, datas={2}!\n",
		chargeID, cbid, datas, dbid, cid));

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, cid);
	if (cid == 0 || cinfos == NULL || cinfos->pChannel == NULL || cinfos->pChannel->isDestroyed())
	{
		ERROR_MSG(fmt::format("InterfacesHandler_Interfaces::onChargeCB: baseapp not found!, chargeID={}, cid={}.\n", 
			chargeID, cid));

		// 此时应该随机找一个baseapp调用onLoseChargeCB
		bool found = false;

		Components::COMPONENTS& components = Components::getSingleton().getComponents(BASEAPP_TYPE);
		for (Components::COMPONENTS::iterator iter = components.begin(); iter != components.end(); ++iter)
		{
			cinfos = &(*iter);
			if (cinfos == NULL || cinfos->pChannel == NULL || cinfos->pChannel->isDestroyed())
			{
				continue;
			}

			WARNING_MSG(fmt::format("InterfacesHandler_Interfaces::onChargeCB: , chargeID={}, not found cid={}, forward to component({}) processing!\n",
				chargeID, cid, cinfos->cid));

			found = true;
			break;
		}

		if (!found)
			return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	(*pBundle).newMessage(BaseappInterface::onChargeCB);
	(*pBundle) << chargeID;
	(*pBundle) << dbid;
	(*pBundle).appendBlob(datas);
	(*pBundle) << cbid;
	(*pBundle) << retcode;

	cinfos->pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Interfaces::eraseClientReq(Network::Channel* pChannel, std::string& logkey)
{
	Network::Channel* pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(addr_);

	if (!pInterfacesChannel || pInterfacesChannel->isDestroyed())
	{
		if (!this->reconnect())
		{
			return;
		}

		pInterfacesChannel = Dbmgr::getSingleton().networkInterface().findChannel(addr_);
	}

	KBE_ASSERT(pInterfacesChannel);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	(*pBundle).newMessage(InterfacesInterface::eraseClientReq);
	(*pBundle) << logkey;
	pInterfacesChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Interfaces::accountActivate(Network::Channel* pChannel, std::string& scode)
{
	// 该功能不支持第三方系统，所以当做本地账号系统执行
	InterfacesHandler_Dbmgr::accountActivate(pChannel, scode);
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Interfaces::accountReqResetPassword(Network::Channel* pChannel, std::string& accountName)
{
	// 该功能不支持第三方系统，所以当做本地账号系统执行
	InterfacesHandler_Dbmgr::accountReqResetPassword(pChannel, accountName);
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Interfaces::accountResetPassword(Network::Channel* pChannel, std::string& accountName, std::string& newpassword, std::string& scode)
{
	// 该功能不支持第三方系统，所以当做本地账号系统执行
	InterfacesHandler_Dbmgr::accountResetPassword(pChannel, accountName, newpassword, scode);
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Interfaces::accountReqBindMail(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName,
												   std::string& password, std::string& email)
{
	// 该功能不支持第三方系统，所以当做本地账号系统执行
	InterfacesHandler_Dbmgr::accountReqBindMail(pChannel, entityID, accountName, password, email);
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Interfaces::accountBindMail(Network::Channel* pChannel, std::string& username, std::string& scode)
{
	// 该功能不支持第三方系统，所以当做本地账号系统执行
	InterfacesHandler_Dbmgr::accountBindMail(pChannel, username, scode);
}

//-------------------------------------------------------------------------------------
void InterfacesHandler_Interfaces::accountNewPassword(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName,
												   std::string& password, std::string& newpassword)
{
	// 该功能不支持第三方系统，所以当做本地账号系统执行
	InterfacesHandler_Dbmgr::accountNewPassword(pChannel, entityID, accountName, password, newpassword);
}

//-------------------------------------------------------------------------------------

}
