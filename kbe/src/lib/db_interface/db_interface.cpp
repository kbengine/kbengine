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


#include "db_interface.h"
#include "db_threadpool.h"
#include "entity_table.h"
#include "common/kbekey.h"
#include "db_mysql/db_interface_mysql.h"
#include "db_redis/db_interface_redis.h"
#include "server/serverconfig.h"
#include "thread/threadpool.h"

namespace KBEngine { 
KBE_SINGLETON_INIT(DBUtil);

DBUtil g_DBUtil;

thread::ThreadPool* DBUtil::pThreadPool_ = new DBThreadPool();

//-------------------------------------------------------------------------------------
DBUtil::DBUtil()
{
}

//-------------------------------------------------------------------------------------
DBUtil::~DBUtil()
{
}

//-------------------------------------------------------------------------------------
bool DBUtil::initThread()
{
	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();
	if(strcmp(dbcfg.db_type, "mysql") == 0)
	{
		if (!mysql_thread_safe()) 
		{
			KBE_ASSERT(false);
		}
		else
		{
			mysql_thread_init();
		}
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
bool DBUtil::finiThread()
{
	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();
	if(strcmp(dbcfg.db_type, "mysql") == 0)
	{
		mysql_thread_end();
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool DBUtil::initialize()
{
	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();

	if(dbcfg.db_passwordEncrypt)
	{
		// 如果小于64则表明当前是明文密码配置
		if(strlen(dbcfg.db_password) < 64)
		{
			WARNING_MSG(fmt::format("DBUtil::createInterface: db password is not encrypted!\nplease use password(rsa):\n{}\n",
				KBEKey::getSingleton().encrypt(dbcfg.db_password)));
		}
		else
		{
			std::string out = KBEKey::getSingleton().decrypt(dbcfg.db_password);

			if(out.size() == 0)
				return false;

			kbe_snprintf(dbcfg.db_password, MAX_BUF, "%s", out.c_str());
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
void DBUtil::finalise()
{
	pThreadPool()->finalise();
	SAFE_RELEASE(pThreadPool_);
}

//-------------------------------------------------------------------------------------
DBInterface* DBUtil::createInterface(bool showinfo)
{
	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();
	DBInterface* dbinterface = NULL;

	if(strcmp(dbcfg.db_type, "mysql") == 0)
	{
		dbinterface = new DBInterfaceMysql(dbcfg.db_unicodeString_characterSet, dbcfg.db_unicodeString_collation);
	}
	else if(strcmp(dbcfg.db_type, "redis") == 0)
	{
		dbinterface = new DBInterfaceRedis();
	}

	if(dbinterface == NULL)
	{
		ERROR_MSG(fmt::format("DBUtil::createInterface: create db_interface error! type={}\n",
			dbcfg.db_type));

		return NULL;
	}
	
	kbe_snprintf(dbinterface->db_type_, MAX_BUF, "%s", dbcfg.db_type);
	dbinterface->db_port_ = dbcfg.db_port;	
	kbe_snprintf(dbinterface->db_ip_, MAX_IP, "%s", dbcfg.db_ip);
	kbe_snprintf(dbinterface->db_username_, MAX_BUF, "%s", dbcfg.db_username);
	dbinterface->db_numConnections_ = dbcfg.db_numConnections;
	kbe_snprintf(dbinterface->db_password_, MAX_BUF, "%s", dbcfg.db_password);

	if(!dbinterface->attach(DBUtil::dbname()))
	{
		ERROR_MSG(fmt::format("DBUtil::createInterface: attach to database failed!\n\tdbinterface={0:p}\n\targs={1}\n",
			(void*)&dbinterface, dbinterface->c_str()));

		delete dbinterface;
		return NULL;
	}
	else
	{
		if(showinfo)
		{
			INFO_MSG(fmt::format("DBUtil::createInterface[{0:p}]: {1}\n", (void*)&dbinterface, 
				dbinterface->c_str()));
		}
	}

	return dbinterface;
}

//-------------------------------------------------------------------------------------
const char* DBUtil::dbname()
{
	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();
	return dbcfg.db_name;
}

//-------------------------------------------------------------------------------------
const char* DBUtil::dbtype()
{
	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();
	return dbcfg.db_type;
}

//-------------------------------------------------------------------------------------
const char* DBUtil::accountScriptName()
{
	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();
	return dbcfg.dbAccountEntityScriptType;
}

//-------------------------------------------------------------------------------------
bool DBUtil::initInterface(DBInterface* dbi)
{
	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();
	if(strcmp(dbcfg.db_type, "mysql") == 0)
	{
		DBInterfaceMysql::initInterface(dbi);
	}
	else if(strcmp(dbcfg.db_type, "redis") == 0)
	{
		DBInterfaceRedis::initInterface(dbi);
	}
	
	if(!pThreadPool_->isInitialize())
	{
		if(!pThreadPool_->createThreadPool(dbcfg.db_numConnections, 
			dbcfg.db_numConnections, dbcfg.db_numConnections))
			return false;
	}

	bool ret = EntityTables::getSingleton().load(dbi);

	if(ret)
	{
		ret = dbi->checkEnvironment();
	}
	
	if(ret)
	{
		ret = dbi->checkErrors();
	}

	return ret && EntityTables::getSingleton().syncToDB(dbi);
}

//-------------------------------------------------------------------------------------
}
