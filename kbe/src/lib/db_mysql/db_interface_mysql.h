// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_DB_INTERFACE_MYSQL_H
#define KBE_DB_INTERFACE_MYSQL_H

#include "common.h"
#include "db_transaction.h"
#include "common/common.h"
#include "common/singleton.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"
#include "db_interface/db_interface.h"

#include "mysql/mysql.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#ifdef X64
// added for VS2015
#if _MSC_VER >= 1900
#pragma comment (lib, "libmysql64_vs140.lib")
#pragma comment (lib, "mysqlclient64_vs140.lib")
#else
#pragma comment (lib, "libmysql64.lib")
#pragma comment (lib, "mysqlclient64.lib")
#endif
#else
// added for VS2015
#if _MSC_VER >= 1900
#pragma comment (lib, "libmysql32_vs140.lib")
#pragma comment (lib, "mysqlclient32_vs140.lib")
#else
#pragma comment (lib, "libmysql32.lib")
#pragma comment (lib, "mysqlclient32.lib")
#endif
#endif
#endif

namespace KBEngine { 

struct MYSQL_TABLE_FIELD
{
	std::string name;
	int32 length;
	uint64 maxlength;
	unsigned int flags;
	enum_field_types type;
};

class DBException;

/*
	数据库接口
*/
class DBInterfaceMysql : public DBInterface
{
public:
	DBInterfaceMysql(const char* name, std::string characterSet, std::string collation);
	virtual ~DBInterfaceMysql();

	static bool initInterface(DBInterface* pdbi);
	
	/**
		与某个数据库关联
	*/
	bool reattach();
	virtual bool attach(const char* databaseName = NULL);
	virtual bool detach();

	bool ping(){ 
		return mysql_ping(pMysql_) == 0; 
	}

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
	
	/**
		检查错误， 对错误的内容进行纠正
		如果纠正不成功返回失败
	*/
	virtual bool checkErrors();

	virtual bool query(const char* strCommand, uint32 size, bool printlog = true, MemoryStream * result = NULL);

	bool write_query_result(MemoryStream * result);

	/**
		获取数据库所有的表名
	*/
	virtual bool getTableNames( std::vector<std::string>& tableNames, const char * pattern);

	/**
		获取数据库某个表所有的字段名称
	*/
	virtual bool getTableItemNames(const char* tableName, std::vector<std::string>& itemNames);

	/** 
		从数据库删除entity表字段
	*/
	virtual bool dropEntityTableItemFromDB(const char* tableName, const char* tableItemName);

	MYSQL* mysql(){ return pMysql_; }

	void throwError(DBException* pDBException);

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

	typedef KBEUnordered_map<std::string, MYSQL_TABLE_FIELD> TABLE_FIELDS;
	void getFields(TABLE_FIELDS& outs, const char* tableName);

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
		如果数据库不存在则创建一个数据库
	*/
	virtual bool createDatabaseIfNotExist();
	
	/**
		创建一个entity存储表
	*/
	virtual EntityTable* createEntityTable(EntityTables* pEntityTables);

	/** 
		从数据库删除entity表
	*/
	virtual bool dropEntityTableFromDB(const char* tableName);

	/**
		锁住接口操作
	*/
	virtual bool lock();
	virtual bool unlock();

	/**
		处理异常
	*/
	bool processException(std::exception & e);

	/**
		SQL命令最长大小
	*/
	static size_t sql_max_allowed_packet(){ return sql_max_allowed_packet_; }

protected:
	MYSQL* pMysql_;

	bool hasLostConnection_;

	bool inTransaction_;

	mysql::DBTransaction lock_;

	std::string characterSet_;
	std::string collation_;

	static size_t sql_max_allowed_packet_;
};


}

#endif // KBE_DB_INTERFACE_MYSQL_H
