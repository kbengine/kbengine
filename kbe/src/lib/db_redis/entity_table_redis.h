// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_ENTITY_TABLE_REDIS_H
#define KBE_ENTITY_TABLE_REDIS_H
#include "db_interface_redis.h"
#include "common.h"
#include "common/common.h"
#include "common/singleton.h"
#include "helper/debug_helper.h"
#include "db_interface/entity_table.h"

namespace KBEngine { 

class ScriptDefModule;
class EntityTableRedis;

/*
	维护entity在数据库表中的一个字段
*/
class EntityTableItemRedisBase : public EntityTableItem
{
public:
	EntityTableItemRedisBase(std::string itemDBType, uint32 datalength, uint32 flags):
	  EntityTableItem(itemDBType, datalength, flags)
	{
		memset(db_item_name_, 0, MAX_BUF);
	};

	virtual ~EntityTableItemRedisBase()
	{
	};

	uint8 type() const{ return TABLE_ITEM_TYPE_UNKONWN; }

	/**
		初始化
	*/
	virtual bool initialize(const PropertyDescription* pPropertyDescription, 
		const DataType* pDataType, std::string name);

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
	virtual void addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID){};

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context) = 0;
	virtual void getReadSqlItem(redis::DBContext& context) = 0;

	virtual void init_db_item_name(const char* exstrFlag = "");
	const char* db_item_name(){ return db_item_name_; }

	virtual bool isSameKey(std::string key){ return key == db_item_name(); }
protected:
	char db_item_name_[MAX_BUF];
};

class EntityTableItemRedis_DIGIT : public EntityTableItemRedisBase
{
public:
	EntityTableItemRedis_DIGIT(std::string dataSType, std::string itemDBType, 
		uint32 datalength, uint32 flags):
	  EntityTableItemRedisBase(itemDBType, datalength, flags),
		  dataSType_(dataSType)
	{
	};

	virtual ~EntityTableItemRedis_DIGIT(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_DIGIT; }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		获取某个表所有的数据放到流中
	*/
	void addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context);
	virtual void getReadSqlItem(redis::DBContext& context);
protected:
	std::string dataSType_;
};

class EntityTableItemRedis_STRING : public EntityTableItemRedisBase
{
public:
	EntityTableItemRedis_STRING(std::string itemDBType, 
		uint32 datalength, uint32 flags):
	  EntityTableItemRedisBase(itemDBType, datalength, flags)
	  {
	  }

	virtual ~EntityTableItemRedis_STRING(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_STRING; }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		获取某个表所有的数据放到流中
	*/
	void addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context);
	virtual void getReadSqlItem(redis::DBContext& context);
};

class EntityTableItemRedis_UNICODE : public EntityTableItemRedisBase
{
public:
	EntityTableItemRedis_UNICODE(std::string itemDBType, 
		uint32 datalength, uint32 flags):
	  EntityTableItemRedisBase(itemDBType, datalength, flags)
	  {
	  }

	virtual ~EntityTableItemRedis_UNICODE(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_UNICODE; }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		获取某个表所有的数据放到流中
	*/
	void addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context);
	virtual void getReadSqlItem(redis::DBContext& context);
};

class EntityTableItemRedis_PYTHON : public EntityTableItemRedisBase
{
public:
	EntityTableItemRedis_PYTHON(std::string itemDBType, 
		uint32 datalength, uint32 flags):
	  EntityTableItemRedisBase(itemDBType, datalength, flags)
	  {
	  }

	virtual ~EntityTableItemRedis_PYTHON(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_PYTHON; }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		获取某个表所有的数据放到流中
	*/
	void addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context);
	virtual void getReadSqlItem(redis::DBContext& context);
};

class EntityTableItemRedis_BLOB : public EntityTableItemRedisBase
{
public:
	EntityTableItemRedis_BLOB(std::string itemDBType, 
		uint32 datalength, uint32 flags):
	  EntityTableItemRedisBase(itemDBType, datalength, flags)
	  {
	  }

	virtual ~EntityTableItemRedis_BLOB(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_BLOB; }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		获取某个表所有的数据放到流中
	*/
	void addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context);
	virtual void getReadSqlItem(redis::DBContext& context);
};

class EntityTableItemRedis_VECTOR2 : public EntityTableItemRedisBase
{
public:
	EntityTableItemRedis_VECTOR2(std::string itemDBType, 
		uint32 datalength, uint32 flags):
	  EntityTableItemRedisBase(itemDBType, datalength, flags)
	  {
	  }

	virtual ~EntityTableItemRedis_VECTOR2(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_VECTOR2; }
	
	virtual bool isSameKey(std::string key);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		获取某个表所有的数据放到流中
	*/
	void addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context);
	virtual void getReadSqlItem(redis::DBContext& context);

	virtual void init_db_item_name(const char* exstrFlag = "")
	{
		for(int i=0; i<2; ++i)
			kbe_snprintf(db_item_names_[i], MAX_BUF, TABLE_ITEM_PERFIX"_%d_%s%s", i, exstrFlag, itemName());
	}

protected:
	char db_item_names_[2][MAX_BUF];
};

class EntityTableItemRedis_VECTOR3 : public EntityTableItemRedisBase
{
public:
	EntityTableItemRedis_VECTOR3(std::string itemDBType, 
		uint32 datalength, uint32 flags):
	  EntityTableItemRedisBase(itemDBType, datalength, flags)
	  {
	  }

	virtual ~EntityTableItemRedis_VECTOR3(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_VECTOR3; }

