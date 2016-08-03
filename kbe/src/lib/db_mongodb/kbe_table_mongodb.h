#pragma once
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
	class KBEEntityLogTableMongodb : public KBEEntityLogTable
	{
	public:
		KBEEntityLogTableMongodb(EntityTables* pEntityTables);
		virtual ~KBEEntityLogTableMongodb(){}

		/**
		同步表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi);
		virtual bool syncIndexToDB(DBInterface* pdbi){ return true; }

		virtual bool logEntity(DBInterface * pdbi, const char* ip, uint32 port, DBID dbid,
			COMPONENT_ID componentID, ENTITY_ID entityID, ENTITY_SCRIPT_UID entityType);

		virtual bool queryEntity(DBInterface * pdbi, DBID dbid, EntityLog& entitylog, ENTITY_SCRIPT_UID entityType);

		virtual bool eraseEntityLog(DBInterface * pdbi, DBID dbid, ENTITY_SCRIPT_UID entityType);
	protected:

	};

	class KBEAccountTableMongodb : public KBEAccountTable
	{
	public:
		KBEAccountTableMongodb(EntityTables* pEntityTables);
		virtual ~KBEAccountTableMongodb(){}

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

	class KBEEmailVerificationTableMongodb : public KBEEmailVerificationTable
	{
	public:

		KBEEmailVerificationTableMongodb(EntityTables* pEntityTables);
		virtual ~KBEEmailVerificationTableMongodb();

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

	protected:
	};

}