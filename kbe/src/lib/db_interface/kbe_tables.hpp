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

#ifndef KBE_KBE_TABLES_HPP
#define KBE_KBE_TABLES_HPP

#include "entity_table.hpp"
#include "common/common.hpp"
#include "common/memorystream.hpp"
#include "helper/debug_helper.hpp"

namespace KBEngine { 

class KBETable : public EntityTable
{
public:
	KBETable():
	EntityTable()
	{
	}
	
	virtual ~KBETable()
	{
	}
	
	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* dbi) = 0;
	
	/**
		初始化
	*/
	virtual bool initialize(ScriptDefModule* sm, std::string name){ return true; };
	
	virtual EntityTableItem* createItem(std::string type) {return NULL;}
	
protected:

};

/*
	kbe系统表
*/
class KBEEntityLogTable : public KBETable
{
public:
	struct EntityLog
	{
		DBID dbid;
		ENTITY_ID entityID;
		char ip[MAX_IP];
		uint16 port;
		COMPONENT_ID componentID;
	};

	KBEEntityLogTable():
	KBETable()
	{
		tableName("kbe_entitylog");
	}
	
	virtual ~KBEEntityLogTable()
	{
	}
	
	virtual bool logEntity(DBInterface * dbi, const char* ip, uint32 port, DBID dbid,
						COMPONENT_ID componentID, ENTITY_ID entityID, ENTITY_SCRIPT_UID entityType) = 0;

	virtual bool queryEntity(DBInterface * dbi, DBID dbid, EntityLog& entitylog, ENTITY_SCRIPT_UID entityType) = 0;

	virtual bool eraseEntityLog(DBInterface * dbi, DBID dbid, ENTITY_SCRIPT_UID entityType) = 0;
protected:
	
};

class KBEAccountTable : public KBETable
{
public:
	KBEAccountTable():
	KBETable(),
	accountDefMemoryStream_()
	{
		tableName("kbe_accountinfos");
	}
	
	virtual ~KBEAccountTable()
	{
	}

	virtual bool queryAccount(DBInterface * dbi, const std::string& name, ACCOUNT_INFOS& info) = 0;
	virtual bool logAccount(DBInterface * dbi, ACCOUNT_INFOS& info) = 0;
	virtual bool setFlagsDeadline(DBInterface * dbi, const std::string& name, uint32 flags, uint64 deadline) = 0;
	virtual bool updateCount(DBInterface * dbi, DBID dbid) = 0;
	virtual bool queryAccountAllInfos(DBInterface * dbi, const std::string& name, ACCOUNT_INFOS& info) = 0;
	virtual bool updatePassword(DBInterface * dbi, const std::string& name, const std::string& password) = 0;

	MemoryStream& accountDefMemoryStream()
	{ 
		return accountDefMemoryStream_; 
	}

	void accountDefMemoryStream(MemoryStream& s)
	{
		accountDefMemoryStream_.clear(false);
		accountDefMemoryStream_.append(s.data() + s.rpos(), s.length()); 
	}
protected:
	MemoryStream accountDefMemoryStream_;
};

class KBEEmailVerificationTable : public KBETable
{
public:
	enum V_TYPE
	{
		V_TYPE_CREATEACCOUNT = 1,
		V_TYPE_RESETPASSWORD = 2,
		V_TYPE_BIND_MAIL = 3
	};

	KBEEmailVerificationTable():
	KBETable()
	{
		tableName("kbe_email_verification");
	}
	
	virtual ~KBEEmailVerificationTable()
	{
	}

	virtual bool queryAccount(DBInterface * dbi, int8 type, const std::string& name, ACCOUNT_INFOS& info) = 0;
	virtual bool logAccount(DBInterface * dbi, int8 type, const std::string& name, const std::string& datas, const std::string& code) = 0;
	virtual bool delAccount(DBInterface * dbi, int8 type, const std::string& name) = 0;
	virtual bool activateAccount(DBInterface * dbi, const std::string& code, ACCOUNT_INFOS& info) = 0;
	virtual bool bindEMail(DBInterface * dbi, const std::string& name, const std::string& code) = 0;
	virtual bool resetpassword(DBInterface * dbi, const std::string& name, const std::string& password, const std::string& code) = 0;
protected:
};

}

#endif // KBE_KBE_TABLES_HPP
