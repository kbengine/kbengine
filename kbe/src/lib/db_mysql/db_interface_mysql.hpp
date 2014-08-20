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

#ifndef KBE_DB_INTERFACE_MYSQL_HPP
#define KBE_DB_INTERFACE_MYSQL_HPP

#include "common.hpp"
#include "db_transaction.hpp"
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

struct TABLE_FIELD
{
	std::string name;
	int32 length;
	uint64 maxlength;
	unsigned int flags;
	enum_field_types type;
};

/*
	数据库接口
*/
class DBInterfaceMysql : public DBInterface
{
public:
	DBInterfaceMysql(std::string characterSet, std::string collation);
	virtual ~DBInterfaceMysql();

	/**
		与某个数据库关联
	*/
	virtual bool attach(const char* databaseName = NULL);
	virtual bool detach();

	bool ping(){ 
		return mysql_ping(pMysql_) == 0; 
	}

	bool reattach();

	void inTransaction(bool value)
	{
		KBE_ASSERT(inTransaction_ != value);
		inTransaction_ = value;
	}

	bool hasLostConnection() const		{ return hasLostConnection_; }
	void hasLostConnection( bool v )	{ hasLostConnection_ = v; }

	/**
		检查环境
	*/
	virtual bool checkEnvironment();
	
	virtual bool query(const char* strCommand, uint32 size, bool showexecinfo = true);

	bool execute(const char* strCommand, uint32 size, MemoryStream * resdata);

	/**
		获取数据库所有的表名
	*/
	virtual bool getTableNames( std::vector<std::string>& tableNames, const char * pattern);

	/**
		获取数据库某个表所有的字段名称
	*/
	virtual bool getTableItemNames(const char* tablename, std::vector<std::string>& itemNames);

	/** 
		从数据库删除entity表字段
	*/
	virtual bool dropEntityTableItemFromDB(const char* tablename, const char* tableItemName);

	MYSQL* mysql(){ return pMysql_; }

	void throwError();

	my_ulonglong insertID()		{ return mysql_insert_id( pMysql_ ); }

	my_ulonglong affectedRows()	{ return mysql_affected_rows( pMysql_ ); }

	const char* info()			{ return mysql_info( pMysql_ ); }

	const char* getLastError()	
	{
		if(pMysql_ == NULL)
			return "pMysql is NULL";

		return mysql_error( pMysql_ ); 
	}

	unsigned int getLastErrorNum() { return mysql_errno( pMysql_ ); }

	typedef KBEUnordered_map<std::string, TABLE_FIELD> TABLE_FIELDS;
	void getFields(TABLE_FIELDS& outs, const char* tablename);

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

	/**
		创建一个entity存储表
	*/
	virtual EntityTable* createEntityTable();

	/** 
		从数据库删除entity表
	*/
	virtual bool dropEntityTableFromDB(const char* tablename);

	/**
		锁住接口操作
	*/
	virtual bool lock();
	virtual bool unlock();

	/**
		处理异常
	*/
	bool processException(std::exception & e);
protected:
	MYSQL* pMysql_;

	bool hasLostConnection_;

	bool inTransaction_;

	DBTransaction lock_;

	std::string characterSet_;
	std::string collation_;
};


}

#endif // KBE_DB_INTERFACE_MYSQL_HPP
