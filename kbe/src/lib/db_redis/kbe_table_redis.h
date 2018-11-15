// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_KBE_TABLE_REDIS_H
#define KBE_KBE_TABLE_REDIS_H

#include "common.h"
#include "common/common.h"
#include "common/singleton.h"
#include "helper/debug_helper.h"
#include "db_interface/entity_table.h"
#include "db_interface/kbe_tables.h"

namespace KBEngine { 

/*
	kbe系统表
*/
class KBEEntityLogTableRedis : public KBEEntityLogTable
{
public:
	KBEEntityLogTableRedis(EntityTables* pEntityTables);
	virtual ~KBEEntityLogTableRedis(){}
	
	/**
		同步表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi);
	virtual bool syncIndexToDB(DBInterface* pdbi){ return true; }

	virtual bool logEntity(DBInterface * pdbi, const char* ip, uint32 port, DBID dbid,
						COMPONENT_ID componentID, ENTITY_ID entityID, ENTITY_SCRIPT_UID entityType);

	virtual bool queryEntity(DBInterface * pdbi, DBID dbid, EntityLog& entitylog, ENTITY_SCRIPT_UID entityType);

	virtual bool eraseEntityLog(DBInterface * pdbi, DBID dbid, ENTITY_SCRIPT_UID entityType);
	virtual bool eraseBaseappEntityLog(DBInterface * pdbi, COMPONENT_ID componentID);

protected:
	
};

class KBEServerLogTableRedis : public KBEServerLogTable
{
public:
	KBEServerLogTableRedis(EntityTables* pEntityTables);
	virtual ~KBEServerLogTableRedis(){}
	
	/**
		同步表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi);
	virtual bool syncIndexToDB(DBInterface* pdbi){ return true; }

	virtual bool updateServer(DBInterface * pdbi);

	virtual bool queryServer(DBInterface * pdbi, ServerLog& serverlog);
	virtual std::vector<COMPONENT_ID> queryServers(DBInterface * pdbi);

	virtual std::vector<COMPONENT_ID> queryTimeOutServers(DBInterface * pdbi);
	
	virtual bool clearServers(DBInterface * pdbi, const std::vector<COMPONENT_ID>& cids);
	
protected:
	
};

class KBEAccountTableRedis : public KBEAccountTable
{
public:
	KBEAccountTableRedis(EntityTables* pEntityTables);
	virtual ~KBEAccountTableRedis(){}
	
	/**
		同步表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi);
	virtual bool syncIndexToDB(DBInterface* pdbi){ return true; }

	bool queryAccount(DBInterface * pdbi, const std::string& name, ACCOUNT_INFOS& info);
	bool queryAccountAllInfos(DBInterface * pdbi, const std::string& name, ACCOUNT_INFOS& info);
	bool logAccount(DBInterface * pdbi, ACCOUNT_INFOS& info);
	bool setFlagsDeadline(DBInterface * pdbi, const std::string& name, uint32 flags, uint64 deadline);
	virtual bool updateCount(DBInterface * pdbi, const std::string& name, DBID dbid);
	virtual bool updatePassword(DBInterface * pdbi, const std::string& name, const std::string& password);
protected:
};

class KBEEmailVerificationTableRedis : public KBEEmailVerificationTable
{
public:

	KBEEmailVerificationTableRedis(EntityTables* pEntityTables);
	virtual ~KBEEmailVerificationTableRedis();

	/**
		同步表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi);
	virtual bool syncIndexToDB(DBInterface* pdbi){ return true; }

	virtual bool queryAccount(DBInterface * pdbi, int8 type, const std::string& name, ACCOUNT_INFOS& info);
	virtual bool logAccount(DBInterface * pdbi, int8 type, const std::string& name, const std::string& datas, const std::string& code);
	virtual bool delAccount(DBInterface * pdbi, int8 type, const std::string& name);
	virtual bool activateAccount(DBInterface * pdbi, const std::string& code, ACCOUNT_INFOS& info);
	virtual bool bindEMail(DBInterface * pdbi, const std::string& name, const std::string& code);
	virtual bool resetpassword(DBInterface * pdbi, const std::string& name, 
		const std::string& password, const std::string& code);

	int getDeadline(int8 type);
	
protected:
};

}

#endif // KBE_KBE_TABLE_REDIS_H
