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

#ifndef KBE_KBE_TABLE_MYSQL_H
#define KBE_KBE_TABLE_MYSQL_H

#include "common.h"
#include "common/common.h"
#include "common/singleton.h"
#include "helper/debug_helper.h"
#include "db_interface/entity_table.h"
#include "db_interface/kbe_tables.h"

namespace KBEngine { 

/*
	kbe系统表
*/
class KBEEntityLogTableMysql : public KBEEntityLogTable
{
public:
	KBEEntityLogTableMysql();
	virtual ~KBEEntityLogTableMysql(){}
	
	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* dbi);
	virtual bool syncIndexToDB(DBInterface* dbi){ return true; }

	virtual bool logEntity(DBInterface * dbi, const char* ip, uint32 port, DBID dbid,
						COMPONENT_ID componentID, ENTITY_ID entityID, ENTITY_SCRIPT_UID entityType);

	virtual bool queryEntity(DBInterface * dbi, DBID dbid, EntityLog& entitylog, ENTITY_SCRIPT_UID entityType);

	virtual bool eraseEntityLog(DBInterface * dbi, DBID dbid, ENTITY_SCRIPT_UID entityType);
protected:
	
};

class KBEAccountTableMysql : public KBEAccountTable
{
public:
	KBEAccountTableMysql();
	virtual ~KBEAccountTableMysql(){}
	
	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* dbi);
	virtual bool syncIndexToDB(DBInterface* dbi){ return true; }

	bool queryAccount(DBInterface * dbi, const std::string& name, ACCOUNT_INFOS& info);
	bool queryAccountAllInfos(DBInterface * dbi, const std::string& name, ACCOUNT_INFOS& info);
	bool logAccount(DBInterface * dbi, ACCOUNT_INFOS& info);
	bool setFlagsDeadline(DBInterface * dbi, const std::string& name, uint32 flags, uint64 deadline);
	virtual bool updateCount(DBInterface * dbi, DBID dbid);
	virtual bool updatePassword(DBInterface * dbi, const std::string& name, const std::string& password);
protected:
};

class KBEEmailVerificationTableMysql : public KBEEmailVerificationTable
{
public:

	KBEEmailVerificationTableMysql();
	virtual ~KBEEmailVerificationTableMysql();

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* dbi);
	virtual bool syncIndexToDB(DBInterface* dbi){ return true; }

	virtual bool queryAccount(DBInterface * dbi, int8 type, const std::string& name, ACCOUNT_INFOS& info);
	virtual bool logAccount(DBInterface * dbi, int8 type, const std::string& name, const std::string& datas, const std::string& code);
	virtual bool delAccount(DBInterface * dbi, int8 type, const std::string& name);
	virtual bool activateAccount(DBInterface * dbi, const std::string& code, ACCOUNT_INFOS& info);
	virtual bool bindEMail(DBInterface * dbi, const std::string& name, const std::string& code);
	virtual bool resetpassword(DBInterface * dbi, const std::string& name, 
		const std::string& password, const std::string& code);

protected:
};

}

#endif // KBE_KBE_TABLE_MYSQL_H
