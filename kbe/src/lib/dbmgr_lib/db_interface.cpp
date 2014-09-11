/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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


#include "db_interface.hpp"
#include "db_threadpool.hpp"
#include "entity_table.hpp"
#include "cstdkbe/kbekey.hpp"
#include "db_mysql/db_interface_mysql.hpp"
#include "db_mysql/kbe_table_mysql.hpp"
#include "server/serverconfig.hpp"
#include "thread/threadpool.hpp"

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
#ifdef USE_OPENSSL
		// 如果小于64则表示目前还是明文密码
		if(strlen(dbcfg.db_password) < 64)
		{
			WARNING_MSG(boost::format("DBUtil::createInterface: db password is not encrypted!\nplease use password(rsa):\n%1%\n") 
				% KBEKey::getSingleton().encrypt(dbcfg.db_password));
		}
		else
		{
			std::string out = KBEKey::getSingleton().decrypt(dbcfg.db_password);

			if(out.size() == 0)
				return false;

			kbe_snprintf(dbcfg.db_password, MAX_BUF, "%s", out.c_str());
		}
#endif
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

	kbe_snprintf(dbinterface->db_type_, MAX_BUF, "%s", dbcfg.db_type);
	dbinterface->db_port_ = dbcfg.db_port;	
	kbe_snprintf(dbinterface->db_ip_, MAX_IP, "%s", dbcfg.db_ip);
	kbe_snprintf(dbinterface->db_username_, MAX_BUF, "%s", dbcfg.db_username);
	dbinterface->db_numConnections_ = dbcfg.db_numConnections;
	kbe_snprintf(dbinterface->db_password_, MAX_BUF, "%s", dbcfg.db_password);

	if(dbinterface == NULL)
	{
		ERROR_MSG("DBUtil::createInterface: can't create dbinterface!\n");
		return NULL;
	}

	if(!dbinterface->attach(DBUtil::dbname()))
	{
		ERROR_MSG(boost::format("DBUtil::createInterface: can't attach to database!\n\tdbinterface=%1%\n\targs=%2%") %
			&dbinterface % dbinterface->c_str());

		delete dbinterface;
		return NULL;
	}
	else
	{
		if(showinfo)
		{
			INFO_MSG(boost::format("DBUtil::createInterface[%1%]: %2%") % &dbinterface % dbinterface->c_str());
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
		EntityTables::getSingleton().addKBETable(new KBEEntityTypeMysql());
		EntityTables::getSingleton().addKBETable(new KBEAccountTableMysql());
		EntityTables::getSingleton().addKBETable(new KBEEntityLogTableMysql());
		EntityTables::getSingleton().addKBETable(new KBEEmailVerificationTableMysql());
	}
	
	if(!pThreadPool_->isInitialize())
	{
		if(!pThreadPool_->createThreadPool(dbcfg.db_numConnections, 
			dbcfg.db_numConnections, dbcfg.db_numConnections))
			return false;
	}

	return EntityTables::getSingleton().load(dbi) && EntityTables::getSingleton().syncToDB(dbi);
}

//-------------------------------------------------------------------------------------
}
