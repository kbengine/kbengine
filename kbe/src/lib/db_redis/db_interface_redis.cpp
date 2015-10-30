/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#include "redis_helper.h"
#include "db_interface_redis.h"
#include "thread/threadguard.h"
#include "helper/watcher.h"
#include "server/serverconfig.h"


namespace KBEngine { 

//-------------------------------------------------------------------------------------
DBInterfaceRedis::DBInterfaceRedis() :
DBInterface(),
pRedisContext_(NULL),
hasLostConnection_(false)
{
}

//-------------------------------------------------------------------------------------
DBInterfaceRedis::~DBInterfaceRedis()
{
}


//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::initInterface(DBInterface* dbi)
{/*
	EntityTables::getSingleton().addKBETable(new KBEAccountTableMysql());
	EntityTables::getSingleton().addKBETable(new KBEEntityLogTableRedis());
	EntityTables::getSingleton().addKBETable(new KBEEmailVerificationTableMysql());	*/
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
		WARNING_MSG(fmt::format("DBInterfaceRedis::checkErrors: not found {} table, reset kbe_* table...\n", 
			DBUtil::accountScriptName()));
		
		RedisHelper::dropTable(this, fmt::format("kbe_*"), false);
		WARNING_MSG(fmt::format("DBInterfaceRedis::checkErrors: reset kbe_* table end!\n"));
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
	
	// 密码验证
	redisReply* r = (redisReply*)redisCommand(pRedisContext, "ping");  
	
	if (NULL == r) 
	{ 
		ERROR_MSG(fmt::format("DBInterfaceRedis::ping: errno={}, error={}\n",
			pRedisContext->err, pRedisContext->errstr));
 
		return false;
	}  
     	
	if (!(r->type == REDIS_REPLY_STATUS && kbe_stricmp(r->str, "PONG") == 0))
	{  
		ERROR_MSG(fmt::format("DBInterfaceRedis::ping: errno={}, error={}\n",
			pRedisContext->err, pRedisContext->errstr));
		
		freeReplyObject(r);  
		return false;
	}
	
	freeReplyObject(r); 
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::attach(const char* databaseName)
{
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
	
	redisReply* r = NULL;
	
	// 密码验证
	if(!ping())
	{
		r = (redisReply*)redisCommand(c, fmt::format("auth {}", db_password_).c_str());  
		
		if (NULL == r) 
		{ 
			ERROR_MSG(fmt::format("DBInterfaceRedis::attach: cmd={}, errno={}, error={}\n",
				fmt::format("auth ***").c_str(), c->err, c->errstr));
			
			redisFree(c);  
			return false;
		}  
	     	
		if (!(r->type == REDIS_REPLY_STATUS && kbe_stricmp(r->str, "OK") == 0))
		{  
			ERROR_MSG(fmt::format("DBInterfaceRedis::attach: cmd={}, errno={}, error={}\n",
				fmt::format("auth ***").c_str(), c->err, c->errstr));
			
			freeReplyObject(r);  
			redisFree(c);  
			return false;
		}

		freeReplyObject(r); 	
		r = NULL;
	}
	
	// 选择数据库
	int db_index = atoi(db_name_);
	if(db_index < 0)
		db_index = 0;
		
	r = (redisReply*)redisCommand(c, fmt::format("select {}", db_index).c_str());
	
	if (NULL == r) 
	{ 
		ERROR_MSG(fmt::format("DBInterfaceRedis::attach: cmd={}, errno={}, error={}\n",
			fmt::format("select {}", db_index).c_str(), c->err, c->errstr));
		
		redisFree(c);  
		return false;
	}  
     	
	if (!(r->type == REDIS_REPLY_STATUS && kbe_stricmp(r->str, "OK") == 0))
	{  
		ERROR_MSG(fmt::format("DBInterfaceRedis::attach: cmd={}, errno={}, error={}\n",
			fmt::format("select {}", db_index).c_str(), c->err, c->errstr));
		
		freeReplyObject(r);  
		redisFree(c);  
		return false;
	}

	freeReplyObject(r); 
	pRedisContext_ = c;  
              
	DEBUG_MSG(fmt::format("DBInterfaceRedis::attach: successfully! addr: {}:{}\n", db_ip_, db_port_));
	return ping();
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
bool DBInterfaceRedis::query(const char* cmd, uint32 size, redisReply** pRedisReply, bool showExecInfo)
{
	KBE_ASSERT(pRedisContext_);
	*pRedisReply = (redisReply*)redisCommand(pRedisContext_, "%b", cmd, size);  
	
	lastquery_.assign(cmd, size);
	
	if (pRedisContext_->err) 
	{
		if(showExecInfo)
		{
			ERROR_MSG(fmt::format("DBInterfaceRedis::query: cmd={}, errno={}, error={}\n",
				cmd, pRedisContext_->err, pRedisContext_->errstr));
		}

		if(*pRedisReply){
			freeReplyObject(*pRedisReply); 
			(*pRedisReply) = NULL;
		}
		
		return false;
	}  
		
	if(showExecInfo)
	{
		INFO_MSG("DBInterfaceRedis::query: successfully!\n"); 
	}
				
	return true;	
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::query(const char* cmd, uint32 size, bool showExecInfo, MemoryStream * result)
{
	KBE_ASSERT(pRedisContext_);
	redisReply* r = (redisReply*)redisCommand(pRedisContext_, "%b", cmd, size);  
	
	lastquery_.assign(cmd, size);
	write_query_result(r, result);
	
	if (pRedisContext_->err) 
	{
		if(showExecInfo)
		{
			ERROR_MSG(fmt::format("DBInterfaceRedis::query: cmd={}, errno={}, error={}\n",
				cmd, pRedisContext_->err, pRedisContext_->errstr));
		}

		if(r)
			freeReplyObject(r); 
		
		return false;
	}  

	freeReplyObject(r); 
		
	if(showExecInfo)
	{
		INFO_MSG("DBInterfaceRedis::query: successfully!\n"); 
	}
				
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::query(bool showExecInfo, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);

	KBE_ASSERT(pRedisContext_);
	redisReply* pRedisReply = (redisReply*)redisvCommand(pRedisContext_, format, ap);
	
	lastquery_ = pRedisContext_->obuf;
	
	if (pRedisContext_->err) 
	{
		if(showExecInfo)
		{
			ERROR_MSG(fmt::format("DBInterfaceRedis::query: cmd={}, errno={}, error={}\n",
				lastquery_, pRedisContext_->err, pRedisContext_->errstr));
		}

		if(pRedisReply){
			freeReplyObject(pRedisReply); 
			pRedisReply = NULL;
		}
		
		va_end(ap);
		return false;
	}  
		
	if(showExecInfo)
	{
		INFO_MSG("DBInterfaceRedis::query: successfully!\n"); 
	}    
	
	va_end(ap);

	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::queryAppend(bool showExecInfo, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);

	KBE_ASSERT(pRedisContext_);
	int ret = redisvAppendCommand(pRedisContext_, format, ap);

	lastquery_ = pRedisContext_->obuf;
	
	if (ret == REDIS_ERR) 
	{
		if(showExecInfo)
		{
			ERROR_MSG(fmt::format("DBInterfaceRedis::query: cmd={}, errno={}, error={}\n",
				lastquery_, pRedisContext_->err, pRedisContext_->errstr));
		}
		
		va_end(ap);
		return false;
	}  
		
	if(showExecInfo)
	{
		INFO_MSG("DBInterfaceRedis::query: successfully!\n"); 
	}    
	
	va_end(ap);

	return true;
}

//-------------------------------------------------------------------------------------
void DBInterfaceRedis::write_query_result(redisReply* r, MemoryStream * result)
{
	if(result == NULL)
	{
		return;
	}

	if(pRedisContext_ && r && !pRedisContext_->err)
	{
		uint32 nrows = 0;
		uint32 nfields = 1;

		if(r->type == REDIS_REPLY_ARRAY)
			nfields = r->elements;

		(*result) << nfields << nrows;
		
		if(r->type == REDIS_REPLY_ARRAY)
		{
			for(size_t j = 0; j < r->elements; ++j) 
			{
				write_query_result_element(r->element[j], result);
			}
		}
		else
		{
			write_query_result_element(r, result);
		}
	}
	else
	{
		uint32 nfields = 0;
		uint64 affectedRows = 0;
		(*result) << ""; // errormsg
		(*result) << nfields;
		(*result) << affectedRows;
	}
}

//-------------------------------------------------------------------------------------
void DBInterfaceRedis::write_query_result_element(redisReply* r, MemoryStream * result)
{
	if(r->type == REDIS_REPLY_ARRAY)
	{
		// 不支持元素中包含数组
		KBE_ASSERT(false);
	}
	else if(r->type == REDIS_REPLY_INTEGER)
	{
		std::stringstream sstream;
		sstream << r->integer;
		result->appendBlob(sstream.str().c_str(), sstream.str().size());
	}
	else if(r->type == REDIS_REPLY_NIL)
	{
		result->appendBlob("NULL", strlen("NULL"));
	}		
	else if(r->type == REDIS_REPLY_STATUS)
	{
		result->appendBlob(r->str, r->len);
	}	
	else if(r->type == REDIS_REPLY_ERROR)
	{
		result->appendBlob(r->str, r->len);
	}			
	else if(r->type == REDIS_REPLY_STRING)
	{
		result->appendBlob(r->str, r->len);
	}
}

//-------------------------------------------------------------------------------------
const char* DBInterfaceRedis::c_str()
{
	static char strdescr[MAX_BUF];
	kbe_snprintf(strdescr, MAX_BUF, "dbtype=redis, ip=%s, port=%u, currdatabase=%s, username=%s, connected=%s.\n", 
		db_ip_, db_port_, db_name_, db_username_, pRedisContext_ == NULL ? "no" : "yes");

	return strdescr;
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
EntityTable* DBInterfaceRedis::createEntityTable()
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

	char sql_str[MAX_BUF];
	kbe_snprintf(sql_str, MAX_BUF, "hdel %s %s", tableName, tableItemName);
	return query(sql_str, strlen(sql_str));
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
bool DBInterfaceRedis::processException(std::exception & e)
{
	return true;
}

//-------------------------------------------------------------------------------------
}
