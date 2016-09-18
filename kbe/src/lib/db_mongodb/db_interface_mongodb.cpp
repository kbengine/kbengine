#include "db_interface_mongodb.h"
#include "entity_table_mongodb.h"
#include "kbe_table_mongodb.h"
#include "db_exception.h"
#include "thread/threadguard.h"
#include "helper/watcher.h"
#include "server/serverconfig.h"

namespace KBEngine {

	static KBEngine::thread::ThreadMutex _g_logMutex;
	static KBEUnordered_map< std::string, uint32 > g_querystatistics;
	static bool _g_installedWatcher = false;
	static bool _g_debug = false;

	static void querystatistics(const char* strCommand, uint32 size)
	{
		std::string op;
		for (uint32 i = 0; i < size; ++i)
		{
			if (strCommand[i] == ' ')
				break;

			op += strCommand[i];
		}

		if (op.size() == 0)
			return;

		std::transform(op.begin(), op.end(), op.begin(), toupper);

		_g_logMutex.lockMutex();

		KBEUnordered_map< std::string, uint32 >::iterator iter = g_querystatistics.find(op);
		if (iter == g_querystatistics.end())
		{
			g_querystatistics[op] = 1;
		}
		else
		{
			iter->second += 1;
		}

		_g_logMutex.unlockMutex();
	}

	static uint32 watcher_query(std::string cmd)
	{
		KBEngine::thread::ThreadGuard tg(&_g_logMutex);

		KBEUnordered_map< std::string, uint32 >::iterator iter = g_querystatistics.find(cmd);
		if (iter != g_querystatistics.end())
		{
			return iter->second;
		}

		return 0;
	}

	static uint32 watcher_select()
	{
		return watcher_query("SELECT");
	}

	static void initializeWatcher()
	{
		if (_g_installedWatcher)
			return;

		_g_installedWatcher = true;
		_g_debug = g_kbeSrvConfig.getDBMgr().debugDBMgr;

		WATCH_OBJECT("db_querys/select", &KBEngine::watcher_select);
	}

	DBInterfaceMongodb::DBInterfaceMongodb(const char* name) :
		DBInterface(name),
		_pMongoClient(NULL),
		hasLostConnection_(false),
		inTransaction_(false),
		lock_(this, false)
	{
	}

	DBInterfaceMongodb::~DBInterfaceMongodb()
	{

	}

	bool DBInterfaceMongodb::initInterface(DBInterface* pdbi)
	{
		EntityTables& entityTables = EntityTables::findByInterfaceName(pdbi->name());

		entityTables.addKBETable(new KBEAccountTableMongodb(&entityTables));
		entityTables.addKBETable(new KBEEntityLogTableMongodb(&entityTables));
		entityTables.addKBETable(new KBEEmailVerificationTableMongodb(&entityTables));
		return true;
	}

	bool DBInterfaceMongodb::attach(const char* databaseName)
	{
		if (!_g_installedWatcher)
		{
			initializeWatcher();
		}

		db_port_ = 27017;
		if (databaseName != NULL)
			kbe_snprintf(db_name_, MAX_BUF, "%s", databaseName);
		else
			kbe_snprintf(db_name_, MAX_BUF, "%s", "0");

		hasLostConnection_ = false;

		mongoc_init();

		_pMongoClient = mongoc_client_new("mongodb://localhost:27017");

		if (!_pMongoClient) {
			fprintf(stderr, "Invalid hostname or port: %u\n", db_port_);
			return false;
		}

		database = mongoc_client_get_database(_pMongoClient, db_name_);

		bson_t *command = BCON_NEW("ping", BCON_INT32(1));

		bson_t reply;
		bson_error_t error;
		bool retval = mongoc_client_command_simple(_pMongoClient, "admin", command, NULL, &reply, &error);

		if (!retval) {
			fprintf(stderr, "%s\n", error.message);
			return false;
		}

		bson_destroy(command);
		bson_destroy(&reply);

		return retval;
	}

	bool DBInterfaceMongodb::checkEnvironment()
	{
		return true;
	}

	bool DBInterfaceMongodb::createDatabaseIfNotExist()
	{
		std::string querycmd = fmt::format("create database {}", db_name_);
		query(querycmd.c_str(), querycmd.size(), false);
		return true;
	}

	bool DBInterfaceMongodb::checkErrors()
	{
		return true;
	}

