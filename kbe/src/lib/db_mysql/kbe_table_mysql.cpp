// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "entity_table_mysql.h"
#include "kbe_table_mysql.h"
#include "db_exception.h"
#include "db_interface_mysql.h"
#include "db_interface/db_interface.h"
#include "db_interface/entity_table.h"
#include "entitydef/entitydef.h"
#include "entitydef/scriptdef_module.h"
#include "server/serverconfig.h"
#include "server/common.h"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::syncToDB(DBInterface* pdbi)
{
	KBEServerLogTableMysql serverLogTable(NULL);

	int ret = serverLogTable.isShareDB(pdbi);
	if (ret == -1)
		return false;

	if (ret == 0)
	{
		std::string sqlstr = "DROP TABLE IF EXISTS kbe_entitylog;";
		try
		{
			pdbi->query(sqlstr.c_str(), sqlstr.size(), false);
		}
		catch (...)
		{
			ERROR_MSG(fmt::format("KBEEntityLogTableMysql::syncToDB(): error({}: {})\n lastQuery: {}.\n",
				pdbi->getlasterror(), pdbi->getstrerror(), static_cast<DBInterfaceMysql*>(pdbi)->lastquery()));

			return false;
		}
	}

	std::string sqlstr = "CREATE TABLE IF NOT EXISTS " KBE_TABLE_PERFIX "_entitylog "
		"(entityDBID bigint(20) unsigned not null DEFAULT 0,"
		"entityType int unsigned not null DEFAULT 0,"
		"entityID int unsigned not null DEFAULT 0,"
		"ip varchar(64),"
		"port int unsigned not null DEFAULT 0,"
		"componentID bigint unsigned not null DEFAULT 0,"
		"serverGroupID bigint unsigned not null DEFAULT 0,"
		"PRIMARY KEY (entityDBID, entityType))"
		"ENGINE=" MYSQL_ENGINE_TYPE;

	if (!pdbi->query(sqlstr.c_str(), sqlstr.size(), true))
		return false;

	// 清除过期的日志
	std::vector<COMPONENT_ID> cids;

	{
		cids = serverLogTable.queryTimeOutServers(pdbi);

		if (!serverLogTable.clearServers(pdbi, cids))
			return false;

		cids.push_back((uint64)getUserUID());

		sqlstr = fmt::format("delete from " KBE_TABLE_PERFIX "_entitylog where serverGroupID in (");

		char tbuf[MAX_BUF];

		std::vector<COMPONENT_ID>::iterator citer = cids.begin();
		for (; citer != cids.end(); ++citer)
		{
			kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, (*citer));
			sqlstr += tbuf;
			sqlstr += ",";
		}

		if (sqlstr[sqlstr.size() - 1] == ',')
			sqlstr.erase(sqlstr.end() - 1);

		sqlstr += ")";

		if (!pdbi->query(sqlstr.c_str(), sqlstr.size(), true))
			return false;
	}

	// 获得所有服务器
	cids = serverLogTable.queryServers(pdbi);

	// 查询所有entitylog筛选出在serverlog中找不到记录的log并清理这些无效记录
	{
		sqlstr = fmt::format("select distinct(serverGroupID) from " KBE_TABLE_PERFIX "_entitylog");

		if (!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
			return false;

		std::vector<COMPONENT_ID> erases_ids;

		MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
		if (pResult)
		{
			MYSQL_ROW arow;
			while ((arow = mysql_fetch_row(pResult)) != NULL)
			{
				COMPONENT_ID serverGroupID = 0;
				KBEngine::StringConv::str2value(serverGroupID, arow[0]);

				// 如果找不到服务器log就添加到删除列表
				if (std::find(cids.begin(), cids.end(), serverGroupID) == cids.end())
					erases_ids.push_back(serverGroupID);
			}

			mysql_free_result(pResult);
		}

		if (erases_ids.size() > 0)
		{
			sqlstr = fmt::format("delete from " KBE_TABLE_PERFIX "_entitylog where serverGroupID in (");

			char tbuf[MAX_BUF];

			std::vector<COMPONENT_ID>::iterator citer = erases_ids.begin();
			for (; citer != erases_ids.end(); ++citer)
			{
				kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, (*citer));
				sqlstr += tbuf;
				sqlstr += ",";
			}

			if (sqlstr[sqlstr.size() - 1] == ',')
				sqlstr.erase(sqlstr.end() - 1);

			sqlstr += ")";

			if (!pdbi->query(sqlstr.c_str(), sqlstr.size(), true))
				return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::logEntity(DBInterface * pdbi, const char* ip, uint32 port, DBID dbid,
					COMPONENT_ID componentID, ENTITY_ID entityID, ENTITY_SCRIPT_UID entityType)
{
	std::string sqlstr = "insert into " KBE_TABLE_PERFIX "_entitylog (entityDBID, entityType, entityID, ip, port, componentID, serverGroupID) values(";

	char* tbuf = new char[MAX_BUF * 3];

	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, dbid);
	sqlstr += tbuf;
	sqlstr += ",";
	
	kbe_snprintf(tbuf, MAX_BUF, "%u", entityType);
	sqlstr += tbuf;
	sqlstr += ",";

	kbe_snprintf(tbuf, MAX_BUF, "%d", entityID);
	sqlstr += tbuf;
	sqlstr += ",\"";

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, ip, strlen(ip));

	sqlstr += tbuf;
	sqlstr += "\",";

	kbe_snprintf(tbuf, MAX_BUF, "%u", port);
	sqlstr += tbuf;
	sqlstr += ",";
	
	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, componentID);
	sqlstr += tbuf;
	sqlstr += ",";

	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, (uint64)getUserUID());
	sqlstr += tbuf;
	sqlstr += ")";

	SAFE_RELEASE_ARRAY(tbuf);

	try
	{
		if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
		{
			// 1062 int err = pdbi->getlasterror(); 
			return false;
		}
	}
	catch (std::exception & e)
	{
		DBException& dbe = static_cast<DBException&>(e);
		if(dbe.isLostConnection())
		{
			if(pdbi->processException(e))
				return true;
		}

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::queryEntity(DBInterface * pdbi, DBID dbid, EntityLog& entitylog, ENTITY_SCRIPT_UID entityType)
{
	std::string sqlstr = "select entityID, ip, port, componentID, serverGroupID from " KBE_TABLE_PERFIX "_entitylog where entityDBID=";

	char tbuf[MAX_BUF];
	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, dbid);
	sqlstr += tbuf;
	
	sqlstr += " and entityType=";
	kbe_snprintf(tbuf, MAX_BUF, "%u", entityType);
	sqlstr += tbuf;
	sqlstr += " LIMIT 1";

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		return true;
	}
	
	entitylog.dbid = dbid;
	entitylog.componentID = 0;
	entitylog.serverGroupID = 0;
	entitylog.entityID = 0;
	entitylog.ip[0] = '\0';
	entitylog.port = 0;

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow = mysql_fetch_row(pResult);
		if(arow != NULL)
		{
			StringConv::str2value(entitylog.entityID, arow[0]);
			kbe_snprintf(entitylog.ip, MAX_IP, "%s", arow[1]);
			StringConv::str2value(entitylog.port, arow[2]);
			StringConv::str2value(entitylog.componentID, arow[3]);
			StringConv::str2value(entitylog.serverGroupID, arow[4]);
		}

		mysql_free_result(pResult);
	}

	return entitylog.componentID > 0;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::eraseEntityLog(DBInterface * pdbi, DBID dbid, ENTITY_SCRIPT_UID entityType)
{
	std::string sqlstr = "delete from " KBE_TABLE_PERFIX "_entitylog where entityDBID=";

	char tbuf[MAX_BUF];

	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, dbid);
	sqlstr += tbuf;

	sqlstr += " and entityType=";
	kbe_snprintf(tbuf, MAX_BUF, "%u", entityType);
	sqlstr += tbuf;

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::eraseBaseappEntityLog(DBInterface * pdbi, COMPONENT_ID componentID)
{
	std::string sqlstr = "delete from " KBE_TABLE_PERFIX "_entitylog where componentID=";

	char tbuf[MAX_BUF];

	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, componentID);
	sqlstr += tbuf;

	if (!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
KBEEntityLogTableMysql::KBEEntityLogTableMysql(EntityTables* pEntityTables):
KBEEntityLogTable(pEntityTables)
{
}

//-------------------------------------------------------------------------------------
bool KBEServerLogTableMysql::syncToDB(DBInterface* pdbi)
{
	std::string sqlstr = "";
	
	bool ret = false;

	sqlstr = "CREATE TABLE IF NOT EXISTS " KBE_TABLE_PERFIX "_serverlog "
			"(heartbeatTime bigint(20) unsigned not null DEFAULT 0,"
			"isShareDB bool not null DEFAULT 0,"
			"serverGroupID bigint unsigned not null DEFAULT 0,"
			"PRIMARY KEY (serverGroupID))"
		"ENGINE=" MYSQL_ENGINE_TYPE;

	ret = pdbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);
	return ret && updateServer(pdbi);
}

