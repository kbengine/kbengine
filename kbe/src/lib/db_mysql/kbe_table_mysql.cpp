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

#include "entity_table_mysql.h"
#include "kbe_table_mysql.h"
#include "db_exception.h"
#include "db_interface_mysql.h"
#include "db_interface/db_interface.h"
#include "db_interface/entity_table.h"
#include "entitydef/entitydef.h"
#include "entitydef/scriptdef_module.h"
#include "server/serverconfig.h"

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
			"(entityDBID bigint(20) unsigned not null DEFAULT 0,"
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

	try
	{
		if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
		{
			// 1062 int err = dbi->getlasterror(); 
			return false;
		}
	}
	catch (std::exception & e)
	{
		DBException& dbe = static_cast<DBException&>(e);
		if(dbe.isLostConnection())
		{
			if(dbi->processException(e))
				return true;
		}

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::queryEntity(DBInterface * dbi, DBID dbid, EntityLog& entitylog, ENTITY_SCRIPT_UID entityType)
{
	std::string sqlstr = "select entityID, ip, port, componentID from kbe_entitylog where entityDBID=";

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
			kbe_snprintf(entitylog.ip, MAX_IP, "%s", arow[1]);
			StringConv::str2value(entitylog.port, arow[2]);
			StringConv::str2value(entitylog.componentID, arow[3]);
			break;
		}

		mysql_free_result(pResult);
	}

	return entitylog.componentID > 0;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::eraseEntityLog(DBInterface * dbi, DBID dbid, ENTITY_SCRIPT_UID entityType)
{
	std::string sqlstr = "delete from kbe_entitylog where entityDBID=";

	char tbuf[MAX_BUF];

	kbe_snprintf(tbuf, MAX_BUF, "%"PRDBID, dbid);
	sqlstr += tbuf;

	sqlstr += " and entityType=";
	kbe_snprintf(tbuf, MAX_BUF, "%u", entityType);
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
			"(`accountName` varchar(255) not null, PRIMARY KEY idKey (`accountName`),"
			"`password` varchar(255),"
			"`bindata` blob,"
			"`email` varchar(255),"
			"`entityDBID` bigint(20) unsigned not null DEFAULT 0, UNIQUE KEY `entityDBID` (`entityDBID`),"
			"`flags` int unsigned not null DEFAULT 0,"
			"`deadline` bigint(20) not null DEFAULT 0,"
			"`regtime` bigint(20) not null DEFAULT 0,"
			"`lasttime` bigint(20) not null DEFAULT 0,"
			"`numlogin` int unsigned not null DEFAULT 0)"
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
bool KBEAccountTableMysql::setFlagsDeadline(DBInterface * dbi, const std::string& name, uint32 flags, uint64 deadline)
{
	char* tbuf = new char[name.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	std::string sqlstr = fmt::format("update kbe_accountinfos set flags={}, deadline={} where accountName like \"{}\"", 
		flags, deadline, tbuf);

	SAFE_RELEASE_ARRAY(tbuf);

	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(dbi->query(sqlstr.c_str(), sqlstr.size(), false))
		return true;

	return false;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::queryAccount(DBInterface * dbi, const std::string& name, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select entityDBID, password, flags, deadline from kbe_accountinfos where accountName like \"";

	char* tbuf = new char[name.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	sqlstr += tbuf;

	SAFE_RELEASE_ARRAY(tbuf);
	sqlstr += "\"";

	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
		return true;

	info.dbid = 0;
	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow;
		while((arow = mysql_fetch_row(pResult)) != NULL)
		{
			KBEngine::StringConv::str2value(info.dbid, arow[0]);
			info.name = name;
			info.password = arow[1];

			KBEngine::StringConv::str2value(info.flags, arow[2]);
			KBEngine::StringConv::str2value(info.deadline, arow[3]);
		}

		mysql_free_result(pResult);
	}

	return info.dbid > 0;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::queryAccountAllInfos(DBInterface * dbi, const std::string& name, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select entityDBID, password, email, flags, deadline from kbe_accountinfos where accountName like \"";

	char* tbuf = new char[name.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	sqlstr += tbuf;

	SAFE_RELEASE_ARRAY(tbuf);
	sqlstr += "\"";

	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
		return true;

	info.dbid = 0;
	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow;
		while((arow = mysql_fetch_row(pResult)) != NULL)
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
bool KBEAccountTableMysql::updateCount(DBInterface * dbi, DBID dbid)
{
	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!dbi->query(fmt::format("update kbe_accountinfos set lasttime={}, numlogin=numlogin+1 where entityDBID={}",
		time(NULL), dbid), false))
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::updatePassword(DBInterface * dbi, const std::string& name, const std::string& password)
{
	char* tbuf = new char[MAX_BUF * 3];
	char* tbuf1 = new char[MAX_BUF * 3];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, password.c_str(), password.size());

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf1, name.c_str(), name.size());

	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!dbi->query(fmt::format("update kbe_accountinfos set password=\"{}\" where accountName like \"{}\"", 
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
bool KBEAccountTableMysql::logAccount(DBInterface * dbi, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "insert into kbe_accountinfos (accountName, password, bindata, email, entityDBID, flags, deadline, regtime, lasttime) values(";

	char* tbuf = new char[MAX_BUF > info.datas.size() ? MAX_BUF * 3 : info.datas.size() * 3];

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

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, info.email.c_str(), info.email.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	kbe_snprintf(tbuf, MAX_BUF, "%"PRDBID, info.dbid);
	sqlstr += tbuf;
	sqlstr += ",";

	kbe_snprintf(tbuf, MAX_BUF, "%u", info.flags);
	sqlstr += tbuf;
	sqlstr += ",";
	
	kbe_snprintf(tbuf, MAX_BUF, "%"PRIu64, info.deadline);
	sqlstr += tbuf;
	sqlstr += ",";
	
	kbe_snprintf(tbuf, MAX_BUF, "%"PRTime, time(NULL));
	sqlstr += tbuf;
	sqlstr += ",";

	kbe_snprintf(tbuf, MAX_BUF, "%"PRTime, time(NULL));
	sqlstr += tbuf;
	sqlstr += ")";

	SAFE_RELEASE_ARRAY(tbuf);

	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEAccountTableMysql::logAccount({}): sql({}) is failed({})!\n", 
				info.name, sqlstr, dbi->getstrerror()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
KBEEmailVerificationTableMysql::KBEEmailVerificationTableMysql():
KBEEmailVerificationTable()
{
}
	
//-------------------------------------------------------------------------------------
KBEEmailVerificationTableMysql::~KBEEmailVerificationTableMysql()
{
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableMysql::queryAccount(DBInterface * dbi, int8 type, const std::string& name, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select code, datas from kbe_email_verification where accountName like \"";

	char* tbuf = new char[name.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", type);
	sqlstr += tbuf;

	SAFE_RELEASE_ARRAY(tbuf);

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::queryAccount({}): sql({}) is failed({})!\n", 
				name, sqlstr, dbi->getstrerror()));

		return false;
	}

	info.datas = "";
	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow;
		while((arow = mysql_fetch_row(pResult)) != NULL)
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
bool KBEEmailVerificationTableMysql::logAccount(DBInterface * dbi, int8 type, const std::string& name, 
												const std::string& datas, const std::string& code)
{
	std::string sqlstr = "insert into kbe_email_verification (accountName, type, datas, code, logtime) values(";

	char* tbuf = new char[MAX_BUF > datas.size() ? MAX_BUF * 3 : 
		(code.size() > datas.size() ? code.size() * 3 : datas.size() * 3)];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	kbe_snprintf(tbuf, MAX_BUF, "%d,", type);
	sqlstr += tbuf;
	
	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, datas.c_str(), datas.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";
	
	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, code.c_str(), code.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	kbe_snprintf(tbuf, MAX_BUF, "%"PRTime, time(NULL));

	sqlstr += tbuf;
	sqlstr += ")";

	SAFE_RELEASE_ARRAY(tbuf);

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::logAccount({}): sql({}) is failed({})!\n", 
				code, sqlstr, dbi->getstrerror()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableMysql::activateAccount(DBInterface * dbi, const std::string& code, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select accountName, datas, logtime from kbe_email_verification where code like \"";

	char* tbuf = new char[code.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, code.c_str(), code.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", (int)KBEEmailVerificationTable::V_TYPE_CREATEACCOUNT);
	sqlstr += tbuf;

	SAFE_RELEASE_ARRAY(tbuf);

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): sql({}) is failed({})!\n", 
				code, sqlstr, dbi->getstrerror()));

		return false;
	}

	uint64 logtime = 1;

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow;
		while((arow = mysql_fetch_row(pResult)) != NULL)
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

	// Ѱ��dblog�Ƿ��д��˺�
	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));
	KBE_ASSERT(pTable);
	
	info.flags = 0;
	if(!pTable->queryAccount(dbi, info.name, info))
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

	if(!pTable->setFlagsDeadline(dbi, info.name, info.flags, info.deadline))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): set deadline is error({})!\n", 
				code, dbi->getstrerror()));
		return false;
	}

	if(!pTable->updatePassword(dbi, info.name, password))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): update password is error({})!\n", 
				code, dbi->getstrerror()));

		return false;
	}

	info.dbid = 0;

	ScriptDefModule* pModule = EntityDef::findScriptModule(DBUtil::accountScriptName());

	// ��ֹ���߳����⣬ ������һ��������
	MemoryStream copyAccountDefMemoryStream(pTable->accountDefMemoryStream());

	info.dbid = EntityTables::getSingleton().writeEntity(dbi, 0, -1,
			&copyAccountDefMemoryStream, pModule);

	KBE_ASSERT(info.dbid > 0);

	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	tbuf = new char[MAX_BUF * 3];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, info.name.c_str(), info.name.size());

	if(!dbi->query(fmt::format("update kbe_accountinfos set entityDBID={} where accountName like \"{}\"", 
		info.dbid, tbuf), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): update kbe_accountinfos is error({})!\n", 
				code, dbi->getstrerror()));

		SAFE_RELEASE_ARRAY(tbuf);
		return false;
	}

	SAFE_RELEASE_ARRAY(tbuf);

	try
	{
		delAccount(dbi, (int8)V_TYPE_CREATEACCOUNT, info.name);
	}
	catch (...)
	{
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableMysql::bindEMail(DBInterface * dbi, const std::string& name, const std::string& code)
{
	std::string sqlstr = "select accountName, datas, logtime from kbe_email_verification where code like \"";

	char* tbuf = new char[code.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, code.c_str(), code.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", (int)KBEEmailVerificationTable::V_TYPE_BIND_MAIL);
	sqlstr += tbuf;

	SAFE_RELEASE_ARRAY(tbuf);

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::bindEMail({}): sql({}) is failed({})!\n", 
				code, sqlstr, dbi->getstrerror()));

		return false;
	}

	uint64 logtime = 1;

	std::string qname, qemail;

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow;
		while((arow = mysql_fetch_row(pResult)) != NULL)
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
	
	if(qname != name)
	{
		WARNING_MSG(fmt::format("KBEEmailVerificationTableMysql::bindEMail: code({}) username({}, {}) not match.\n" 
			, code, name, qname));

		return false;
	}

	tbuf = new char[code.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, qemail.c_str(), qemail.size());

	sqlstr = "update kbe_accountinfos set email=\"";
	sqlstr += tbuf;
	sqlstr += "\" where accountName like \"";

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, name.c_str(), name.size());
	
	sqlstr += tbuf;
	sqlstr += "\"";

	SAFE_RELEASE_ARRAY(tbuf);

	if(!dbi->query(sqlstr, false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::bindEMail({}): update kbe_accountinfos is error({})!\n", 
				code, dbi->getstrerror()));

		return false;
	}

	try
	{
		delAccount(dbi, (int8)V_TYPE_BIND_MAIL, name);
	}
	catch (...)
	{
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableMysql::resetpassword(DBInterface * dbi, const std::string& name, 
												   const std::string& password, const std::string& code)
{
	std::string sqlstr = "select accountName, logtime from kbe_email_verification where code like \"";

	char* tbuf = new char[code.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, code.c_str(), code.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", (int)KBEEmailVerificationTable::V_TYPE_RESETPASSWORD);
	sqlstr += tbuf;

	SAFE_RELEASE_ARRAY(tbuf);

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::resetpassword({}): sql({}) is failed({})!\n", 
				code, sqlstr, dbi->getstrerror()));

		return false;
	}

	uint64 logtime = 1;
	
	std::string qname;

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow;
		while((arow = mysql_fetch_row(pResult)) != NULL)
		{
			qname = arow[0];
			KBEngine::StringConv::str2value(logtime, arow[1]);
		}

		mysql_free_result(pResult);
	}

	if(logtime > 0 && time(NULL) - logtime > g_kbeSrvConfig.emailResetPasswordInfo_.deadline)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::bindEMail({}): is expired! {} > {}.\n", 
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
		WARNING_MSG(fmt::format("KBEEmailVerificationTableMysql::resetpassword: code({}) username({}, {}) not match.\n" 
			, code, name, qname));

		return false;
	}

	// Ѱ��dblog�Ƿ��д��˺�
	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));
	KBE_ASSERT(pTable);

	if(!pTable->updatePassword(dbi, name, KBE_MD5::getDigest(password.data(), password.length())))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::resetpassword({}): update password is error({})!\n", 
				code, dbi->getstrerror()));

		return false;
	}


	try
	{
		delAccount(dbi, (int8)V_TYPE_RESETPASSWORD, name);
	}
	catch (...)
	{
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableMysql::delAccount(DBInterface * dbi, int8 type, const std::string& name)
{
	std::string sqlstr = "delete from kbe_email_verification where accountName=";

	char* tbuf = new char[MAX_BUF * 3];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\" and type=";
	
	kbe_snprintf(tbuf, MAX_BUF, "%d", type);
	sqlstr += tbuf;

	SAFE_RELEASE_ARRAY(tbuf);

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::delAccount({}): sql({}) is failed({})!\n", 
				name, sqlstr, dbi->getstrerror()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableMysql::syncToDB(DBInterface* dbi)
{
	bool ret = false;

	std::string sqlstr = "CREATE TABLE IF NOT EXISTS kbe_email_verification "
			"(accountName varchar(255) not null,"
			"type tinyint not null DEFAULT 0,"
			"datas varchar(255),"
			"code varchar(255), PRIMARY KEY idKey (code),"
			"logtime bigint(20) not null DEFAULT 0)"
		"ENGINE="MYSQL_ENGINE_TYPE;

	ret = dbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);

	// ɾ��xxСʱ֮ǰ�ļ�¼
	sqlstr = fmt::format("delete from kbe_email_verification where logtime<{} and type={}", 
		KBEngine::StringConv::val2str(time(NULL) - g_kbeSrvConfig.emailAtivationInfo_.deadline), 
		((int)KBEEmailVerificationTable::V_TYPE_CREATEACCOUNT));

	ret = dbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);

	sqlstr = fmt::format("delete from kbe_email_verification where logtime<{} and type={}", 
		KBEngine::StringConv::val2str(time(NULL) - g_kbeSrvConfig.emailResetPasswordInfo_.deadline),
		((int)KBEEmailVerificationTable::V_TYPE_RESETPASSWORD));

	ret = dbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);

	sqlstr = fmt::format("delete from kbe_email_verification where logtime<{} and type={}", 
		KBEngine::StringConv::val2str(time(NULL) - g_kbeSrvConfig.emailBindInfo_.deadline), 
		((int)KBEEmailVerificationTable::V_TYPE_BIND_MAIL));

	ret = dbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);
	return ret;
}

//-------------------------------------------------------------------------------------
}
