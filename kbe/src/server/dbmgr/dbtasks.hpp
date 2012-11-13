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

#ifndef __DBTASKS_H__
#define __DBTASKS_H__

// common include	
// #define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "thread/threadtask.hpp"
#include "helper/debug_helper.hpp"
#include "entitydef/entitydef.hpp"
#include "network/address.hpp"

namespace KBEngine{ 

class DBInterface;

/*
	数据库线程任务基础类
*/

class DBTask : public thread::TPTask
{
public:
	DBTask(const Mercury::Address& addr, MemoryStream& datas);
	DBTask():
	pDatas_(0),
	addr_(Mercury::Address::NONE)
	{
	}

	DBTask(const Mercury::Address& addr):
	pDatas_(0),
	addr_(addr)
	{
	}

	virtual ~DBTask();
	virtual bool process();
	virtual bool db_thread_process() = 0;
	virtual void presentMainThread(){}

	bool send(Mercury::Bundle& bundle);
protected:
	MemoryStream* pDatas_;
	Mercury::Address addr_;
	DBInterface* pdbi_;
};

/**
	执行一条sql语句
*/
class DBTaskExecuteRawDatabaseCommand : public DBTask
{
public:
	DBTaskExecuteRawDatabaseCommand(const Mercury::Address& addr, MemoryStream& datas);
	virtual ~DBTaskExecuteRawDatabaseCommand();
	virtual bool db_thread_process();
	virtual void presentMainThread();
protected:
	COMPONENT_ID componentID_;
	COMPONENT_TYPE componentType_;
	std::string sdatas_;
	CALLBACK_ID callbackID_;
	std::string error_;
	MemoryStream execret_;
};

/**
	向数据库写entity， 备份entity时也是这个机制
*/
class DBTaskWriteEntity : public DBTask
{
public:
	DBTaskWriteEntity(const Mercury::Address& addr, MemoryStream& datas);
	virtual ~DBTaskWriteEntity();
	virtual bool db_thread_process();
	virtual void presentMainThread();
protected:
	ENTITY_ID eid_;
	DBID entityDBID_;
	ENTITY_SCRIPT_UID sid_;
	CALLBACK_ID callbackID_;
	bool success_;
};

/**
	创建一个账号到数据库
*/
class DBTaskCreateAccount : public DBTask
{
public:
	DBTaskCreateAccount(const Mercury::Address& addr, std::string& accountName, std::string& password);
	virtual ~DBTaskCreateAccount();
	virtual bool db_thread_process();
	virtual void presentMainThread();
protected:
	std::string accountName_;
	std::string password_;
	bool success_;
	
};

/**
	baseapp请求查询account信息
*/
class DBTaskQueryAccount : public DBTask
{
public:
	DBTaskQueryAccount(const Mercury::Address& addr, std::string& accountName, std::string& password, 
		COMPONENT_ID componentID, ENTITY_ID entityID);
	virtual ~DBTaskQueryAccount();
	virtual bool db_thread_process();
	virtual void presentMainThread();
protected:
	std::string accountName_;
	std::string password_;
	bool success_;
	MemoryStream s_;
	DBID dbid_;
	COMPONENT_ID componentID_;
	ENTITY_ID entityID_;
};

/**
	账号上线
*/
class DBTaskAccountOnline : public DBTask
{
public:
	DBTaskAccountOnline(const Mercury::Address& addr, std::string& accountName,
		COMPONENT_ID componentID, ENTITY_ID entityID);
	virtual ~DBTaskAccountOnline();
	virtual bool db_thread_process();
	virtual void presentMainThread();
protected:
	std::string accountName_;
	COMPONENT_ID componentID_;
	ENTITY_ID entityID_;
};


/**
	entity下线
*/
class DBTaskEntityOffline : public DBTask
{
public:
	DBTaskEntityOffline(const Mercury::Address& addr, DBID dbid);
	virtual ~DBTaskEntityOffline();
	virtual bool db_thread_process();
	virtual void presentMainThread();
protected:
	DBID dbid_;
};


/**
	一个新用户登录， 需要检查合法性
*/
class DBTaskAccountLogin : public DBTask
{
public:
	DBTaskAccountLogin(const Mercury::Address& addr, std::string& accountName, std::string& password);
	virtual ~DBTaskAccountLogin();
	virtual bool db_thread_process();
	virtual void presentMainThread();
protected:
	std::string accountName_;
	std::string password_;
	bool success_;
	COMPONENT_ID componentID_;
	ENTITY_ID entityID_;
};

/**
	baseapp请求查询entity信息
*/
class DBTaskQueryEntity : public DBTask
{
public:
	DBTaskQueryEntity(const Mercury::Address& addr, std::string& entityType, DBID dbid, 
		COMPONENT_ID componentID, CALLBACK_ID callbackID, ENTITY_ID entityID);

	virtual ~DBTaskQueryEntity();
	virtual bool db_thread_process();
	virtual void presentMainThread();
protected:
	std::string entityType_;
	DBID dbid_;
	COMPONENT_ID componentID_;
	CALLBACK_ID callbackID_;
	bool success_;
	MemoryStream s_;
	ENTITY_ID entityID_;
	bool wasActive_;
};

}
#endif
