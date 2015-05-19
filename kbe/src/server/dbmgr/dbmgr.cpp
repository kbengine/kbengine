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
#include "dbmgr_interface.h"
#include "dbtasks.h"
#include "interfaces_handler.h"
#include "sync_app_datas_handler.h"
#include "db_mysql/kbe_table_mysql.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "thread/threadpool.h"
#include "server/components.h"
#include "db_interface/db_interface.h"
#include "db_mysql/db_interface_mysql.h"
#include "entitydef/scriptdef_module.h"

#include "baseapp/baseapp_interface.h"
#include "cellapp/cellapp_interface.h"
#include "baseappmgr/baseappmgr_interface.h"
#include "cellappmgr/cellappmgr_interface.h"
#include "loginapp/loginapp_interface.h"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Dbmgr);

//-------------------------------------------------------------------------------------
Dbmgr::Dbmgr(Network::EventDispatcher& dispatcher, 
			 Network::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID),
	loopCheckTimerHandle_(),
	mainProcessTimer_(),
	idServer_(1, 1024),
	pGlobalData_(NULL),
	pBaseAppData_(NULL),
	pCellAppData_(NULL),
	bufferedDBTasks_(),
	numWrittenEntity_(0),
	numRemovedEntity_(0),
	numQueryEntity_(0),
	numExecuteRawDatabaseCommand_(0),
	numCreatedAccount_(0),
	pInterfacesAccountHandler_(NULL),
	pInterfacesChargeHandler_(NULL),
	pSyncAppDatasHandler_(NULL)
{
}

//-------------------------------------------------------------------------------------
Dbmgr::~Dbmgr()
{
	loopCheckTimerHandle_.cancel();
	mainProcessTimer_.cancel();
	KBEngine::sleep(300);

	SAFE_RELEASE(pInterfacesAccountHandler_);
	SAFE_RELEASE(pInterfacesChargeHandler_);
}

