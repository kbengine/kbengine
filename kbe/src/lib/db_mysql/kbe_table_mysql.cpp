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

#include "entity_table_mysql.hpp"
#include "kbe_table_mysql.hpp"
#include "dbmgr_lib/db_interface.hpp"
#include "dbmgr_lib/entity_table.hpp"
#include "entitydef/entitydef.hpp"
#include "entitydef/scriptdef_module.hpp"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::syncToDB(DBInterface* dbi)
{
	DEBUG_MSG("KBEEntityLogTableMysql::syncToDB(): kbe_entitylog.\n");
	std::string sqlstr = "DROP TABLE kbe_entitylog;";
	dbi->query(sqlstr.c_str(), sqlstr.size(), false);
	
	bool ret = false;

	sqlstr = "CREATE TABLE IF NOT EXISTS kbe_entitylog "
			"(entityDBID bigint(20), PRIMARY KEY idKey (entityDBID),"
			"ip int unsigned not null DEFAULT 0,"
			"port int unsigned not null DEFAULT 0,"
			"componentID bigint unsigned not null DEFAULT 0)"
		"ENGINE="MYSQL_ENGINE_TYPE;

	ret = dbi->query(sqlstr.c_str(), sqlstr.size(), false);
	KBE_ASSERT(ret);
	return ret;
}

//-------------------------------------------------------------------------------------
KBEEntityLogTableMysql::KBEEntityLogTableMysql():
	EntityTable()
{
	tableName("kbe_entitylog");
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::initialize(ScriptDefModule* sm, std::string name)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::syncToDB(DBInterface* dbi)
{
	DEBUG_MSG("KBEAccountTableMysql::syncToDB(): kbe_accountinfos.\n");
	
	bool ret = false;

	std::string sqlstr = "CREATE TABLE IF NOT EXISTS kbe_accountinfos "
			"(accountName varchar(255) not null, PRIMARY KEY idKey (accountName),"
			"password varchar(255),"
			"entityDBID bigint(20) not null DEFAULT 0)"
		"ENGINE="MYSQL_ENGINE_TYPE;

	ret = dbi->query(sqlstr.c_str(), sqlstr.size(), false);
	KBE_ASSERT(ret);
	return ret;
}

//-------------------------------------------------------------------------------------
KBEAccountTableMysql::KBEAccountTableMysql():
	EntityTable()
{
	tableName("kbe_accountinfos");
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::initialize(ScriptDefModule* sm, std::string name)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::hasAccount(std::string& name)
{
	return true;
}

//-------------------------------------------------------------------------------------
}
