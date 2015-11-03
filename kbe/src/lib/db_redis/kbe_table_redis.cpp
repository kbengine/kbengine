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

//#include "entity_table_redis.h"
#include "redis_helper.h"
#include "kbe_table_redis.h"
#include "db_interface_redis.h"
#include "db_interface/db_interface.h"
#include "db_interface/entity_table.h"
#include "entitydef/entitydef.h"
#include "entitydef/scriptdef_module.h"
#include "server/serverconfig.h"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableRedis::syncToDB(DBInterface* dbi)
{
	/*
	有数据时才产生表数据
	kbe_entitylog:dbid:entityType = hashes(entityID, ip, port, componentID)
	*/

	return RedisHelper::dropTable(static_cast<DBInterfaceRedis*>(dbi), fmt::format("kbe_entitylog:*:*"), false);
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableRedis::logEntity(DBInterface * dbi, const char* ip, uint32 port, DBID dbid,
					COMPONENT_ID componentID, ENTITY_ID entityID, ENTITY_SCRIPT_UID entityType)
{
	/*
	kbe_entitylog:dbid:entityType = hashes(entityID, ip, port, componentID)
	*/
	std::string sqlstr = fmt::format("HSET kbe_entitylog:{}:{} entityID {} ip {} port {} componentID {}", 
		dbid, entityType, entityID, ip, port, componentID);

	try
	{
		dbi->query(sqlstr.c_str(), sqlstr.size(), false);
	}
	catch(...)
	{
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableRedis::queryEntity(DBInterface * dbi, DBID dbid, EntityLog& entitylog, ENTITY_SCRIPT_UID entityType)
{
	/*
	kbe_entitylog:dbid:entityType = hashes(entityID, ip, port, componentID)
	*/
	redisReply* pRedisReply = NULL;

	try
	{
		static_cast<DBInterfaceRedis*>(dbi)->query(fmt::format("HMGET kbe_entitylog:{}:{} entityID ip port componentID",
			dbid, entityType), &pRedisReply, false);
	}
	catch(...)
	{
	}

	entitylog.dbid = dbid;
	entitylog.componentID = 0;
	entitylog.entityID = 0;
	entitylog.ip[0] = '\0';
	entitylog.port = 0;

	if(pRedisReply)
	{
		if(pRedisReply->type == REDIS_REPLY_ARRAY)
		{
			if(RedisHelper::check_array_results(pRedisReply))
			{
				StringConv::str2value(entitylog.entityID, pRedisReply->element[0]->str);
				kbe_snprintf(entitylog.ip, MAX_IP, "%s", pRedisReply->element[1]->str);
				StringConv::str2value(entitylog.port, pRedisReply->element[2]->str);
				StringConv::str2value(entitylog.componentID, pRedisReply->element[3]->str);
			}
		}
		
		freeReplyObject(pRedisReply); 
	}

	return entitylog.componentID > 0;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableRedis::eraseEntityLog(DBInterface * dbi, DBID dbid, ENTITY_SCRIPT_UID entityType)
{
	std::string sqlstr = fmt::format("HDEL kbe_entitylog:{}:{}", 
		dbid, entityType);
	
	try
	{
		dbi->query(sqlstr.c_str(), sqlstr.size(), false);
	}
	catch(...)
	{
	}

	return true;
}

//-------------------------------------------------------------------------------------
KBEEntityLogTableRedis::KBEEntityLogTableRedis():
	KBEEntityLogTable()
{
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::syncToDB(DBInterface* dbi)
{
	return true;
}

//-------------------------------------------------------------------------------------
KBEAccountTableRedis::KBEAccountTableRedis():
	KBEAccountTable()
{
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::setFlagsDeadline(DBInterface * dbi, const std::string& name, uint32 flags, uint64 deadline)
{
	/*
	kbe_accountinfos:accountName = hashes(password, bindata, email, entityDBID, flags, deadline, regtime, lasttime, numlogin)
	*/
	
	// 如果查询失败则返回存在， 避免可能产生的错误
	if(dbi->query(fmt::format("HSET kbe_accountinfos:{} flags {} deadline {}", 
		name, flags, deadline), false))
		return true;

	return false;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::queryAccount(DBInterface * dbi, const std::string& name, ACCOUNT_INFOS& info)
{
	/*
	kbe_accountinfos:accountName = hashes(password, bindata, email, entityDBID, flags, deadline, regtime, lasttime, numlogin)
	*/
	redisReply* pRedisReply = NULL;

	try
	{
		static_cast<DBInterfaceRedis*>(dbi)->query(fmt::format("HMGET kbe_accountinfos:{} entityDBID password flags deadline",
			name), &pRedisReply, false);
	}
	catch(...)
	{
	}
	
	info.dbid = 0;
	
	if(pRedisReply)
	{
		if(pRedisReply->type == REDIS_REPLY_ARRAY)
		{
			if(RedisHelper::check_array_results(pRedisReply))
			{			
				StringConv::str2value(info.dbid, pRedisReply->element[0]->str);
				info.name = name;
				info.password = pRedisReply->element[1]->str;

				StringConv::str2value(info.flags, pRedisReply->element[2]->str);
				StringConv::str2value(info.deadline, pRedisReply->element[3]->str);
			}
		}
		
		freeReplyObject(pRedisReply); 
	}

	return info.dbid > 0;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::queryAccountAllInfos(DBInterface * dbi, const std::string& name, ACCOUNT_INFOS& info)
{
	/*
	kbe_accountinfos:accountName = hashes(password, bindata, email, entityDBID, flags, deadline, regtime, lasttime, numlogin)
	*/
	redisReply* pRedisReply = NULL;

	try
	{
		static_cast<DBInterfaceRedis*>(dbi)->query(fmt::format("HMGET kbe_accountinfos:{} entityDBID password email flags deadline",
			name), &pRedisReply, false);
	}
	catch(...)
	{
	}
	
	info.dbid = 0;
	
	if(pRedisReply)
	{
		if(pRedisReply->type == REDIS_REPLY_ARRAY)
		{
			if(RedisHelper::check_array_results(pRedisReply))
			{			
				StringConv::str2value(info.dbid, pRedisReply->element[0]->str);
				info.name = name;
				info.password = pRedisReply->element[1]->str;
				info.email = pRedisReply->element[2]->str;
				StringConv::str2value(info.flags, pRedisReply->element[3]->str);
				StringConv::str2value(info.deadline, pRedisReply->element[4]->str);
			}
		}
		
		freeReplyObject(pRedisReply); 
	}

	return info.dbid > 0;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::updateCount(DBInterface * dbi, const std::string& name, DBID dbid)
{
	/*
	kbe_accountinfos:accountName = hashes(password, bindata, email, entityDBID, flags, deadline, regtime, lasttime, numlogin)
	*/
	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!static_cast<DBInterfaceRedis*>(dbi)->queryAppend(false, "HINCRBY kbe_accountinfos:{} numlogin", name))
		return false;

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!static_cast<DBInterfaceRedis*>(dbi)->queryAppend(false, "HSET kbe_accountinfos:{} lasttime {}", name, time(NULL)))
		return false;

	redisReply* pRedisReply = NULL;
	int replys = 0;
	
	static_cast<DBInterfaceRedis*>(dbi)->getQueryReply(&pRedisReply);
	if(pRedisReply)
	{
		++replys;
		freeReplyObject(pRedisReply); 
		pRedisReply = NULL;
	}

	static_cast<DBInterfaceRedis*>(dbi)->getQueryReply(&pRedisReply);
	if(pRedisReply)
	{
		++replys;
		freeReplyObject(pRedisReply); 
		pRedisReply = NULL;
	}
	
	return replys == 2;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::updatePassword(DBInterface * dbi, const std::string& name, const std::string& password)
{
	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!dbi->query(fmt::format("HSET kbe_accountinfos:{} password {}", name, password), false))
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::logAccount(DBInterface * dbi, ACCOUNT_INFOS& info)
{
	std::string sqlstr = fmt::format("HSET kbe_accountinfos:{} accountName {} password {} bindata {} ",
		"email {} entityDBID {} flags {} deadline {} regtime {} lasttime {}", 
		info.name, KBE_MD5::getDigest(info.password.data(), info.password.length()).c_str(),
		info.datas, info.email, info.dbid, info.flags, info.deadline, time(NULL), time(NULL));

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEAccountTableRedis::logAccount({}): sql({}) is failed({})!\n", 
				info.name, sqlstr, dbi->getstrerror()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
KBEEmailVerificationTableRedis::KBEEmailVerificationTableRedis():
KBEEmailVerificationTable()
{
}
	
//-------------------------------------------------------------------------------------
KBEEmailVerificationTableRedis::~KBEEmailVerificationTableRedis()
{
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableRedis::queryAccount(DBInterface * dbi, int8 type, const std::string& name, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select code, datas from kbe_email_verification where accountName=\"";

	char* tbuf = new char[name.size() * 2 + 1];

	//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
	//	tbuf, name.c_str(), name.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", type);
	sqlstr += tbuf;
	sqlstr += " LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::queryAccount({}): sql({}) is failed({})!\n", 
				name, sqlstr, dbi->getstrerror()));

		return false;
	}

	info.datas = "";
	//MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	//if(pResult)
	//{
	//	MYSQL_ROW arow = mysql_fetch_row(pResult);
	//	if(arow != NULL)
	//	{
	//		info.name = name;
	//		info.datas = arow[0];
	//		info.password = arow[1];
	//	}

	//	mysql_free_result(pResult);
	//}

	return info.datas.size() > 0;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableRedis::logAccount(DBInterface * dbi, int8 type, const std::string& name, 
												const std::string& datas, const std::string& code)
{
	std::string sqlstr = "insert into kbe_email_verification (accountName, type, datas, code, logtime) values(";

	char* tbuf = new char[MAX_BUF > datas.size() ? MAX_BUF * 3 : 
		(code.size() > datas.size() ? code.size() * 3 : datas.size() * 3)];

	//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
	//	tbuf, name.c_str(), name.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	kbe_snprintf(tbuf, MAX_BUF, "%d,", type);
	sqlstr += tbuf;
	
	//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
	//	tbuf, datas.c_str(), datas.size());

	//sqlstr += "\"";
	//sqlstr += tbuf;
	//sqlstr += "\",";
	//
	//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
	//	tbuf, code.c_str(), code.size());

	//sqlstr += "\"";
	//sqlstr += tbuf;
	//sqlstr += "\",";

	//kbe_snprintf(tbuf, MAX_BUF, "%" PRTime, time(NULL));

	//sqlstr += tbuf;
	//sqlstr += ")";

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
bool KBEEmailVerificationTableRedis::activateAccount(DBInterface * dbi, const std::string& code, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select accountName, datas, logtime from kbe_email_verification where code=\"";

	char* tbuf = new char[code.size() * 2 + 1];

	//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
	//	tbuf, code.c_str(), code.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", (int)KBEEmailVerificationTable::V_TYPE_CREATEACCOUNT);
	sqlstr += tbuf;
	sqlstr += " LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): sql({}) is failed({})!\n", 
				code, sqlstr, dbi->getstrerror()));

		return false;
	}

	uint64 logtime = 1;

	//MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	//if(pResult)
	//{
	//	MYSQL_ROW arow = mysql_fetch_row(pResult);
	//	if(arow != NULL)
	//	{
	//		info.name = arow[0];
	//		info.password = arow[1];
	//		
	//		KBEngine::StringConv::str2value(logtime, arow[2]);
	//	}

	//	mysql_free_result(pResult);
	//}

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
	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));
	KBE_ASSERT(pTable);
	
	info.flags = 0;
	if(!pTable->queryAccount(dbi, info.name, info))
	{
		return false;
	}

	if((info.flags & ACCOUNT_FLAG_NOT_ACTIVATED) <= 0)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::activateAccount({}): Has been activated, flags={}.\n", 
				code, info.flags));

		return false;
	}

	info.flags &= ~ACCOUNT_FLAG_NOT_ACTIVATED; 

	if(!pTable->setFlagsDeadline(dbi, info.name, info.flags, info.deadline))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::activateAccount({}): set deadline is error({})!\n", 
				code, dbi->getstrerror()));
		return false;
	}

	if(!pTable->updatePassword(dbi, info.name, password))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::activateAccount({}): update password is error({})!\n", 
				code, dbi->getstrerror()));

		return false;
	}

	info.dbid = 0;

	ScriptDefModule* pModule = EntityDef::findScriptModule(DBUtil::accountScriptName());

	// 防止多线程问题， 这里做一个拷贝。
	MemoryStream copyAccountDefMemoryStream(pTable->accountDefMemoryStream());

	info.dbid = EntityTables::getSingleton().writeEntity(dbi, 0, -1,
			&copyAccountDefMemoryStream, pModule);

	KBE_ASSERT(info.dbid > 0);

	// 如果查询失败则返回存在， 避免可能产生的错误
	tbuf = new char[MAX_BUF * 3];

