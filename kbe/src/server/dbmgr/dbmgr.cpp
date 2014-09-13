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
#include "dbmgr_interface.hpp"
#include "dbtasks.hpp"
#include "billinghandler.hpp"
#include "sync_app_datas_handler.hpp"
#include "db_mysql/kbe_table_mysql.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
#include "dbmgr_lib/db_interface.hpp"
#include "db_mysql/db_interface_mysql.hpp"
#include "entitydef/scriptdef_module.hpp"

#include "baseapp/baseapp_interface.hpp"
#include "cellapp/cellapp_interface.hpp"
#include "baseappmgr/baseappmgr_interface.hpp"
#include "cellappmgr/cellappmgr_interface.hpp"
#include "loginapp/loginapp_interface.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Dbmgr);

//-------------------------------------------------------------------------------------
Dbmgr::Dbmgr(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
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
	pBillingAccountHandler_(NULL),
	pBillingChargeHandler_(NULL),
	pSyncAppDatasHandler_(NULL)
{
}

//-------------------------------------------------------------------------------------
Dbmgr::~Dbmgr()
{
	loopCheckTimerHandle_.cancel();
	mainProcessTimer_.cancel();
	KBEngine::sleep(300);

	SAFE_RELEASE(pBillingAccountHandler_);
	SAFE_RELEASE(pBillingChargeHandler_);
}

//-------------------------------------------------------------------------------------
bool Dbmgr::canShutdown()
{
	bool ret = bufferedDBTasks_.size() == 0;
	
	if(ret)
	{
		WARNING_MSG(boost::format("Dbmgr::canShutdown(): tasks=%1%, threads=%2%, threadpoolDestroyed=%3%!\n") % 
			bufferedDBTasks_.size() % DBUtil::pThreadPool()->currentThreadCount() % DBUtil::pThreadPool()->isDestroyed());
	}

	return ret;
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
	networkInterface().processAllChannelPackets(&DbmgrInterface::messageHandlers);
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
	// demo/res/scripts/
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
	loopCheckTimerHandle_ = this->mainDispatcher().addTimer(1000000, this,
							reinterpret_cast<void *>(TIMEOUT_CHECK_STATUS));

	mainProcessTimer_ = this->mainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_TICK));

	// 添加globalData, baseAppData, cellAppData支持
	pGlobalData_ = new GlobalDataServer(GlobalDataServer::GLOBAL_DATA);
	pBaseAppData_ = new GlobalDataServer(GlobalDataServer::BASEAPP_DATA);
	pCellAppData_ = new GlobalDataServer(GlobalDataServer::CELLAPP_DATA);
	pGlobalData_->addConcernComponentType(CELLAPP_TYPE);
	pGlobalData_->addConcernComponentType(BASEAPP_TYPE);
	pBaseAppData_->addConcernComponentType(BASEAPP_TYPE);
	pCellAppData_->addConcernComponentType(CELLAPP_TYPE);

	INFO_MSG(boost::format("Dbmgr::initializeEnd: digest(%1%)\n") % 
		EntityDef::md5().getDigestStr());
	
	return initBillingHandler() && initDB();
}