//-------------------------------------------------------------------------------------
bool Dbmgr::canShutdown()
{
	if(bufferedDBTasks_.size() > 0)
	{
		INFO_MSG(fmt::format("Dbmgr::canShutdown(): Wait for the task to complete, tasks={}, threads={}, threadpoolDestroyed={}!\n", 
			bufferedDBTasks_.size(), DBUtil::pThreadPool()->currentThreadCount(), DBUtil::pThreadPool()->isDestroyed()));

		return false;
	}

	Components::COMPONENTS& cellapp_components = Components::getSingleton().getComponents(CELLAPP_TYPE);
	if(cellapp_components.size() > 0)
	{
		std::string s;
		for(size_t i=0; i<cellapp_components.size(); ++i)
		{
			s += fmt::format("{}, ", cellapp_components[i].cid);
		}

		INFO_MSG(fmt::format("Dbmgr::canShutdown(): Waiting for cellapp[{}] destruction!\n", 
			s));

		return false;
	}

	Components::COMPONENTS& baseapp_components = Components::getSingleton().getComponents(BASEAPP_TYPE);
	if(baseapp_components.size() > 0)
	{
		std::string s;
		for(size_t i=0; i<baseapp_components.size(); ++i)
		{
			s += fmt::format("{}, ", baseapp_components[i].cid);
		}

		INFO_MSG(fmt::format("Dbmgr::canShutdown(): Waiting for baseapp[{}] destruction!\n", 
			s));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Dbmgr::initializeWatcher()
{
	WATCH_OBJECT("numWrittenEntity", numWrittenEntity_);
	WATCH_OBJECT("numRemovedEntity", numRemovedEntity_);
	WATCH_OBJECT("numQueryEntity", numQueryEntity_);
	WATCH_OBJECT("numExecuteRawDatabaseCommand", numExecuteRawDatabaseCommand_);
	WATCH_OBJECT("numCreatedAccount", numCreatedAccount_);

	WATCH_OBJECT("DBThreadPool/dbid_tasksSize", &bufferedDBTasks_, &Buffered_DBTasks::dbid_tasksSize);
	WATCH_OBJECT("DBThreadPool/entityid_tasksSize", &bufferedDBTasks_, &Buffered_DBTasks::entityid_tasksSize);
	WATCH_OBJECT("DBThreadPool/printBuffered_dbid", &bufferedDBTasks_, &Buffered_DBTasks::printBuffered_dbid);
	WATCH_OBJECT("DBThreadPool/printBuffered_entityID", &bufferedDBTasks_, &Buffered_DBTasks::printBuffered_entityID);

	return ServerApp::initializeWatcher() && DBUtil::pThreadPool()->initializeWatcher();
}

//-------------------------------------------------------------------------------------
bool Dbmgr::run()
{
	return ServerApp::run();
}

//-------------------------------------------------------------------------------------
void Dbmgr::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_TICK:
			this->handleMainTick();
			break;
		case TIMEOUT_CHECK_STATUS:
			this->handleCheckStatusTick();
			break;
		default:
			break;
	}

	ServerApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
void Dbmgr::handleMainTick()
{
	 //time_t t = ::time(NULL);
	 //DEBUG_MSG("Dbmgr::handleGameTick[%"PRTime"]:%u\n", t, time_);
	
	g_kbetime++;
	threadPool_.onMainThreadTick();
	DBUtil::pThreadPool()->onMainThreadTick();
	networkInterface().processChannels(&DbmgrInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
void Dbmgr::handleCheckStatusTick()
{
}

//-------------------------------------------------------------------------------------
bool Dbmgr::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Dbmgr::inInitialize()
{
	// 初始化所有扩展模块
	// assets/scripts/
	std::vector<PyTypeObject*>	scriptBaseTypes;
	if(!EntityDef::initialize(scriptBaseTypes, componentType_)){
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Dbmgr::initializeEnd()
{
	// 添加一个timer， 每秒检查一些状态
	loopCheckTimerHandle_ = this->dispatcher().addTimer(1000000, this,
							reinterpret_cast<void *>(TIMEOUT_CHECK_STATUS));

	mainProcessTimer_ = this->dispatcher().addTimer(1000000 / 50, this,
							reinterpret_cast<void *>(TIMEOUT_TICK));

	// 添加globalData, baseAppData, cellAppData支持
	pGlobalData_ = new GlobalDataServer(GlobalDataServer::GLOBAL_DATA);
	pBaseAppData_ = new GlobalDataServer(GlobalDataServer::BASEAPP_DATA);
	pCellAppData_ = new GlobalDataServer(GlobalDataServer::CELLAPP_DATA);
	pGlobalData_->addConcernComponentType(CELLAPP_TYPE);
	pGlobalData_->addConcernComponentType(BASEAPP_TYPE);
	pBaseAppData_->addConcernComponentType(BASEAPP_TYPE);
	pCellAppData_->addConcernComponentType(CELLAPP_TYPE);

	INFO_MSG(fmt::format("Dbmgr::initializeEnd: digest({})\n", 
		EntityDef::md5().getDigestStr()));
	
	return initInterfacesHandler() && initDB();
}

//-------------------------------------------------------------------------------------		
bool Dbmgr::initInterfacesHandler()
{
	pInterfacesAccountHandler_ = InterfacesHandlerFactory::create(g_kbeSrvConfig.interfacesAccountType(), threadPool(), *static_cast<KBEngine::DBThreadPool*>(DBUtil::pThreadPool()));
	pInterfacesChargeHandler_ = InterfacesHandlerFactory::create(g_kbeSrvConfig.interfacesChargeType(), threadPool(), *static_cast<KBEngine::DBThreadPool*>(DBUtil::pThreadPool()));

	INFO_MSG(fmt::format("Dbmgr::initInterfacesHandler: interfaces addr({}), accountType:({}), chargeType:({}).\n", 
		g_kbeSrvConfig.interfacesAddr().c_str(),
		g_kbeSrvConfig.interfacesAccountType(),
		g_kbeSrvConfig.interfacesChargeType()));

	if(strlen(g_kbeSrvConfig.interfacesThirdpartyAccountServiceAddr()) > 0)
	{
		INFO_MSG(fmt::format("Dbmgr::initInterfacesHandler: thirdpartyAccountService_addr({}:{}).\n", 
			g_kbeSrvConfig.interfacesThirdpartyAccountServiceAddr(),
			g_kbeSrvConfig.interfacesThirdpartyAccountServicePort()));
	}

	if(strlen(g_kbeSrvConfig.interfacesThirdpartyChargeServiceAddr()) > 0)
	{
		INFO_MSG(fmt::format("Dbmgr::initInterfacesHandler: thirdpartyChargeService_addr({}:{}).\n", 
			g_kbeSrvConfig.interfacesThirdpartyChargeServiceAddr(),
			g_kbeSrvConfig.interfacesThirdpartyChargeServicePort()));
	}

	return pInterfacesAccountHandler_->initialize() && pInterfacesChargeHandler_->initialize();
}

//-------------------------------------------------------------------------------------		
bool Dbmgr::initDB()
{
	ScriptDefModule* pModule = EntityDef::findScriptModule(DBUtil::accountScriptName());
	if(pModule == NULL)
	{
		ERROR_MSG(fmt::format("Dbmgr::initDB(): not found account script[{}]!\n", 
			DBUtil::accountScriptName()));

		return false;
	}

	if(!DBUtil::initialize())
	{
		ERROR_MSG("Dbmgr::initDB(): can't initialize dbinterface!\n");
		return false;
	}

	DBInterface* pDBInterface = DBUtil::createInterface();
	if(pDBInterface == NULL)
	{
		ERROR_MSG("Dbmgr::initDB(): can't create dbinterface!\n");
		return false;
	}

	bool ret = DBUtil::initInterface(pDBInterface);
	pDBInterface->detach();
	SAFE_RELEASE(pDBInterface);

	if(!ret)
		return false;

	return ret;
}

//-------------------------------------------------------------------------------------
void Dbmgr::finalise()
{
	SAFE_RELEASE(pGlobalData_);
	SAFE_RELEASE(pBaseAppData_);
	SAFE_RELEASE(pCellAppData_);

	DBUtil::finalise();
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------
void Dbmgr::onReqAllocEntityID(Network::Channel* pChannel, COMPONENT_ORDER componentType, COMPONENT_ID componentID)
{
	KBEngine::COMPONENT_TYPE ct = static_cast<KBEngine::COMPONENT_TYPE>(componentType);

	// 获取一个id段 并传输给IDClient
	std::pair<ENTITY_ID, ENTITY_ID> idRange = idServer_.allocRange();
	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();

	if(ct == BASEAPP_TYPE)
		(*pBundle).newMessage(BaseappInterface::onReqAllocEntityID);
	else	
		(*pBundle).newMessage(CellappInterface::onReqAllocEntityID);

	(*pBundle) << idRange.first;
	(*pBundle) << idRange.second;
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Dbmgr::onRegisterNewApp(Network::Channel* pChannel, int32 uid, std::string& username, 
						COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER globalorderID, COMPONENT_ORDER grouporderID,
						uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx)
{
	if(pChannel->isExternal())
		return;

	ServerApp::onRegisterNewApp(pChannel, uid, username, componentType, componentID, globalorderID, grouporderID,
						intaddr, intport, extaddr, extport, extaddrEx);

	KBEngine::COMPONENT_TYPE tcomponentType = (KBEngine::COMPONENT_TYPE)componentType;
	
	COMPONENT_ORDER startGroupOrder = 1;
	COMPONENT_ORDER startGlobalOrder = Components::getSingleton().getGlobalOrderLog()[getUserUID()];

	if(grouporderID > 0)
		startGroupOrder = grouporderID;

	if(globalorderID > 0)
		startGlobalOrder = globalorderID;

	if(pSyncAppDatasHandler_ == NULL)
		pSyncAppDatasHandler_ = new SyncAppDatasHandler(this->networkInterface());

	// 下一步:
	// 如果是连接到dbmgr则需要等待接收app初始信息
	// 例如：初始会分配entityID段以及这个app启动的顺序信息（是否第一个baseapp启动）
	if(tcomponentType == BASEAPP_TYPE || 
		tcomponentType == CELLAPP_TYPE || 
		tcomponentType == LOGINAPP_TYPE)
	{
		switch(tcomponentType)
		{
		case BASEAPP_TYPE:
			{
				if(grouporderID <= 0)
					startGroupOrder = Components::getSingleton().getBaseappGroupOrderLog()[getUserUID()];
			}
			break;
		case CELLAPP_TYPE:
			{
				if(grouporderID <= 0)
					startGroupOrder = Components::getSingleton().getCellappGroupOrderLog()[getUserUID()];
			}
			break;
		case LOGINAPP_TYPE:
			if(grouporderID <= 0)
				startGroupOrder = Components::getSingleton().getLoginappGroupOrderLog()[getUserUID()];

			break;
		default:
			break;
		}
	}

	pSyncAppDatasHandler_->pushApp(componentID, startGroupOrder, startGlobalOrder);

	// 如果是baseapp或者cellapp则将自己注册到所有其他baseapp和cellapp
	if(tcomponentType == BASEAPP_TYPE || 
		tcomponentType == CELLAPP_TYPE)
	{
		KBEngine::COMPONENT_TYPE broadcastCpTypes[2] = {BASEAPP_TYPE, CELLAPP_TYPE};
		for(int idx = 0; idx < 2; ++idx)
		{
			Components::COMPONENTS& cts = Components::getSingleton().getComponents(broadcastCpTypes[idx]);
			Components::COMPONENTS::iterator fiter = cts.begin();
			for(; fiter != cts.end(); ++fiter)
			{
				if((*fiter).cid == componentID)
					continue;

				Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
				ENTITTAPP_COMMON_NETWORK_MESSAGE(broadcastCpTypes[idx], (*pBundle), onGetEntityAppFromDbmgr);
				
				if(tcomponentType == BASEAPP_TYPE)
				{
					BaseappInterface::onGetEntityAppFromDbmgrArgs11::staticAddToBundle((*pBundle), 
						uid, username, componentType, componentID, startGlobalOrder, startGroupOrder,
							intaddr, intport, extaddr, extport, g_kbeSrvConfig.getConfig().externalAddress);
				}
				else
				{
					CellappInterface::onGetEntityAppFromDbmgrArgs11::staticAddToBundle((*pBundle), 
						uid, username, componentType, componentID, startGlobalOrder, startGroupOrder,
							intaddr, intport, extaddr, extport, g_kbeSrvConfig.getConfig().externalAddress);
				}
				
				KBE_ASSERT((*fiter).pChannel != NULL);
				(*fiter).pChannel->send(pBundle);
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Dbmgr::onGlobalDataClientLogon(Network::Channel* pChannel, COMPONENT_TYPE componentType)
{
	if(BASEAPP_TYPE == componentType)
	{
		pBaseAppData_->onGlobalDataClientLogon(pChannel, componentType);
		pGlobalData_->onGlobalDataClientLogon(pChannel, componentType);
	}
	else if(CELLAPP_TYPE == componentType)
	{
		pGlobalData_->onGlobalDataClientLogon(pChannel, componentType);
		pCellAppData_->onGlobalDataClientLogon(pChannel, componentType);
	}
	else
	{
		ERROR_MSG(fmt::format("Dbmgr::onGlobalDataClientLogon: nonsupport {}!\n",
			COMPONENT_NAME_EX(componentType)));
	}
}

//-------------------------------------------------------------------------------------
void Dbmgr::onBroadcastGlobalDataChanged(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	uint8 dataType;
	std::string key, value;
	bool isDelete;
	COMPONENT_TYPE componentType;
	
	s >> dataType;
	s >> isDelete;

	s.readBlob(key);

	if(!isDelete)
	{
		s.readBlob(value);
	}

	s >> componentType;

	switch(dataType)
	{
	case GlobalDataServer::GLOBAL_DATA:
		if(isDelete)
			pGlobalData_->del(pChannel, componentType, key);
		else
			pGlobalData_->write(pChannel, componentType, key, value);
		break;
	case GlobalDataServer::BASEAPP_DATA:
		if(isDelete)
			pBaseAppData_->del(pChannel, componentType, key);
		else
			pBaseAppData_->write(pChannel, componentType, key, value);
		break;
	case GlobalDataServer::CELLAPP_DATA:
		if(isDelete)
			pCellAppData_->del(pChannel, componentType, key);
		else
			pCellAppData_->write(pChannel, componentType, key, value);
		break;
	default:
		KBE_ASSERT(false && "dataType is error!\n");
		break;
	};
}

//-------------------------------------------------------------------------------------
void Dbmgr::reqCreateAccount(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string registerName, password, datas;
	uint8 uatype = 0;

	s >> registerName >> password >> uatype;
	s.readBlob(datas);

	if(registerName.size() == 0)
	{
		ERROR_MSG("Dbmgr::reqCreateAccount: registerName is empty.\n");
		return;
	}

	pInterfacesAccountHandler_->createAccount(pChannel, registerName, password, datas, ACCOUNT_TYPE(uatype));
	numCreatedAccount_++;
}

//-------------------------------------------------------------------------------------
void Dbmgr::onCreateAccountCBFromInterfaces(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	pInterfacesAccountHandler_->onCreateAccountCB(s);
}

//-------------------------------------------------------------------------------------
void Dbmgr::onAccountLogin(Network::Channel* pChannel, KBEngine::MemoryStream& s) 
{
	std::string loginName, password, datas;
	s >> loginName >> password;
	s.readBlob(datas);

	if(loginName.size() == 0)
	{
		ERROR_MSG("Dbmgr::onAccountLogin: loginName is empty.\n");
		return;
	}

	pInterfacesAccountHandler_->loginAccount(pChannel, loginName, password, datas);
}

//-------------------------------------------------------------------------------------
void Dbmgr::onLoginAccountCBBFromInterfaces(Network::Channel* pChannel, KBEngine::MemoryStream& s) 
{
	pInterfacesAccountHandler_->onLoginAccountCB(s);
}

//-------------------------------------------------------------------------------------
void Dbmgr::queryAccount(Network::Channel* pChannel, 
						 std::string& accountName, 
						 std::string& password,
						 COMPONENT_ID componentID,
						 ENTITY_ID entityID,
						 DBID entityDBID, 
						 uint32 ip, 
						 uint16 port)
{
	if(accountName.size() == 0)
	{
		ERROR_MSG("Dbmgr::queryAccount: accountName is empty.\n");
		return;
	}

	bufferedDBTasks_.addTask(new DBTaskQueryAccount(pChannel->addr(), 
		accountName, password, componentID, entityID, entityDBID, ip, port));

	numQueryEntity_++;
}

//-------------------------------------------------------------------------------------
void Dbmgr::onAccountOnline(Network::Channel* pChannel, 
							std::string& accountName, 
							COMPONENT_ID componentID, 
							ENTITY_ID entityID)
{
	// bufferedDBTasks_.addTask(new DBTaskAccountOnline(pChannel->addr(), 
	//	accountName, componentID, entityID));
}

//-------------------------------------------------------------------------------------
void Dbmgr::onEntityOffline(Network::Channel* pChannel, DBID dbid, ENTITY_SCRIPT_UID sid)
{
	bufferedDBTasks_.addTask(new DBTaskEntityOffline(pChannel->addr(), dbid, sid));
}

//-------------------------------------------------------------------------------------
void Dbmgr::executeRawDatabaseCommand(Network::Channel* pChannel, 
									  KBEngine::MemoryStream& s)
{
	ENTITY_ID entityID = -1;
	s >> entityID;

	if(entityID == -1)
		DBUtil::pThreadPool()->addTask(new DBTaskExecuteRawDatabaseCommand(pChannel->addr(), s));
	else
		bufferedDBTasks_.addTask(new DBTaskExecuteRawDatabaseCommandByEntity(pChannel->addr(), s, entityID));

	s.done();

	numExecuteRawDatabaseCommand_++;
}

//-------------------------------------------------------------------------------------
void Dbmgr::writeEntity(Network::Channel* pChannel, 
						KBEngine::MemoryStream& s)
{
	ENTITY_ID eid;
	DBID entityDBID;
	COMPONENT_ID componentID;

	s >> componentID >> eid >> entityDBID;

	bufferedDBTasks_.addTask(new DBTaskWriteEntity(pChannel->addr(), componentID, eid, entityDBID, s));
	s.done();

	numWrittenEntity_++;
}

//-------------------------------------------------------------------------------------
void Dbmgr::removeEntity(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID eid;
	DBID entityDBID;
	COMPONENT_ID componentID;

	s >> componentID >> eid >> entityDBID;
	KBE_ASSERT(entityDBID > 0);

	bufferedDBTasks_.addTask(new DBTaskRemoveEntity(pChannel->addr(), 
		componentID, eid, entityDBID, s));

	s.done();

	numRemovedEntity_++;
}

//-------------------------------------------------------------------------------------
void Dbmgr::entityAutoLoad(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	COMPONENT_ID componentID;
	ENTITY_SCRIPT_UID entityType;
	ENTITY_ID start;
	ENTITY_ID end;

	s >> componentID >> entityType >> start >> end;

	DBUtil::pThreadPool()->addTask(new DBTaskEntityAutoLoad(pChannel->addr(), 
		componentID, entityType, start, end));
}

//-------------------------------------------------------------------------------------
void Dbmgr::deleteBaseByDBID(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	COMPONENT_ID componentID;
	ENTITY_SCRIPT_UID sid;
	CALLBACK_ID callbackID = 0;
	DBID entityDBID;

	s >> componentID >> entityDBID >> callbackID >> sid;
	KBE_ASSERT(entityDBID > 0);

	DBUtil::pThreadPool()->addTask(new DBTaskDeleteBaseByDBID(pChannel->addr(), 
		componentID, entityDBID, callbackID, sid));
}

//-------------------------------------------------------------------------------------
void Dbmgr::lookUpBaseByDBID(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	COMPONENT_ID componentID;
	ENTITY_SCRIPT_UID sid;
	CALLBACK_ID callbackID = 0;
	DBID entityDBID;

	s >> componentID >> entityDBID >> callbackID >> sid;
	KBE_ASSERT(entityDBID > 0);

	DBUtil::pThreadPool()->addTask(new DBTaskLookUpBaseByDBID(pChannel->addr(), 
		componentID, entityDBID, callbackID, sid));
}

//-------------------------------------------------------------------------------------
void Dbmgr::queryEntity(Network::Channel* pChannel, COMPONENT_ID componentID, int8 queryMode, DBID dbid, 
	std::string& entityType, CALLBACK_ID callbackID, ENTITY_ID entityID)
{
	bufferedDBTasks_.addTask(new DBTaskQueryEntity(pChannel->addr(), queryMode, entityType, 
		dbid, componentID, callbackID, entityID));

	numQueryEntity_++;
}

//-------------------------------------------------------------------------------------
void Dbmgr::syncEntityStreamTemplate(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	KBEAccountTable* pTable = 
		static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));

	KBE_ASSERT(pTable);

	pTable->accountDefMemoryStream(s);
	s.done();
}

//-------------------------------------------------------------------------------------
void Dbmgr::charge(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	pInterfacesChargeHandler_->charge(pChannel, s);
}

//-------------------------------------------------------------------------------------
void Dbmgr::onChargeCB(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	pInterfacesChargeHandler_->onChargeCB(s);
}

//-------------------------------------------------------------------------------------
void Dbmgr::eraseClientReq(Network::Channel* pChannel, std::string& logkey)
{
	pInterfacesAccountHandler_->eraseClientReq(pChannel, logkey);
}

//-------------------------------------------------------------------------------------
void Dbmgr::accountActivate(Network::Channel* pChannel, std::string& scode)
{
	INFO_MSG(fmt::format("Dbmgr::accountActivate: code={}.\n", scode));
	pInterfacesAccountHandler_->accountActivate(pChannel, scode);
}

//-------------------------------------------------------------------------------------
void Dbmgr::accountReqResetPassword(Network::Channel* pChannel, std::string& accountName)
{
	INFO_MSG(fmt::format("Dbmgr::accountReqResetPassword: accountName={}.\n", accountName));
	pInterfacesAccountHandler_->accountReqResetPassword(pChannel, accountName);
}

//-------------------------------------------------------------------------------------
void Dbmgr::accountResetPassword(Network::Channel* pChannel, std::string& accountName, std::string& newpassword, std::string& code)
{
	INFO_MSG(fmt::format("Dbmgr::accountResetPassword: accountName={}.\n", accountName));
	pInterfacesAccountHandler_->accountResetPassword(pChannel, accountName, newpassword, code);
}

//-------------------------------------------------------------------------------------
void Dbmgr::accountReqBindMail(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
							   std::string& password, std::string& email)
{
	INFO_MSG(fmt::format("Dbmgr::accountReqBindMail: accountName={}, email={}.\n", accountName, email));
	pInterfacesAccountHandler_->accountReqBindMail(pChannel, entityID, accountName, password, email);
}

//-------------------------------------------------------------------------------------
void Dbmgr::accountBindMail(Network::Channel* pChannel, std::string& username, std::string& scode)
{
	INFO_MSG(fmt::format("Dbmgr::accountBindMail: username={}, scode={}.\n", username, scode));
	pInterfacesAccountHandler_->accountBindMail(pChannel, username, scode);
}

//-------------------------------------------------------------------------------------
void Dbmgr::accountNewPassword(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
							   std::string& password, std::string& newpassword)
{
	INFO_MSG(fmt::format("Dbmgr::accountNewPassword: accountName={}.\n", accountName));
	pInterfacesAccountHandler_->accountNewPassword(pChannel, entityID, accountName, password, newpassword);
}

//-------------------------------------------------------------------------------------

}