//-------------------------------------------------------------------------------------
bool KBEServerLogTableMysql::updateServer(DBInterface * pdbi)
{
	int ret = isShareDB(pdbi);
	if (ret == -1)
		return false;

	std::string sqlstr = "insert into " KBE_TABLE_PERFIX "_serverlog (heartbeatTime, isShareDB, serverGroupID) values(";

	char* tbuf = new char[MAX_BUF * 3];

	kbe_snprintf(tbuf, MAX_BUF, "%" PRTime, time(NULL));
	sqlstr += tbuf;
	sqlstr += ",";

	kbe_snprintf(tbuf, MAX_BUF, "%d", ret);
	sqlstr += tbuf;
	sqlstr += ",";

	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, (uint64)getUserUID());
	sqlstr += tbuf;
	sqlstr += ") ON DUPLICATE KEY UPDATE heartbeatTime=";

	kbe_snprintf(tbuf, MAX_BUF, "%" PRTime, time(NULL));
	sqlstr += tbuf;
	sqlstr += ",";

	kbe_snprintf(tbuf, MAX_BUF, "isShareDB=%d", ret);
	sqlstr += tbuf;
	
	SAFE_RELEASE_ARRAY(tbuf);

	try
	{
		if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
		{
			// 1062 int err = pdbi->getlasterror(); 
			return false;
		}
	}
	catch (std::exception & e)
	{
		DBException& dbe = static_cast<DBException&>(e);
		if(dbe.isLostConnection())
		{
			if(pdbi->processException(e))
				return true;
		}

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEServerLogTableMysql::queryServer(DBInterface * pdbi, ServerLog& serverlog)
{
	std::string sqlstr = "select heartbeatTime from " KBE_TABLE_PERFIX "_serverlog where serverGroupID=";

	char tbuf[MAX_BUF];
	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, (uint64)getUserUID());
	sqlstr += tbuf;

	sqlstr += " LIMIT 1";

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		return true;
	}
	
	serverlog.heartbeatTime = 0;

	bool get = false;

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow = mysql_fetch_row(pResult);
		if(arow != NULL)
		{
			StringConv::str2value(serverlog.heartbeatTime, arow[0]);
			get = true;
		}

		mysql_free_result(pResult);
	}

	return get;
}

