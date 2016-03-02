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
#include "db_transaction.h"
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
	������ʱ�Ų���������
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

	pdbi->query(sqlstr.c_str(), sqlstr.size(), false);
	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableRedis::queryEntity(DBInterface * pdbi, DBID dbid, EntityLog& entitylog, ENTITY_SCRIPT_UID entityType)
{
	/*
	kbe_entitylog:dbid:entityType = hashes(entityID, ip, port, componentID)
	*/
	redisReply* pRedisReply = NULL;

	static_cast<DBInterfaceRedis*>(pdbi)->query(fmt::format("HMGET kbe_entitylog:{}:{} entityID ip port componentID",
		dbid, entityType), &pRedisReply, false);

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
	
	pdbi->query(sqlstr.c_str(), sqlstr.size(), false);
	return true;
}

//-------------------------------------------------------------------------------------
KBEEntityLogTableRedis::KBEEntityLogTableRedis(EntityTables* pEntityTables) :
KBEEntityLogTable(pEntityTables)
{
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::syncToDB(DBInterface* pdbi)
{
	return true;
}

//-------------------------------------------------------------------------------------
KBEAccountTableRedis::KBEAccountTableRedis(EntityTables* pEntityTables) :
KBEAccountTable(pEntityTables)
{
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::setFlagsDeadline(DBInterface * pdbi, const std::string& name, uint32 flags, uint64 deadline)
{
	/*
	kbe_accountinfos:accountName = hashes(password, bindata, email, entityDBID, flags, deadline, regtime, lasttime, numlogin)
	*/
	
	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
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

	static_cast<DBInterfaceRedis*>(pdbi)->query(fmt::format("HMGET kbe_accountinfos:{} entityDBID password flags deadline",
		name), &pRedisReply, false);
	
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

	static_cast<DBInterfaceRedis*>(pdbi)->query(fmt::format("HMGET kbe_accountinfos:{} entityDBID password email flags deadline",
		name), &pRedisReply, false);

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
	redis::DBTransaction transaction(pdbi);
	
	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!pdbi->query(fmt::format("HINCRBY kbe_accountinfos:{} numlogin", name), false))
	{
		transaction.rollback();
		return false;
	}
	
	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!pdbi->query(fmt::format("HSET kbe_accountinfos:{} lasttime {}", name, time(NULL)), false))
	{
		transaction.rollback();
		return false;
	}
	
	transaction.commit();
	
	redisReply* pRedisReply = transaction.pRedisReply();

	return pRedisReply && pRedisReply->type == REDIS_REPLY_ARRAY && pRedisReply->elements == 2;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableRedis::updatePassword(DBInterface * pdbi, const std::string& name, const std::string& password)
{
	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
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

	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!pdbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEAccountTableRedis::logAccount({}): cmd({}) is failed({})!\n", 
				info.name, sqlstr, pdbi->getstrerror()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
KBEEmailVerificationTableRedis::KBEEmailVerificationTableRedis(EntityTables* pEntityTables) :
KBEEmailVerificationTable(pEntityTables)
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

	if (!static_cast<DBInterfaceRedis*>(pdbi)->query(fmt::format("GET kbe_email_verification:{}", name), &pRedisReply, false))
		return false;
	
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
		if (!static_cast<DBInterfaceRedis*>(pdbi)->query(fmt::format("HMGET kbe_email_verification:{} datas", info.datas), &pRedisReply, false))
			return false;
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
	
	redis::DBTransaction transaction(pdbi);
	
	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!pdbi->query(fmt::format("HSET kbe_email_verification:{} accountName {} type {} datas {} logtime {}", 
		code, name, type, datas, time(NULL)), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::logAccount({}): cmd({}) is failed({})!\n", 
				code, pdbi->lastquery(), pdbi->getstrerror()));
		
		transaction.rollback();
		return false;
	}

	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!pdbi->query(fmt::format("SET kbe_email_verification:{} {}", name, code), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::logAccount({}): cmd({}) is failed({})!\n", 
				code, pdbi->lastquery(), pdbi->getstrerror()));	
		
		transaction.rollback();	
		return false;
	}

	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!pdbi->query(fmt::format("EXPIRE kbe_email_verification:{} {}", 
		code.c_str(), getDeadline(type)), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::logAccount({}): cmd({}) is failed({})!\n", 
				code, pdbi->lastquery(), pdbi->getstrerror()));
		
		transaction.rollback();	
		return false;
	}
	
	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!pdbi->query(fmt::format("EXPIRE kbe_email_verification:{} {}", 
		name.c_str(), getDeadline(type)), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::logAccount({}): cmd({}) is failed({})!\n", 
				code, pdbi->lastquery(), pdbi->getstrerror()));
		
		transaction.rollback();	
		return false;
	}
	
	transaction.commit();
	
	redisReply* pRedisReply = transaction.pRedisReply();

	return pRedisReply && pRedisReply->type == REDIS_REPLY_ARRAY && pRedisReply->elements == 4;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableRedis::activateAccount(DBInterface * pdbi, const std::string& code, ACCOUNT_INFOS& info)
{
	/*
	kbe_email_verification:code = hashes(accountName, type, datas, logtime)
	kbe_email_verification:accountName = code
	*/	
	redisReply* pRedisReply = NULL;

	if (!static_cast<DBInterfaceRedis*>(pdbi)->query(fmt::format("HMGET kbe_email_verification:{} accountName type, datas logtime", code), &pRedisReply, false))
		return false;

	uint64 logtime = 1;
	int type = -1;
	
	if(pRedisReply)
	{
		if(pRedisReply->type == REDIS_REPLY_ARRAY)
		{
			if(RedisHelper::check_array_results(pRedisReply))
			{			
				info.name = pRedisReply->element[0]->str;
				StringConv::str2value(type, pRedisReply->element[1]->str);
				info.password = pRedisReply->element[2]->str;
				StringConv::str2value(logtime, pRedisReply->element[3]->str);
			}
		}
		
		freeReplyObject(pRedisReply); 
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
	
	if((int)KBEEmailVerificationTable::V_TYPE_CREATEACCOUNT != type)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::activateAccount({}): type({}) error!\n", 
				code, type));

		return false;
	}

	std::string password = info.password;

	// Ѱ��dblog�Ƿ��д��˺�
	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::findByInterfaceName(pdbi->name()).findKBETable("kbe_accountinfos"));
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

	// ��ֹ���߳����⣬ ������һ��������
	MemoryStream copyAccountDefMemoryStream(pTable->accountDefMemoryStream());

	info.dbid = EntityTables::findByInterfaceName(pdbi->name()).writeEntity(pdbi, 0, -1,
			&copyAccountDefMemoryStream, pModule);

	KBE_ASSERT(info.dbid > 0);

	/*
	kbe_accountinfos:accountName = hashes(password, bindata, email, entityDBID, flags, deadline, regtime, lasttime, numlogin)
	*/
	
	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!pdbi->query(fmt::format("HSET kbe_accountinfos:{} entityDBID {}", 
		info.name, info.dbid), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::activateAccount({}): update kbe_accountinfos is error({})!\n", 
				code, pdbi->getstrerror()));

		return false;
	}

	delAccount(pdbi, (int8)V_TYPE_CREATEACCOUNT, info.name);
	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableRedis::bindEMail(DBInterface * pdbi, const std::string& name, const std::string& code)
{
	/*
	kbe_email_verification:code = hashes(accountName, type, datas, logtime)
	kbe_email_verification:accountName = code
	*/	
	redisReply* pRedisReply = NULL;

	if (!pdbi->query(fmt::format("HMGET kbe_email_verification:{} accountName type, datas logtime", code), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::bindEMail({}): cmd({}) is failed({})!\n", 
				code, pdbi->lastquery(), pdbi->getstrerror()));
	}	

	uint64 logtime = 1;
	int type = -1;
	std::string qname, qemail;
	
	if(pRedisReply)
	{
		if(pRedisReply->type == REDIS_REPLY_ARRAY)
		{
			if(RedisHelper::check_array_results(pRedisReply))
			{			
				qname = pRedisReply->element[0]->str;
				StringConv::str2value(type, pRedisReply->element[1]->str);
				qemail = pRedisReply->element[2]->str;
				StringConv::str2value(logtime, pRedisReply->element[3]->str);
			}
		}
		
		freeReplyObject(pRedisReply); 
	}

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

	if((int)KBEEmailVerificationTable::V_TYPE_BIND_MAIL != type)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::bindEMail({}): type({}) error!\n", 
				code, type));

		return false;
	}
	
	/*
	kbe_accountinfos:accountName = hashes(password, bindata, email, entityDBID, flags, deadline, regtime, lasttime, numlogin)
	*/
	
	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!pdbi->query(fmt::format("HSET kbe_accountinfos:{} email {}", 
		qname, qemail), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::bindEMail({}): update kbe_accountinfos({}) error({})!\n", 
				code, qname, pdbi->getstrerror()));

		return false;
	}

	delAccount(pdbi, (int8)V_TYPE_BIND_MAIL, name);
	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableRedis::resetpassword(DBInterface * pdbi, const std::string& name, 
												   const std::string& password, const std::string& code)
{
	/*
	kbe_email_verification:code = hashes(accountName, type, datas, logtime)
	kbe_email_verification:accountName = code
	*/	
	redisReply* pRedisReply = NULL;

	if (!pdbi->query(fmt::format("HMGET kbe_email_verification:{} accountName type, datas logtime", code), false))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::bindEMail({}): cmd({}) is failed({})!\n", 
				code, pdbi->lastquery(), pdbi->getstrerror()));
	}	

	uint64 logtime = 1;
	int type = -1;
	std::string qname, qemail;
	
	if(pRedisReply)
	{
		if(pRedisReply->type == REDIS_REPLY_ARRAY)
		{
			if(RedisHelper::check_array_results(pRedisReply))
			{			
				qname = pRedisReply->element[0]->str;
				StringConv::str2value(type, pRedisReply->element[1]->str);
				qemail = pRedisReply->element[2]->str;
				StringConv::str2value(logtime, pRedisReply->element[3]->str);
			}
		}
		
		freeReplyObject(pRedisReply); 
	}

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

	if((int)KBEEmailVerificationTable::V_TYPE_RESETPASSWORD != type)
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::resetpassword({}): type({}) error!\n", 
				code, type));

		return false;
	}
	
	// Ѱ��dblog�Ƿ��д��˺�
	KBEAccountTable* pTable = static_cast<KBEAccountTable*>(EntityTables::findByInterfaceName(pdbi->name()).findKBETable("kbe_accountinfos"));
	KBE_ASSERT(pTable);

	if(!pTable->updatePassword(pdbi, qname, KBE_MD5::getDigest(password.data(), password.length())))
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableRedis::resetpassword({}): update accountName({}) password error({})!\n", 
				code, qname, pdbi->getstrerror()));

		return false;
	}

	delAccount(pdbi, (int8)V_TYPE_RESETPASSWORD, qname);
	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableRedis::delAccount(DBInterface * pdbi, int8 type, const std::string& name)
{
	/*
	kbe_email_verification:code = hashes(accountName, type, datas, logtime)
	kbe_email_verification:accountName = code
	*/
	redisReply* pRedisReply = NULL;
	
	// �����ѯʧ���򷵻ش��ڣ� ������ܲ����Ĵ���
	if(!pdbi->query(fmt::format("GET kbe_email_verification:{}", name), false))
	{
		return false;
	}

	std::string code;
	
	if(pRedisReply)
	{
		if(pRedisReply->type == REDIS_REPLY_STRING)
		{
			code = pRedisReply->str;
		}
		
		freeReplyObject(pRedisReply); 
		pRedisReply = NULL;
	}

	// ����ʼ	
	redis::DBTransaction transaction(pdbi);
	
	if(code.size() > 0)
	{
		if (!pdbi->query(fmt::format("DEL kbe_email_verification:{}", code), false))
			return false;
	
		if (!pdbi->query(fmt::format("DEL kbe_email_verification:{}", name), false))
			return false;
	}
	else
	{
		ERROR_MSG(fmt::format("KBEEmailVerificationTableMysql::queryAccount({}): cmd({}) is failed({})!\n", 
				name, pdbi->lastquery(), pdbi->getstrerror()));
				
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEmailVerificationTableRedis::syncToDB(DBInterface* pdbi)
{
	return true;
}

//-------------------------------------------------------------------------------------
}
