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
bool KBEEntityLogTableRedis::syncToDB(DBInterface* pdbi)
{
	/*
	有数据时才产生表数据
	kbe_entitylog:dbid:entityType = hashes(entityID, ip, port, componentID)
	*/

	return RedisHelper::dropTable(static_cast<DBInterfaceRedis*>(pdbi), fmt::format("kbe_entitylog:*:*"), false);
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableRedis::logEntity(DBInterface * pdbi, const char* ip, uint32 port, DBID dbid,
					COMPONENT_ID componentID, ENTITY_ID entityID, ENTITY_SCRIPT_UID entityType)
{
	/*
	kbe_entitylog:dbid:entityType = hashes(entityID, ip, port, componentID)
	*/
	std::string sqlstr = fmt::format("HSET kbe_entitylog:{}:{} entityID {} ip {} port {} componentID {}", 
		dbid, entityType, entityID, ip, port, componentID);

	try
	{
		pdbi->query(sqlstr.c_str(), sqlstr.size(), false);
	}
	catch(...)
	{
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableRedis::queryEntity(DBInterface * pdbi, DBID dbid, EntityLog& entitylog, ENTITY_SCRIPT_UID entityType)
{
	/*
	kbe_entitylog:dbid:entityType = hashes(entityID, ip, port, componentID)
	*/
	redisReply* pRedisReply = NULL;

	try
	{
		static_cast<DBInterfaceRedis*>(pdbi)->query(fmt::format("HMGET kbe_entitylog:{}:{} entityID ip port componentID",
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
bool KBEEntityLogTableRedis::eraseEntityLog(DBInterface * pdbi, DBID dbid, ENTITY_SCRIPT_UID entityType)
{
	std::string sqlstr = fmt::format("HDEL kbe_entitylog:{}:{}", 
		dbid, entityType);
	
	try
	{
		pdbi->query(sqlstr.c_str(), sqlstr.size(), false);
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
bool KBEAccountTableRedis::syncToDB(DBInterface* pdbi)
{
	return true;
}

//-------------------------------------------------------------------------------------
KBEAccountTableRedis::KBEAccountTableRedis():
	KBEAccountTable()
{
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::setFlagsDeadline(DBInterface * pdbi, const std::string& name, uint32 flags, uint64 deadline)
{
	/*
	kbe_accountinfos:accountName = hashes(password, bindata, email, entityDBID, flags, deadline, regtime, lasttime, numlogin)
	*/
	
	// 如果查询失败则返回存在， 避免可能产生的错误
	if(pdbi->query(fmt::format("HSET kbe_accountinfos:{} flags {} deadline {}", 
		name, flags, deadline), false))
		return true;

	return false;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::queryAccount(DBInterface * pdbi, const std::string& name, ACCOUNT_INFOS& info)
{
	/*
	kbe_accountinfos:accountName = hashes(password, bindata, email, entityDBID, flags, deadline, regtime, lasttime, numlogin)
	*/
	redisReply* pRedisReply = NULL;

	try
	{
		static_cast<DBInterfaceRedis*>(pdbi)->query(fmt::format("HMGET kbe_accountinfos:{} entityDBID password flags deadline",
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
bool KBEAccountTableRedis::queryAccountAllInfos(DBInterface * pdbi, const std::string& name, ACCOUNT_INFOS& info)
{
	/*
	kbe_accountinfos:accountName = hashes(password, bindata, email, entityDBID, flags, deadline, regtime, lasttime, numlogin)
	*/
	redisReply* pRedisReply = NULL;

	try
	{
		static_cast<DBInterfaceRedis*>(pdbi)->query(fmt::format("HMGET kbe_accountinfos:{} entityDBID password email flags deadline",
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
bool KBEAccountTableRedis::updateCount(DBInterface * pdbi, const std::string& name, DBID dbid)
{
	/*
	kbe_accountinfos:accountName = hashes(password, bindata, email, entityDBID, flags, deadline, regtime, lasttime, numlogin)
	*/
	try
	{	
		// 如果查询失败则返回存在， 避免可能产生的错误
		if(!static_cast<DBInterfaceRedis*>(pdbi)->queryAppend(false, "HINCRBY kbe_accountinfos:%s numlogin", name.c_str()))
			return false;
	}
	catch(...)
	{
	}
	
	try
	{
		// 如果查询失败则返回存在， 避免可能产生的错误
		if(!static_cast<DBInterfaceRedis*>(pdbi)->queryAppend(false, "%s", fmt::format("HSET kbe_accountinfos:{} lasttime {}", name, time(NULL)).c_str()))
			return false;
	}
	catch(...)
	{
	}
	
	redisReply* pRedisReply = NULL;
	int replys = 0;
	
	static_cast<DBInterfaceRedis*>(pdbi)->getQueryReply(&pRedisReply);
	if(pRedisReply)
	{
		++replys;
		freeReplyObject(pRedisReply); 
		pRedisReply = NULL;
	}

	static_cast<DBInterfaceRedis*>(pdbi)->getQueryReply(&pRedisReply);
	if(pRedisReply)
	{
		++replys;
		freeReplyObject(pRedisReply); 
		pRedisReply = NULL;
	}
	
	return replys == 2;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::updatePassword(DBInterface * pdbi, const std::string& name, const std::string& password)
{
	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!pdbi->query(fmt::format("HSET kbe_accountinfos:{} password {}", name, password), false))
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::logAccount(DBInterface * pdbi, ACCOUNT_INFOS& info)
{
	std::string sqlstr = fmt::format("HSET kbe_accountinfos:{} accountName {} password {} bindata {} ",
		"email {} entityDBID {} flags {} deadline {} regtime {} lasttime {}", 
		info.name, KBE_MD5::getDigest(info.password.data(), info.password.length()).c_str(),
		info.datas, info.email, info.dbid, info.flags, info.deadline, time(NULL), time(NULL));

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEAccountTableRedis::logAccount({}): cmd({}) is failed({})!\n", 
				info.name, sqlstr, pdbi->getstrerror()));

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
bool KBEEmailVerificationTableRedis::queryAccount(DBInterface * pdbi, int8 type, const std::string& name, ACCOUNT_INFOS& info)
{
	/*
	kbe_email_verification:code = hashes(accountName, type, datas, logtime)
	kbe_email_verification:accountName = code
	*/
	redisReply* pRedisReply = NULL;

	try
	{
		if (!static_cast<DBInterfaceRedis*>(pdbi)->query(fmt::format("GET kbe_email_verification:{}", name), &pRedisReply, false))
			return false;
	}
	catch(...)
	{
	}
	
	info.datas = "";
	
	if(pRedisReply)
	{
		if(pRedisReply->type == REDIS_REPLY_STRING)
		{
			info.datas = pRedisReply->str;
		}
		
		freeReplyObject(pRedisReply); 
		pRedisReply = NULL;
	}

	if(info.datas.size() > 0)
	{
		try
		{
			if (!static_cast<DBInterfaceRedis*>(pdbi)->query(fmt::format("HMGET kbe_email_verification:{} datas", info.datas), &pRedisReply, false))
				return false;
		}
		catch(...)
		{
		}		
	}
	else
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::queryAccount({}): cmd({}) is failed({})!\n", 
				name, pdbi->lastquery(), pdbi->getstrerror()));
				
		return false;
	}

	if(pRedisReply)
	{
		if(pRedisReply->type == REDIS_REPLY_STRING)
		{
			info.password = pRedisReply->str;
		}
		
		freeReplyObject(pRedisReply); 
		pRedisReply = NULL;
	}
	else
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::queryAccount({}): cmd({}) is failed({})!\n", 
				name, pdbi->lastquery(), pdbi->getstrerror()));
				
		return false;
	}

	info.name = name;

	return info.datas.size() > 0;
}

//-------------------------------------------------------------------------------------
int KBEEmailVerificationTableRedis::getDeadline(int8 type)
{
	int deadline = 3600;
	if(type == (int8)KBEEmailVerificationTable::V_TYPE_CREATEACCOUNT)
		deadline = g_kbeSrvConfig.emailAtivationInfo_.deadline;
	else if(type == (int8)KBEEmailVerificationTable::V_TYPE_RESETPASSWORD)
		deadline = g_kbeSrvConfig.emailResetPasswordInfo_.deadline;
	else if(type == (int8)KBEEmailVerificationTable::V_TYPE_BIND_MAIL)
		deadline = g_kbeSrvConfig.emailBindInfo_.deadline;
	
	return deadline;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableRedis::logAccount(DBInterface * pdbi, int8 type, const std::string& name, 
												const std::string& datas, const std::string& code)
{
	/*
	kbe_email_verification:code = hashes(accountName, type, datas, logtime)
	kbe_email_verification:accountName = code
	*/
	try
	{	
		// 如果查询失败则返回存在， 避免可能产生的错误
		if(!static_cast<DBInterfaceRedis*>(pdbi)->queryAppend(false, "%s", fmt::format("HSET kbe_email_verification:{} accountName {} type {} datas {} logtime {}", 
			code, name, type, datas, time(NULL)).c_str()))
		{
			ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::logAccount({}): cmd({}) is failed({})!\n", 
					code, pdbi->lastquery(), pdbi->getstrerror()));
			
			return false;
		}
	}
	catch(...)
	{
	}
	
	try
	{
		// 如果查询失败则返回存在， 避免可能产生的错误
		if(!static_cast<DBInterfaceRedis*>(pdbi)->queryAppend(false, "%s", fmt::format("SET kbe_email_verification:{} {}", name, code).c_str()))
		{
			ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::logAccount({}): cmd({}) is failed({})!\n", 
					code, pdbi->lastquery(), pdbi->getstrerror()));			
			return false;
		}
	}
	catch(...)
	{
	}
		
	try
	{	
		// 如果查询失败则返回存在， 避免可能产生的错误
		if(!static_cast<DBInterfaceRedis*>(pdbi)->queryAppend(false, "EXPIRE kbe_email_verification:%s %d", 
			code.c_str(), getDeadline(type)))
		{
			ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::logAccount({}): cmd({}) is failed({})!\n", 
					code, pdbi->lastquery(), pdbi->getstrerror()));
			
			return false;
		}
	}
	catch(...)
	{
	}
	
	try
	{	
		// 如果查询失败则返回存在， 避免可能产生的错误
		if(!static_cast<DBInterfaceRedis*>(pdbi)->queryAppend(false, "EXPIRE kbe_email_verification:%s %d", 
			name.c_str(), getDeadline(type)))
		{
			ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::logAccount({}): cmd({}) is failed({})!\n", 
					code, pdbi->lastquery(), pdbi->getstrerror()));
			
			return false;
		}
	}
	catch(...)
	{
	}
	
	redisReply* pRedisReply = NULL;
	int replys = 0;
	
	static_cast<DBInterfaceRedis*>(pdbi)->getQueryReply(&pRedisReply);
	if(pRedisReply)
	{
		++replys;
		freeReplyObject(pRedisReply); 
		pRedisReply = NULL;
	}

	static_cast<DBInterfaceRedis*>(pdbi)->getQueryReply(&pRedisReply);
	if(pRedisReply)
	{
		++replys;
		freeReplyObject(pRedisReply); 
		pRedisReply = NULL;
	}

	static_cast<DBInterfaceRedis*>(pdbi)->getQueryReply(&pRedisReply);
	if(pRedisReply)
	{
		++replys;
		freeReplyObject(pRedisReply); 
		pRedisReply = NULL;
	}
	
	static_cast<DBInterfaceRedis*>(pdbi)->getQueryReply(&pRedisReply);
	if(pRedisReply)
	{
		++replys;
		freeReplyObject(pRedisReply); 
		pRedisReply = NULL;
	}

	return replys == 4;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableRedis::activateAccount(DBInterface * pdbi, const std::string& code, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select accountName, datas, logtime from kbe_email_verification where code=\"";

	char* tbuf = new char[code.size() * 2 + 1];

	//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
	//	tbuf, code.c_str(), code.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", (int)KBEEmailVerificationTable::V_TYPE_CREATEACCOUNT);
	sqlstr += tbuf;
	sqlstr += " LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): cmd({}) is failed({})!\n", 
				code, sqlstr, pdbi->getstrerror()));

		return false;
	}

	uint64 logtime = 1;

	//MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
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
	if(!pTable->queryAccount(pdbi, info.name, info))
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

	if(!pTable->setFlagsDeadline(pdbi, info.name, info.flags, info.deadline))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::activateAccount({}): set deadline is error({})!\n", 
				code, pdbi->getstrerror()));
		return false;
	}

	if(!pTable->updatePassword(pdbi, info.name, password))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::activateAccount({}): update password is error({})!\n", 
				code, pdbi->getstrerror()));

		return false;
	}

	info.dbid = 0;

	ScriptDefModule* pModule = EntityDef::findScriptModule(DBUtil::accountScriptName());

	// 防止多线程问题， 这里做一个拷贝。
	MemoryStream copyAccountDefMemoryStream(pTable->accountDefMemoryStream());

	info.dbid = EntityTables::getSingleton().writeEntity(pdbi, 0, -1,
			&copyAccountDefMemoryStream, pModule);

	KBE_ASSERT(info.dbid > 0);

	// 如果查询失败则返回存在， 避免可能产生的错误
	tbuf = new char[MAX_BUF * 3];

