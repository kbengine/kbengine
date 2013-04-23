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
#include "db_interface_mysql.hpp"
#include "dbmgr_lib/db_interface.hpp"
#include "dbmgr_lib/entity_table.hpp"
#include "entitydef/entitydef.hpp"
#include "entitydef/scriptdef_module.hpp"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::syncToDB(DBInterface* dbi)
{
	std::string sqlstr = "DROP TABLE kbe_entitylog;";

	try
	{
		dbi->query(sqlstr.c_str(), sqlstr.size(), false);
	}
	catch(...)
	{
	}
	
	bool ret = false;

	sqlstr = "CREATE TABLE IF NOT EXISTS kbe_entitylog "
			"(entityDBID bigint(20) not null DEFAULT 0,"
			"entityType int unsigned not null DEFAULT 0,"
			"entityID int unsigned not null DEFAULT 0,"
			"ip varchar(64),"
			"port int unsigned not null DEFAULT 0,"
			"componentID bigint unsigned not null DEFAULT 0,"
			"PRIMARY KEY (entityDBID, entityType))"
		"ENGINE="MYSQL_ENGINE_TYPE;

	ret = dbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);
	return ret;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::logEntity(DBInterface * dbi, const char* ip, uint32 port, DBID dbid,
					COMPONENT_ID componentID, ENTITY_ID entityID, ENTITY_SCRIPT_UID entityType)
{
	std::string sqlstr = "insert into kbe_entitylog (entityDBID, entityType, entityID, ip, port, componentID) values(";

	char* tbuf = new char[MAX_BUF * 3];

	kbe_snprintf(tbuf, MAX_BUF, "%"PRDBID, dbid);
	sqlstr += tbuf;
	sqlstr += ",";
	
	kbe_snprintf(tbuf, MAX_BUF, "%u", entityType);
	sqlstr += tbuf;
	sqlstr += ",";

	kbe_snprintf(tbuf, MAX_BUF, "%d", entityID);
	sqlstr += tbuf;
	sqlstr += ",\"";

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, ip, strlen(ip));

	sqlstr += tbuf;
	sqlstr += "\",";

	kbe_snprintf(tbuf, MAX_BUF, "%u", port);
	sqlstr += tbuf;
	sqlstr += ",";
	
	kbe_snprintf(tbuf, MAX_BUF, "%"PRDBID, componentID);
	sqlstr += tbuf;
	sqlstr += ")";

	SAFE_RELEASE_ARRAY(tbuf);

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		// 1062 int err = dbi->getlasterror(); 
		return false;
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::queryEntity(DBInterface * dbi, DBID dbid, EntityLog& entitylog, ENTITY_SCRIPT_UID entityType)
{
	std::string sqlstr = "select entityID,ip,port,componentID from kbe_entitylog where entityDBID=";

	char tbuf[MAX_BUF];
	kbe_snprintf(tbuf, MAX_BUF, "%"PRDBID, dbid);
	sqlstr += tbuf;
	
	sqlstr += " and entityType=";
	kbe_snprintf(tbuf, MAX_BUF, "%u", entityType);
	sqlstr += tbuf;

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		return true;
	}
	
	entitylog.dbid = dbid;
	entitylog.componentID = 0;
	entitylog.entityID = 0;
	entitylog.ip[0] = '\0';
	entitylog.port = 0;

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow;
		while((arow = mysql_fetch_row(pResult)) != NULL)
		{
			StringConv::str2value(entitylog.entityID, arow[0]);
			kbe_snprintf(entitylog.ip, MAX_IP, arow[1]);
			StringConv::str2value(entitylog.port, arow[2]);
			StringConv::str2value(entitylog.componentID, arow[3]);
			break;
		}

		mysql_free_result(pResult);
	}

	return entitylog.componentID > 0;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::eraseEntityLog(DBInterface * dbi, DBID dbid)
{
	std::string sqlstr = "delete from kbe_entitylog where entityDBID=";

	char tbuf[MAX_BUF];

	kbe_snprintf(tbuf, MAX_BUF, "%"PRDBID, dbid);
	sqlstr += tbuf;

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
KBEEntityLogTableMysql::KBEEntityLogTableMysql():
	KBEEntityLogTable()
{
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::syncToDB(DBInterface* dbi)
{
	bool ret = false;

	std::string sqlstr = "CREATE TABLE IF NOT EXISTS kbe_accountinfos "
			"(accountName varchar(255) not null, PRIMARY KEY idKey (accountName),"
			"password varchar(255),"
			"bindata blob,"
			"entityDBID bigint(20) not null DEFAULT 0)"
		"ENGINE="MYSQL_ENGINE_TYPE;

	ret = dbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);
	return ret;
}

//-------------------------------------------------------------------------------------
KBEAccountTableMysql::KBEAccountTableMysql():
	KBEAccountTable()
{
}


//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::queryAccount(DBInterface * dbi, const std::string& name, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select entityDBID, password from kbe_accountinfos where accountName like \"";

	char* tbuf = new char[name.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	sqlstr += tbuf;

	SAFE_RELEASE_ARRAY(tbuf);
	sqlstr += "\"";

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
		return true;

	info.dbid = 0;
	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow;
		while((arow = mysql_fetch_row(pResult)) != NULL)
		{
			std::stringstream sval;
			sval << arow[0];
			DBID old_dbid;
			
			sval >> old_dbid;
			info.dbid = old_dbid;
			info.name = name;
			info.password = arow[1];
		}

		mysql_free_result(pResult);
	}

	return info.dbid > 0;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::logAccount(DBInterface * dbi, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "insert into kbe_accountinfos (accountName, password, bindata, entityDBID) values(";

	char* tbuf = new char[MAX_BUF * 3];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, info.name.c_str(), info.name.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, info.password.c_str(), info.password.size());

	sqlstr += "md5(\"";
	sqlstr += tbuf;
	sqlstr += "\"),";

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, info.datas.data(), info.datas.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	kbe_snprintf(tbuf, MAX_BUF, "%"PRDBID, info.dbid);

	sqlstr += tbuf;
	sqlstr += ")";

	SAFE_RELEASE_ARRAY(tbuf);

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
KBEEntityTypeMysql::KBEEntityTypeMysql():
	KBEEntityType()
{
}

//-------------------------------------------------------------------------------------
bool KBEEntityTypeMysql::syncToDB(DBInterface* dbi)
{
	std::string sqlstr = "DROP TABLE kbe_entitytypes;";

	try
	{
		dbi->query(sqlstr.c_str(), sqlstr.size(), false);
	}
	catch(...)
	{
	}

	bool ret = false;

	sqlstr = "CREATE TABLE IF NOT EXISTS kbe_entitytypes "
			"(entityType int unsigned not null DEFAULT 0,"
			"typeid int unsigned not null DEFAULT 0,"
			"PRIMARY KEY (entityType))"
		"ENGINE="MYSQL_ENGINE_TYPE;

	ret = dbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);
	return ret;
}

//-------------------------------------------------------------------------------------
}
