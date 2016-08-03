#pragma once
#include "common.h"
#include "db_transaction.h"
#include "common/common.h"
#include "common/singleton.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"
#include "db_interface/db_interface.h"

#include "mongoc.h"
#include <bson.h>
#include <bcon.h>
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma comment (lib, "bson-static_d.lib")
#pragma comment (lib, "mongoc_static_d.lib")
#endif

namespace KBEngine 
{
	class DBInterfaceMongodb : public DBInterface
	{
	public:
		DBInterfaceMongodb(const char* name);
		virtual ~DBInterfaceMongodb();

		static bool initInterface(DBInterface* pdbi);

		/**
		与某个数据库关联
		*/
		bool reattach();
		virtual bool attach(const char* databaseName = NULL);
		virtual bool detach();

		/*bool ping(){
			return mysql_ping(pMysql_) == 0;
		}*/

		bool ping(mongoc_client_t* pMongoClient = NULL);

		void inTransaction(bool value)
		{
			KBE_ASSERT(inTransaction_ != value);
			inTransaction_ = value;
		}

		bool hasLostConnection() const		{ return hasLostConnection_; }
		void hasLostConnection(bool v)	{ hasLostConnection_ = v; }

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
		virtual bool getTableNames(std::vector<std::string>& tableNames, const char * pattern);

		/**
		获取数据库某个表所有的字段名称
		*/
		virtual bool getTableItemNames(const char* tableName, std::vector<std::string>& itemNames);

		/*const char* getLastError()
		{
		if (pMongoClient == NULL)
		return "pMongoClient is NULL";

		return pMongoClient->in_exhaust;
		}*/

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
		virtual bool createDatabaseIfNotExist();//有待验证是否需要

		/**
		创建一个entity存储表
		*/
		virtual EntityTable* createEntityTable(EntityTables* pEntityTables);

		/**
		从数据库删除entity表
		*/
		virtual bool dropEntityTableFromDB(const char* tableName);

		/**
		从数据库删除entity表字段
		*/
		virtual bool dropEntityTableItemFromDB(const char* tableName, const char* tableItemName);

		mongoc_client_t* mongo(){ return _pMongoClient; }

		/**
		锁住接口操作
		*/
		virtual bool lock();
		virtual bool unlock();

		void throwError();

		/**
		处理异常
		*/
		bool processException(std::exception & e);

	protected:
		mongoc_client_t *_pMongoClient;
		mongoc_collection_t  *collection;
		mongoc_database_t *database;
		bson_t               *command,
			reply,
			*insert;
		bson_error_t          error;
		char                 *str;
		bool                  retval;
		/*mongoc_cursor_t *cursor;
		const bson_t *reply;
		uint16_t port;
		bson_error_t error;
		bson_t b;
		char *host_and_port;
		char *str;*/

		mongodb::DBTransaction lock_;

		bool hasLostConnection_;

		bool inTransaction_;
	};
}