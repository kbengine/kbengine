#pragma once
#include "db_interface_mongodb.h"
#include "common.h"
#include "common/common.h"
#include "common/singleton.h"
#include "helper/debug_helper.h"
#include "db_interface/entity_table.h"

namespace KBEngine {

	class ScriptDefModule;
	class EntityTableMongodb;

	/*
	维护entity在数据库表中的一个字段
	*/
	class EntityTableItemMongodbBase : public EntityTableItem
	{
	public:
		EntityTableItemMongodbBase(std::string itemDBType, uint32 datalength, uint32 flags) :
			EntityTableItem(itemDBType, datalength, flags)
		{
			memset(db_item_name_, 0, MAX_BUF);
		};

		virtual ~EntityTableItemMongodbBase()
		{
		};

		uint8 type() const{ return TABLE_ITEM_TYPE_UNKONWN; }

		/**
		初始化
		*/
		virtual bool initialize(const PropertyDescription* pPropertyDescription,
			const DataType* pDataType, std::string name)
		{
			return true;
		}

		/**
		同步entity表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL) = 0;

		/**
		更新数据
		*/
		virtual bool writeItem(DBInterface* pdbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule){ return true; }

		/**
		查询表
		*/
		virtual bool queryTable(DBInterface* pdbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule){ return true; }

		/**
		获取某个表所有的数据放到流中
		*/
		virtual void addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID){};

