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

#ifndef __KBE_DB_INTERFACE_MYSQL__
#define __KBE_DB_INTERFACE_MYSQL__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/singleton.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/debug_helper.hpp"
#include "dbmgr_lib/db_interface.hpp"

#include "mysql/mysql.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#ifdef _DEBUG
#pragma comment (lib, "libmysql_d.lib")
#pragma comment (lib, "mysqlclient_d.lib")
#else
#pragma comment (lib, "libmysql.lib")
#pragma comment (lib, "mysqlclient.lib")
#endif
#endif

namespace KBEngine { 

/*
	数据库接口
*/
class DBInterfaceMysql : public DBInterface
{
public:
	DBInterfaceMysql();
	virtual ~DBInterfaceMysql();

	/**
		与某个数据库关联
	*/
	virtual bool attach(const char* databaseName);
	virtual bool detach();

	virtual bool query(const char* strCommand, uint32 size);

	bool execute(const char* strCommand, uint32 size, MemoryStream * resdata);
	bool getTableNames( std::vector<std::string>& tableNames, const char * pattern);

	MYSQL* mysql(){ return pMysql_; }

	/**
		返回这个接口的描述
	*/
	virtual const char* c_str();

	/** 
		获取错误
	*/
	virtual const char* getstrerror();

	/** 
		获取错误编号
	*/
	virtual int getlasterror();
protected:
	MYSQL* pMysql_;
};


}

#endif // __KBE_DB_INTERFACE_MYSQL__