	virtual bool isSameKey(std::string key);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		获取某个表所有的数据放到流中
	*/
	void addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context);
	virtual void getReadSqlItem(redis::DBContext& context);

	virtual void init_db_item_name(const char* exstrFlag = "")
	{
		for(int i=0; i<3; ++i)
			kbe_snprintf(db_item_names_[i], MAX_BUF, TABLE_ITEM_PERFIX"_%d_%s%s", i, exstrFlag, itemName());
	}

protected:
	char db_item_names_[3][MAX_BUF];
};

class EntityTableItemRedis_VECTOR4 : public EntityTableItemRedisBase
{
public:
	EntityTableItemRedis_VECTOR4(std::string itemDBType, 
		uint32 datalength, uint32 flags):
	  EntityTableItemRedisBase(itemDBType, datalength, flags)
	  {
	  }

	virtual ~EntityTableItemRedis_VECTOR4(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_VECTOR4; }

	virtual bool isSameKey(std::string key);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		获取某个表所有的数据放到流中
	*/
	void addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context);
	virtual void getReadSqlItem(redis::DBContext& context);

	virtual void init_db_item_name(const char* exstrFlag = "")
	{
		for(int i=0; i<4; ++i)
			kbe_snprintf(db_item_names_[i], MAX_BUF, TABLE_ITEM_PERFIX"_%d_%s%s", i, exstrFlag, itemName());
	}

protected:
	char db_item_names_[4][MAX_BUF];
};

class EntityTableItemRedis_ENTITYCALL : public EntityTableItemRedisBase
{
public:
	EntityTableItemRedis_ENTITYCALL(std::string itemDBType, 
		uint32 datalength, uint32 flags):
	  EntityTableItemRedisBase(itemDBType, datalength, flags)
	  {
	  }

	virtual ~EntityTableItemRedis_ENTITYCALL(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_ENTITYCALL; }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		获取某个表所有的数据放到流中
	*/
	void addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context);
	virtual void getReadSqlItem(redis::DBContext& context);
};

class EntityTableItemRedis_ARRAY : public EntityTableItemRedisBase
{
public:
	EntityTableItemRedis_ARRAY(std::string itemDBType, 
		uint32 datalength, uint32 flags):
	  EntityTableItemRedisBase(itemDBType, datalength, flags),
	  pChildTable_(NULL)
	  {
	  }

	virtual ~EntityTableItemRedis_ARRAY(){};

	virtual bool isSameKey(std::string key);

	/**
		初始化
	*/
	virtual bool initialize(const PropertyDescription* pPropertyDescription, 
		const DataType* pDataType, std::string name);

	uint8 type() const{ return TABLE_ITEM_TYPE_FIXEDARRAY; }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		获取某个表所有的数据放到流中
	*/
	void addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context);
	virtual void getReadSqlItem(redis::DBContext& context);

	virtual void init_db_item_name(const char* exstrFlag = "");

protected:
	EntityTable* pChildTable_;
};

class EntityTableItemRedis_FIXED_DICT : public EntityTableItemRedisBase
{
public:
	EntityTableItemRedis_FIXED_DICT(std::string itemDBType, 
		uint32 datalength, uint32 flags):
	  EntityTableItemRedisBase(itemDBType, datalength, flags)
	  {
	  }

	virtual ~EntityTableItemRedis_FIXED_DICT(){};

	typedef std::vector< std::pair< std::string, KBEShared_ptr<EntityTableItem> > > FIXEDDICT_KEYTYPES;

	uint8 type() const{ return TABLE_ITEM_TYPE_FIXEDDICT; }

	virtual bool isSameKey(std::string key);

	/**
		初始化
	*/
	virtual bool initialize(const PropertyDescription* pPropertyDescription, 
		const DataType* pDataType, std::string name);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		获取某个表所有的数据放到流中
	*/
	void addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context);
	virtual void getReadSqlItem(redis::DBContext& context);

	virtual void init_db_item_name(const char* exstrFlag = "");

protected:
	EntityTableItemRedis_FIXED_DICT::FIXEDDICT_KEYTYPES			keyTypes_;		// 这个固定字典里的各个key的类型
};


/*
	维护entity在数据库中的表
*/
class EntityTableRedis : public EntityTable
{
public:
	EntityTableRedis(EntityTables* pEntityTables);
	virtual ~EntityTableRedis();
	
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
	virtual bool syncIndexToDB(DBInterface* pdbi);

	/** 
		创建一个表item
	*/
	virtual EntityTableItem* createItem(std::string type, std::string defaultVal);

	DBID writeTable(DBInterface* pdbi, DBID dbid, int8 shouldAutoLoad, MemoryStream* s, ScriptDefModule* pModule);

	/**
		从数据库删除entity
	*/
	bool removeEntity(DBInterface* pdbi, DBID dbid, ScriptDefModule* pModule);

	/**
		获取所有的数据放到流中
	*/
	virtual bool queryTable(DBInterface* pdbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		设置是否自动加载
	*/
	virtual void entityShouldAutoLoad(DBInterface* pdbi, DBID dbid, bool shouldAutoLoad);

	/**
		查询自动加载的实体
	*/
	virtual void queryAutoLoadEntities(DBInterface* pdbi, ScriptDefModule* pModule, 
		ENTITY_ID start, ENTITY_ID end, std::vector<DBID>& outs);

	/**
		获取某个表所有的数据放到流中
	*/
	void addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context);
	virtual void getReadSqlItem(redis::DBContext& context);

	void init_db_item_name();

protected:
	
};


}

#ifdef CODE_INLINE
#include "entity_table_redis.inl"
#endif
#endif // KBE_ENTITY_TABLE_REDIS_H
