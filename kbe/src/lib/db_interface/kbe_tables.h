// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_KBE_TABLES_H
#define KBE_KBE_TABLES_H

#include "entity_table.h"
#include "common/common.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"

namespace KBEngine { 

class KBETable : public EntityTable
{
public:
	KBETable(EntityTables* pEntityTables) :
	EntityTable(pEntityTables)
	{
	}
	
	virtual ~KBETable()
	{
	}
	
	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi) = 0;
	
	/**
		初始化
	*/
	virtual bool initialize(ScriptDefModule* sm, std::string name) { return true; };
	
	virtual EntityTableItem* createItem(std::string type, std::string defaultVal) { return NULL; }
	
protected:

};

/*
	kbe系统表
*/
class KBEEntityLogTable : public KBETable
{
public:
	struct EntityLog
	{
		DBID dbid;
		ENTITY_ID entityID;
		char ip[MAX_IP];
		uint16 port;
		COMPONENT_ID componentID;

		// 由谁记录
		COMPONENT_ID serverGroupID;
	};

	KBEEntityLogTable(EntityTables* pEntityTables) :
	KBETable(pEntityTables)
	{
		tableName(KBE_TABLE_PERFIX "_entitylog");
	}
	
	virtual ~KBEEntityLogTable()
	{
	}
	
	virtual bool logEntity(DBInterface * pdbi, const char* ip, uint32 port, DBID dbid,
						COMPONENT_ID componentID, ENTITY_ID entityID, ENTITY_SCRIPT_UID entityType) = 0;

	virtual bool queryEntity(DBInterface * pdbi, DBID dbid, EntityLog& entitylog, ENTITY_SCRIPT_UID entityType) = 0;

	virtual bool eraseEntityLog(DBInterface * pdbi, DBID dbid, ENTITY_SCRIPT_UID entityType) = 0;
	virtual bool eraseBaseappEntityLog(DBInterface * pdbi, COMPONENT_ID componentID) = 0;

protected:
	
};

/*
	kbe系统表
*/
class KBEServerLogTable : public KBETable
{
public:
	const static uint32 TIMEOUT = 3600;

	struct ServerLog
	{
		uint64 heartbeatTime;

		// 由谁记录
		COMPONENT_ID serverGroupID;

		uint8 isShareDB;
	};

	KBEServerLogTable(EntityTables* pEntityTables) :
	KBETable(pEntityTables)
	{
		tableName(KBE_TABLE_PERFIX "_serverlog");
	}
	
	virtual ~KBEServerLogTable()
	{
	}
	
	virtual bool updateServer(DBInterface * pdbi) = 0;

	virtual bool queryServer(DBInterface * pdbi, ServerLog& serverlog) = 0;
	virtual std::vector<COMPONENT_ID> queryServers(DBInterface * pdbi) = 0;

	virtual std::vector<COMPONENT_ID> queryTimeOutServers(DBInterface * pdbi) = 0;

	virtual bool clearServers(DBInterface * pdbi, const std::vector<COMPONENT_ID>& cids) = 0;
	
protected:
	
};

class KBEAccountTable : public KBETable
{
public:
	KBEAccountTable(EntityTables* pEntityTables) :
	KBETable(pEntityTables),
	accountDefMemoryStream_()
	{
		tableName(KBE_TABLE_PERFIX "_accountinfos");
	}
	
	virtual ~KBEAccountTable()
	{
	}

	virtual bool queryAccount(DBInterface * pdbi, const std::string& name, ACCOUNT_INFOS& info) = 0;
	virtual bool logAccount(DBInterface * pdbi, ACCOUNT_INFOS& info) = 0;
	virtual bool setFlagsDeadline(DBInterface * pdbi, const std::string& name, uint32 flags, uint64 deadline) = 0;
	virtual bool updateCount(DBInterface * pdbi, const std::string& name, DBID dbid) = 0;
	virtual bool queryAccountAllInfos(DBInterface * pdbi, const std::string& name, ACCOUNT_INFOS& info) = 0;
	virtual bool updatePassword(DBInterface * pdbi, const std::string& name, const std::string& password) = 0;

	MemoryStream& accountDefMemoryStream()
	{ 
		return accountDefMemoryStream_; 
	}

	void accountDefMemoryStream(MemoryStream& s)
	{
		accountDefMemoryStream_.clear(false);
		accountDefMemoryStream_.append(s.data() + s.rpos(), s.length()); 
	}

protected:
	MemoryStream accountDefMemoryStream_;
};

class KBEEmailVerificationTable : public KBETable
{
public:
	enum V_TYPE
	{
		V_TYPE_CREATEACCOUNT = 1,
		V_TYPE_RESETPASSWORD = 2,
		V_TYPE_BIND_MAIL = 3
	};

	KBEEmailVerificationTable(EntityTables* pEntityTables) :
	KBETable(pEntityTables)
	{
		tableName(KBE_TABLE_PERFIX "_email_verification");
	}
	
	virtual ~KBEEmailVerificationTable()
	{
	}

	virtual bool queryAccount(DBInterface * pdbi, int8 type, const std::string& name, ACCOUNT_INFOS& info) = 0;
	virtual bool logAccount(DBInterface * pdbi, int8 type, const std::string& name, const std::string& datas, const std::string& code) = 0;
	virtual bool delAccount(DBInterface * pdbi, int8 type, const std::string& name) = 0;
	virtual bool activateAccount(DBInterface * pdbi, const std::string& code, ACCOUNT_INFOS& info) = 0;
	virtual bool bindEMail(DBInterface * pdbi, const std::string& name, const std::string& code) = 0;
	virtual bool resetpassword(DBInterface * pdbi, const std::string& name, const std::string& password, const std::string& code) = 0;

protected:
};

}

#endif // KBE_KBE_TABLES_H