	/*bool DBInterfaceMongodb::ping(mongoc_client_t* pMongoClient)
	{
	if (!pMongoClient)
	pMongoClient = _pMongoClient;

	if (!pMongoClient)
	return false;

	bson_init(&b);
	bson_append_int32(&b, "ping", 4, 1);
	database = mongoc_client_get_database(pMongoClient, db_name_);
	cursor = mongoc_database_command(database, (mongoc_query_flags_t)0, 0, 1, 0, &b, NULL, NULL);
	if (mongoc_cursor_next(cursor, &reply)) {
	str = bson_as_json(reply, NULL);
	fprintf(stdout, "%s\n", str);
	bson_free(str);
	}
	else if (mongoc_cursor_error(cursor, &error)) {
	fprintf(stderr, "Ping failure: %s\n", error.message);
	return false;
	}

	mongoc_cursor_destroy(cursor);

	return true;
	}*/

	bool DBInterfaceMongodb::reattach()
	{
		detach();

		bool ret = false;

		try
		{
			ret = attach();
		}
		catch (...)
		{
			return false;
		}

		return ret;
	}

	bool DBInterfaceMongodb::detach()
	{
		if (mongo())
		{
			/*bson_destroy(insert);
			bson_destroy(&reply);
			bson_destroy(command);
			bson_free(str);*/

			//mongoc_collection_destroy(collection);
			mongoc_database_destroy(database);
			mongoc_client_destroy(_pMongoClient);
			mongoc_cleanup();
			_pMongoClient = NULL;
		}

		return true;
	}

	EntityTable* DBInterfaceMongodb::createEntityTable(EntityTables* pEntityTables)
	{
		return new EntityTableMongodb(pEntityTables);
	}

	bool DBInterfaceMongodb::dropEntityTableFromDB(const char* tableName)
	{
		KBE_ASSERT(tableName != NULL);

		DEBUG_MSG(fmt::format("DBInterfaceMongodb::dropEntityTableFromDB: {}.\n", tableName));

		return true; char sql_str[MAX_BUF];
		kbe_snprintf(sql_str, MAX_BUF, "Drop table if exists %s;", tableName);
		return query(sql_str, strlen(sql_str));
	}

	bool DBInterfaceMongodb::dropEntityTableItemFromDB(const char* tableName, const char* tableItemName)
	{
		KBE_ASSERT(tableName != NULL && tableItemName != NULL);

		DEBUG_MSG(fmt::format("DBInterfaceMongodb::dropEntityTableItemFromDB: {} {}.\n",
			tableName, tableItemName));

		char sql_str[MAX_BUF];
		kbe_snprintf(sql_str, MAX_BUF, "alter table %s drop column %s;", tableName, tableItemName);
		return query(sql_str, strlen(sql_str));
	}

	bool DBInterfaceMongodb::query(const char* cmd, uint32 size, bool printlog, MemoryStream * result)
	{
		if (_pMongoClient == NULL)
		{
			if (printlog)
			{
				ERROR_MSG(fmt::format("DBInterfaceMongodb::query: has no attach(db).sql:({})\n", lastquery_));
			}

			if (result)
				write_query_result(result);

			return false;
		}

		/*querystatistics(cmd, size);

		lastquery_.assign(cmd, size);*/

		if (_g_debug)
		{
			DEBUG_MSG(fmt::format("DBInterfaceMongodb::query({:p}): {}\n", (void*)this, lastquery_));
		}

		std::string strCommand(cmd);

		int index = strCommand.find_first_of(".");
		std::string str_tableName = strCommand.substr(0, index);

		std::string strQuerCommand = strCommand.substr(index + 1, strCommand.length());

		int k = strQuerCommand.find_first_of(".");
		std::string str_queryType = strQuerCommand.substr(0, k);

		std::string t_command = strQuerCommand.substr(k + 1, strQuerCommand.length());

		bson_error_t error;
		bson_t * bsons = bson_new_from_json((const uint8_t *)t_command.c_str(), t_command.length(), &error);

		return result == NULL || write_query_result(result, bsons, str_tableName.c_str(), str_queryType);
	}

	bool DBInterfaceMongodb::write_query_result(MemoryStream * result, bson_t * bsons, const char *tableName, std::string str_queryType)
	{
		if (result == NULL)
		{
			return true;
		}

		if (bsons == NULL)
		{
			uint32 nfields = 0;
			uint64 affectedRows = 0;

			(*result) << nfields;
			(*result) << affectedRows;

			return true;
		}

		bool resultFlag = true;
		if (str_queryType == "find")
		{
			bson_error_t  error;
			const bson_t *doc;
			mongoc_cursor_t * cursor = collectionFind(tableName, MONGOC_QUERY_NONE,0,0,0, bsons, NULL, NULL);

			uint32 nrows = 0;
			uint32 nfields = 1;

			std::list<const bson_t *> value;
			while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &doc))
			{
				nrows++;
				value.push_back(doc);
			}

			

