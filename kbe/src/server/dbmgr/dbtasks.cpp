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
#include "network/common.hpp"
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

	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();
	pdbi_ = DBUtil::create(dbcfg.db_type, dbcfg.db_ip, dbcfg.db_port, 
		dbcfg.db_username, dbcfg.db_password, dbcfg.db_numConnections);

	if(pdbi_ == NULL)
	{
		ERROR_MSG("DBTask::process: can't create dbinterface!\n");
		return false;
	}

	if(!pdbi_->attach(dbcfg.db_name))
	{
		ERROR_MSG("DBTask::process: can't attach to database! %s.\n", pdbi_->c_str());
		return false;
	}
	else
	{
		INFO_MSG("DBTask::process: %s\n", pdbi_->c_str());
	}

	bool ret = db_thread_process();
	if(!ret)
		mysql_thread_end();

	if(pdbi_)
		pdbi_->detach();
	
	pdbi_ = NULL;
	return ret;
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
DBTaskWriteEntity::DBTaskWriteEntity(const Mercury::Address& addr, MemoryStream& datas):
DBTask(addr, datas),
eid_(0),
entityDBID_(0),
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
	(*pDatas_) >> eid_ >> entityDBID_ >> sid_ >> callbackID_;

	ScriptDefModule* pModule = EntityDef::findScriptModule(sid_);
	entityDBID_ = EntityTables::getSingleton().writeEntity(pdbi_, entityDBID_, pDatas_, pModule);
	success_ = entityDBID_ > 0;
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskWriteEntity::presentMainThread()
{
	ScriptDefModule* pModule = EntityDef::findScriptModule(sid_);
	DEBUG_MSG("Dbmgr::writeEntity: %s(%"PRIu64"), size=%u.\n", pModule->getName(), entityDBID_, (*pDatas_).opsize());

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
}

//-------------------------------------------------------------------------------------
DBTaskCreateAccount::DBTaskCreateAccount(const Mercury::Address& addr, std::string& accountName, std::string& password):
DBTask(addr),
accountName_(accountName),
password_(password)
{
}

//-------------------------------------------------------------------------------------
DBTaskCreateAccount::~DBTaskCreateAccount()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskCreateAccount::db_thread_process()
{
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskCreateAccount::presentMainThread()
{
	DEBUG_MSG("Dbmgr::reqCreateAccount:%s.\n", accountName_.c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::onReqCreateAccountResult);
	SERVER_ERROR_CODE failedcode = SERVER_SUCCESS;

	// 如果没有连接db则从log中查找账号是否有此账号(这个功能是为了测试使用)
	/*
	if(!pdbi_)
	{
		PROXICES_ONLINE_LOG::iterator iter = proxicesOnlineLogs_.find(accountName);
		if(iter != proxicesOnlineLogs_.end())
		{
			failedcode = SERVER_ERR_ACCOUNT_CREATE;
		}
	}
	*/

	LoginappInterface::onReqCreateAccountResultArgs3::staticAddToBundle((*pBundle), failedcode, accountName_, password_);

	if(!this->send((*pBundle)))
	{
		ERROR_MSG("DBTaskCreateAccount::presentMainThread: channel(%s) not found.\n", addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
DBTaskQueryAccount::DBTaskQueryAccount(const Mercury::Address& addr, std::string& accountName, std::string& password):
DBTask(addr),
accountName_(accountName),
password_(password)
{
}

//-------------------------------------------------------------------------------------
DBTaskQueryAccount::~DBTaskQueryAccount()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskQueryAccount::db_thread_process()
{
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
	(*pBundle) << "";

	if(!this->send((*pBundle)))
	{
		ERROR_MSG("DBTaskQueryAccount::presentMainThread: channel(%s) not found.\n", addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
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
DBTaskAccountOffline::DBTaskAccountOffline(const Mercury::Address& addr, std::string& accountName):
DBTask(addr),
accountName_(accountName)
{
}

//-------------------------------------------------------------------------------------
DBTaskAccountOffline::~DBTaskAccountOffline()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskAccountOffline::db_thread_process()
{
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskAccountOffline::presentMainThread()
{
	DEBUG_MSG("Dbmgr::onAccountOffline:%s.\n", accountName_.c_str());
	
	/*
	// 如果没有连接db则从log中查找账号是否还在线
	if(!pDBInterface_)
	{
		proxicesOnlineLogs_.erase(accountName);
	}
	*/
}

//-------------------------------------------------------------------------------------
DBTaskAccountLogin::DBTaskAccountLogin(const Mercury::Address& addr, std::string& accountName, std::string& password):
DBTask(addr),
accountName_(accountName),
password_(password)
{
}

//-------------------------------------------------------------------------------------
DBTaskAccountLogin::~DBTaskAccountLogin()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskAccountLogin::db_thread_process()
{
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskAccountLogin::presentMainThread()
{
	DEBUG_MSG("Dbmgr::onAccountLogin:%s.\n", accountName_.c_str());

	// 一个用户登录， 构造一个数据库查询指令并加入到执行队列， 执行完毕将结果返回给loginapp
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::onLoginAccountQueryResultFromDbmgr);

	bool success = true;
	COMPONENT_ID componentID = 0;
	ENTITY_ID entityID = 0;
	
	/*
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
	*/

	(*pBundle) << success;
	(*pBundle) << accountName_;
	(*pBundle) << password_;
	(*pBundle) << componentID;   // 如果大于0则表示账号还存活在某个baseapp上
	(*pBundle) << entityID;

	if(!this->send((*pBundle)))
	{
		ERROR_MSG("DBTaskAccountLogin::presentMainThread: channel(%s) not found.\n", addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
DBTaskQueryEntity::DBTaskQueryEntity(const Mercury::Address& addr, std::string& entityType, DBID dbid, 
		COMPONENT_ID componentID, CALLBACK_ID callbackID):
DBTask(addr),
entityType_(entityType),
dbid_(dbid),
componentID_(componentID),
callbackID_(callbackID),
success_(false),
s_()
{
}

//-------------------------------------------------------------------------------------
DBTaskQueryEntity::~DBTaskQueryEntity()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskQueryEntity::db_thread_process()
{
	EntityTables::getSingleton().queryEntity(pdbi_, dbid_, &s_, EntityDef::findScriptModule(entityType_.c_str()));
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskQueryEntity::presentMainThread()
{
	DEBUG_MSG("Dbmgr::DBTaskQueryEntity:%s.\n", entityType_.c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	pBundle->append(s_);
	if(!this->send((*pBundle)))
	{
		ERROR_MSG("DBTaskQueryAccount::presentMainThread: channel(%s) not found.\n", addr_.c_str());
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
}
