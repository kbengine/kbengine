#include "entity_table_mongodb.h"
#include "kbe_table_mongodb.h"
#include "db_exception.h"
#include "db_interface_mongodb.h"
#include "db_interface/db_interface.h"
#include "db_interface/entity_table.h"
#include "entitydef/entitydef.h"
#include "entitydef/scriptdef_module.h"
#include "server/serverconfig.h"

namespace KBEngine {

	bool KBEEntityLogTableMongodb::syncToDB(DBInterface* pdbi)
	{
		return true;
	}

	bool KBEEntityLogTableMongodb::logEntity(DBInterface * pdbi, const char* ip, uint32 port, DBID dbid,
		COMPONENT_ID componentID, ENTITY_ID entityID, ENTITY_SCRIPT_UID entityType)
	{
		return true;
	}

	bool KBEEntityLogTableMongodb::queryEntity(DBInterface * pdbi, DBID dbid, EntityLog& entitylog, ENTITY_SCRIPT_UID entityType)
	{
		return true;
	}

	bool KBEEntityLogTableMongodb::eraseEntityLog(DBInterface * pdbi, DBID dbid, ENTITY_SCRIPT_UID entityType)
	{
		std::string sqlstr = fmt::format("HDEL kbe_entitylog:{}:{}",
			dbid, entityType);

		pdbi->query(sqlstr.c_str(), sqlstr.size(), false);
		return true;
	}

	KBEEntityLogTableMongodb::KBEEntityLogTableMongodb(EntityTables* pEntityTables) :
		KBEEntityLogTable(pEntityTables)
	{
	}

	bool KBEAccountTableMongodb::syncToDB(DBInterface* pdbi)
	{
		return true;
	}

	KBEAccountTableMongodb::KBEAccountTableMongodb(EntityTables* pEntityTables) :
		KBEAccountTable(pEntityTables)
	{
	}

	bool KBEAccountTableMongodb::setFlagsDeadline(DBInterface * pdbi, const std::string& name, uint32 flags, uint64 deadline)
	{
		// 如果查询失败则返回存在， 避免可能产生的错误
		if (pdbi->query(fmt::format("HSET kbe_accountinfos:{} flags {} deadline {}",
			name, flags, deadline), false))
			return true;

		return false;
	}

	bool KBEAccountTableMongodb::queryAccount(DBInterface * pdbi, const std::string& name, ACCOUNT_INFOS& info)
	{
		return info.dbid > 0;
	}

	bool KBEAccountTableMongodb::queryAccountAllInfos(DBInterface * pdbi, const std::string& name, ACCOUNT_INFOS& info)
	{
		return info.dbid > 0;
	}

	bool KBEAccountTableMongodb::updateCount(DBInterface * pdbi, const std::string& name, DBID dbid)
	{
		return true;
	}

	bool KBEAccountTableMongodb::updatePassword(DBInterface * pdbi, const std::string& name, const std::string& password)
	{
		return true;
	}


	bool KBEAccountTableMongodb::logAccount(DBInterface * pdbi, ACCOUNT_INFOS& info)
	{
		return true;
	}

	KBEEmailVerificationTableMongodb::KBEEmailVerificationTableMongodb(EntityTables* pEntityTables) :
		KBEEmailVerificationTable(pEntityTables)
	{

	}

	KBEEmailVerificationTableMongodb::~KBEEmailVerificationTableMongodb()
	{

	}

	bool KBEEmailVerificationTableMongodb::syncToDB(DBInterface* pdbi)
	{
		return true;
	}

	bool KBEEmailVerificationTableMongodb::queryAccount(DBInterface * pdbi, int8 type, const std::string& name, ACCOUNT_INFOS& info)
	{
		return true;
	}

	bool KBEEmailVerificationTableMongodb::logAccount(DBInterface * pdbi, int8 type, const std::string& name, const std::string& datas, const std::string& code)
	{
		return true;
	}

	bool KBEEmailVerificationTableMongodb::delAccount(DBInterface * pdbi, int8 type, const std::string& name)
	{
		return true;
	}

	bool KBEEmailVerificationTableMongodb::activateAccount(DBInterface * pdbi, const std::string& code, ACCOUNT_INFOS& info)
	{
		return true;
	}

	bool KBEEmailVerificationTableMongodb::bindEMail(DBInterface * pdbi, const std::string& name, const std::string& code)
	{
		return true;
	}

	bool KBEEmailVerificationTableMongodb::resetpassword(DBInterface * pdbi, const std::string& name, const std::string& password, const std::string& code)
	{
		return true;
	}
}