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
	pGlobalBases_(NULL),
	pCellAppData_(NULL),
	proxicesOnlineLogs_(),
	pDBInterface_(NULL)
{
}

//-------------------------------------------------------------------------------------
Dbmgr::~Dbmgr()
{
	loopCheckTimerHandle_.cancel();
	mainProcessTimer_.cancel();
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
	thread::ThreadPool::getSingleton().onMainThreadTick();
	getNetworkInterface().handleChannels(&DbmgrInterface::messageHandlers);
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
	// 初始化数据类别
	// demo/res/scripts/entity_defs/alias.xml
	if(!DataTypes::initialize("scripts/entity_defs/alias.xml"))
		return false;

	// 初始化所有扩展模块
	// demo/res/scripts/
	std::vector<PyTypeObject*>	scriptBaseTypes;
	if(!EntityDef::initialize(Resmgr::respaths()[1] + "res/scripts/", scriptBaseTypes, componentType_)){
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Dbmgr::initializeEnd()
{
	// 添加一个timer， 每秒检查一些状态
	loopCheckTimerHandle_ = this->getMainDispatcher().addTimer(1000000, this,
							reinterpret_cast<void *>(TIMEOUT_CHECK_STATUS));

	mainProcessTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_TICK));

	// 添加globalData, globalBases, cellAppData支持
	pGlobalData_ = new GlobalDataServer(GlobalDataServer::GLOBAL_DATA);
	pGlobalBases_ = new GlobalDataServer(GlobalDataServer::GLOBAL_BASES);
	pCellAppData_ = new GlobalDataServer(GlobalDataServer::CELLAPP_DATA);
	pGlobalData_->addConcernComponentType(CELLAPP_TYPE);
	pGlobalData_->addConcernComponentType(BASEAPP_TYPE);
	pGlobalBases_->addConcernComponentType(BASEAPP_TYPE);
	pCellAppData_->addConcernComponentType(CELLAPP_TYPE);

	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();
	pDBInterface_ = DBUtil::create(dbcfg.db_type, dbcfg.db_ip, dbcfg.db_port, 
		dbcfg.db_username, dbcfg.db_password, dbcfg.db_numConnections);

	if(pDBInterface_ == NULL)
	{
		ERROR_MSG("Dbmgr::initializeEnd: can't create dbinterface!\n");
		return false;
	}

	if(!pDBInterface_->attach(dbcfg.db_name))
	{
		ERROR_MSG("Dbmgr::initializeEnd: can't attach to database! %s.\n", pDBInterface_->c_str());
		return false;
	}
	else
	{
		INFO_MSG("Dbmgr::initializeEnd: %s\n", pDBInterface_->c_str());
	}

	return EntityTables::getSingleton().load(pDBInterface_) && EntityTables::getSingleton().syncToDB();
}