//-------------------------------------------------------------------------------------
std::vector<COMPONENT_ID> KBEServerLogTableMysql::queryServers(DBInterface * pdbi)
{
	std::vector<COMPONENT_ID> cids;

	std::string sqlstr = "select heartbeatTime,serverGroupID from " KBE_TABLE_PERFIX "_serverlog";

	if (!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		return cids;
	}

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
	if (pResult)
	{
		MYSQL_ROW arow;
		while ((arow = mysql_fetch_row(pResult)) != NULL)
		{
			ServerLog serverlog;
			KBEngine::StringConv::str2value(serverlog.heartbeatTime, arow[0]);
			KBEngine::StringConv::str2value(serverlog.serverGroupID, arow[1]);

			cids.push_back(serverlog.serverGroupID);
		}

		mysql_free_result(pResult);
	}

	return cids;
}

//-------------------------------------------------------------------------------------
std::vector<COMPONENT_ID> KBEServerLogTableMysql::queryTimeOutServers(DBInterface * pdbi)
{
	std::vector<COMPONENT_ID> cids;

	std::string sqlstr = "select heartbeatTime,serverGroupID from " KBE_TABLE_PERFIX "_serverlog";

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		return cids;
	}

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow;
		while((arow = mysql_fetch_row(pResult)) != NULL)
		{
			ServerLog serverlog;
			KBEngine::StringConv::str2value(serverlog.heartbeatTime, arow[0]);
			KBEngine::StringConv::str2value(serverlog.serverGroupID, arow[1]);
			
			if(serverlog.serverGroupID == (uint64)getUserUID())
				continue;
			
			if ((uint64)time(NULL) > serverlog.heartbeatTime + KBEServerLogTable::TIMEOUT * 2)
				cids.push_back(serverlog.serverGroupID);
		}

		mysql_free_result(pResult);
	}

	return cids;
}