//-------------------------------------------------------------------------------------		
bool Dbmgr::initBillingHandler()
{
	pBillingAccountHandler_ = BillingHandlerFactory::create(g_kbeSrvConfig.billingSystemAccountType(), threadPool(), *static_cast<KBEngine::DBThreadPool*>(DBUtil::pThreadPool()));
	pBillingChargeHandler_ = BillingHandlerFactory::create(g_kbeSrvConfig.billingSystemChargeType(), threadPool(), *static_cast<KBEngine::DBThreadPool*>(DBUtil::pThreadPool()));

	INFO_MSG(boost::format("Dbmgr::initBillingHandler: billing addr(%1%), accountType:(%2%), chargeType:(%3%).\n") % 
		g_kbeSrvConfig.billingSystemAddr().c_str() %
		g_kbeSrvConfig.billingSystemAccountType() %
		g_kbeSrvConfig.billingSystemChargeType());

	if(strlen(g_kbeSrvConfig.billingSystemThirdpartyAccountServiceAddr()) > 0)
	{
		INFO_MSG(boost::format("Dbmgr::initBillingHandler: thirdpartyAccountService_addr(%1%:%2%).\n") % 
			g_kbeSrvConfig.billingSystemThirdpartyAccountServiceAddr() %
			g_kbeSrvConfig.billingSystemThirdpartyAccountServicePort());
	}

	if(strlen(g_kbeSrvConfig.billingSystemThirdpartyChargeServiceAddr()) > 0)
	{
		INFO_MSG(boost::format("Dbmgr::initBillingHandler: thirdpartyChargeService_addr(%1%:%2%).\n") % 
			g_kbeSrvConfig.billingSystemThirdpartyChargeServiceAddr() %
			g_kbeSrvConfig.billingSystemThirdpartyChargeServicePort());
	}

	return pBillingAccountHandler_->initialize() && pBillingChargeHandler_->initialize();
}

