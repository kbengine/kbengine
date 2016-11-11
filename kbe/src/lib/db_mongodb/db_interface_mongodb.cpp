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
		database(NULL),
		hasLostConnection_(false),
		inTransaction_(false),
		lock_(this, false),
		strError(NULL)
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

		//db_port_ = 27017;
		if (databaseName != NULL)
			kbe_snprintf(db_name_, MAX_BUF, "%s", databaseName);
		else
			kbe_snprintf(db_name_, MAX_BUF, "%s", "0");

		hasLostConnection_ = false;

		mongoc_init();

		char uri_string[1024];
		kbe_snprintf(uri_string, sizeof(uri_string), "mongodb://%s:%i", db_ip_, db_port_);
		_pMongoClient = mongoc_client_new(uri_string);
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
			bson_destroy(command);
			ERROR_MSG(fmt::format("{}\n", error.message));
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

		return result == NULL || write_query_result(result, cmd);
	}

	bool DBInterfaceMongodb::write_query_result(MemoryStream * result, const char* cmd)
	{
		if (result == NULL)
		{
			return true;
		}

		std::string strCommand(cmd);

		int index = strCommand.find_first_of(".");
		std::string str_tableName = strCommand.substr(0, index);

		std::string strQuerCommand = strCommand.substr(index + 1, strCommand.length());

		int k = strQuerCommand.find_first_of("(");
		std::string str_operationType = strQuerCommand.substr(0, k);

		int h = strQuerCommand.length();

		std::string t_command = strQuerCommand.substr(k + 1, h - k - 2);

		std::vector<std::string> strArrayCmd = splitParameter(t_command);

		bool isFindOp = false; //判断是否是查询操作
		bool resultFlag = false;
		if (str_operationType == "find")
		{
			isFindOp = true;
			resultFlag = executeFindCommand(result, strArrayCmd, str_tableName.c_str());
		}
		else if (str_operationType == "update")
		{
			resultFlag = executeUpdateCommand(strArrayCmd, str_tableName.c_str());
		}
		else if (str_operationType == "remove")
		{
			resultFlag = executeRemoveCommand(strArrayCmd, str_tableName.c_str());
		}
		else if (str_operationType == "insert")
		{
			resultFlag = executeInsertCommand(strArrayCmd, str_tableName.c_str());
		}
		else
		{
			strArrayCmd.clear();
			resultFlag = executeFunctionCommand(result, strCommand);
		}

		//除了查询操作，其它操作都没有结果集，所以要压入默认值，查询失败的时候也要压入默认值
		if (!isFindOp || (isFindOp && !resultFlag))
		{
			uint32 nfields = 0;
			uint64 affectedRows = 0;

			(*result) << nfields;
			(*result) << affectedRows;
		}

		return resultFlag;
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

		return strError;
	}

	int DBInterfaceMongodb::getlasterror()
	{
		return 0;
	}

	bool DBInterfaceMongodb::lock()
	{
		//lock_.start();
		return true;
	}

	//-------------------------------------------------------------------------------------
	bool DBInterfaceMongodb::unlock()
	{
		//lock_.commit();
		//lock_.end();
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
			strError = error.message;
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
			strError = error.message;
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
			strError = error.message;
			ERROR_MSG(fmt::format("{}\n", error.message));
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

	bool DBInterfaceMongodb::extuteFunction(const bson_t *command, const mongoc_read_prefs_t *read_prefs, bson_t *reply)
	{
		bson_error_t  error;
		bool r = mongoc_database_command_simple(database, command, read_prefs, reply, &error);
		if (!r)
		{
			strError = error.message;
			ERROR_MSG(fmt::format("{}\n", error.message));
		}

		return r;
	}

	bool DBInterfaceMongodb::executeFindCommand(MemoryStream * result, std::vector<std::string> strcmd, const char *tableName)
	{
		bool flag = true;
		std::string query = "";
		std::string field = "";
		int limit = 0;
		int skip = 0;
		int batchSize = 0;
		int options = 0;
		int size = strcmd.size();
		query = strcmd[0];

		if (size >= 2)
		{
			field = strcmd[1];
		}

		if (size >= 3)
		{
			limit = atoi(strcmd[2].c_str());
		}

		if (size >= 4)
		{
			skip = atoi(strcmd[3].c_str());
		}

		if (size >= 5)
		{
			batchSize = atoi(strcmd[4].c_str());
		}

		if (size >= 6)
		{
			options = atoi(strcmd[5].c_str());
		}

		bson_error_t  error;
		bson_t *q = bson_new_from_json((const uint8_t *)query.c_str(), query.length(), &error);
		if (!q)
		{
			strError = error.message;
			ERROR_MSG(fmt::format("{}\n", error.message));
			return false;
		}

		bson_t *f = NULL;
		if (field != "")
		{
			f = bson_new_from_json((const uint8_t *)field.c_str(), field.length(), &error);
			if (!f)
			{
				strError = error.message;
				ERROR_MSG(fmt::format("{}\n", error.message));
				return false;
			}
		}		

		mongoc_query_flags_t queryOptions = (mongoc_query_flags_t)options;
		const bson_t *doc;
		mongoc_cursor_t * cursor = collectionFind(tableName, queryOptions, skip, limit, batchSize, q, f, NULL);
		uint32 nrows = 0;
		uint32 nfields = 1;

		std::vector<char *> value;
		while (mongoc_cursor_next(cursor, &doc))
		{
			nrows++;
			char *str = bson_as_json(doc, NULL);
			value.push_back(str);			
		}

		(*result) << nfields << nrows;

		std::vector<char *>::iterator it;
		for (it = value.begin(); it != value.end(); it++)
		{
			result->appendBlob(*it, strlen(*it));
			bson_free(*it);
		}

		if (mongoc_cursor_error(cursor, &error))
		{
			strError = error.message;
			ERROR_MSG(fmt::format("An error occurred: {}\n", error.message));
			flag = false;
		}
		
		bson_destroy(q);
		bson_destroy(f);
		mongoc_cursor_destroy(cursor);
		return flag;
	}

	bool DBInterfaceMongodb::executeUpdateCommand(std::vector<std::string> strcmd, const char *tableName)
	{
		bool successFlag = false;
		std::string query = "";
		std::string update = "";
		int size = strcmd.size();
		query = strcmd[0];

		bool upsert = false;
		bool multi = false;

		if (size >= 2)
		{
			update = strcmd[1];
		}

		if (size >= 3)
		{
			std::istringstream(strcmd[2]) >> std::boolalpha >> upsert;
		}

		if (size >= 4)
		{
			std::istringstream(strcmd[3]) >> std::boolalpha >> multi;
		}

		bson_error_t  error;
		bson_t *q = bson_new_from_json((const uint8_t *)query.c_str(), query.length(), &error);
		if (!q)
		{
			strError = error.message;			
			ERROR_MSG(fmt::format("{}\n", error.message));
			return false;
		}

		bson_t *u = bson_new_from_json((const uint8_t *)update.c_str(), update.length(), &error);
		if (!u)
		{
			strError = error.message;
			ERROR_MSG(fmt::format("{}\n", error.message));
			return false;
		}


		mongoc_update_flags_t uflags;
		if (!upsert && !multi)
		{
			uflags = MONGOC_UPDATE_NONE;
		}
		else if (upsert && !multi)
		{
			uflags = MONGOC_UPDATE_UPSERT;
		}
		else if (!upsert && multi)
		{
			uflags = MONGOC_UPDATE_MULTI_UPDATE;
		}

		successFlag = updateCollection(tableName, uflags, q, u, NULL);

		bson_destroy(q);
		bson_destroy(u);
		return successFlag;
	}

	bool DBInterfaceMongodb::executeRemoveCommand(std::vector<std::string> strcmd, const char *tableName)
	{
		bool successFlag = false;
		std::string query = "";
		bool justOne = false;
		int size = strcmd.size();
		query = strcmd[0];

		if (size >= 2)
		{
			std::istringstream(strcmd[1]) >> std::boolalpha >> justOne;
		}

		bson_error_t  error;
		bson_t *q = bson_new_from_json((const uint8_t *)query.c_str(), query.length(), &error);
		if (!q)
		{
			strError = error.message;
			ERROR_MSG(fmt::format("{}\n", error.message));
			return false;
		}

		mongoc_remove_flags_t flags;
		if (justOne)
		{
			flags = MONGOC_REMOVE_SINGLE_REMOVE;
		}
		else
		{
			flags = MONGOC_REMOVE_NONE;
		}

		successFlag = collectionRemove(tableName, flags, q, NULL);

		bson_destroy(q);
		return successFlag;
	}

	bool DBInterfaceMongodb::executeInsertCommand(std::vector<std::string> strcmd, const char *tableName)
	{
		bool successFlag = false;
		std::string query = "";
		bool ordered = true;
		int size = strcmd.size();
		query = strcmd[0];

		if (size >= 2)
		{
			if (strcmd[1].find("false") != std::string::npos)
			{
				ordered = false;
			}
		}

		bson_error_t  error;
		bson_t *q = bson_new_from_json((const uint8_t *)query.c_str(), query.length(), &error);
		if (!q)
		{
			strError = error.message;
			ERROR_MSG(fmt::format("{}\n", error.message));
			return false;
		}

		mongoc_insert_flags_t flags;
		if (ordered)
		{
			flags = MONGOC_INSERT_NONE;
		}
		else
		{
			flags = MONGOC_INSERT_CONTINUE_ON_ERROR;
		}

		successFlag = insertCollection(tableName, flags, q, NULL);

		bson_destroy(q);
		return successFlag;
	}

	//用于mongodb执行js函数
	bool DBInterfaceMongodb::executeFunctionCommand(MemoryStream * result, std::string strcmd)
	{
		bson_error_t  error;
		bson_t reply;
		bson_init(&reply);
		bson_t *q = bson_new_from_json((const uint8_t *)strcmd.c_str(), strcmd.length(), &error);
		if (!q)
		{
			strError = error.message;
			ERROR_MSG(fmt::format("{}\n", error.message));
			return false;
		}

		uint32 nrows = 1;
		uint32 nfields = 1;

		(*result) << nfields << nrows;

		bool r = extuteFunction(q, NULL, &reply);
		if (r)
		{
			char *str = bson_as_json(&reply, NULL);
			result->appendBlob(str, strlen(str));
			bson_free(str);
		}

		bson_free(q);
		bson_destroy(&reply);

		return r;
	}

	//字符串分割
	std::vector<std::string> DBInterfaceMongodb::splitParameter(std::string value)
	{
		std::vector<std::string> result;
		char * queue = new char[512];

		int index = value.length();
		int whole = 0; //为0就等待下一个'，'号
		int wpos = 0; //写入的位置
		for (int i = 0; i < index; i++)
		{
			switch (value[i])
			{
				case '{':
				{
					queue[wpos] = value[i];
					wpos++;
					whole++;
					break;
				}
				case '}':
				{
					queue[wpos] = value[i];
					wpos++;
					whole--;
					break;
				}
				case '[':
				{
					queue[wpos] = value[i];
					wpos++;
					whole++;
					break;
				}
				case ']':
				{
					queue[wpos] = value[i];
					wpos++;
					whole--;
					break;
				}
				case ',':
				{
					if (whole == 0)
					{
						std::string part(queue, wpos);
						wpos = 0; //重置

						result.push_back(part);
					}
					else
					{
						queue[wpos] = value[i];
						wpos++;
					}
					break;
				}
				case ' ':
					break;
				default:
				{
					queue[wpos] = value[i];
					wpos++;
					break;
				}
			}

		}

		//最后一个存储
		if (whole == 0)
		{
			std::string part(queue, wpos);
			wpos = 0; //重置

			result.push_back(part);
		}

		delete[] queue;

		return result;
	}

}
