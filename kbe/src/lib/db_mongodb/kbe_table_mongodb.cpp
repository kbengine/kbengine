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
		//创建表
		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		if (!pdbiMongodb->createCollection("kbe_entitylog"))
		{
			//存在表格，要清除里面的所有数据
			bson_t options;
			bson_init(&options);

			DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
			pdbiMongodb->collectionRemove("kbe_entitylog", MONGOC_REMOVE_NONE, &options, NULL);

			bson_destroy(&options);
		}
		return true;
	}

	bool KBEEntityLogTableMongodb::logEntity(DBInterface * pdbi, const char* ip, uint32 port, DBID dbid,
		COMPONENT_ID componentID, ENTITY_ID entityID, ENTITY_SCRIPT_UID entityType)
	{
		bson_t options;
		bson_init(&options);
		BSON_APPEND_INT64(&options, "entityDBID", dbid);
		BSON_APPEND_INT32(&options, "entityType", entityType);
		BSON_APPEND_INT32(&options, "entityID", entityID);
		BSON_APPEND_UTF8(&options, "ip", ip);
		BSON_APPEND_INT32(&options, "port", port);
		BSON_APPEND_INT64(&options, "componentID", componentID);

		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		pdbiMongodb->insertCollection("kbe_entitylog", MONGOC_INSERT_NONE, &options, NULL);

		bson_destroy(&options);

		return true;
	}

	bool KBEEntityLogTableMongodb::queryEntity(DBInterface * pdbi, DBID dbid, EntityLog& entitylog, ENTITY_SCRIPT_UID entityType)
	{
		bson_t query;
		bson_init(&query);
		BSON_APPEND_INT64(&query, "entityDBID", dbid);

		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		//const std::list<const bson_t *> value = pdbiMongodb->collectionFind("kbe_entitylog", &query);

		mongoc_cursor_t * cursor = pdbiMongodb->collectionFind("kbe_entitylog", MONGOC_QUERY_NONE, 0, 0, 0, &query, NULL, NULL);

		entitylog.dbid = dbid;
		entitylog.componentID = 0;
		entitylog.entityID = 0;
		entitylog.ip[0] = '\0';
		entitylog.port = 0;

		std::list<const bson_t *> value;
		const bson_t *doc;
		bson_error_t  error;
		while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &doc)) {
			value.push_back(doc);
		}

		if (mongoc_cursor_error(cursor, &error)) {
			ERROR_MSG(fmt::format("An error occurred: {}\n", error.message));
		}

		if (value.size() == 0)
		{
			mongoc_cursor_destroy(cursor);
			return false;
		}

		bson_iter_t iter;
		bson_iter_init(&iter, value.front());

		if (bson_iter_find(&iter, "entityID"))
		{
			entitylog.entityID = bson_iter_int32(&iter);
		}

		if (bson_iter_find(&iter, "ip"))
		{
			uint32_t len = 0;
			const char * ip = bson_iter_utf8(&iter, &len);
			kbe_snprintf(entitylog.ip, MAX_IP, "%s", ip);
		}

		if (bson_iter_find(&iter, "port"))
		{

			entitylog.port = bson_iter_int32(&iter);
		}

		if (bson_iter_find(&iter, "componentID"))
		{
			entitylog.componentID = bson_iter_int64(&iter);
		}

		mongoc_cursor_destroy(cursor);
		return true;
	}

	bool KBEEntityLogTableMongodb::eraseEntityLog(DBInterface * pdbi, DBID dbid, ENTITY_SCRIPT_UID entityType)
	{
		bool r = true;
		bson_t doc;
		bson_init(&doc);
		BSON_APPEND_INT64(&doc, "entityDBID", dbid);
		

		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		if (pdbiMongodb->collectionRemove("kbe_entitylog", MONGOC_REMOVE_SINGLE_REMOVE, &doc, NULL))
			r = false;

		return r;
	}

	KBEEntityLogTableMongodb::KBEEntityLogTableMongodb(EntityTables* pEntityTables) :
		KBEEntityLogTable(pEntityTables)
	{
	}

	bool KBEAccountTableMongodb::syncToDB(DBInterface* pdbi)
	{
		//创建表
		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		pdbiMongodb->createCollection("kbe_accountinfos");
		return true;
	}

	KBEAccountTableMongodb::KBEAccountTableMongodb(EntityTables* pEntityTables) :
		KBEAccountTable(pEntityTables)
	{
	}

	bool KBEAccountTableMongodb::setFlagsDeadline(DBInterface * pdbi, const std::string& name, uint32 flags, uint64 deadline)
	{
		bson_t doc;
		bson_init(&doc);
		BSON_APPEND_INT32(&doc, "flags", flags);
		BSON_APPEND_INT64(&doc, "deadline", deadline);

		bson_t query;
		bson_init(&query);
		std::string accountName = "accountName";
		bson_append_utf8(&query, accountName.c_str(), accountName.size(), name.c_str(), name.size());

		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		pdbiMongodb->updateCollection("kbe_accountinfos", MONGOC_UPDATE_NONE, &query, &doc, NULL);

		bson_destroy(&query);
		bson_destroy(&doc);

		return true;
	}

	bool KBEAccountTableMongodb::queryAccount(DBInterface * pdbi, const std::string& name, ACCOUNT_INFOS& info)
	{
		bson_t query;
		bson_init(&query);
		std::string accountName = "accountName";
		bson_append_utf8(&query, accountName.c_str(), accountName.size(), name.c_str(), name.size());

		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		mongoc_cursor_t * cursor = pdbiMongodb->collectionFind("kbe_accountinfos", MONGOC_QUERY_NONE, 0, 0, 0, &query, NULL, NULL);

		std::list<const bson_t *> value;
		const bson_t *doc;
		bson_error_t  error;
		while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &doc)) {
			value.push_back(doc);
		}

		if (mongoc_cursor_error(cursor, &error)) {
			ERROR_MSG(fmt::format("An error occurred: {}\n", error.message));
		}

		if (value.size() == 0)
		{
			mongoc_cursor_destroy(cursor);
			return false;
		}

		bson_iter_t iter;
		bson_iter_init(&iter, value.front());

		info.name = name;

		if (bson_iter_find(&iter, "password"))
		{
			uint32_t len = 0;
			const char * password = bson_iter_utf8(&iter, &len);
			info.password = std::string(password, len);
		}

		if (bson_iter_find(&iter, "entityDBID"))
		{
			info.dbid = bson_iter_int64(&iter);
		}

		if (bson_iter_find(&iter, "flags"))
		{
			info.flags = bson_iter_int32(&iter);
		}

		if (bson_iter_find(&iter, "deadline"))
		{
			info.deadline = bson_iter_int64(&iter);
		}

		mongoc_cursor_destroy(cursor);
		return info.dbid > 0;
	}

	bool KBEAccountTableMongodb::queryAccountAllInfos(DBInterface * pdbi, const std::string& name, ACCOUNT_INFOS& info)
	{
		bson_t query;
		bson_init(&query);
		std::string accountName = "accountName";
		bson_append_utf8(&query, accountName.c_str(), accountName.size(), name.c_str(), name.size());

		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		mongoc_cursor_t * cursor = pdbiMongodb->collectionFind("kbe_accountinfos", MONGOC_QUERY_NONE, 0, 0, 0, &query, NULL, NULL);

		std::list<const bson_t *> value;
		const bson_t *doc;
		bson_error_t  error;
		while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &doc)) {
			value.push_back(doc);
		}

		if (mongoc_cursor_error(cursor, &error)) {
			ERROR_MSG(fmt::format("An error occurred: {}\n", error.message));
		}

		if (value.size() == 0)
		{
			mongoc_cursor_destroy(cursor);
			return false;
		}

		bson_iter_t iter;
		bson_iter_init(&iter, value.front());

		info.name = name;

		if (bson_iter_find(&iter, "password"))
		{
			uint32_t len = 0;
			const char * password = bson_iter_utf8(&iter, &len);
			info.password = std::string(password, len);
		}

		if (bson_iter_find(&iter, "email"))
		{
			uint32_t len = 0;
			const char * email = bson_iter_utf8(&iter, &len);
			info.email = std::string(email, len);
		}

		if (bson_iter_find(&iter, "entityDBID"))
		{
			info.dbid = bson_iter_int64(&iter);
		}

		if (bson_iter_find(&iter, "flags"))
		{
			info.flags = bson_iter_int32(&iter);
		}

		if (bson_iter_find(&iter, "deadline"))
		{
			info.deadline = bson_iter_int64(&iter);
		}

		mongoc_cursor_destroy(cursor);
		return info.dbid > 0;
	}

	bool KBEAccountTableMongodb::updateCount(DBInterface * pdbi, const std::string& name, DBID dbid)
	{
		bson_t *doc = bson_new();
		bson_t child;
		bson_append_document_begin(doc, "$inc", -1, &child);
		BSON_APPEND_INT32(&child, "numlogin", 1);
		bson_append_document_end(doc, &child);
		
		bson_t query;
		bson_init(&query);
		BSON_APPEND_INT64(&query, "entityDBID", dbid);

		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		pdbiMongodb->updateCollection("kbe_accountinfos", MONGOC_UPDATE_NONE, &query, doc, NULL);

		bson_destroy(&query);
		bson_destroy(doc);

		return true;
	}

	bool KBEAccountTableMongodb::updatePassword(DBInterface * pdbi, const std::string& name, const std::string& password)
	{
		bson_t doc;
		bson_init(&doc);
		bson_t child;
		bson_append_document_begin(&doc, "$set", -1, &child);
		bson_append_utf8(&child, "password", (int)strlen("password"), password.c_str(), password.size());
		bson_append_document_end(&doc, &child);

		bson_t query;
		bson_init(&query);
		bson_append_utf8(&query, "entityDBID", (int)strlen("entityDBID"), name.c_str(), name.size());

		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		pdbiMongodb->updateCollection("kbe_accountinfos", MONGOC_UPDATE_NONE, &query, &doc, NULL);

		bson_destroy(&query);
		bson_destroy(&doc);

		return true;
	}


	bool KBEAccountTableMongodb::logAccount(DBInterface * pdbi, ACCOUNT_INFOS& info)
	{
		bson_t options;
		bson_init(&options);
		bson_append_utf8(&options, "accountName", (int)strlen("accountName"), info.name.c_str(), info.name.size());
		std::string password = KBE_MD5::getDigest(info.password.data(), info.password.length());
		bson_append_utf8(&options, "password", (int)strlen("password"), password.c_str(), password.size());
		bson_append_utf8(&options, "bindata", (int)strlen("bindata"), info.datas.data(), info.datas.size());
		bson_append_utf8(&options, "email", (int)strlen("email"), info.email.c_str(), info.email.size());
		BSON_APPEND_INT64(&options, "entityDBID", info.dbid);
		BSON_APPEND_INT32(&options, "flags", info.flags);
		BSON_APPEND_INT64(&options, "deadline", info.deadline);
		BSON_APPEND_INT64(&options, "regtime", time(NULL));
		BSON_APPEND_INT64(&options, "lasttime", time(NULL));
		BSON_APPEND_INT32(&options, "numlogin", 0);

		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		pdbiMongodb->insertCollection("kbe_accountinfos", MONGOC_INSERT_NONE, &options, NULL);

		bson_destroy(&options);

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
		//创建表
		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		pdbiMongodb->createCollection("kbe_email_verification");
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