//-------------------------------------------------------------------------------------		
bool Dbmgr::initDB()
{
	if(!DBUtil::initialize())
	{
		ERROR_MSG("Dbmgr::initDB: can't initialize dbinterface!\n");
		return false;
	}

	DBInterface* pDBInterface = DBUtil::createInterface();
	if(pDBInterface == NULL)
	{
		ERROR_MSG("Dbmgr::initDB: can't create dbinterface!\n");
		return false;
	}

	bool ret = DBUtil::initInterface(pDBInterface);
	
	if(ret)
	{
		ret = pDBInterface->checkEnvironment();
	}
	
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
void Dbmgr::onReqAllocEntityID(Mercury::Channel* pChannel, int8 componentType, COMPONENT_ID componentID)
{
	KBEngine::COMPONENT_TYPE ct = static_cast<KBEngine::COMPONENT_TYPE>(componentType);

	// 获取一个id段 并传输给IDClient
	std::pair<ENTITY_ID, ENTITY_ID> idRange = idServer_.allocRange();
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	if(ct == BASEAPP_TYPE)
		(*pBundle).newMessage(BaseappInterface::onReqAllocEntityID);
	else	
		(*pBundle).newMessage(CellappInterface::onReqAllocEntityID);

	(*pBundle) << idRange.first;
	(*pBundle) << idRange.second;
	(*pBundle).send(this->networkInterface(), pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Dbmgr::onRegisterNewApp(Mercury::Channel* pChannel, int32 uid, std::string& username, 
						int8 componentType, uint64 componentID, int8 globalorderID, int8 grouporderID,
						uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx)
{
	if(pChannel->isExternal())
		return;

	ServerApp::onRegisterNewApp(pChannel, uid, username, componentType, componentID, globalorderID, grouporderID,
						intaddr, intport, extaddr, extport, extaddrEx);

	KBEngine::COMPONENT_TYPE tcomponentType = (KBEngine::COMPONENT_TYPE)componentType;
	
	int32 startGroupOrder = 1;
	int32 startGlobalOrder = Componentbridge::getComponents().getGlobalOrderLog()[getUserUID()];

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
					startGroupOrder = Componentbridge::getComponents().getBaseappGroupOrderLog()[getUserUID()];
			}
			break;
		case CELLAPP_TYPE:
			{
				if(grouporderID <= 0)
					startGroupOrder = Componentbridge::getComponents().getCellappGroupOrderLog()[getUserUID()];
			}
			break;
		case LOGINAPP_TYPE:
			if(grouporderID <= 0)
				startGroupOrder = Componentbridge::getComponents().getLoginappGroupOrderLog()[getUserUID()];

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
		for(int idx = 0; idx < 2; idx++)
		{
			Components::COMPONENTS& cts = Components::getSingleton().getComponents(broadcastCpTypes[idx]);
			Components::COMPONENTS::iterator fiter = cts.begin();
			for(; fiter != cts.end(); fiter++)
			{
				if((*fiter).cid == componentID)
					continue;

				Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
				ENTITTAPP_COMMON_MERCURY_MESSAGE(broadcastCpTypes[idx], (*pBundle), onGetEntityAppFromDbmgr);
				
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
				(*pBundle).send(networkInterface_, (*fiter).pChannel);
				Mercury::Bundle::ObjPool().reclaimObject(pBundle);
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Dbmgr::onGlobalDataClientLogon(Mercury::Channel* pChannel, COMPONENT_TYPE componentType)
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
		ERROR_MSG(boost::format("Dbmgr::onGlobalDataClientLogon: nonsupport %1%!\n") %
			COMPONENT_NAME_EX(componentType));
	}
}

//-------------------------------------------------------------------------------------
void Dbmgr::onBroadcastGlobalDataChanged(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
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
void Dbmgr::reqCreateAccount(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
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

	pBillingAccountHandler_->createAccount(pChannel, registerName, password, datas, ACCOUNT_TYPE(uatype));
	numCreatedAccount_++;
}

//-------------------------------------------------------------------------------------
void Dbmgr::onCreateAccountCBFromBilling(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	pBillingAccountHandler_->onCreateAccountCB(s);
}

//-------------------------------------------------------------------------------------
void Dbmgr::onAccountLogin(Mercury::Channel* pChannel, KBEngine::MemoryStream& s) 
{
	std::string loginName, password, datas;
	s >> loginName >> password;
	s.readBlob(datas);

	if(loginName.size() == 0)
	{
		ERROR_MSG("Dbmgr::onAccountLogin: loginName is empty.\n");
		return;
	}

	pBillingAccountHandler_->loginAccount(pChannel, loginName, password, datas);
}

//-------------------------------------------------------------------------------------
void Dbmgr::onLoginAccountCBBFromBilling(Mercury::Channel* pChannel, KBEngine::MemoryStream& s) 
{
	pBillingAccountHandler_->onLoginAccountCB(s);
}

//-------------------------------------------------------------------------------------
void Dbmgr::queryAccount(Mercury::Channel* pChannel, 
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
void Dbmgr::onAccountOnline(Mercury::Channel* pChannel, 
							std::string& accountName, 
							COMPONENT_ID componentID, 
							ENTITY_ID entityID)
{
	// bufferedDBTasks_.addTask(new DBTaskAccountOnline(pChannel->addr(), 
	//	accountName, componentID, entityID));
}

//-------------------------------------------------------------------------------------
void Dbmgr::onEntityOffline(Mercury::Channel* pChannel, DBID dbid, ENTITY_SCRIPT_UID sid)
{
	bufferedDBTasks_.addTask(new DBTaskEntityOffline(pChannel->addr(), dbid, sid));
}

//-------------------------------------------------------------------------------------
void Dbmgr::executeRawDatabaseCommand(Mercury::Channel* pChannel, 
									  KBEngine::MemoryStream& s)
{
	ENTITY_ID entityID = -1;
	s >> entityID;

	if(entityID == -1)
		DBUtil::pThreadPool()->addTask(new DBTaskExecuteRawDatabaseCommand(pChannel->addr(), s));
	else
		bufferedDBTasks_.addTask(new DBTaskExecuteRawDatabaseCommandByEntity(pChannel->addr(), s, entityID));

	s.opfini();

	numExecuteRawDatabaseCommand_++;
}

//-------------------------------------------------------------------------------------
void Dbmgr::writeEntity(Mercury::Channel* pChannel, 
						KBEngine::MemoryStream& s)
{
	ENTITY_ID eid;
	DBID entityDBID;
	COMPONENT_ID componentID;

	s >> componentID >> eid >> entityDBID;

	bufferedDBTasks_.addTask(new DBTaskWriteEntity(pChannel->addr(), componentID, eid, entityDBID, s));
	s.opfini();

	numWrittenEntity_++;
}

//-------------------------------------------------------------------------------------
void Dbmgr::removeEntity(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID eid;
	DBID entityDBID;
	COMPONENT_ID componentID;

	s >> componentID >> eid >> entityDBID;
	KBE_ASSERT(entityDBID > 0);

	bufferedDBTasks_.addTask(new DBTaskRemoveEntity(pChannel->addr(), 
		componentID, eid, entityDBID, s));

	s.opfini();

	numRemovedEntity_++;
}

//-------------------------------------------------------------------------------------
void Dbmgr::deleteBaseByDBID(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
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
void Dbmgr::queryEntity(Mercury::Channel* pChannel, COMPONENT_ID componentID, int8 queryMode, DBID dbid, 
	std::string& entityType, CALLBACK_ID callbackID, ENTITY_ID entityID)
{
	bufferedDBTasks_.addTask(new DBTaskQueryEntity(pChannel->addr(), queryMode, entityType, 
		dbid, componentID, callbackID, entityID));

	numQueryEntity_++;
}

//-------------------------------------------------------------------------------------
void Dbmgr::syncEntityStreamTemplate(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	KBEAccountTable* pTable = 
		static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));

	KBE_ASSERT(pTable);

	pTable->accountDefMemoryStream(s);
	s.opfini();
}

//-------------------------------------------------------------------------------------
void Dbmgr::charge(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	pBillingChargeHandler_->charge(pChannel, s);
}

//-------------------------------------------------------------------------------------
void Dbmgr::onChargeCB(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	pBillingChargeHandler_->onChargeCB(s);
}

//-------------------------------------------------------------------------------------
void Dbmgr::eraseClientReq(Mercury::Channel* pChannel, std::string& logkey)
{
	pBillingAccountHandler_->eraseClientReq(pChannel, logkey);
}

//-------------------------------------------------------------------------------------
void Dbmgr::accountActivate(Mercury::Channel* pChannel, std::string& scode)
{
	INFO_MSG(boost::format("Dbmgr::accountActivate: code=%1%.\n") % scode);
	pBillingAccountHandler_->accountActivate(pChannel, scode);
}

//-------------------------------------------------------------------------------------
void Dbmgr::accountReqResetPassword(Mercury::Channel* pChannel, std::string& accountName)
{
	INFO_MSG(boost::format("Dbmgr::accountReqResetPassword: accountName=%1%.\n") % accountName);
	pBillingAccountHandler_->accountReqResetPassword(pChannel, accountName);
}

//-------------------------------------------------------------------------------------
void Dbmgr::accountResetPassword(Mercury::Channel* pChannel, std::string& accountName, std::string& newpassword, std::string& code)
{
	INFO_MSG(boost::format("Dbmgr::accountResetPassword: accountName=%1%.\n") % accountName);
	pBillingAccountHandler_->accountResetPassword(pChannel, accountName, newpassword, code);
}

//-------------------------------------------------------------------------------------
void Dbmgr::accountReqBindMail(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
							   std::string& password, std::string& email)
{
	INFO_MSG(boost::format("Dbmgr::accountReqBindMail: accountName=%1%, email=%2%.\n") % accountName % email);
	pBillingAccountHandler_->accountReqBindMail(pChannel, entityID, accountName, password, email);
}

//-------------------------------------------------------------------------------------
void Dbmgr::accountBindMail(Mercury::Channel* pChannel, std::string& username, std::string& scode)
{
	INFO_MSG(boost::format("Dbmgr::accountBindMail: username=%1%, scode=%2%.\n") % username % scode);
	pBillingAccountHandler_->accountBindMail(pChannel, username, scode);
}

//-------------------------------------------------------------------------------------
void Dbmgr::accountNewPassword(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
							   std::string& password, std::string& newpassword)
{
	INFO_MSG(boost::format("Dbmgr::accountNewPassword: accountName=%1%.\n") % accountName);
	pBillingAccountHandler_->accountNewPassword(pChannel, entityID, accountName, password, newpassword);
}

//-------------------------------------------------------------------------------------

}