//	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
	//	tbuf, info.name.c_str(), info.name.size());

	if(!dbi->query(fmt::format("update kbe_accountinfos set entityDBID={} where accountName like \"{}\"", 
		info.dbid, tbuf), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::activateAccount({}): update kbe_accountinfos is error({})!\n", 
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
bool KBEEmailVerificationTableRedis::bindEMail(DBInterface * dbi, const std::string& name, const std::string& code)
{
	std::string sqlstr = "select accountName, datas, logtime from kbe_email_verification where code=\"";

	char* tbuf = new char[code.size() * 2 + 1];

//	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
//		tbuf, code.c_str(), code.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", (int)KBEEmailVerificationTable::V_TYPE_BIND_MAIL);
	sqlstr += tbuf;
	sqlstr += " LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::bindEMail({}): sql({}) is failed({})!\n", 
				code, sqlstr, dbi->getstrerror()));

		return false;
	}

	uint64 logtime = 1;

	std::string qname, qemail;

	//MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	//if(pResult)
	//{
	//	MYSQL_ROW arow = mysql_fetch_row(pResult);
	//	if(arow != NULL)
	//	{
	//		qname = arow[0];
	//		qemail = arow[1];
	//		
	//		KBEngine::StringConv::str2value(logtime, arow[2]);
	//	}

	//	mysql_free_result(pResult);
	//}

	if(logtime > 0 && time(NULL) - logtime > g_kbeSrvConfig.emailBindInfo_.deadline)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::bindEMail({}): is expired! {} > {}.\n", 
				code, (time(NULL) - logtime), g_kbeSrvConfig.emailBindInfo_.deadline));

		return false;
	}

	if(qname.size() == 0 || qemail.size() == 0)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::bindEMail({}): name or email is NULL.\n", 
				code));

		return false;
	}
	
	if(qemail != name)
	{
		WARNING_MSG(fmt::format("KBEEmailVerificationTableRedis::bindEMail: code({}) username({}:{}, {}) not match.\n" 
			, code, name, qname, qemail));

		return false;
	}

	tbuf = new char[code.size() * 2 + 1];