//-------------------------------------------------------------------------------------
bool KBEServerLogTableMysql::clearServers(DBInterface * pdbi, const std::vector<COMPONENT_ID>& cids)
{
	if(cids.size() == 0)
		return true;
	
	std::string sqlstr = "delete from " KBE_TABLE_PERFIX "_serverlog where serverGroupID in (";

	char tbuf[MAX_BUF];

	std::vector<COMPONENT_ID>::const_iterator citer = cids.begin();
	for(; citer != cids.end(); ++citer)
	{
		if((*citer) == (uint64)getUserUID())
			continue;

		kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, (*citer));
		sqlstr += tbuf;
		sqlstr += ",";
	}

	if (sqlstr[sqlstr.size() - 1] == ',')
		sqlstr.erase(sqlstr.end() - 1);

	sqlstr += ")";

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
std::map<KBEngine::COMPONENT_ID, bool> KBEServerLogTableMysql::queryAllServerShareDBState(DBInterface * pdbi)
{
	std::vector<COMPONENT_ID> cids = queryTimeOutServers(pdbi);
	clearServers(pdbi, cids);

	std::map<KBEngine::COMPONENT_ID, bool> cidMap;

	std::string sqlstr = "select isShareDB, serverGroupID from " KBE_TABLE_PERFIX "_serverlog";

	if (!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		return cidMap;
	}

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
	if (pResult)
	{
		MYSQL_ROW arow;
		while ((arow = mysql_fetch_row(pResult)) != NULL)
		{
			ServerLog serverlog;
			KBEngine::StringConv::str2value(serverlog.isShareDB, arow[0]);
			KBEngine::StringConv::str2value(serverlog.serverGroupID, arow[1]);

			cidMap.insert(std::make_pair(serverlog.serverGroupID, serverlog.isShareDB));
		}

		mysql_free_result(pResult);
	}

	return cidMap;
}

//-------------------------------------------------------------------------------------
int KBEServerLogTableMysql::isShareDB(DBInterface * pdbi)
{
	bool isShareDB = g_kbeSrvConfig.getDBMgr().isShareDB;
	uint64 uid = getUserUID();

	try
	{
		std::map<COMPONENT_ID, bool> cidMap = queryAllServerShareDBState(pdbi);
		std::map<COMPONENT_ID, bool>::const_iterator citer = cidMap.begin();
		for (; citer != cidMap.end(); ++citer)
		{
			if (citer->first != uid)
			{
				bool isOtherServerShareDB = citer->second;
				if (!isOtherServerShareDB || (isOtherServerShareDB && !isShareDB))
				{
					ERROR_MSG(fmt::format("KBEServerLogTableMysql::isShareDB: The database interface({}) is{} shared, uid={}! Check 'kbe_serverlog' table and 'kbengine[_defs].xml->dbmgr->shareDB'.\n", pdbi->name(),
						isOtherServerShareDB ? "" : " not", citer->first));

					return -1;
				}
			}
		}
	}
	catch (...)
	{
	}

	return isShareDB;
}

//-------------------------------------------------------------------------------------
KBEServerLogTableMysql::KBEServerLogTableMysql(EntityTables* pEntityTables):
KBEServerLogTable(pEntityTables)
{
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::syncToDB(DBInterface* pdbi)
{
	bool ret = false;

	std::string sqlstr = fmt::format("CREATE TABLE IF NOT EXISTS " KBE_TABLE_PERFIX "_accountinfos "
		"(`accountName` varchar({}) not null, PRIMARY KEY idKey (`accountName`),"
		"`password` varchar({}) not null,"
			"`bindata` blob,"
			"`email` varchar(191) not null, UNIQUE KEY `email` (`email`),"
			"`entityDBID` bigint(20) unsigned not null DEFAULT 0, UNIQUE KEY `entityDBID` (`entityDBID`),"
			"`flags` int unsigned not null DEFAULT 0,"
			"`deadline` bigint(20) not null DEFAULT 0,"
			"`regtime` bigint(20) not null DEFAULT 0,"
			"`lasttime` bigint(20) not null DEFAULT 0,"
			"`numlogin` int unsigned not null DEFAULT 0)"
			"ENGINE=" MYSQL_ENGINE_TYPE, ACCOUNT_NAME_MAX_LENGTH, ACCOUNT_PASSWD_MAX_LENGTH);

	ret = pdbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);
	return ret;
}