//-------------------------------------------------------------------------------------
void Dbmgr::finalise()
{
	SAFE_RELEASE(pGlobalData_);
	SAFE_RELEASE(pGlobalBases_);
	SAFE_RELEASE(pCellAppData_);

	if(pDBInterface_)
		pDBInterface_->detach();

	SAFE_RELEASE(pDBInterface_);
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------
void Dbmgr::onReqAllocEntityID(Mercury::Channel* pChannel, int8 componentType, COMPONENT_ID componentID)
{
	KBEngine::COMPONENT_TYPE ct = static_cast<KBEngine::COMPONENT_TYPE>(componentType);

	// 获取一个id段 并传输给IDClient
	std::pair<ENTITY_ID, ENTITY_ID> idRange = idServer_.allocRange();
	Mercury::Bundle bundle(pChannel);

	if(ct == BASEAPP_TYPE)
		bundle.newMessage(BaseappInterface::onReqAllocEntityID);
	else	
		bundle.newMessage(CellappInterface::onReqAllocEntityID);

	bundle << idRange.first;
	bundle << idRange.second;
	bundle.send(this->getNetworkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------
void Dbmgr::onRegisterNewApp(Mercury::Channel* pChannel, int32 uid, std::string& username, 
						int8 componentType, uint64 componentID, 
						uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport)
{
	ServerApp::onRegisterNewApp(pChannel, uid, username, componentType, componentID, 
						intaddr, intport, extaddr, extport);

	KBEngine::COMPONENT_TYPE tcomponentType = (KBEngine::COMPONENT_TYPE)componentType;

	// 下一步:
	// 如果是连接到dbmgr则需要等待接收app初始信息
	// 例如：初始会分配entityID段以及这个app启动的顺序信息（是否第一个baseapp启动）
	if(tcomponentType == BASEAPP_TYPE || 
		tcomponentType == CELLAPP_TYPE || 
		tcomponentType == LOGINAPP_TYPE)
	{
		Mercury::Bundle bundle;
		int32 startGlobalOrder = Componentbridge::getComponents().getGlobalOrderLog()[getUserUID()];
		int32 startGroupOrder = 0;

		switch(tcomponentType)
		{
		case BASEAPP_TYPE:
			{
				startGroupOrder = Componentbridge::getComponents().getBaseappGroupOrderLog()[getUserUID()];
				
				onGlobalDataClientLogon(pChannel, BASEAPP_TYPE);

				std::pair<ENTITY_ID, ENTITY_ID> idRange = idServer_.allocRange();
				bundle.newMessage(BaseappInterface::onDbmgrInitCompleted);
				BaseappInterface::onDbmgrInitCompletedArgs4::staticAddToBundle(bundle, idRange.first, 
					idRange.second, startGlobalOrder, startGroupOrder);
			}
			break;
		case CELLAPP_TYPE:
			{
				startGroupOrder = Componentbridge::getComponents().getCellappGroupOrderLog()[getUserUID()];
				
				onGlobalDataClientLogon(pChannel, CELLAPP_TYPE);

				std::pair<ENTITY_ID, ENTITY_ID> idRange = idServer_.allocRange();
				bundle.newMessage(CellappInterface::onDbmgrInitCompleted);
				CellappInterface::onDbmgrInitCompletedArgs4::staticAddToBundle(bundle, idRange.first, 
					idRange.second, startGlobalOrder, startGroupOrder);
			}
			break;
		case LOGINAPP_TYPE:
			startGroupOrder = Componentbridge::getComponents().getLoginappGroupOrderLog()[getUserUID()];

			bundle.newMessage(LoginappInterface::onDbmgrInitCompleted);
			LoginappInterface::onDbmgrInitCompletedArgs2::staticAddToBundle(bundle, startGlobalOrder, startGroupOrder);
			break;
		default:
			break;
		}

		bundle.send(networkInterface_, pChannel);
	}

	// 如果是baseapp或者cellapp则将自己注册到所有其他baseapp和cellapp
	if(tcomponentType == BASEAPP_TYPE || 
		tcomponentType == CELLAPP_TYPE)
	{
		KBEngine::COMPONENT_TYPE broadcastCpTypes[2] = {BASEAPP_TYPE, CELLAPP_TYPE};
		for(int idx = 0; idx < 2; idx++)
		{
			Components::COMPONENTS cts = Components::getSingleton().getComponents(broadcastCpTypes[idx]);
			Components::COMPONENTS::iterator fiter = cts.begin();
			for(; fiter != cts.end(); fiter++)
			{
				if((*fiter).cid == componentID)
					continue;

				Mercury::Bundle bundle;
				ENTITTAPP_COMMON_MERCURY_MESSAGE(broadcastCpTypes[idx], bundle, onGetEntityAppFromDbmgr);
				
				if(tcomponentType == BASEAPP_TYPE)
				{
					BaseappInterface::onGetEntityAppFromDbmgrArgs8::staticAddToBundle(bundle, uid, username, componentType, componentID, 
							intaddr, intport, extaddr, extport);
				}
				else
				{
					CellappInterface::onGetEntityAppFromDbmgrArgs8::staticAddToBundle(bundle, uid, username, componentType, componentID, 
							intaddr, intport, extaddr, extport);
				}
				
				KBE_ASSERT((*fiter).pChannel != NULL);
				bundle.send(networkInterface_, (*fiter).pChannel);
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Dbmgr::onGlobalDataClientLogon(Mercury::Channel* pChannel, COMPONENT_TYPE componentType)
{
	if(BASEAPP_TYPE == componentType)
	{
		pGlobalBases_->onGlobalDataClientLogon(pChannel, componentType);
		pGlobalData_->onGlobalDataClientLogon(pChannel, componentType);
	}
	else if(CELLAPP_TYPE == componentType)
	{
		pGlobalData_->onGlobalDataClientLogon(pChannel, componentType);
		pCellAppData_->onGlobalDataClientLogon(pChannel, componentType);
	}
	else
	{
		ERROR_MSG("Dbmgr::onGlobalDataClientLogon: nonsupport %s!\n", COMPONENT_NAME_EX(componentType));
	}
}

//-------------------------------------------------------------------------------------
void Dbmgr::onBroadcastGlobalDataChange(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	uint8 dataType;
	int32 slen;
	std::string key, value;
	bool isDelete;
	COMPONENT_TYPE componentType;
	
	s >> dataType;
	s >> isDelete;
	s >> slen;
	key.assign((char*)(s.data() + s.rpos()), slen);
	s.read_skip(slen);

	if(!isDelete)
	{
		s >> slen;
		value.assign((char*)(s.data() + s.rpos()), slen);
		s.read_skip(slen);
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
	case GlobalDataServer::GLOBAL_BASES:
		if(isDelete)
			pGlobalBases_->del(pChannel, componentType, key);
		else
			pGlobalBases_->write(pChannel, componentType, key, value);
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
void Dbmgr::reqCreateAccount(Mercury::Channel* pChannel, std::string& accountName, std::string& password)
{
	DEBUG_MSG("Dbmgr::reqCreateAccount:%s.\n", accountName.c_str());

	Mercury::Bundle bundle;
	bundle.newMessage(LoginappInterface::onReqCreateAccountResult);
	MERCURY_ERROR_CODE failedcode = MERCURY_SUCCESS;

	// 如果没有连接db则从log中查找账号是否有此账号(这个功能是为了测试使用)
	if(!pDBInterface_)
	{
		PROXICES_ONLINE_LOG::iterator iter = proxicesOnlineLogs_.find(accountName);
		if(iter != proxicesOnlineLogs_.end())
		{
			failedcode = MERCURY_ERR_ACCOUNT_CREATE;
		}
	}

	LoginappInterface::onReqCreateAccountResultArgs3::staticAddToBundle(bundle, failedcode, accountName, password);
	bundle.send(this->getNetworkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------
void Dbmgr::onAccountLogin(Mercury::Channel* pChannel, std::string& accountName, std::string& password)
{
	DEBUG_MSG("Dbmgr::onAccountLogin:%s.\n", accountName.c_str());
	// 一个用户登录， 构造一个数据库查询指令并加入到执行队列， 执行完毕将结果返回给loginapp
	Mercury::Bundle bundle;
	bundle.newMessage(LoginappInterface::onLoginAccountQueryResultFromDbmgr);
	bool success = true;
	COMPONENT_ID componentID = 0;
	ENTITY_ID entityID = 0;
	
	// 如果没有连接db则从log中查找账号是否还在线
	if(!pDBInterface_)
	{
		PROXICES_ONLINE_LOG::iterator iter = proxicesOnlineLogs_.find(accountName);
		if(iter != proxicesOnlineLogs_.end())
		{
			componentID = iter->second.cid;
			entityID = iter->second.eid;
		}
	}

	bundle << success;
	bundle << accountName;
	bundle << password;
	bundle << componentID;   // 如果大于0则表示账号还存活在某个baseapp上
	bundle << entityID;
	bundle.send(this->getNetworkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------
void Dbmgr::queryAccount(Mercury::Channel* pChannel, std::string& accountName, std::string& password)
{
	DEBUG_MSG("Dbmgr::queryAccount:%s.\n", accountName.c_str());

	Mercury::Bundle bundle;
	bundle.newMessage(BaseappInterface::onQueryAccountCBFromDbmgr);
	bundle << accountName;
	bundle << password;
	bundle << "";
	bundle.send(this->getNetworkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------
void Dbmgr::onAccountOnline(Mercury::Channel* pChannel, std::string& accountName, COMPONENT_ID componentID, ENTITY_ID entityID)
{
	DEBUG_MSG("Dbmgr::onAccountOnline:componentID:%"PRAppID", entityID:%d.\n", componentID, entityID);

	// 如果没有连接db则从log中查找账号是否还在线
	if(!pDBInterface_)
	{
		PROXICES_ONLINE_LOG::iterator iter = proxicesOnlineLogs_.find(accountName);
		if(iter != proxicesOnlineLogs_.end())
		{
			iter->second.cid = componentID;
			iter->second.eid = entityID;
		}
		else
		{
			proxicesOnlineLogs_[accountName].cid = componentID;
			proxicesOnlineLogs_[accountName].eid = entityID;
		}
	}
}

//-------------------------------------------------------------------------------------
void Dbmgr::onAccountOffline(Mercury::Channel* pChannel, std::string& accountName)
{
	DEBUG_MSG("Dbmgr::onAccountOffline:%s.\n", accountName.c_str());

	// 如果没有连接db则从log中查找账号是否还在线
	if(!pDBInterface_)
	{
		proxicesOnlineLogs_.erase(accountName);
	}
}

//-------------------------------------------------------------------------------------
void Dbmgr::executeRawDatabaseCommand(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	COMPONENT_ID componentID = 0;
	COMPONENT_TYPE componentType;
	std::string datas;
	CALLBACK_ID callbackID = 0;

	s >> componentID >> componentType;
	s >> callbackID;
	s.readBlob(datas);

	DEBUG_MSG("Dbmgr::executeRawDatabaseCommand:%s.\n", datas.c_str());

	std::string error;
	MemoryStream ret;
	if(!static_cast<DBInterfaceMysql*>(pDBInterface_)->execute(datas.data(), datas.size(), &ret))
	{
		error = pDBInterface_->getstrerror();
	}

	// 如果不需要回调则结束
	if(callbackID <= 0)
		return;

	Mercury::Bundle bundle;
	if(componentType == BASEAPP_TYPE)
		bundle.newMessage(BaseappInterface::onExecuteRawDatabaseCommandCB);
	else if(componentType == CELLAPP_TYPE)
		bundle.newMessage(CellappInterface::onExecuteRawDatabaseCommandCB);
	else
	{
		KBE_ASSERT(false && "no support!\n");
	}

	bundle << callbackID;
	bundle << error;
	if(error.size() <= 0)
		bundle.append(ret);

	bundle.send(this->getNetworkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------
void Dbmgr::writeEntity(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID entityID = 0;
	ENTITY_SCRIPT_UID sid = 0;
	s >> entityID >> sid;

	ScriptDefModule* pModule = EntityDef::findScriptModule(sid);
	DEBUG_MSG("Dbmgr::writeEntity: %s(%d), size=%u.\n", pModule->getName(), entityID, s.opsize());

	//EntityTables::getSingleton().updateFromStream(pModule, entityID, s);
}

//-------------------------------------------------------------------------------------

}