			(*result) << nfields << nrows;

			std::list<const bson_t *>::iterator it;
			for (it = value.begin(); it != value.end(); it++)
			{
				const bson_t *tem = *it;
				char *str = bson_as_json(tem, NULL);
				result->appendBlob(str, strlen(str));
				bson_free(str);
			}

			if (mongoc_cursor_error(cursor, &error))
			{
				ERROR_MSG(fmt::format("An error occurred: {}\n", error.message));
				resultFlag = false;
			}

			mongoc_cursor_destroy(cursor);
		}
		else if (str_queryType == "delete")
		{
			resultFlag = false;
			collectionRemove(tableName, MONGOC_REMOVE_SINGLE_REMOVE, bsons, NULL);
		}
		else if (str_queryType == "insert")
		{
			resultFlag = false;
			insertCollection(tableName, MONGOC_INSERT_NONE, bsons, NULL);
		}
		else if (str_queryType == "update")
		{
			resultFlag = false;
			bson_t *doc = bson_new();
			updateCollection(tableName, MONGOC_UPDATE_NONE, bsons, doc, NULL);
			bson_destroy(doc);
		}

		if (!resultFlag)
		{
			uint32 nfields = 0;
			uint64 affectedRows = 0;

			(*result) << nfields;
			(*result) << affectedRows;
		}	

		if (bsons != NULL)
		{
			bson_destroy(bsons);
		}

		return true;
	}

	bool DBInterfaceMongodb::getTableNames(std::vector<std::string>& tableNames, const char * pattern)
	{
		if (_pMongoClient == NULL)
		{
			ERROR_MSG("DBInterfaceMysql::query: has no attach(db).\n");
			return false;
		}

		tableNames.clear();

		return true;
	}

	bool DBInterfaceMongodb::getTableItemNames(const char* tableName, std::vector<std::string>& itemNames)
	{
		return true;
	}

	const char* DBInterfaceMongodb::c_str()
	{
		static char strdescr[MAX_BUF];
		kbe_snprintf(strdescr, MAX_BUF, "interface=%s, dbtype=mysql, ip=%s, port=%u, currdatabase=%s, username=%s, connected=%s.\n",
			name_, db_ip_, db_port_, db_name_, db_username_, _pMongoClient == NULL ? "no" : "yes");

		return strdescr;
	}

	const char* DBInterfaceMongodb::getstrerror()
	{
		if (_pMongoClient == NULL)
			return "_pMongoClient is NULL";

		return NULL;
	}

	int DBInterfaceMongodb::getlasterror()
	{
		return 0;
	}

	bool DBInterfaceMongodb::lock()
	{
		lock_.start();
		return true;
	}

	//-------------------------------------------------------------------------------------
	bool DBInterfaceMongodb::unlock()
	{
		lock_.commit();
		lock_.end();
		return true;
	}

	void DBInterfaceMongodb::throwError()
	{
		DBException e(this);

		if (e.isLostConnection())
		{
			this->hasLostConnection(true);
		}

		throw e;
	}

	bool DBInterfaceMongodb::processException(std::exception & e)
	{
		DBException* dbe = static_cast<DBException*>(&e);
		bool retry = false;

		if (dbe->isLostConnection())
		{
			ERROR_MSG(fmt::format("DBInterfaceMongodb::processException: "
				"Thread {:p} lost connection to database. Exception: {}. "
				"Attempting to reconnect.\n",
				(void*)this,
				dbe->what()));

			int attempts = 1;

			while (!this->reattach())
			{
				ERROR_MSG(fmt::format("DBInterfaceMongodb::processException: "
					"Thread {:p} reconnect({}) attempt {} failed({}).\n",
					(void*)this,
					db_name_,
					attempts,
					getstrerror()));

				KBEngine::sleep(30);
				++attempts;
			}

			INFO_MSG(fmt::format("DBInterfaceMongodb::processException: "
				"Thread {:p} reconnected({}). Attempts = {}\n",
				(void*)this,
				db_name_,
				attempts));

			retry = true;
		}
		else if (dbe->shouldRetry())
		{
			WARNING_MSG(fmt::format("DBInterfaceMongodb::processException: Retrying {:p}\nException:{}\nnlastquery={}\n",
				(void*)this, dbe->what(), lastquery_));

			retry = true;
		}
		else
		{
			WARNING_MSG(fmt::format("DBInterfaceMongodb::processException: "
				"Exception: {}\nlastquery={}\n",
				dbe->what(), lastquery_));
		}

		return retry;
	}

	bool DBInterfaceMongodb::createCollection(const char *tableName)
	{
		bson_t options;
		bson_error_t  error;

		//如果存在则离开返回
		if (mongoc_database_has_collection(database, tableName, &error))
			return false;

		//不存在则创建
		bson_init(&options);
		mongoc_collection_t * collection = mongoc_database_create_collection(database, tableName, &options, &error);

		bson_destroy(&options);
		mongoc_collection_destroy(collection);

		return true;
	}

	bool DBInterfaceMongodb::insertCollection(const char *tableName, mongoc_insert_flags_t flags, const bson_t *document, const mongoc_write_concern_t *write_concern)
	{
		bson_error_t  error;
		mongoc_collection_t * collection = mongoc_database_get_collection(database, tableName);
		bool r = mongoc_collection_insert(collection, flags, document, write_concern, &error);
		if (!r) 
		{
			ERROR_MSG(fmt::format("{}\n", error.message));
		}

		mongoc_collection_destroy(collection);

		return r;
	}

	mongoc_cursor_t *  DBInterfaceMongodb::collectionFind(const char *tableName, mongoc_query_flags_t flags, uint32_t skip, uint32_t limit, uint32_t  batch_size, const bson_t *query, const bson_t *fields, const mongoc_read_prefs_t *read_prefs)
	{
		mongoc_collection_t * collection = mongoc_database_get_collection(database, tableName);
		mongoc_cursor_t * cursor = mongoc_collection_find(collection, flags, skip, limit, batch_size, query, fields, read_prefs);

		mongoc_collection_destroy(collection);
		return cursor;
	}

	bool DBInterfaceMongodb::updateCollection(const char *tableName, mongoc_update_flags_t uflags, const bson_t *selector, const bson_t *update, const mongoc_write_concern_t *write_concern)
	{
		bson_error_t  error;
		mongoc_collection_t * collection = mongoc_database_get_collection(database, tableName);
		bool r = mongoc_collection_update(collection, uflags, selector, update, write_concern, &error);
		if (!r)
		{
			ERROR_MSG(fmt::format("{}\n", error.message));
		}

		mongoc_collection_destroy(collection);
		return r;
	}

	bool DBInterfaceMongodb::collectionRemove(const char *tableName, mongoc_remove_flags_t flags, const bson_t *selector, const mongoc_write_concern_t *write_concern)
	{
		bson_error_t  error;
		mongoc_collection_t * collection = mongoc_database_get_collection(database, tableName);
		bool r = mongoc_collection_remove(collection, flags, selector, write_concern, &error);
		if (!r)
		{
			ERROR_MSG("%s\n", error.message);
		}

		mongoc_collection_destroy(collection);
		return r;
	}

	mongoc_cursor_t * DBInterfaceMongodb::collectionFindIndexes(const char *tableName)
	{
		bson_error_t error = { 0 };
		mongoc_collection_t * collection = mongoc_database_get_collection(database, tableName);
		mongoc_cursor_t *cursor = mongoc_collection_find_indexes(collection, &error);

		mongoc_collection_destroy(collection);
		return cursor;
	}

	bool DBInterfaceMongodb::collectionCreateIndex(const char *tableName, const bson_t *keys, const mongoc_index_opt_t *opt)
	{
		mongoc_collection_t * collection = mongoc_database_get_collection(database, tableName);

		bson_error_t error;
		bool r = mongoc_collection_create_index(collection, keys, opt, &error);
		if (!r)
		{
			ERROR_MSG(fmt::format("{}\n", error.message));
		}

		mongoc_collection_destroy(collection);

		return r;
	}

	bool DBInterfaceMongodb::collectionDropIndex(const char *tableName, const char *index_name)
	{
		mongoc_collection_t * collection = mongoc_database_get_collection(database, tableName);

		bson_error_t  error;
		bool r = mongoc_collection_drop_index(collection, index_name, &error);
		if (!r)
		{
			ERROR_MSG(fmt::format("{}\n", error.message));
		}

		mongoc_collection_destroy(collection);

		return r;
	}
}
