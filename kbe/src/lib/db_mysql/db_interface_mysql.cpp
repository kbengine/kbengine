// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "db_interface_mysql.h"
#include "entity_table_mysql.h"
#include "kbe_table_mysql.h"
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
	for(uint32 i=0; i<size; ++i)
	{
		if(strCommand[i] == ' ')
			break;

		op += strCommand[i];
	}

	if(op.size() == 0)
		return;

	std::transform(op.begin(), op.end(), op.begin(), toupper);

	_g_logMutex.lockMutex();

	KBEUnordered_map< std::string, uint32 >::iterator iter = g_querystatistics.find(op);
	if(iter == g_querystatistics.end())
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
	if(iter != g_querystatistics.end())
	{
		return iter->second;
	}

	return 0;
}

static uint32 watcher_select(const std::string&)
{
	return watcher_query("SELECT");
}

static uint32 watcher_delete(const std::string&)
{
	return watcher_query("DELETE");
}

static uint32 watcher_insert(const std::string&)
{
	return watcher_query("INSERT");
}

static uint32 watcher_update(const std::string&)
{
	return watcher_query("UPDATE");
}

static uint32 watcher_create(const std::string&)
{
	return watcher_query("CREATE");
}

static uint32 watcher_drop(const std::string&)
{
	return watcher_query("DROP");
}

static uint32 watcher_show(const std::string&)
{
	return watcher_query("SHOW");
}

static uint32 watcher_alter(const std::string&)
{
	return watcher_query("ALTER");
}

static uint32 watcher_grant(const std::string&)
{
	return watcher_query("GRANT");
}

static void initializeWatcher()
{
	if(_g_installedWatcher)
		return;

	_g_installedWatcher = true;
	_g_debug = g_kbeSrvConfig.getDBMgr().debugDBMgr;

	WATCH_OBJECT("db_querys/select", &KBEngine::watcher_select);
	WATCH_OBJECT("db_querys/delete", &KBEngine::watcher_delete);
	WATCH_OBJECT("db_querys/insert", &KBEngine::watcher_insert);
	WATCH_OBJECT("db_querys/update", &KBEngine::watcher_update);
	WATCH_OBJECT("db_querys/create", &KBEngine::watcher_create);
	WATCH_OBJECT("db_querys/drop", &KBEngine::watcher_drop);
	WATCH_OBJECT("db_querys/show", &KBEngine::watcher_show);
	WATCH_OBJECT("db_querys/alter", &KBEngine::watcher_alter);
	WATCH_OBJECT("db_querys/grant", &KBEngine::watcher_grant);
}

size_t DBInterfaceMysql::sql_max_allowed_packet_ = 0;
//-------------------------------------------------------------------------------------
DBInterfaceMysql::DBInterfaceMysql(const char* name, std::string characterSet, std::string collation) :
DBInterface(name),
pMysql_(NULL),
hasLostConnection_(false),
inTransaction_(false),
lock_(NULL, false),
characterSet_(characterSet),
collation_(collation)
{
	lock_.pdbi(this);
}