//	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
//		tbuf, qemail.c_str(), qemail.size());

	sqlstr = "update kbe_accountinfos set email=\"";
	sqlstr += tbuf;
	sqlstr += "\" where accountName like \"";

//	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
	//	tbuf, qname.c_str(), qname.size());
	
	sqlstr += tbuf;
	sqlstr += "\"";

	SAFE_RELEASE_ARRAY(tbuf);

	if(!dbi->query(sqlstr, false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::bindEMail({}): update kbe_accountinfos({}) error({})!\n", 
				code, qname, dbi->getstrerror()));

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
bool KBEEmailVerificationTableRedis::resetpassword(DBInterface * dbi, const std::string& name, 
												   const std::string& password, const std::string& code)
{
	std::string sqlstr = "select accountName, datas, logtime from kbe_email_verification where code=\"";

	char* tbuf = new char[code.size() * 2 + 1];

	//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
	//	tbuf, code.c_str(), code.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", (int)KBEEmailVerificationTable::V_TYPE_RESETPASSWORD);
	sqlstr += tbuf;
	sqlstr += " LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::resetpassword({}): sql({}) is failed({})!\n", 
				code, sqlstr, dbi->getstrerror()));

		return false;
	}

	uint64 logtime = 1;
	
	std::string qname, qemail;

	//MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	//if(pResult)
	//{
	//	MYSQL_ROW arow = mysql_fetch_row(pResult);
	//	if(arow != NULL)
	//	{
	//		qname = arow[0];
	//		qemail = arow[1];
	//		KBEngine::StringConv::str2value(logtime, arow[2]);
	//	}

	//	mysql_free_result(pResult);
	//}

	if(logtime > 0 && time(NULL) - logtime > g_kbeSrvConfig.emailResetPasswordInfo_.deadline)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::resetpassword({}): is expired! {} > {}.\n", 
				code, (time(NULL) - logtime), g_kbeSrvConfig.emailResetPasswordInfo_.deadline));

		return false;
	}

	if(qname.size() == 0 || password.size() == 0)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::resetpassword({}): name or password is NULL.\n", 
				code));

		return false;
	}

	if(qname != name)
	{
		WARNING_MSG(fmt::format("KBEEmailVerificationTableRedis::resetpassword: code({}) username({} != {}) not match.\n" 
			, code, name, qname));

		return false;
	}

	// 寻找dblog是否有此账号
	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::getSingleton().findKBETable("kbe_accountinfos"));
	KBE_ASSERT(pTable);

	if(!pTable->updatePassword(dbi, qname, KBE_MD5::getDigest(password.data(), password.length())))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::resetpassword({}): update accountName({}) password error({})!\n", 
				code, qname, dbi->getstrerror()));

		return false;
	}


	try
	{
		delAccount(dbi, (int8)V_TYPE_RESETPASSWORD, qname);
	}
	catch (...)
	{
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableRedis::delAccount(DBInterface * dbi, int8 type, const std::string& name)
{
	std::string sqlstr = "delete from kbe_email_verification where accountName=";

	char* tbuf = new char[MAX_BUF * 3];

	//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
	//	tbuf, name.c_str(), name.size());

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
bool KBEEmailVerificationTableRedis::syncToDB(DBInterface* dbi)
{
	/*
	kbe_email_verification:code = hashes(entityID, ip, port, componentID)
	*/
	bool ret = false;

	std::string sqlstr = "CREATE TABLE IF NOT EXISTS kbe_email_verification "
			"(accountName varchar(255) not null,"
			"type tinyint not null DEFAULT 0,"
			"datas varchar(255),"
			"code varchar(255), PRIMARY KEY idKey (code),"
			"logtime bigint(20) not null DEFAULT 0)"
		"ENGINE=" ;

	ret = dbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);

	// 删除xx小时之前的记录
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
