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

#include "dbtasks.hpp"
#include "dbmgr.hpp"
#include "buffered_dbtasks.hpp"
#include "network/common.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
#include "server/serverconfig.hpp"
#include "dbmgr_lib/db_interface.hpp"
#include "dbmgr_lib/kbe_tables.hpp"
#include "db_mysql/db_interface_mysql.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "openssl/md5.h"

#include "baseapp/baseapp_interface.hpp"
#include "cellapp/cellapp_interface.hpp"
#include "baseappmgr/baseappmgr_interface.hpp"
#include "cellappmgr/cellappmgr_interface.hpp"
#include "loginapp/loginapp_interface.hpp"

#if KBE_PLATFORM == PLATFORM_WIN32
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#endif

namespace KBEngine{

//-------------------------------------------------------------------------------------
DBTask::DBTask(const Mercury::Address& addr, MemoryStream& datas):
pDatas_(0),
addr_(addr),
pdbi_(NULL)
{
	pDatas_ = MemoryStream::ObjPool().createObject();
	*pDatas_ = datas;
}

//-------------------------------------------------------------------------------------
DBTask::~DBTask()
{
	if(pDatas_)
		MemoryStream::ObjPool().reclaimObject(pDatas_);
}

//-------------------------------------------------------------------------------------
bool DBTask::send(Mercury::Bundle& bundle)
{
	Mercury::Channel* pChannel = Dbmgr::getSingleton().getNetworkInterface().findChannel(addr_);
	
	if(pChannel){
		bundle.send(Dbmgr::getSingleton().getNetworkInterface(), pChannel);
	}
	else{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool DBTask::process()
{
	mysql_thread_init();

	for(int tryi = 0; tryi < 10; tryi++)
	{
		pdbi_ = DBUtil::createInterface(false);
		if(pdbi_ == NULL)
		{
			KBEngine::sleep(1000);
		}
		else
		{
			break;
		}
	}

	bool ret = false;
	
	if(pdbi_)
	{
		ret = db_thread_process();
	}

	mysql_thread_end();

	if(pdbi_)
		pdbi_->detach();
	
	SAFE_RELEASE(pdbi_);
	return ret;
}

//-------------------------------------------------------------------------------------
void EntityDBTask::presentMainThread()
{
	KBE_ASSERT(_pBuffered_DBTasks != NULL);
	_pBuffered_DBTasks->onFiniTask(this);
}

//-------------------------------------------------------------------------------------
DBTaskExecuteRawDatabaseCommand::DBTaskExecuteRawDatabaseCommand(const Mercury::Address& addr, MemoryStream& datas):
DBTask(addr, datas),
componentID_(0),
componentType_(UNKNOWN_COMPONENT_TYPE),
sdatas_(),
callbackID_(0),
error_(),
execret_()
{
}

//-------------------------------------------------------------------------------------
DBTaskExecuteRawDatabaseCommand::~DBTaskExecuteRawDatabaseCommand()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskExecuteRawDatabaseCommand::db_thread_process()
{
	(*pDatas_) >> componentID_ >> componentType_;
	(*pDatas_) >> callbackID_;
	(*pDatas_).readBlob(sdatas_);

	if(!static_cast<DBInterfaceMysql*>(pdbi_)->execute(sdatas_.data(), sdatas_.size(), &execret_))
	{
		error_ = pdbi_->getstrerror();
	}
	
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskExecuteRawDatabaseCommand::presentMainThread()
{
	DEBUG_MSG("Dbmgr::executeRawDatabaseCommand:%s.\n", sdatas_.c_str());

	// 如果不需要回调则结束
	if(callbackID_ <= 0)
		return;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	if(componentType_ == BASEAPP_TYPE)
		(*pBundle).newMessage(BaseappInterface::onExecuteRawDatabaseCommandCB);
	else if(componentType_ == CELLAPP_TYPE)
		(*pBundle).newMessage(CellappInterface::onExecuteRawDatabaseCommandCB);
	else
	{
		KBE_ASSERT(false && "no support!\n");
	}

	(*pBundle) << callbackID_;
	(*pBundle) << error_;
	if(error_.size() <= 0)
		(*pBundle).append(execret_);

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(componentType_, componentID_);

	if(cinfos && cinfos->pChannel)
	{
		(*pBundle).send(Dbmgr::getSingleton().getNetworkInterface(), cinfos->pChannel);
	}
	else
	{
		ERROR_MSG("DBTaskExecuteRawDatabaseCommand::presentMainThread: %s not found.", COMPONENT_NAME_EX(componentType_));
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
DBTaskWriteEntity::DBTaskWriteEntity(const Mercury::Address& addr, 
									 COMPONENT_ID componentID, ENTITY_ID eid, 
									 DBID entityDBID, MemoryStream& datas):
EntityDBTask(addr, datas, eid, entityDBID),
componentID_(componentID),
eid_(eid),
entityDBID_(entityDBID),
sid_(0),
callbackID_(0),
success_(false)
{
}

//-------------------------------------------------------------------------------------
DBTaskWriteEntity::~DBTaskWriteEntity()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskWriteEntity::db_thread_process()
{
	(*pDatas_) >> sid_ >> callbackID_;

	ScriptDefModule* pModule = EntityDef::findScriptModule(sid_);
	bool writeEntityLog = (entityDBID_ == 0);

	entityDBID_ = EntityTables::getSingleton().writeEntity(pdbi_, entityDBID_, pDatas_, pModule);
	success_ = entityDBID_ > 0;

	if(writeEntityLog && success_)
	{
		// 先写log， 如果写失败则可能这个entity已经在线
		KBEEntityLogTable* pELTable = static_cast<KBEEntityLogTable*>
						(EntityTables::getSingleton().findKBETable("kbe_entitylog"));
		KBE_ASSERT(pELTable);

		success_ = pELTable->logEntity(pdbi_, addr_.ipAsString(), addr_.port, entityDBID_, 
			componentID_, eid_, pModule->getUType());

		if(!success_)
		{
			entityDBID_ = 0;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskWriteEntity::presentMainThread()
{
	ScriptDefModule* pModule = EntityDef::findScriptModule(sid_);
	DEBUG_MSG("Dbmgr::writeEntity: %s(%"PRIu64").\n", pModule->getName(), entityDBID_);

	// 返回写entity的结果， 成功或者失败
	// callbackID_

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::onWriteToDBCallback);
	BaseappInterface::onWriteToDBCallbackArgs4::staticAddToBundle((*pBundle), 
		eid_, entityDBID_, callbackID_, success_);

	if(!this->send((*pBundle)))
	{
		ERROR_MSG("DBTaskWriteEntity::presentMainThread: channel(%s) not found.\n", addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	EntityDBTask::presentMainThread();
}

//-------------------------------------------------------------------------------------
DBTaskRemoveEntity::DBTaskRemoveEntity(const Mercury::Address& addr, 
									 COMPONENT_ID componentID, ENTITY_ID eid, 
									 DBID entityDBID, MemoryStream& datas):
EntityDBTask(addr, datas, eid, entityDBID),
componentID_(componentID),
eid_(eid),
entityDBID_(entityDBID),
sid_(0)
{
}

//-------------------------------------------------------------------------------------
DBTaskRemoveEntity::~DBTaskRemoveEntity()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskRemoveEntity::db_thread_process()
{
	(*pDatas_) >> sid_;

	KBEEntityLogTable* pELTable = static_cast<KBEEntityLogTable*>
					(EntityTables::getSingleton().findKBETable("kbe_entitylog"));
	KBE_ASSERT(pELTable);
	pELTable->eraseEntityLog(pdbi_, entityDBID_);

	EntityTables::getSingleton().removeEntity(pdbi_, entityDBID_, EntityDef::findScriptModule(sid_));
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskRemoveEntity::presentMainThread()
{
	ScriptDefModule* pModule = EntityDef::findScriptModule(sid_);
	DEBUG_MSG("Dbmgr::removeEntity: %s(%"PRIu64").\n", pModule->getName(), entityDBID_);
	EntityDBTask::presentMainThread();
}

//-------------------------------------------------------------------------------------
DBTaskCreateAccount::DBTaskCreateAccount(const Mercury::Address& addr, std::string& accountName, std::string& password):
DBTask(addr),
accountName_(accountName),
password_(password),
success_(false)
{
}

//-------------------------------------------------------------------------------------
DBTaskCreateAccount::~DBTaskCreateAccount()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskCreateAccount::db_thread_process()
{
	// 寻找dblog是否有此账号， 如果有则创建失败
	// 如果没有则向account表新建一个entity数据同时在accountlog表写入一个log关联dbid
	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));
	KBE_ASSERT(pTable);

	ScriptDefModule* pModule = EntityDef::findScriptModule(DBUtil::accountScriptName());

	ACCOUNT_INFOS info;
	if(pTable->queryAccount(pdbi_, accountName_, info))
		return false;

	DBID entityDBID = EntityTables::getSingleton().writeEntity(pdbi_, 0, &pTable->accountDefMemoryStream(), pModule);
	pTable->accountDefMemoryStream().rpos(0);
	KBE_ASSERT(entityDBID > 0);

	info.name = accountName_;
	info.password = password_;
	info.dbid = entityDBID;
	if(!pTable->logAccount(pdbi_, info))
		return false;

	success_ = true;
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskCreateAccount::presentMainThread()
{
	DEBUG_MSG("Dbmgr::reqCreateAccount:%s.\n", accountName_.c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::onReqCreateAccountResult);
	SERVER_ERROR_CODE failedcode = SERVER_SUCCESS;

	if(!success_)
		failedcode = SERVER_ERR_ACCOUNT_CREATE;

	LoginappInterface::onReqCreateAccountResultArgs3::staticAddToBundle((*pBundle), failedcode, accountName_, password_);

	if(!this->send((*pBundle)))
	{
		ERROR_MSG("DBTaskCreateAccount::presentMainThread: channel(%s) not found.\n", addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
DBTaskQueryAccount::DBTaskQueryAccount(const Mercury::Address& addr, std::string& accountName, std::string& password, 
		COMPONENT_ID componentID, ENTITY_ID entityID, DBID entityDBID):
EntityDBTask(addr, entityID, entityDBID),
accountName_(accountName),
password_(password),
success_(false),
s_(),
dbid_(entityDBID),
componentID_(componentID),
entityID_(entityID)
{
}

//-------------------------------------------------------------------------------------
DBTaskQueryAccount::~DBTaskQueryAccount()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskQueryAccount::db_thread_process()
{
	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));
	KBE_ASSERT(pTable);

	ACCOUNT_INFOS info;
	info.name = "";
	info.password = "";
	info.dbid = dbid_;

	if(dbid_  == 0)
	{
		if(!pTable->queryAccount(pdbi_, accountName_, info))
			return false;

		if(info.dbid == 0)
			return false;
		
		unsigned char md[16];
		MD5((unsigned char *)password_.c_str(), password_.length(), md);

		char tmp[3]={'\0'}, md5password[33] = {'\0'};
		for (int i = 0; i < 16; i++)
		{
			sprintf(tmp,"%2.2X", md[i]);
			strcat(md5password, tmp);
		}

		if(kbe_stricmp(info.password.c_str(), md5password) != 0)
			return false;
	}

	ScriptDefModule* pModule = EntityDef::findScriptModule(g_kbeSrvConfig.getDBMgr().dbAccountEntityScriptType);
	success_ = EntityTables::getSingleton().queryEntity(pdbi_, info.dbid, &s_, pModule);

	dbid_ = info.dbid;

	// 先写log， 如果写失败则可能这个entity已经在线
	KBEEntityLogTable* pELTable = static_cast<KBEEntityLogTable*>
					(EntityTables::getSingleton().findKBETable("kbe_entitylog"));
	KBE_ASSERT(pELTable);
	
	success_ = pELTable->logEntity(pdbi_, addr_.ipAsString(), addr_.port, dbid_, 
		componentID_, entityID_, pModule->getUType());

	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskQueryAccount::presentMainThread()
{
	DEBUG_MSG("Dbmgr::queryAccount:%s.\n", accountName_.c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::onQueryAccountCBFromDbmgr);
	(*pBundle) << accountName_;
	(*pBundle) << password_;
	(*pBundle) << dbid_;
	(*pBundle) << success_;
	(*pBundle) << entityID_;

	if(success_)
	{
		pBundle->append(s_);
	}

	if(!this->send((*pBundle)))
	{
		ERROR_MSG("DBTaskQueryAccount::presentMainThread: channel(%s) not found.\n", addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	EntityDBTask::presentMainThread();
}

//-------------------------------------------------------------------------------------
DBTaskAccountOnline::DBTaskAccountOnline(const Mercury::Address& addr, std::string& accountName,
		COMPONENT_ID componentID, ENTITY_ID entityID):
DBTask(addr),
accountName_(accountName),
componentID_(componentID),
entityID_(entityID)
{
}

//-------------------------------------------------------------------------------------
DBTaskAccountOnline::~DBTaskAccountOnline()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskAccountOnline::db_thread_process()
{
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskAccountOnline::presentMainThread()
{
	DEBUG_MSG("Dbmgr::onAccountOnline:componentID:%"PRAppID", entityID:%d.\n", componentID_, entityID_);

	/*
	// 如果没有连接db则从log中查找账号是否还在线
	if(!pDBInterface_)
	{
		PROXICES_ONLINE_LOG::iterator iter = proxicesOnlineLogs_.find(accountName_);
		if(iter != proxicesOnlineLogs_.end())
		{
			iter->second.cid = componentID_;
			iter->second.eid = entityID_;
		}
		else
		{
			proxicesOnlineLogs_[accountName].cid = componentID_;
			proxicesOnlineLogs_[accountName].eid = entityID_;
		}
	}
	*/
}

//-------------------------------------------------------------------------------------
DBTaskEntityOffline::DBTaskEntityOffline(const Mercury::Address& addr, DBID dbid):
DBTask(addr),
dbid_(dbid)
{
}

//-------------------------------------------------------------------------------------
DBTaskEntityOffline::~DBTaskEntityOffline()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskEntityOffline::db_thread_process()
{
	KBEEntityLogTable* pELTable = static_cast<KBEEntityLogTable*>
					(EntityTables::getSingleton().findKBETable("kbe_entitylog"));
	KBE_ASSERT(pELTable);
	pELTable->eraseEntityLog(pdbi_, dbid_);
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskEntityOffline::presentMainThread()
{
	DEBUG_MSG("Dbmgr::onEntityOffline:%"PRDBID".\n", dbid_);
}

//-------------------------------------------------------------------------------------
DBTaskAccountLogin::DBTaskAccountLogin(const Mercury::Address& addr, std::string& accountName, std::string& password):
DBTask(addr),
accountName_(accountName),
password_(password),
success_(false),
componentID_(0),
entityID_(0),
dbid_(0)
{
}

//-------------------------------------------------------------------------------------
DBTaskAccountLogin::~DBTaskAccountLogin()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskAccountLogin::db_thread_process()
{
	KBEEntityLogTable* pELTable = static_cast<KBEEntityLogTable*>
					(EntityTables::getSingleton().findKBETable("kbe_entitylog"));
	KBE_ASSERT(pELTable);

	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));
	KBE_ASSERT(pTable);

	ACCOUNT_INFOS info;
	info.dbid = 0;
	if(!pTable->queryAccount(pdbi_, accountName_, info))
		return false;

	if(info.dbid == 0)
		return false;

	KBEEntityLogTable::EntityLog entitylog;

	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();
	ScriptDefModule* pModule = EntityDef::findScriptModule(dbcfg.dbAccountEntityScriptType);
	success_ = !pELTable->queryEntity(pdbi_, info.dbid, entitylog, pModule->getUType());

	// 如果有在线纪录
	if(!success_)
	{
		componentID_ = entitylog.componentID;
		entityID_ = entitylog.entityID;
	}

	dbid_ = info.dbid;
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskAccountLogin::presentMainThread()
{
	DEBUG_MSG("Dbmgr::onAccountLogin:%s.\n", accountName_.c_str());

	// 一个用户登录， 构造一个数据库查询指令并加入到执行队列， 执行完毕将结果返回给loginapp
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::onLoginAccountQueryResultFromDbmgr);

	(*pBundle) << success_;
	(*pBundle) << accountName_;
	(*pBundle) << password_;
	(*pBundle) << componentID_;   // 如果大于0则表示账号还存活在某个baseapp上
	(*pBundle) << entityID_;
	(*pBundle) << dbid_;

	if(!this->send((*pBundle)))
	{
		ERROR_MSG("DBTaskAccountLogin::presentMainThread: channel(%s) not found.\n", addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
DBTaskQueryEntity::DBTaskQueryEntity(const Mercury::Address& addr, std::string& entityType, DBID dbid, 
		COMPONENT_ID componentID, CALLBACK_ID callbackID, ENTITY_ID entityID):
EntityDBTask(addr, entityID, dbid),
entityType_(entityType),
dbid_(dbid),
componentID_(componentID),
callbackID_(callbackID),
success_(false),
s_(),
entityID_(entityID),
wasActive_(false)
{
}

//-------------------------------------------------------------------------------------
DBTaskQueryEntity::~DBTaskQueryEntity()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskQueryEntity::db_thread_process()
{
	ScriptDefModule* pModule = EntityDef::findScriptModule(entityType_.c_str());
	success_ = EntityTables::getSingleton().queryEntity(pdbi_, dbid_, &s_, pModule);

	if(success_)
	{
		// 先写log， 如果写失败则可能这个entity已经在线
		KBEEntityLogTable* pELTable = static_cast<KBEEntityLogTable*>
						(EntityTables::getSingleton().findKBETable("kbe_entitylog"));
		KBE_ASSERT(pELTable);

		success_ = pELTable->logEntity(pdbi_, addr_.ipAsString(), addr_.port, dbid_, 
			componentID_, entityID_, pModule->getUType());

		if(!success_)
			wasActive_ = true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskQueryEntity::presentMainThread()
{
	DEBUG_MSG("Dbmgr::DBTaskQueryEntity:%s.\n", entityType_.c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	pBundle->newMessage(BaseappInterface::onCreateBaseFromDBIDCallback);
	(*pBundle) << entityType_;
	(*pBundle) << dbid_;
	(*pBundle) << callbackID_;
	(*pBundle) << success_;
	(*pBundle) << entityID_;
	(*pBundle) << wasActive_;

	if(success_)
	{
		pBundle->append(s_);
	}

	if(!this->send((*pBundle)))
	{
		ERROR_MSG("DBTaskQueryAccount::presentMainThread: channel(%s) not found.\n", addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	EntityDBTask::presentMainThread();
}

//-------------------------------------------------------------------------------------
}