//-------------------------------------------------------------------------------------
DBInterfaceMysql::~DBInterfaceMysql()
{
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::initInterface(DBInterface* pdbi)
{
	EntityTables& entityTables = EntityTables::findByInterfaceName(pdbi->name());

	entityTables.addKBETable(new KBEAccountTableMysql(&entityTables));
	entityTables.addKBETable(new KBEServerLogTableMysql(&entityTables));
	entityTables.addKBETable(new KBEEntityLogTableMysql(&entityTables));
	entityTables.addKBETable(new KBEEmailVerificationTableMysql(&entityTables));
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::attach(const char* databaseName)
{
	if(!_g_installedWatcher)
	{
		initializeWatcher();
	}

	if(db_port_ == 0)
		db_port_ = 3306;
	
	if(databaseName != NULL)
		kbe_snprintf(db_name_, MAX_BUF, "%s", databaseName);

	hasLostConnection_ = false;

	try
	{
		pMysql_ = mysql_init(0);
		if(pMysql_ == NULL)
		{
			ERROR_MSG("DBInterfaceMysql::attach: mysql_init error!\n");
			return false;
		}
		
		DEBUG_MSG(fmt::format("DBInterfaceMysql::attach: connect: {}:{} starting...\n", db_ip_, db_port_));

		int ntry = 0;

__RECONNECT:
		if(mysql_real_connect(mysql(), db_ip_, db_username_, 
    		db_password_, db_name_, db_port_, NULL, 0)) // CLIENT_MULTI_STATEMENTS  
		{
			if(mysql_select_db(mysql(), db_name_) != 0)
			{
				ERROR_MSG(fmt::format("DBInterfaceMysql::attach: Could not set active db[{}]\n",
					db_name_));

				detach();
				return false;
			}
		}
		else
		{
			if (mysql_errno(pMysql_) == 1049 && ntry++ == 0)
			{
				if (mysql())
				{
					::mysql_close(mysql());
					pMysql_ = NULL;
				}

				pMysql_ = mysql_init(0);
				if (pMysql_ == NULL)
				{
					ERROR_MSG("DBInterfaceMysql::attach: mysql_init error!\n");
					return false;
				}

				if (mysql_real_connect(mysql(), db_ip_, db_username_,
					db_password_, NULL, db_port_, NULL, 0)) // CLIENT_MULTI_STATEMENTS  
				{
					this->createDatabaseIfNotExist();
					if (mysql_select_db(mysql(), db_name_) != 0)
					{
						goto __RECONNECT;
					}
				}
				else
				{
					goto __RECONNECT;
				}
			}
			else
			{
				ERROR_MSG(fmt::format("DBInterfaceMysql::attach: mysql_errno={}, mysql_error={}\n",
					mysql_errno(pMysql_), mysql_error(pMysql_)));

				if (mysql_errno(pMysql_) == 2059)
				{
					ERROR_MSG(fmt::format("DBInterfaceMysql::attach: Does not support caching_sha2_password, https://github.com/kbengine/kbengine/issues/625\n"));
				}

				detach();
				return false;
			}
		}

		if (mysql_set_character_set(mysql(), characterSet_.c_str()) != 0)
		{
			ERROR_MSG(fmt::format("DBInterfaceMysql::attach: Could not set client connection character set to {}\n", characterSet_));
			return false;
		}

		// 不需要关闭自动提交，底层会START TRANSACTION之后再COMMIT
		// mysql_autocommit(mysql(), 0);

		char characterset_sql[MAX_BUF];
		kbe_snprintf(characterset_sql, MAX_BUF, "ALTER DATABASE CHARACTER SET %s COLLATE %s", 
			characterSet_.c_str(), collation_.c_str());

		query(&characterset_sql[0], strlen(characterset_sql), false);
	}
	catch (std::exception& e)
	{
		ERROR_MSG(fmt::format("DBInterfaceMysql::attach: {}\n", e.what()));
		hasLostConnection_ = true;
		detach();
		return false;
	}

	bool ret = mysql() != NULL && ping();

	if(ret)
	{
		DEBUG_MSG(fmt::format("DBInterfaceMysql::attach: successfully! addr: {}:{}\n", db_ip_, db_port_));
	}

    return ret;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::checkEnvironment()
{
	/*
	std::string querycmd = "SHOW VARIABLES";
	if(!query(querycmd.c_str(), querycmd.size(), true))
	{
		ERROR_MSG(fmt::format("DBInterfaceMysql::checkEnvironment: {}, query error!\n", querycmd));
		return false;
	}

	bool lower_case_table_names = false;
	MYSQL_RES * pResult = mysql_store_result(mysql());
	if(pResult)
	{
		MYSQL_ROW arow;
		while((arow = mysql_fetch_row(pResult)) != NULL)
		{
			std::string s = arow[0];
			std::string v = arow[1];
			
			if(s == "lower_case_table_names")
			{
				if(v != "1")
				{
					lower_case_table_names = true;
				}
				else
				{
					CRITICAL_MSG(fmt::format("DBInterfaceMysql::checkEnvironment: [my.cnf or my.ini]->lower_case_table_names != 2 or 0, curr={}!\n"
						"Windows use CMD(wmic service where \"name like 'mysql%'\") to view the configuration directory.\n", v));
				}
			}
			else if(s == "max_allowed_packet")
			{
				uint64 size;
				KBEngine::StringConv::str2value(size, v.c_str());
				sql_max_allowed_packet_ = (size_t)size;
			}
		}

		mysql_free_result(pResult);
	}
	
	return lower_case_table_names;
	*/
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::createDatabaseIfNotExist()
{
	std::string querycmd = fmt::format("create database {}", db_name_);
	query(querycmd.c_str(), querycmd.size(), false);
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::checkErrors()
{
	DBInterfaceInfo* pDBInfo = g_kbeSrvConfig.dbInterface(name());
	if (!pDBInfo)
	{
		ERROR_MSG(fmt::format("DBInterfaceMysql::checkErrors: not found dbInterface({})\n",
			name()));

		return false;
	}

	if (!pDBInfo->isPure)
	{
		std::string querycmd = fmt::format("SHOW TABLES LIKE \"" ENTITY_TABLE_PERFIX "_{}\"", DBUtil::accountScriptName());
		if (!query(querycmd.c_str(), querycmd.size(), true))
		{
			ERROR_MSG(fmt::format("DBInterfaceMysql::checkErrors: {}, query(dbInterface={}) error!\n", querycmd, name()));
			return false;
		}

		bool foundAccountTable = false;
		MYSQL_RES * pResult = mysql_store_result(mysql());
		if (pResult)
		{
			foundAccountTable = mysql_num_rows(pResult) > 0;
			mysql_free_result(pResult);
		}

		if (!foundAccountTable)
		{
			querycmd = "DROP TABLE `" KBE_TABLE_PERFIX "_email_verification`, `" KBE_TABLE_PERFIX "_accountinfos`";

			WARNING_MSG(fmt::format("DBInterfaceMysql::checkErrors: not found {} table(dbInterface={}), reset " KBE_TABLE_PERFIX "_* table...\n",
				DBUtil::accountScriptName(), name()));

			try
			{
				query(querycmd.c_str(), querycmd.size(), false);
			}
			catch (...)
			{
			}

			WARNING_MSG(fmt::format("DBInterfaceMysql::checkErrors: reset " KBE_TABLE_PERFIX "_* table(dbInterface={}) end!\n", name()));
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::reattach()
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

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::detach()
{
	if(mysql())
	{
		::mysql_close(mysql());
		pMysql_ = NULL;
	}

	return true;
}

//-------------------------------------------------------------------------------------
EntityTable* DBInterfaceMysql::createEntityTable(EntityTables* pEntityTables)
{
	return new EntityTableMysql(pEntityTables);
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::dropEntityTableFromDB(const char* tableName)
{
	KBE_ASSERT(tableName != NULL);
  
	DEBUG_MSG(fmt::format("DBInterfaceMysql::dropEntityTableFromDB: {}.\n", tableName));

	char sql_str[SQL_BUF];
	kbe_snprintf(sql_str, SQL_BUF, "Drop table if exists %s;", tableName);
	return query(sql_str, strlen(sql_str));
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::dropEntityTableItemFromDB(const char* tableName, const char* tableItemName)
{
	KBE_ASSERT(tableName != NULL && tableItemName != NULL);
  
	DEBUG_MSG(fmt::format("DBInterfaceMysql::dropEntityTableItemFromDB: {} {}.\n", 
		tableName, tableItemName));

	char sql_str[SQL_BUF];
	kbe_snprintf(sql_str, SQL_BUF, "alter table %s drop column %s;", tableName, tableItemName);
	return query(sql_str, strlen(sql_str));
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::query(const char* cmd, uint32 size, bool printlog, MemoryStream * result)
{
	if(pMysql_ == NULL)
	{
		if(printlog)
		{
			ERROR_MSG(fmt::format("DBInterfaceMysql::query: has no attach(db)!\nsql:({})\n", lastquery_));
		}

		if(result)
			write_query_result(result);

		return false;
	}

	querystatistics(cmd, size);

	lastquery_.assign(cmd, size);

	if(_g_debug)
	{
		DEBUG_MSG(fmt::format("DBInterfaceMysql::query({:p}): {}\n", (void*)this, lastquery_));
	}

    int nResult = mysql_real_query(pMysql_, cmd, size);  

    if(nResult != 0)  
    {
		if(printlog)
		{
			ERROR_MSG(fmt::format("DBInterfaceMysql::query: error({}:{})!\nsql:({})\n", 
				mysql_errno(pMysql_), mysql_error(pMysql_), lastquery_)); 
		}

		this->throwError(NULL);
		
		if(result)
			write_query_result(result);

        return false;
    } 
    else
    {
		if(printlog)
		{
			INFO_MSG("DBInterfaceMysql::query: successfully!\n"); 
		}
    }

    return result == NULL || write_query_result(result);
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::write_query_result(MemoryStream * result)
{
	if(result == NULL)
	{
		return true;
	}

	MYSQL_RES * pResult = mysql_store_result(mysql());

	if(pResult)
	{
		size_t wpos = result->wpos();
		uint32 nrows = (uint32)mysql_num_rows(pResult);
		uint32 nfields = (uint32)mysql_num_fields(pResult);

		try
		{
			(*result) << nfields << nrows;

			MYSQL_ROW arow;

			while ((arow = mysql_fetch_row(pResult)) != NULL)
			{
				unsigned long *lengths = mysql_fetch_lengths(pResult);

				for (uint32 i = 0; i < nfields; ++i)
				{
					if (arow[i] == NULL)
					{
						result->appendBlob("KBE_QUERY_DB_NULL", strlen("KBE_QUERY_DB_NULL"));
					}
					else
					{
						result->appendBlob(arow[i], lengths[i]);
					}
				}
			}
		}
		catch (MemoryStreamWriteOverflow & e)
		{
			mysql_free_result(pResult);
			result->wpos(wpos);

			DBException e1(NULL);
			e1.setError(fmt::format("DBException: {}, SQL({})", e.what(), lastquery_), 0);
			throwError(&e1);

			return false;
		}

		mysql_free_result(pResult);
	}
	else
	{
		uint32 nfields = 0;
		uint64 affectedRows = 0;
		uint64 lastInsertID = 0;
		
		if(mysql())
		{
			affectedRows = mysql()->affected_rows;
			lastInsertID = mysql_insert_id(mysql());
		}
		
		(*result) << nfields;
		(*result) << affectedRows;
		(*result) << lastInsertID;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::getTableNames(std::vector<std::string>& tableNames, const char * pattern)
{
	if(pMysql_ == NULL)
	{
		ERROR_MSG("DBInterfaceMysql::getTableNames: has no attach(db).\n");
		return false;
	}

	tableNames.clear();

	MYSQL_RES * pResult = mysql_list_tables(pMysql_, pattern);

	if(pResult)
	{
		tableNames.reserve((unsigned int)mysql_num_rows(pResult));

		MYSQL_ROW row;
		while((row = mysql_fetch_row(pResult)) != NULL)
		{
			unsigned long *lengths = mysql_fetch_lengths(pResult);
			tableNames.push_back(std::string(row[0], lengths[0]));
		}

		mysql_free_result(pResult);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::getTableItemNames(const char* tableName, std::vector<std::string>& itemNames)
{
	MYSQL_RES*	result = mysql_list_fields(mysql(), tableName, NULL);
	if(result)
	{
		unsigned int numFields = mysql_num_fields(result);
		MYSQL_FIELD* fields = mysql_fetch_fields(result);

		for(unsigned int i = 0; i < numFields; ++i)
		{
			itemNames.push_back(fields[i].name);
		}
	}
	else
		return false;

	mysql_free_result(result);
	return true;
}

//-------------------------------------------------------------------------------------
const char* DBInterfaceMysql::c_str()
{
	static std::string strdescr;
	strdescr = fmt::format("interface={}, dbtype=mysql, ip={}, port={}, currdatabase={}, username={}, connected={}.\n",
		name_, db_ip_, db_port_, db_name_, db_username_, (pMysql_ == NULL ? "no" : "yes"));

	return strdescr.c_str();
}

//-------------------------------------------------------------------------------------
const char* DBInterfaceMysql::getstrerror()
{
	return mysql_error(pMysql_);
}

//-------------------------------------------------------------------------------------
int DBInterfaceMysql::getlasterror()
{
	return mysql_errno(pMysql_);
}

//-------------------------------------------------------------------------------------
void DBInterfaceMysql::getFields(TABLE_FIELDS& outs, const char* tableName)
{
	std::string sqlname = ENTITY_TABLE_PERFIX"_";
	sqlname += tableName;

	MYSQL_RES*	result = mysql_list_fields(mysql(), sqlname.c_str(), NULL);
	if(result == NULL)
	{
		ERROR_MSG(fmt::format("EntityTableMysql::getFields:{}\n", getstrerror()));
		return;
	}

	unsigned int numFields;
	MYSQL_FIELD* fields;

	numFields = mysql_num_fields(result);
	fields = mysql_fetch_fields(result);

	for(unsigned int i=0; i<numFields; ++i)
	{
		MYSQL_TABLE_FIELD& info = outs[fields[i].name];
		info.name = fields[i].name;
		info.length = fields[i].length;
		info.maxlength = fields[i].max_length;
		info.flags = fields[i].flags;
		info.type = fields[i].type;
	}

	mysql_free_result(result);
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::lock()
{
	lock_.start();
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::unlock()
{
	lock_.commit();
	lock_.end();
	return true;
}

//-------------------------------------------------------------------------------------
void DBInterfaceMysql::throwError(DBException* pDBException)
{
	if (pDBException)
	{
		throw *pDBException;
	}
	else
	{
		DBException e(this);

		if (e.isLostConnection())
		{
			this->hasLostConnection(true);
		}

		throw e;
	}
}

//-------------------------------------------------------------------------------------
bool DBInterfaceMysql::processException(std::exception & e)
{
	DBException* dbe = static_cast<DBException*>(&e);
	bool retry = false;

	if (dbe->isLostConnection())
	{
		ERROR_MSG(fmt::format("DBInterfaceMysql::processException: "
				"Thread {:p} lost connection to database. Exception: {}. "
				"Attempting to reconnect.\n",
			(void*)this,
			dbe->what()));

		int attempts = 1;

		while (!this->reattach())
		{
			ERROR_MSG(fmt::format("DBInterfaceMysql::processException: "
							"Thread {:p} reconnect({}) attempt {} failed({}).\n",
						(void*)this,
						db_name_,
						attempts,
						getLastError()));

			KBEngine::sleep(30);
			++attempts;
		}

		INFO_MSG(fmt::format("DBInterfaceMysql::processException: "
					"Thread {:p} reconnected({}). Attempts = {}\n",
				(void*)this,
				db_name_,
				attempts));

		retry = true;
	}
	else if (dbe->shouldRetry())
	{
		WARNING_MSG(fmt::format("DBInterfaceMysql::processExceptionn(db={}): Retrying {:p}\nException:{}\nnlastquery={}\n",
			db_name_, (void*)this, dbe->what(), lastquery_));

		retry = true;
	}
	else
	{
		WARNING_MSG(fmt::format("DBInterfaceMysql::processExceptionn(db={}): "
				"Exception: {}\nlastquery={}\n",
			db_name_, dbe->what(), lastquery_));
	}

	return retry;
}

//-------------------------------------------------------------------------------------
}
