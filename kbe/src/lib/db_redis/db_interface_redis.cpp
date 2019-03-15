// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "redis_helper.h"
#include "kbe_table_redis.h"
#include "db_exception.h"
#include "redis_watcher.h"
#include "db_interface_redis.h"
#include "thread/threadguard.h"
#include "helper/watcher.h"
#include "server/serverconfig.h"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
DBInterfaceRedis::DBInterfaceRedis(const char* name) :
DBInterface(name),
pRedisContext_(NULL),
hasLostConnection_(false),
inTransaction_(false)
{
}

//-------------------------------------------------------------------------------------
DBInterfaceRedis::~DBInterfaceRedis()
{
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::initInterface(DBInterface* pdbi)
{
	EntityTables& entityTables = EntityTables::findByInterfaceName(pdbi->name());

	entityTables.addKBETable(new KBEAccountTableRedis(&entityTables));
	entityTables.addKBETable(new KBEServerLogTableRedis(&entityTables));
	entityTables.addKBETable(new KBEEntityLogTableRedis(&entityTables));
	entityTables.addKBETable(new KBEEmailVerificationTableRedis(&entityTables));
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::checkEnvironment()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::checkErrors()
{
	if (!RedisHelper::hasTable(this, fmt::format("{}:*", DBUtil::accountScriptName()), true))
	{
		WARNING_MSG(fmt::format("DBInterfaceRedis::checkErrors: not found {} table, reset " KBE_TABLE_PERFIX "_* table...\n", 
			DBUtil::accountScriptName()));
		
		RedisHelper::dropTable(this, fmt::format(KBE_TABLE_PERFIX "_*"), false);
		WARNING_MSG(fmt::format("DBInterfaceRedis::checkErrors: reset " KBE_TABLE_PERFIX "_* table end!\n"));
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::ping(redisContext* pRedisContext)
{ 
	if(!pRedisContext)
		pRedisContext = pRedisContext_;
	
	if(!pRedisContext)
		return false;

	redisReply* pRedisReply = (redisReply*)redisCommand(pRedisContext, "ping");
	
	if (NULL == pRedisReply)
	{ 
		ERROR_MSG(fmt::format("DBInterfaceRedis::ping: errno={}, error={}\n",
			pRedisContext->err, pRedisContext->errstr));
 
		return false;
	}  
     	
	if (!(pRedisReply->type == REDIS_REPLY_STATUS && kbe_stricmp(pRedisReply->str, "PONG") == 0))
	{  
		ERROR_MSG(fmt::format("DBInterfaceRedis::ping: errno={}, error={}\n",
			pRedisContext->err, pRedisReply->str));
		
		freeReplyObject(pRedisReply);
		return false;
	}
	
	freeReplyObject(pRedisReply);
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::attach(const char* databaseName)
{
	RedisWatcher::initializeWatcher();
		
	if(db_port_ == 0)
		db_port_ = 6379;
		
	if(databaseName != NULL)
		kbe_snprintf(db_name_, MAX_BUF, "%s", databaseName);
	else
		kbe_snprintf(db_name_, MAX_BUF, "%s", "0");
	
	hasLostConnection_ = false;
	
	DEBUG_MSG(fmt::format("DBInterfaceRedis::attach: connect: {}:{} starting...\n", db_ip_, db_port_));
	
	struct timeval timeout = { 5, 0 }; // 5 seconds
	redisContext* c = redisConnectWithTimeout(db_ip_, db_port_, timeout);  
	if (c->err) 
	{  
		ERROR_MSG(fmt::format("DBInterfaceRedis::attach: errno={}, error={}\n",
			c->err, c->errstr));
		
		redisFree(c);  
		return false;  
	}
	
	redisReply* pRedisReply = NULL;
	
	// 密码验证
	if(!ping())
	{
		pRedisReply = (redisReply*)redisCommand(c, fmt::format("auth {}", db_password_).c_str());  
		
		if (NULL == pRedisReply) 
		{ 
			ERROR_MSG(fmt::format("DBInterfaceRedis::attach: cmd={}, errno={}, error={}\n",
				fmt::format("auth ***").c_str(), c->err, c->errstr));
			
			redisFree(c);  
			return false;
		}  
	     	
		if (!(pRedisReply->type == REDIS_REPLY_STATUS && kbe_stricmp(pRedisReply->str, "OK") == 0))
		{  
			if(!kbe_stricmp(pRedisReply->str, "ERR Client sent AUTH, but no password is set") == 0)
			{
				ERROR_MSG(fmt::format("DBInterfaceRedis::attach: cmd={}, errno={}, error={}\n",
					fmt::format("auth ***").c_str(), c->err, pRedisReply->str));

				freeReplyObject(pRedisReply);
				redisFree(c);
				return false;
			}
		}

		freeReplyObject(pRedisReply); 
		pRedisReply = NULL;
	}
	
	// 选择数据库
	int db_index = atoi(db_name_);
	if(db_index <= 0)
	{
		kbe_snprintf(db_name_, MAX_BUF, "%s", "0");
		db_index = 0;
	}
		
	pRedisReply = (redisReply*)redisCommand(c, fmt::format("select {}", db_index).c_str());
	
	if (NULL == pRedisReply) 
	{ 
		ERROR_MSG(fmt::format("DBInterfaceRedis::attach: cmd={}, errno={}, error={}\n",
			fmt::format("select {}", db_index).c_str(), c->err, c->errstr));
		
		redisFree(c);  
		return false;
	}  
     	
	if (!(pRedisReply->type == REDIS_REPLY_STATUS && kbe_stricmp(pRedisReply->str, "OK") == 0))
	{  
		ERROR_MSG(fmt::format("DBInterfaceRedis::attach: cmd={}, errno={}, error={}\n",
			fmt::format("select {}", db_index).c_str(), c->err, pRedisReply->str));
		
		freeReplyObject(pRedisReply);  
		redisFree(c);  
		return false;
	}

	freeReplyObject(pRedisReply); 
	pRedisContext_ = c;  
              
	DEBUG_MSG(fmt::format("DBInterfaceRedis::attach: successfully! addr: {}:{}\n", db_ip_, db_port_));
	return ping();
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::reattach()
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
bool DBInterfaceRedis::detach()
{
	if(pRedisContext_)
	{
		redisFree(pRedisContext_);
		pRedisContext_ = NULL;
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::getTableNames( std::vector<std::string>& tableNames, const char * pattern)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::getTableItemNames(const char* tableName, std::vector<std::string>& itemNames)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::query(const std::string& cmd, redisReply** pRedisReply, bool printlog)
{
	KBE_ASSERT(pRedisContext_);
	*pRedisReply = (redisReply*)redisCommand(pRedisContext_, cmd.c_str());  
	
	lastquery_ = cmd;
	RedisWatcher::querystatistics(lastquery_.c_str(), (uint32)lastquery_.size());
	
	if (pRedisContext_->err) 
	{
		if(printlog)
		{
			ERROR_MSG(fmt::format("DBInterfaceRedis::query: cmd={}, errno={}, error={}\n",
				cmd, pRedisContext_->err, pRedisContext_->errstr));
		}

		if(*pRedisReply){
			freeReplyObject(*pRedisReply); 
			(*pRedisReply) = NULL;
		}

		this->throwError(NULL);
		return false;
	}

	if(printlog)
	{
		INFO_MSG("DBInterfaceRedis::query: successfully!\n"); 
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::query(const char* cmd, uint32 size, bool printlog, MemoryStream * result)
{
	KBE_ASSERT(pRedisContext_);
	redisReply* pRedisReply = (redisReply*)redisCommand(pRedisContext_, cmd);
	
	lastquery_ = cmd;
	RedisWatcher::querystatistics(lastquery_.c_str(), (uint32)lastquery_.size());
	write_query_result(pRedisReply, result);
	
	if (pRedisContext_->err) 
	{
		if(printlog)
		{
			ERROR_MSG(fmt::format("DBInterfaceRedis::query: cmd={}, errno={}, error={}\n",
				cmd, pRedisContext_->err, pRedisContext_->errstr));
		}

		if(pRedisReply)
			freeReplyObject(pRedisReply); 
		
		this->throwError(NULL);
		return false;
	}  

	freeReplyObject(pRedisReply); 

	if(printlog)
	{
		INFO_MSG("DBInterfaceRedis::query: successfully!\n"); 
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::query(bool printlog, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);

	KBE_ASSERT(pRedisContext_);
	redisReply* pRedisReply = (redisReply*)redisvCommand(pRedisContext_, format, ap);
	
	char buffer[1024];
	int cnt	= vsnprintf(buffer, sizeof(buffer) - 1, format, ap);

	if(cnt > 0)
	{
		lastquery_ = buffer;
		RedisWatcher::querystatistics(lastquery_.c_str(), (uint32)lastquery_.size());
	}
	
	if (pRedisContext_->err) 
	{
		if(printlog)
		{
			ERROR_MSG(fmt::format("DBInterfaceRedis::query: cmd={}, errno={}, error={}\n",
				lastquery_, pRedisContext_->err, pRedisContext_->errstr));
		}

		if(pRedisReply){
			freeReplyObject(pRedisReply); 
			pRedisReply = NULL;
		}

		va_end(ap);
		
		this->throwError(NULL);
		return false;
	}

	if(printlog)
	{
		INFO_MSG("DBInterfaceRedis::query: successfully!\n"); 
	}    
	
	va_end(ap);

	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::queryAppend(bool printlog, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);

	KBE_ASSERT(pRedisContext_);
	int ret = redisvAppendCommand(pRedisContext_, format, ap);

	if(lastquery_.size() > 0 && lastquery_[lastquery_.size() - 1] != ';')
		lastquery_ = "";
	
	char buffer[1024];
	int cnt	= vsnprintf(buffer, sizeof(buffer) - 1, format, ap);

	if(cnt > 0)
	{
		lastquery_ += buffer;
		lastquery_ += ";";
		RedisWatcher::querystatistics(buffer, cnt);
	}
	
	if (ret == REDIS_ERR) 
	{	
		if(printlog)
		{
			ERROR_MSG(fmt::format("DBInterfaceRedis::queryAppend: cmd={}, errno={}, error={}\n",
				lastquery_, pRedisContext_->err, pRedisContext_->errstr));
		}

		va_end(ap);
		
		this->throwError(NULL);
		return false;
	}  

	if(printlog)
	{
		INFO_MSG("DBInterfaceRedis::queryAppend: successfully!\n"); 
	}

	va_end(ap);

	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::getQueryReply(redisReply **pRedisReply)
{
	return redisGetReply(pRedisContext_, (void**)pRedisReply) == REDIS_OK;
}

//-------------------------------------------------------------------------------------
void DBInterfaceRedis::write_query_result(redisReply* pRedisReply, MemoryStream * result)
{
	if(result == NULL)
	{
		return;
	}

	if(pRedisContext_ && pRedisReply && !pRedisContext_->err)
	{
		uint32 nrows = 0;
		uint32 nfields = 1;

		if(pRedisReply->type == REDIS_REPLY_ARRAY)
			nfields = (uint32)pRedisReply->elements;

		(*result) << nfields << nrows;
		
		if(pRedisReply->type == REDIS_REPLY_ARRAY)
		{
			for(size_t j = 0; j < pRedisReply->elements; ++j) 
			{
				write_query_result_element(pRedisReply->element[j], result);
			}
		}
		else
		{
			write_query_result_element(pRedisReply, result);
		}
	}
	else
	{
		uint32 nfields = 0;
		uint64 affectedRows = 0;
		uint64 lastInsertID = 0;

		(*result) << nfields;
		(*result) << affectedRows;
		(*result) << lastInsertID;
	}
}

//-------------------------------------------------------------------------------------
void DBInterfaceRedis::write_query_result_element(redisReply* pRedisReply, MemoryStream * result)
{
	if(pRedisReply->type == REDIS_REPLY_ARRAY)
	{
		// 不支持元素中包含数组
		KBE_ASSERT(false);
	}
	else if(pRedisReply->type == REDIS_REPLY_INTEGER)
	{
		std::stringstream sstream;
		sstream << pRedisReply->integer;
		result->appendBlob(sstream.str().c_str(), sstream.str().size());
	}
	else if(pRedisReply->type == REDIS_REPLY_NIL)
	{
		result->appendBlob("NULL", strlen("NULL"));
	}		
	else if(pRedisReply->type == REDIS_REPLY_STATUS)
	{
		result->appendBlob(pRedisReply->str, pRedisReply->len);
	}	
	else if(pRedisReply->type == REDIS_REPLY_ERROR)
	{
		result->appendBlob(pRedisReply->str, pRedisReply->len);
	}			
	else if(pRedisReply->type == REDIS_REPLY_STRING)
	{
		result->appendBlob(pRedisReply->str, pRedisReply->len);
	}
}

//-------------------------------------------------------------------------------------
const char* DBInterfaceRedis::c_str()
{
	static std::string strdescr;
	strdescr = fmt::format("interface={}, dbtype=redis, ip={}, port={}, currdatabase={}, username={}, connected={}.\n",
		name_, db_ip_, db_port_, db_name_, db_username_, (pRedisContext_ == NULL ? "no" : "yes"));

	return strdescr.c_str();
}

//-------------------------------------------------------------------------------------
const char* DBInterfaceRedis::getstrerror()
{
	if(pRedisContext_ == NULL)
		return "pRedisContext_ is NULL";

	return pRedisContext_->errstr;
}

//-------------------------------------------------------------------------------------
int DBInterfaceRedis::getlasterror()
{
	if(pRedisContext_ == NULL)
		return 0;
		
	return pRedisContext_->err;
}

//-------------------------------------------------------------------------------------
EntityTable* DBInterfaceRedis::createEntityTable(EntityTables* pEntityTables)
{
	return NULL;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::dropEntityTableFromDB(const char* tableName)
{
	KBE_ASSERT(tableName != NULL);
  
	DEBUG_MSG(fmt::format("DBInterfaceRedis::dropEntityTableFromDB: {}.\n", tableName));
	return RedisHelper::dropTable(this, tableName);
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::dropEntityTableItemFromDB(const char* tableName, const char* tableItemName)
{
	KBE_ASSERT(tableName != NULL && tableItemName != NULL);
  
	DEBUG_MSG(fmt::format("DBInterfaceRedis::dropEntityTableItemFromDB: {} {}.\n", 
		tableName, tableItemName));

	return RedisHelper::dropTableItem(this, tableName, tableItemName);
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::lock()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::unlock()
{
	return true;
}

//-------------------------------------------------------------------------------------
void DBInterfaceRedis::throwError(DBException* pDBException)
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
bool DBInterfaceRedis::processException(std::exception & e)
{
	DBException* dbe = static_cast<DBException*>(&e);
	bool retry = false;

	if (dbe->isLostConnection())
	{
		INFO_MSG(fmt::format("DBInterfaceRedis::processException: "
				"Thread {:p} lost connection to database. Exception: {}. "
				"Attempting to reconnect.\n",
			(void*)this,
			dbe->what()));

		int attempts = 1;

		while (!this->reattach())
		{
			ERROR_MSG(fmt::format("DBInterfaceRedis::processException: "
							"Thread {:p} reconnect({}) attempt {} failed({}).\n",
						(void*)this,
						db_name_,
						attempts,
						getstrerror()));

			KBEngine::sleep(30);
			++attempts;
		}

		INFO_MSG(fmt::format("DBInterfaceRedis::processException: "
					"Thread {:p} reconnected({}). Attempts = {}\n",
				(void*)this,
				db_name_,
				attempts));

		retry = true;
	}
	else if (dbe->shouldRetry())
	{
		WARNING_MSG(fmt::format("DBInterfaceRedis::processException: Retrying {:p}\nException:{}\nnlastquery={}\n",
				(void*)this, dbe->what(), lastquery_));

		retry = true;
	}
	else
	{
		WARNING_MSG(fmt::format("DBInterfaceRedis::processException: "
				"Exception: {}\nlastquery={}\n",
			dbe->what(), lastquery_));
	}

	return retry;
}

//-------------------------------------------------------------------------------------
}