		/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
		*/
		virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context) = 0;
		virtual void getReadSqlItem(mongodb::DBContext& context) = 0;

		virtual void init_db_item_name(const char* exstrFlag = "");
		const char* db_item_name(){ return db_item_name_; }

		virtual bool isSameKey(std::string key){ return key == db_item_name(); }

	protected:
		char db_item_name_[MAX_BUF];
	};

	class EntityTableItemMongodb_DIGIT : public EntityTableItemMongodbBase
	{
	public:
		EntityTableItemMongodb_DIGIT(std::string dataSType, std::string itemDBType,
			uint32 datalength, uint32 flags) :
			EntityTableItemMongodbBase(itemDBType, datalength, flags),
			dataSType_(dataSType)
		{
		};

		virtual ~EntityTableItemMongodb_DIGIT(){};

		uint8 type() const{ return TABLE_ITEM_TYPE_DIGIT; }

		/**
		同步entity表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL){ return true; }

		/**
		获取某个表所有的数据放到流中
		*/
		void addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID){}

		/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
		*/
		virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context){}
		virtual void getReadSqlItem(mongodb::DBContext& context){}
	protected:
		std::string dataSType_;
	};

	class EntityTableItemMongodb_STRING : public EntityTableItemMongodbBase
	{
	public:
		EntityTableItemMongodb_STRING(std::string itemDBType,
			uint32 datalength, uint32 flags) :
			EntityTableItemMongodbBase(itemDBType, datalength, flags)
		{
		}

		virtual ~EntityTableItemMongodb_STRING(){};

		uint8 type() const{ return TABLE_ITEM_TYPE_STRING; }

		/**
		同步entity表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL){ return true; }

		/**
		获取某个表所有的数据放到流中
		*/
		void addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID){}

		/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
		*/
		virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context){}
		virtual void getReadSqlItem(mongodb::DBContext& context){}
	};

	class EntityTableItemMongodb_UNICODE : public EntityTableItemMongodbBase
	{
	public:
		EntityTableItemMongodb_UNICODE(std::string itemDBType,
			uint32 datalength, uint32 flags) :
			EntityTableItemMongodbBase(itemDBType, datalength, flags)
		{
		}

		virtual ~EntityTableItemMongodb_UNICODE(){};

		uint8 type() const{ return TABLE_ITEM_TYPE_UNICODE; }

		/**
		同步entity表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL){ return true; }

		/**
		获取某个表所有的数据放到流中
		*/
		void addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID){}

		/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
		*/
		virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context){}
		virtual void getReadSqlItem(mongodb::DBContext& context){}
	};

	class EntityTableItemMongodb_PYTHON : public EntityTableItemMongodbBase
	{
	public:
		EntityTableItemMongodb_PYTHON(std::string itemDBType,
			uint32 datalength, uint32 flags) :
			EntityTableItemMongodbBase(itemDBType, datalength, flags)
		{
		}

		virtual ~EntityTableItemMongodb_PYTHON(){};

		uint8 type() const{ return TABLE_ITEM_TYPE_PYTHON; }

		/**
		同步entity表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL){ return true; }

		/**
		获取某个表所有的数据放到流中
		*/
		void addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID){}

		/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
		*/
		virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context){}
		virtual void getReadSqlItem(mongodb::DBContext& context){}
	};

	class EntityTableItemMongodb_BLOB : public EntityTableItemMongodbBase
	{
	public:
		EntityTableItemMongodb_BLOB(std::string itemDBType,
			uint32 datalength, uint32 flags) :
			EntityTableItemMongodbBase(itemDBType, datalength, flags)
		{
		}

		virtual ~EntityTableItemMongodb_BLOB(){};

		uint8 type() const{ return TABLE_ITEM_TYPE_BLOB; }

		/**
		同步entity表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL){ return true; }

		/**
		获取某个表所有的数据放到流中
		*/
		void addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID){}

		/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
		*/
		virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context){}
		virtual void getReadSqlItem(mongodb::DBContext& context){}
	};

	class EntityTableItemMongodb_VECTOR2 : public EntityTableItemMongodbBase
	{
	public:
		EntityTableItemMongodb_VECTOR2(std::string itemDBType,
			uint32 datalength, uint32 flags) :
			EntityTableItemMongodbBase(itemDBType, datalength, flags)
		{
		}

		virtual ~EntityTableItemMongodb_VECTOR2(){};

		uint8 type() const{ return TABLE_ITEM_TYPE_VECTOR2; }

		virtual bool isSameKey(std::string key){ return true; }

		/**
		同步entity表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL){ return true; }

		/**
		获取某个表所有的数据放到流中
		*/
		void addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID){}

		/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
		*/
		virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context){}
		virtual void getReadSqlItem(mongodb::DBContext& context){}

		virtual void init_db_item_name(const char* exstrFlag = "")
		{
			for (int i = 0; i<2; ++i)
				kbe_snprintf(db_item_names_[i], MAX_BUF, TABLE_ITEM_PERFIX"_%d_%s%s", i, exstrFlag, itemName());
		}

	protected:
		char db_item_names_[2][MAX_BUF];
	};

	class EntityTableItemMongodb_VECTOR3 : public EntityTableItemMongodbBase
	{
	public:
		EntityTableItemMongodb_VECTOR3(std::string itemDBType,
			uint32 datalength, uint32 flags) :
			EntityTableItemMongodbBase(itemDBType, datalength, flags)
		{
		}

		virtual ~EntityTableItemMongodb_VECTOR3(){};

		uint8 type() const{ return TABLE_ITEM_TYPE_VECTOR3; }

		virtual bool isSameKey(std::string key){ return true; }

		/**
		同步entity表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL){ return true; }

		/**
		获取某个表所有的数据放到流中
		*/
		void addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID){}

		/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
		*/
		virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context){}
		virtual void getReadSqlItem(mongodb::DBContext& context){}

		virtual void init_db_item_name(const char* exstrFlag = "")
		{
			for (int i = 0; i<3; ++i)
				kbe_snprintf(db_item_names_[i], MAX_BUF, TABLE_ITEM_PERFIX"_%d_%s%s", i, exstrFlag, itemName());
		}

	protected:
		char db_item_names_[3][MAX_BUF];
	};

	class EntityTableItemMongodb_VECTOR4 : public EntityTableItemMongodbBase
	{
	public:
		EntityTableItemMongodb_VECTOR4(std::string itemDBType,
			uint32 datalength, uint32 flags) :
			EntityTableItemMongodbBase(itemDBType, datalength, flags)
		{
		}

		virtual ~EntityTableItemMongodb_VECTOR4(){};

		uint8 type() const{ return TABLE_ITEM_TYPE_VECTOR4; }

		virtual bool isSameKey(std::string key){ return true; }

		/**
		同步entity表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL){ return true; }

		/**
		获取某个表所有的数据放到流中
		*/
		void addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID){}

		/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
		*/
		virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context){}
		virtual void getReadSqlItem(mongodb::DBContext& context){}

		virtual void init_db_item_name(const char* exstrFlag = "")
		{
			for (int i = 0; i<4; ++i)
				kbe_snprintf(db_item_names_[i], MAX_BUF, TABLE_ITEM_PERFIX"_%d_%s%s", i, exstrFlag, itemName());
		}

	protected:
		char db_item_names_[4][MAX_BUF];
	};

	class EntityTableItemMongodb_MAILBOX : public EntityTableItemMongodbBase
	{
	public:
		EntityTableItemMongodb_MAILBOX(std::string itemDBType,
			uint32 datalength, uint32 flags) :
			EntityTableItemMongodbBase(itemDBType, datalength, flags)
		{
		}

		virtual ~EntityTableItemMongodb_MAILBOX(){};

		uint8 type() const{ return TABLE_ITEM_TYPE_MAILBOX; }

		/**
		同步entity表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL){ return true; }

		/**
		获取某个表所有的数据放到流中
		*/
		void addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID){}

		/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
		*/
		virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context){}
		virtual void getReadSqlItem(mongodb::DBContext& context){}
	};

	class EntityTableItemMongodb_ARRAY : public EntityTableItemMongodbBase
	{
	public:
		EntityTableItemMongodb_ARRAY(std::string itemDBType,
			uint32 datalength, uint32 flags) :
			EntityTableItemMongodbBase(itemDBType, datalength, flags),
			pChildTable_(NULL)
		{
		}

		virtual ~EntityTableItemMongodb_ARRAY(){};

		virtual bool isSameKey(std::string key){ return true; }

		/**
		初始化
		*/
		virtual bool initialize(const PropertyDescription* pPropertyDescription,
			const DataType* pDataType, std::string name)
		{
			return true;
		}

		uint8 type() const{ return TABLE_ITEM_TYPE_FIXEDARRAY; }

		/**
		同步entity表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL){ return true; }

		/**
		获取某个表所有的数据放到流中
		*/
		void addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID){}

		/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
		*/
		virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context){}
		virtual void getReadSqlItem(mongodb::DBContext& context){}

		virtual void init_db_item_name(const char* exstrFlag = ""){}

	protected:
		EntityTable* pChildTable_;
	};

	class EntityTableItemMongodb_FIXED_DICT : public EntityTableItemMongodbBase
	{
	public:
		EntityTableItemMongodb_FIXED_DICT(std::string itemDBType,
			uint32 datalength, uint32 flags) :
			EntityTableItemMongodbBase(itemDBType, datalength, flags)
		{
		}

		virtual ~EntityTableItemMongodb_FIXED_DICT(){};

		typedef std::vector< std::pair< std::string, KBEShared_ptr<EntityTableItem> > > FIXEDDICT_KEYTYPES;

		uint8 type() const{ return TABLE_ITEM_TYPE_FIXEDDICT; }

		virtual bool isSameKey(std::string key){ return true; }

		/**
		初始化
		*/
		virtual bool initialize(const PropertyDescription* pPropertyDescription,
			const DataType* pDataType, std::string name)
		{
			return true;
		}

		/**
		同步entity表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL){ return true; }

		/**
		获取某个表所有的数据放到流中
		*/
		void addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID){}

		/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
		*/
		virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context){}
		virtual void getReadSqlItem(mongodb::DBContext& context){}

		virtual void init_db_item_name(const char* exstrFlag = ""){}

	protected:
		EntityTableItemMongodb_FIXED_DICT::FIXEDDICT_KEYTYPES			keyTypes_;		// 这个固定字典里的各个key的类型
	};


	/*
	维护entity在数据库中的表
	*/
	class EntityTableMongodb : public EntityTable
	{
	public:
		EntityTableMongodb(EntityTables* pEntityTables);
		virtual ~EntityTableMongodb();

		/**
		初始化
		*/
		virtual bool initialize(ScriptDefModule* sm, std::string name);

		/**
		同步entity表到数据库中
		*/
		virtual bool syncToDB(DBInterface* pdbi);

		/**
		同步表索引
		*/
		virtual bool syncIndexToDB(DBInterface* pdbi){ return true; }

		/**
		创建一个表item
		*/
		virtual EntityTableItem* createItem(std::string type, std::string defaultVal);

		DBID writeTable(DBInterface* pdbi, DBID dbid, int8 shouldAutoLoad, MemoryStream* s, ScriptDefModule* pModule);

		/**
		从数据库删除entity
		*/
		bool removeEntity(DBInterface* pdbi, DBID dbid, ScriptDefModule* pModule){ return true; }

		/**
		获取所有的数据放到流中
		*/
		virtual bool queryTable(DBInterface* pdbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule){ return true; }

		/**
		设置是否自动加载
		*/
		virtual void entityShouldAutoLoad(DBInterface* pdbi, DBID dbid, bool shouldAutoLoad){  }

		/**
		查询自动加载的实体
		*/
		virtual void queryAutoLoadEntities(DBInterface* pdbi, ScriptDefModule* pModule,
			ENTITY_ID start, ENTITY_ID end, std::vector<DBID>& outs){}

		/**
		获取某个表所有的数据放到流中
		*/
		void addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID);

		/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
		*/
		virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context){}
		virtual void getReadSqlItem(mongodb::DBContext& context){}

		void init_db_item_name();

	protected:

	};
}

#ifdef CODE_INLINE
#include "entity_table_mongodb.inl"
#endif