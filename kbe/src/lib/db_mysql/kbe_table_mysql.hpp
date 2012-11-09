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

#ifndef __KBE_KBE_TABLE_MYSQL__
#define __KBE_KBE_TABLE_MYSQL__

#include "common.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/singleton.hpp"
#include "helper/debug_helper.hpp"
#include "dbmgr_lib/entity_table.hpp"

namespace KBEngine { 

/*
	kbe系统表
*/
class KBEEntityLogTableMysql : public EntityTable
{
public:
	KBEEntityLogTableMysql();
	virtual ~KBEEntityLogTableMysql(){}
	
	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* dbi);
	
	/**
		初始化
	*/
	virtual bool initialize(ScriptDefModule* sm, std::string name);
	
	virtual EntityTableItem* createItem(std::string type) {return NULL;}
protected:
	
};

class KBEAccountTableMysql : public EntityTable
{
public:
	KBEAccountTableMysql();
	virtual ~KBEAccountTableMysql(){}
	
	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* dbi);
	
	/**
		初始化
	*/
	virtual bool initialize(ScriptDefModule* sm, std::string name);
	
	virtual EntityTableItem* createItem(std::string type) {return NULL;}
	
	bool queryAccount(DBInterface * dbi, std::string& name, ACCOUNT_INFOS& info);
	bool logAccount(DBInterface * dbi, ACCOUNT_INFOS& info);

	MemoryStream& accountDefMemoryStream(){ return accountDefMemoryStream_; }
	void accountDefMemoryStream(MemoryStream& s){accountDefMemoryStream_.append(s.data() + s.rpos(), s.opsize()); }
protected:
	MemoryStream accountDefMemoryStream_;
};

}

#endif // __KBE_KBE_TABLE_MYSQL__
