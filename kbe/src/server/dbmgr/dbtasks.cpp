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
addr_(addr)
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
bool DBTaskExecuteRawDatabaseCommand::process()
{
	(*pDatas_) >> componentID_ >> componentType_;
	(*pDatas_) >> callbackID_;
	(*pDatas_).readBlob(sdatas_);

	if(!static_cast<DBInterfaceMysql*>(Dbmgr::getSingleton().pDBInterface())->execute(sdatas_.data(), sdatas_.size(), &execret_))
	{
		error_ = Dbmgr::getSingleton().pDBInterface()->getstrerror();
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

	Mercury::Bundle bundle;
	if(componentType_ == BASEAPP_TYPE)
		bundle.newMessage(BaseappInterface::onExecuteRawDatabaseCommandCB);
	else if(componentType_ == CELLAPP_TYPE)
		bundle.newMessage(CellappInterface::onExecuteRawDatabaseCommandCB);
	else
	{
		KBE_ASSERT(false && "no support!\n");
	}

	bundle << callbackID_;
	bundle << error_;
	if(error_.size() <= 0)
		bundle.append(execret_);

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(componentType_, componentID_);

	if(cinfos && cinfos->pChannel)
	{
		bundle.send(Dbmgr::getSingleton().getNetworkInterface(), cinfos->pChannel);
	}
	else
	{
		ERROR_MSG("DBTaskExecuteRawDatabaseCommand::presentMainThread: %s not found.", COMPONENT_NAME_EX(componentType_));
	}
}

//-------------------------------------------------------------------------------------
DBTaskWriteEntity::DBTaskWriteEntity(const Mercury::Address& addr, MemoryStream& datas):
DBTask(addr, datas),
entityDBID_(0),
sid_(0)
{
}

//-------------------------------------------------------------------------------------
DBTaskWriteEntity::~DBTaskWriteEntity()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskWriteEntity::process()
{
	(*pDatas_) >> entityDBID_ >> sid_;

	ScriptDefModule* pModule = EntityDef::findScriptModule(sid_);
	EntityTables::getSingleton().writeEntity(entityDBID_, pDatas_, pModule);
	
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskWriteEntity::presentMainThread()
{
	ScriptDefModule* pModule = EntityDef::findScriptModule(sid_);
	DEBUG_MSG("Dbmgr::writeEntity: %s(%"PRIu64"), size=%u.\n", pModule->getName(), entityDBID_, (*pDatas_).opsize());
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
bool DBTaskCreateAccount::process()
{
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskCreateAccount::presentMainThread()
{
	DEBUG_MSG("Dbmgr::reqCreateAccount:%s.\n", accountName_.c_str());

	Mercury::Bundle bundle;
	bundle.newMessage(LoginappInterface::onReqCreateAccountResult);
	SERVER_ERROR_CODE failedcode = SERVER_SUCCESS;

	// 如果没有连接db则从log中查找账号是否有此账号(这个功能是为了测试使用)
	/*
	if(!Dbmgr::getSingleton().pDBInterface())
	{
		PROXICES_ONLINE_LOG::iterator iter = proxicesOnlineLogs_.find(accountName);
		if(iter != proxicesOnlineLogs_.end())
		{
			failedcode = SERVER_ERR_ACCOUNT_CREATE;
		}
	}
	*/

	LoginappInterface::onReqCreateAccountResultArgs3::staticAddToBundle(bundle, failedcode, accountName_, password_);

	Mercury::Channel* pChannel = Dbmgr::getSingleton().getNetworkInterface().findChannel(addr_);
	
	if(pChannel){
		bundle.send(Dbmgr::getSingleton().getNetworkInterface(), pChannel);
	}
	else{
		ERROR_MSG("DBTaskCreateAccount::presentMainThread: channel(%s) not found.\n", addr_.c_str());
	}
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
bool DBTaskQueryAccount::process()
{
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskQueryAccount::presentMainThread()
{
	DEBUG_MSG("Dbmgr::queryAccount:%s.\n", accountName_.c_str());

	Mercury::Bundle bundle;
	bundle.newMessage(BaseappInterface::onQueryAccountCBFromDbmgr);
	bundle << accountName_;
	bundle << password_;
	bundle << "";

	Mercury::Channel* pChannel = Dbmgr::getSingleton().getNetworkInterface().findChannel(addr_);
	
	if(pChannel){
		bundle.send(Dbmgr::getSingleton().getNetworkInterface(), pChannel);
	}
	else{
		ERROR_MSG("DBTaskQueryAccount::presentMainThread: channel(%s) not found.\n", addr_.c_str());
	}
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
bool DBTaskAccountOnline::process()
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
bool DBTaskAccountOffline::process()
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
bool DBTaskAccountLogin::process()
{
	return false;
}

//-------------------------------------------------------------------------------------
void DBTaskAccountLogin::presentMainThread()
{
	DEBUG_MSG("Dbmgr::onAccountLogin:%s.\n", accountName_.c_str());

	// 一个用户登录， 构造一个数据库查询指令并加入到执行队列， 执行完毕将结果返回给loginapp
	Mercury::Bundle bundle;
	bundle.newMessage(LoginappInterface::onLoginAccountQueryResultFromDbmgr);

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

	bundle << success;
	bundle << accountName_;
	bundle << password_;
	bundle << componentID;   // 如果大于0则表示账号还存活在某个baseapp上
	bundle << entityID;

	Mercury::Channel* pChannel = Dbmgr::getSingleton().getNetworkInterface().findChannel(addr_);

	if(pChannel){
		bundle.send(Dbmgr::getSingleton().getNetworkInterface(), pChannel);
	}
	else{
		ERROR_MSG("DBTaskAccountLogin::presentMainThread: channel(%s) not found.\n", addr_.c_str());
	}
}

//-------------------------------------------------------------------------------------
}