//	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
	//	tbuf, info.name.c_str(), info.name.size());

	if(!pdbi->query(fmt::format("update kbe_accountinfos set entityDBID={} where accountName like \"{}\"", 
		info.dbid, tbuf), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::activateAccount({}): update kbe_accountinfos is error({})!\n", 
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
bool KBEEmailVerificationTableRedis::bindEMail(DBInterface * pdbi, const std::string& name, const std::string& code)
{
	std::string sqlstr = "select accountName, datas, logtime from kbe_email_verification where code=\"";

	char* tbuf = new char[code.size() * 2 + 1];

//	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
//		tbuf, code.c_str(), code.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", (int)KBEEmailVerificationTable::V_TYPE_BIND_MAIL);
	sqlstr += tbuf;
	sqlstr += " LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::bindEMail({}): cmd({}) is failed({})!\n", 
				code, sqlstr, pdbi->getstrerror()));

		return false;
	}

	uint64 logtime = 1;

	std::string qname, qemail;

	//MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
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

//	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
//		tbuf, qemail.c_str(), qemail.size());

	sqlstr = "update kbe_accountinfos set email=\"";
	sqlstr += tbuf;
	sqlstr += "\" where accountName like \"";

//	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
	//	tbuf, qname.c_str(), qname.size());
	
	sqlstr += tbuf;
	sqlstr += "\"";

	SAFE_RELEASE_ARRAY(tbuf);

	if(!pdbi->query(sqlstr, false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::bindEMail({}): update kbe_accountinfos({}) error({})!\n", 
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
bool KBEEmailVerificationTableRedis::resetpassword(DBInterface * pdbi, const std::string& name, 
												   const std::string& password, const std::string& code)
{
	std::string sqlstr = "select accountName, datas, logtime from kbe_email_verification where code=\"";

	char* tbuf = new char[code.size() * 2 + 1];

	//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
	//	tbuf, code.c_str(), code.size());

	sqlstr += tbuf;

	sqlstr += "\" and type=";
	kbe_snprintf(tbuf, MAX_BUF, "%d", (int)KBEEmailVerificationTable::V_TYPE_RESETPASSWORD);
	sqlstr += tbuf;
	sqlstr += " LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::resetpassword({}): cmd({}) is failed({})!\n", 
				code, sqlstr, pdbi->getstrerror()));

		return false;
	}

	uint64 logtime = 1;
	
	std::string qname, qemail;

	//MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
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

	if(!pTable->updatePassword(pdbi, qname, KBE_MD5::getDigest(password.data(), password.length())))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::resetpassword({}): update accountName({}) password error({})!\n", 
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
bool KBEEmailVerificationTableRedis::delAccount(DBInterface * pdbi, int8 type, const std::string& name)
{
	/*
	kbe_email_verification:code:accountName = hashes(type, datas, logtime)
	*/
		
	std::string sqlstr = "delete from kbe_email_verification where accountName=";

	char* tbuf = new char[MAX_BUF * 3];

	//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
	//	tbuf, name.c_str(), name.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\" and type=";
	
	kbe_snprintf(tbuf, MAX_BUF, "%d", type);
	sqlstr += tbuf;

	SAFE_RELEASE_ARRAY(tbuf);

	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::delAccount({}): cmd({}) is failed({})!\n", 
				name, sqlstr, pdbi->getstrerror()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableRedis::syncToDB(DBInterface* pdbi)
{
/*
	// 删除xx小时之前的记录
	sqlstr = fmt::format("delete from kbe_email_verification where logtime<{} and type={}", 
		KBEngine::StringConv::val2str(time(NULL) - g_kbeSrvConfig.emailAtivationInfo_.deadline), 
		((int)KBEEmailVerificationTable::V_TYPE_CREATEACCOUNT));

	ret = pdbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);

	sqlstr = fmt::format("delete from kbe_email_verification where logtime<{} and type={}", 
		KBEngine::StringConv::val2str(time(NULL) - g_kbeSrvConfig.emailResetPasswordInfo_.deadline),
		((int)KBEEmailVerificationTable::V_TYPE_RESETPASSWORD));

	ret = pdbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);

	sqlstr = fmt::format("delete from kbe_email_verification where logtime<{} and type={}", 
		KBEngine::StringConv::val2str(time(NULL) - g_kbeSrvConfig.emailBindInfo_.deadline), 
		((int)KBEEmailVerificationTable::V_TYPE_BIND_MAIL));

	ret = pdbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);*/
	return true;
}

//-------------------------------------------------------------------------------------
}
