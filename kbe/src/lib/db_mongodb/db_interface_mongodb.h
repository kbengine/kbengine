#pragma once
#include "common.h"
#include "db_transaction.h"
#include "common/common.h"
#include "common/singleton.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"
#include "db_interface/db_interface.h"
#include "entitydef/entitydef.h"

#ifdef _MSC_VER //���mongodb��python��ssize_t��ͻ���������
#define _SSIZE_T_DEFINED
#endif
#include "mongoc.h"
#include <bson.h>
#include <bcon.h>

namespace KBEngine 
{
	class DBInterfaceMongodb : public DBInterface
	{
	public:
		DBInterfaceMongodb(const char* name);
		virtual ~DBInterfaceMongodb();

		static bool initInterface(DBInterface* pdbi);

		/**
		��ĳ�����ݿ����
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
		��黷��
		*/
		virtual bool checkEnvironment();

		/**
		������ �Դ�������ݽ��о���
		����������ɹ�����ʧ��
		*/
		virtual bool checkErrors();

		virtual bool query(const char* strCommand, uint32 size, bool printlog = true, MemoryStream * result = NULL);
		bool executeFindCommand(MemoryStream * result, std::vector<std::string> strcmd, const char *tableName);
		bool executeUpdateCommand(std::vector<std::string> strcmd, const char *tableName);
		bool executeRemoveCommand(std::vector<std::string> strcmd, const char *tableName);
		bool executeInsertCommand(std::vector<std::string> strcmd, const char *tableName);
		bool executeFunctionCommand(MemoryStream * result,std::string strcmd);
		bool extuteFunction(const bson_t *command, const mongoc_read_prefs_t *read_prefs, bson_t *reply);
		std::vector<std::string> splitParameter(std::string value);

		bool write_query_result(MemoryStream * result, const char* strcmd = NULL);

		/**
		��ȡ���ݿ����еı���
		*/
		virtual bool getTableNames(std::vector<std::string>& tableNames, const char * pattern);

		/**
		��ȡ���ݿ�ĳ�������е��ֶ�����
		*/
		virtual bool getTableItemNames(const char* tableName, std::vector<std::string>& itemNames);

		/*const char* getLastError()
		{
		if (pMongoClient == NULL)
		return "pMongoClient is NULL";

		return pMongoClient->in_exhaust;
		}*/

		/**
		��������ӿڵ�����
		*/
		virtual const char* c_str();

		/**
		��ȡ����
		*/
		virtual const char* getstrerror();

		/**
		��ȡ������
		*/
		virtual int getlasterror();

		/**
		������ݿⲻ�����򴴽�һ�����ݿ�
		*/
		virtual bool createDatabaseIfNotExist();//�д���֤�Ƿ���Ҫ

		/**
		����һ��entity�洢��
		*/
		virtual EntityTable* createEntityTable(EntityTables* pEntityTables);

		/**
		�����ݿ�ɾ��entity��
		*/
		virtual bool dropEntityTableFromDB(const char* tableName);

		/**
		�����ݿ�ɾ��entity���ֶ�
		*/
		virtual bool dropEntityTableItemFromDB(const char* tableName, const char* tableItemName);

		mongoc_client_t* mongo(){ return _pMongoClient; }

		/**
		��ס�ӿڲ���
		*/
		virtual bool lock();
		virtual bool unlock();

		void throwError();

		/**
		�����쳣
		*/
		bool processException(std::exception & e);

		/**
		ִ�������ݿ���صĲ���
		*/
		bool createCollection(const char *tableName);

		bool insertCollection(const char *tableName, mongoc_insert_flags_t flags, const bson_t *document, const mongoc_write_concern_t *write_concern);

		mongoc_cursor_t *  collectionFind(const char *tableName, mongoc_query_flags_t flags, uint32_t skip, uint32_t limit, uint32_t  batch_size, const bson_t *query, const bson_t *fields, const mongoc_read_prefs_t *read_prefs);

		bool updateCollection(const char *tableName, mongoc_update_flags_t uflags, const bson_t *selector, const bson_t *update, const mongoc_write_concern_t *write_concern);

		bool collectionRemove(const char *tableName, mongoc_remove_flags_t flags, const bson_t *selector, const mongoc_write_concern_t *write_concern);

		mongoc_cursor_t * collectionFindIndexes(const char *tableName);

		bool collectionCreateIndex(const char *tableName, const bson_t *keys, const mongoc_index_opt_t *opt);

		bool collectionDropIndex(const char *tableName, const char *index_name);		

	protected:
		mongoc_client_t *_pMongoClient;
		mongoc_database_t *database;
		bool hasLostConnection_;
		bool inTransaction_;
		mongodb::DBTransaction lock_;
		const char *strError;
	};
}