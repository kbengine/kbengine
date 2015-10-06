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
bool DBInterfaceRedis::checkEnvironment()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::checkErrors()
{
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
		ERROR_MSG(fmt::format("DBInterfaceMysql::ping: errno={}, error={}\n",
			pRedisContext->err, pRedisContext->errstr));
 
		return false;
	}  
     	
	if (!(r->type == REDIS_REPLY_STATUS && kbe_stricmp(r->str, "PONG") == 0))
	{  
		ERROR_MSG(fmt::format("DBInterfaceMysql::ping: errno={}, error={}\n",
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
		ERROR_MSG(fmt::format("DBInterfaceMysql::attach: errno={}, error={}\n",
			c->err, c->errstr));
		
		redisFree(c);  
		return false;  
	}
	
	redisReply* r = NULL;
	
	// 密码验证
	if(!ping())
	{
		redisReply* r = (redisReply*)redisCommand(c, fmt::format("auth {}", db_password_).c_str());  
		
		if (NULL == r) 
		{ 
			ERROR_MSG(fmt::format("DBInterfaceMysql::attach: cmd={}, errno={}, error={}\n",
				fmt::format("auth ***").c_str(), c->err, c->errstr));
			
			redisFree(c);  
			return false;
		}  
	     	
		if (!(r->type == REDIS_REPLY_STATUS && kbe_stricmp(r->str, "OK") == 0))
		{  
			ERROR_MSG(fmt::format("DBInterfaceMysql::attach: cmd={}, errno={}, error={}\n",
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
		ERROR_MSG(fmt::format("DBInterfaceMysql::attach: cmd={}, errno={}, error={}\n",
			fmt::format("select {}", db_index).c_str(), c->err, c->errstr));
		
		redisFree(c);  
		return false;
	}  
     	
	if (!(r->type == REDIS_REPLY_STATUS && kbe_stricmp(r->str, "OK") == 0))
	{  
		ERROR_MSG(fmt::format("DBInterfaceMysql::attach: cmd={}, errno={}, error={}\n",
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
bool DBInterfaceRedis::query(const char* strCommand, uint32 size, bool showexecinfo)
{
	return true;
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
	return true;
}

//-------------------------------------------------------------------------------------
bool DBInterfaceRedis::dropEntityTableItemFromDB(const char* tableName, const char* tableItemName)
{
	return true;
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