//-------------------------------------------------------------------------------------
KBEAccountTableMysql::KBEAccountTableMysql(EntityTables* pEntityTables) :
KBEAccountTable(pEntityTables)
{
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::setFlagsDeadline(DBInterface * pdbi, const std::string& name, uint32 flags, uint64 deadline)
{
	char* tbuf = new char[name.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	std::string sqlstr = fmt::format("update " KBE_TABLE_PERFIX "_accountinfos set flags={}, deadline={} where accountName=\"{}\"", 
		flags, deadline, tbuf);

	SAFE_RELEASE_ARRAY(tbuf);

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
		return true;

	return false;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::queryAccount(DBInterface * pdbi, const std::string& name, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select entityDBID, password, flags, deadline, bindata from " KBE_TABLE_PERFIX "_accountinfos where accountName=\"";

	char* tbuf = new char[name.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	sqlstr += tbuf;
	sqlstr += "\" or email=\"";
	sqlstr += tbuf;
	sqlstr += "\" LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
		return true;

	info.dbid = 0;
	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow = mysql_fetch_row(pResult);
		if(arow != NULL)
		{
			unsigned long *lengths = mysql_fetch_lengths(pResult);

			KBEngine::StringConv::str2value(info.dbid, arow[0]);
			info.name = name;
			info.password = arow[1];

			KBEngine::StringConv::str2value(info.flags, arow[2]);
			KBEngine::StringConv::str2value(info.deadline, arow[3]);

			info.datas.assign(arow[4], lengths[4]);
		}

		mysql_free_result(pResult);
	}

	return info.dbid > 0;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::queryAccountAllInfos(DBInterface * pdbi, const std::string& name, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select entityDBID, password, email, flags, deadline from " KBE_TABLE_PERFIX "_accountinfos where accountName=\"";

	char* tbuf = new char[name.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	sqlstr += tbuf;
	sqlstr += "\" or email=\"";
	sqlstr += tbuf;
	sqlstr += "\" LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
		return true;

	info.dbid = 0;
	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow = mysql_fetch_row(pResult);
		if(arow != NULL)
		{
			KBEngine::StringConv::str2value(info.dbid, arow[0]);
			info.name = name;
			info.password = arow[1];
			info.email = arow[2];
			KBEngine::StringConv::str2value(info.flags, arow[3]);
			KBEngine::StringConv::str2value(info.deadline, arow[4]);
		}

		mysql_free_result(pResult);
	}

	return info.dbid > 0;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::updateCount(DBInterface * pdbi, const std::string& name, DBID dbid)
{
	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!pdbi->query(fmt::format("update " KBE_TABLE_PERFIX "_accountinfos set lasttime={}, numlogin=numlogin+1 where entityDBID={}",
		time(NULL), dbid), false))
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::updatePassword(DBInterface * pdbi, const std::string& name, const std::string& password)
{
	char* tbuf = new char[MAX_BUF * 3];
	char* tbuf1 = new char[MAX_BUF * 3];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, password.c_str(), password.size());

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf1, name.c_str(), name.size());

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!pdbi->query(fmt::format("update " KBE_TABLE_PERFIX "_accountinfos set password=\"{}\" where accountName like \"{}\"", 
		password, tbuf1), false))
	{
		SAFE_RELEASE_ARRAY(tbuf);
		SAFE_RELEASE_ARRAY(tbuf1);
		return false;
	}
	
	SAFE_RELEASE_ARRAY(tbuf);
	SAFE_RELEASE_ARRAY(tbuf1);
	return true;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::logAccount(DBInterface * pdbi, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "insert into " KBE_TABLE_PERFIX "_accountinfos (accountName, password, bindata, email, entityDBID, flags, deadline, regtime, lasttime) values(";

	char* tbuf = new char[MAX_BUF > info.datas.size() ? MAX_BUF * 3 : info.datas.size() * 3];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, info.name.c_str(), info.name.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, info.password.c_str(), info.password.size());

	sqlstr += "md5(\"";
	sqlstr += tbuf;
	sqlstr += "\"),";

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, info.datas.data(), info.datas.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, info.email.c_str(), info.email.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, info.dbid);
	sqlstr += tbuf;
	sqlstr += ",";

	kbe_snprintf(tbuf, MAX_BUF, "%u", info.flags);
	sqlstr += tbuf;
	sqlstr += ",";
	
	kbe_snprintf(tbuf, MAX_BUF, "%" PRIu64, info.deadline);
	sqlstr += tbuf;
	sqlstr += ",";
	
	kbe_snprintf(tbuf, MAX_BUF, "%" PRTime, time(NULL));
	sqlstr += tbuf;
	sqlstr += ",";

	kbe_snprintf(tbuf, MAX_BUF, "%" PRTime, time(NULL));
	sqlstr += tbuf;
	sqlstr += ")";

	SAFE_RELEASE_ARRAY(tbuf);

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEAccountTableMysql::logAccount({}): sql({}) is failed({})!\n", 
				info.name, sqlstr, pdbi->getstrerror()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
KBEEmailVerificationTableMysql::KBEEmailVerificationTableMysql(EntityTables* pEntityTables) :
KBEEmailVerificationTable(pEntityTables)
{
}
	
//-------------------------------------------------------------------------------------
KBEEmailVerificationTableMysql::~KBEEmailVerificationTableMysql()
{
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableMysql::queryAccount(DBInterface * pdbi, int8 type, const std::string& name, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select code, datas from " KBE_TABLE_PERFIX "_email_verification where accountName=\"";

	char* tbuf = new char[name.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", type);
	sqlstr += tbuf;
	sqlstr += " LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::queryAccount({}): sql({}) is failed({})!\n", 
				name, sqlstr, pdbi->getstrerror()));

		return false;
	}

	info.datas = "";
	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow = mysql_fetch_row(pResult);
		if(arow != NULL)
		{
			info.name = name;
			info.datas = arow[0];
			info.password = arow[1];
		}

		mysql_free_result(pResult);
	}

	return info.datas.size() > 0;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableMysql::logAccount(DBInterface * pdbi, int8 type, const std::string& name, 
												const std::string& datas, const std::string& code)
{
	std::string sqlstr = "insert into " KBE_TABLE_PERFIX "_email_verification (accountName, type, datas, code, logtime) values(";

	char* tbuf = new char[MAX_BUF > datas.size() ? MAX_BUF * 3 : 
		(code.size() > datas.size() ? code.size() * 3 : datas.size() * 3)];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	kbe_snprintf(tbuf, MAX_BUF, "%d,", type);
	sqlstr += tbuf;
	
	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, datas.c_str(), datas.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";
	
	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, code.c_str(), code.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	kbe_snprintf(tbuf, MAX_BUF, "%" PRTime, time(NULL));

	sqlstr += tbuf;
	sqlstr += ")";

	SAFE_RELEASE_ARRAY(tbuf);

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::logAccount({}): sql({}) is failed({})!\n", 
				code, sqlstr, pdbi->getstrerror()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableMysql::activateAccount(DBInterface * pdbi, const std::string& code, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select accountName, datas, logtime from " KBE_TABLE_PERFIX "_email_verification where code=\"";

	char* tbuf = new char[code.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, code.c_str(), code.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", (int)KBEEmailVerificationTable::V_TYPE_CREATEACCOUNT);
	sqlstr += tbuf;
	sqlstr += " LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): sql({}) is failed({})!\n", 
				code, sqlstr, pdbi->getstrerror()));

		return false;
	}

	uint64 logtime = 1;

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow = mysql_fetch_row(pResult);
		if(arow != NULL)
		{
			info.name = arow[0];
			info.password = arow[1];
			
			KBEngine::StringConv::str2value(logtime, arow[2]);
		}

		mysql_free_result(pResult);
	}

	if(logtime > 0 && time(NULL) - logtime > g_kbeSrvConfig.emailAtivationInfo_.deadline)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): is expired! {} > {}.\n", 
				code, (time(NULL) - logtime), g_kbeSrvConfig.emailAtivationInfo_.deadline));

		return false;
	}

	if(info.name.size() == 0)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): name is NULL.\n", 
				code));

		return false;
	}
	
	std::string password = info.password;

	// 寻找dblog是否有此账号
	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::findByInterfaceName(pdbi->name()).findKBETable(KBE_TABLE_PERFIX "_accountinfos"));
	KBE_ASSERT(pTable);
	
	info.flags = 0;
	if(!pTable->queryAccount(pdbi, info.name, info))
	{
		return false;
	}

	if((info.flags & ACCOUNT_FLAG_NOT_ACTIVATED) <= 0)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): Has been activated, flags={}.\n", 
				code, info.flags));

		return false;
	}

	info.flags &= ~ACCOUNT_FLAG_NOT_ACTIVATED; 

	if(!pTable->setFlagsDeadline(pdbi, info.name, info.flags, info.deadline))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): set deadline error({})!\n", 
				code, pdbi->getstrerror()));
		return false;
	}

	if(!pTable->updatePassword(pdbi, info.name, password))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): update password error({})!\n", 
				code, pdbi->getstrerror()));

		return false;
	}

	info.dbid = 0;

	ScriptDefModule* pModule = EntityDef::findScriptModule(DBUtil::accountScriptName());

	// 防止多线程问题， 这里做一个拷贝。
	MemoryStream copyAccountDefMemoryStream(pTable->accountDefMemoryStream());

	info.dbid = EntityTables::findByInterfaceName(pdbi->name()).writeEntity(pdbi, 0, -1,
			&copyAccountDefMemoryStream, pModule);

	KBE_ASSERT(info.dbid > 0);

	// 如果查询失败则返回存在， 避免可能产生的错误
	tbuf = new char[MAX_BUF * 3];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, info.name.c_str(), info.name.size());

	if(!pdbi->query(fmt::format("update " KBE_TABLE_PERFIX "_accountinfos set entityDBID={} where accountName like \"{}\"", 
		info.dbid, tbuf), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): update " KBE_TABLE_PERFIX "_accountinfos error({})!\n", 
				code, pdbi->getstrerror()));

		SAFE_RELEASE_ARRAY(tbuf);
		return false;
	}

	SAFE_RELEASE_ARRAY(tbuf);

	try
	{
		delAccount(pdbi, (int8)V_TYPE_CREATEACCOUNT, info.name);
	}
	catch (...)
	{
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableMysql::bindEMail(DBInterface * pdbi, const std::string& name, const std::string& code)
{
	std::string sqlstr = "select accountName, datas, logtime from " KBE_TABLE_PERFIX "_email_verification where code=\"";

	char* tbuf = new char[code.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, code.c_str(), code.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", (int)KBEEmailVerificationTable::V_TYPE_BIND_MAIL);
	sqlstr += tbuf;
	sqlstr += " LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::bindEMail({}): sql({}) is failed({})!\n", 
				code, sqlstr, pdbi->getstrerror()));

		return false;
	}

	uint64 logtime = 1;

	std::string qname, qemail;

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow = mysql_fetch_row(pResult);
		if(arow != NULL)
		{
			qname = arow[0];
			qemail = arow[1];
			
			KBEngine::StringConv::str2value(logtime, arow[2]);
		}

		mysql_free_result(pResult);
	}

	if(logtime > 0 && time(NULL) - logtime > g_kbeSrvConfig.emailBindInfo_.deadline)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::bindEMail({}): is expired! {} > {}.\n", 
				code, (time(NULL) - logtime), g_kbeSrvConfig.emailBindInfo_.deadline));

		return false;
	}

	if(qname.size() == 0 || qemail.size() == 0)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::bindEMail({}): name or email is NULL.\n", 
				code));

		return false;
	}
	
	if(qemail != name)
	{
		WARNING_MSG(fmt::format("KBEEmailVerificationTableMysql::bindEMail: code({}) username({}:{}, {}) not match.\n" 
			, code, name, qname, qemail));

		return false;
	}

	tbuf = new char[code.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, qemail.c_str(), qemail.size());

	sqlstr = "update " KBE_TABLE_PERFIX "_accountinfos set email=\"";
	sqlstr += tbuf;
	sqlstr += "\" where accountName like \"";

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, qname.c_str(), qname.size());
	
	sqlstr += tbuf;
	sqlstr += "\"";

	SAFE_RELEASE_ARRAY(tbuf);

	if(!pdbi->query(sqlstr, false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::bindEMail({}): update " KBE_TABLE_PERFIX "_accountinfos({}) error({})!\n", 
				code, qname, pdbi->getstrerror()));

		return false;
	}

	try
	{
		delAccount(pdbi, (int8)V_TYPE_BIND_MAIL, name);
	}
	catch (...)
	{
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableMysql::resetpassword(DBInterface * pdbi, const std::string& name, 
												   const std::string& password, const std::string& code)
{
	std::string sqlstr = "select accountName, datas, logtime from " KBE_TABLE_PERFIX "_email_verification where code=\"";

	char* tbuf = new char[code.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, code.c_str(), code.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", (int)KBEEmailVerificationTable::V_TYPE_RESETPASSWORD);
	sqlstr += tbuf;
	sqlstr += " LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::resetpassword({}): sql({}) is failed({})!\n", 
				code, sqlstr, pdbi->getstrerror()));

		return false;
	}

	uint64 logtime = 1;
	
	std::string qname, qemail;

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow = mysql_fetch_row(pResult);
		if(arow != NULL)
		{
			qname = arow[0];
			qemail = arow[1];
			KBEngine::StringConv::str2value(logtime, arow[2]);
		}

		mysql_free_result(pResult);
	}

	if(logtime > 0 && time(NULL) - logtime > g_kbeSrvConfig.emailResetPasswordInfo_.deadline)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::resetpassword({}): is expired! {} > {}.\n", 
				code, (time(NULL) - logtime), g_kbeSrvConfig.emailResetPasswordInfo_.deadline));

		return false;
	}

	if(qname.size() == 0 || password.size() == 0)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::resetpassword({}): name or password is NULL.\n", 
				code));

		return false;
	}

	if(qname != name)
	{
		WARNING_MSG(fmt::format("KBEEmailVerificationTableMysql::resetpassword: code({}) username({} != {}) not match.\n" 
			, code, name, qname));

		return false;
	}

	// 寻找dblog是否有此账号
	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::findByInterfaceName(pdbi->name()).findKBETable(KBE_TABLE_PERFIX "_accountinfos"));
	KBE_ASSERT(pTable);

	if(!pTable->updatePassword(pdbi, qname, KBE_MD5::getDigest(password.data(), password.length())))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::resetpassword({}): update accountName({}) password error({})!\n", 
				code, qname, pdbi->getstrerror()));

		return false;
	}


	try
	{
		delAccount(pdbi, (int8)V_TYPE_RESETPASSWORD, qname);
	}
	catch (...)
	{
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableMysql::delAccount(DBInterface * pdbi, int8 type, const std::string& name)
{
	std::string sqlstr = "delete from " KBE_TABLE_PERFIX "_email_verification where accountName=";

	char* tbuf = new char[MAX_BUF * 3];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\" and type=";
	
	kbe_snprintf(tbuf, MAX_BUF, "%d", type);
	sqlstr += tbuf;

	SAFE_RELEASE_ARRAY(tbuf);

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::delAccount({}): sql({}) is failed({})!\n", 
				name, sqlstr, pdbi->getstrerror()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableMysql::syncToDB(DBInterface* pdbi)
{
	bool ret = false;

	std::string sqlstr = fmt::format("CREATE TABLE IF NOT EXISTS " KBE_TABLE_PERFIX "_email_verification "
			"(accountName varchar({}) not null,"
			"type tinyint not null DEFAULT 0,"
			"datas varchar(255),"
			"code varchar(128), PRIMARY KEY idKey (code),"
			"logtime bigint(20) not null DEFAULT 0)"
			"ENGINE=" MYSQL_ENGINE_TYPE, ACCOUNT_NAME_MAX_LENGTH);

	ret = pdbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);

	// 删除xx小时之前的记录
	sqlstr = fmt::format("delete from " KBE_TABLE_PERFIX "_email_verification where logtime<{} and type={}", 
		KBEngine::StringConv::val2str(time(NULL) - g_kbeSrvConfig.emailAtivationInfo_.deadline), 
		((int)KBEEmailVerificationTable::V_TYPE_CREATEACCOUNT));

	ret = pdbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);

	sqlstr = fmt::format("delete from " KBE_TABLE_PERFIX "_email_verification where logtime<{} and type={}", 
		KBEngine::StringConv::val2str(time(NULL) - g_kbeSrvConfig.emailResetPasswordInfo_.deadline),
		((int)KBEEmailVerificationTable::V_TYPE_RESETPASSWORD));

	ret = pdbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);

	sqlstr = fmt::format("delete from " KBE_TABLE_PERFIX "_email_verification where logtime<{} and type={}", 
		KBEngine::StringConv::val2str(time(NULL) - g_kbeSrvConfig.emailBindInfo_.deadline), 
		((int)KBEEmailVerificationTable::V_TYPE_BIND_MAIL));

	ret = pdbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);
	return ret;
}

//-------------------------------------------------------------------------------------
}
