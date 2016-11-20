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

#ifndef KBE_ENTITY_TABLE_MYSQL_H
#define KBE_ENTITY_TABLE_MYSQL_H
#include "db_interface_mysql.h"
#include "common.h"
#include "common/common.h"
#include "common/singleton.h"
#include "helper/debug_helper.h"
#include "db_interface/entity_table.h"

namespace KBEngine { 

class ScriptDefModule;
class EntityTableMysql;

#define MYSQL_ENGINE_TYPE "InnoDB"

/*
	ά��entity�����ݿ���е�һ���ֶ�
*/
class EntityTableItemMysqlBase : public EntityTableItem
{
public:
	EntityTableItemMysqlBase(std::string itemDBType, uint32 datalength, uint32 flags, enum_field_types mysqlItemtype):
	  EntityTableItem(itemDBType, datalength, flags),
	  mysqlItemtype_(mysqlItemtype)
	{
		// ��¼�����������õ����б��
		ALL_MYSQL_SET_FLAGS |= flags;

		memset(db_item_name_, 0, MAX_BUF);
	};

	virtual ~EntityTableItemMysqlBase()
	{
	};

	uint8 type() const{ return TABLE_ITEM_TYPE_UNKONWN; }

	/**
		��ʼ��
	*/
	virtual bool initialize(const PropertyDescription* pPropertyDescription, 
		const DataType* pDataType, std::string name);

	/**
		ͬ��entity�����ݿ���
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL) = 0;

	/**
		��������
	*/
	virtual bool writeItem(DBInterface* pdbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule){ return true; }

	/**
		��ѯ��
	*/
	virtual bool queryTable(DBInterface* pdbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule){ return true; }

	/**
		��ȡĳ�������е����ݷŵ�����
	*/
	virtual void addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID){};

	/**
		��ȡ��Ҫ�洢�ı����� �ֶ�����ת��Ϊsql�洢ʱ���ַ���ֵ
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context) = 0;
	virtual void getReadSqlItem(mysql::DBContext& context) = 0;

	virtual void init_db_item_name(const char* exstrFlag = "");
	const char* db_item_name(){ return db_item_name_; }

	virtual bool isSameKey(std::string key){ return key == db_item_name(); }

protected:
	char db_item_name_[MAX_BUF];
	enum_field_types mysqlItemtype_;
};

class EntityTableItemMysql_DIGIT : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_DIGIT(std::string dataSType, std::string itemDBType, 
		uint32 datalength, uint32 flags, enum_field_types mysqlItemtype):
	  EntityTableItemMysqlBase(itemDBType, datalength, flags, mysqlItemtype),
		  dataSType_(dataSType)
	{
	};

	virtual ~EntityTableItemMysql_DIGIT(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_DIGIT; }

	/**
		ͬ��entity�����ݿ���
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		��ȡĳ�������е����ݷŵ�����
	*/
	void addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID);

	/**
		��ȡ��Ҫ�洢�ı����� �ֶ�����ת��Ϊsql�洢ʱ���ַ���ֵ
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context);
	virtual void getReadSqlItem(mysql::DBContext& context);

protected:
	std::string dataSType_;
};

class EntityTableItemMysql_STRING : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_STRING(std::string itemDBType, 
		uint32 datalength, uint32 flags, enum_field_types mysqlItemtype):
	  EntityTableItemMysqlBase(itemDBType, datalength, flags, mysqlItemtype)
	  {
	  }

	virtual ~EntityTableItemMysql_STRING(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_STRING; }

	/**
		ͬ��entity�����ݿ���
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		��ȡĳ�������е����ݷŵ�����
	*/
	void addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID);

	/**
		��ȡ��Ҫ�洢�ı����� �ֶ�����ת��Ϊsql�洢ʱ���ַ���ֵ
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context);
	virtual void getReadSqlItem(mysql::DBContext& context);
};

class EntityTableItemMysql_UNICODE : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_UNICODE(std::string itemDBType, 
		uint32 datalength, uint32 flags, enum_field_types mysqlItemtype):
	  EntityTableItemMysqlBase(itemDBType, datalength, flags, mysqlItemtype)
	  {
	  }

	virtual ~EntityTableItemMysql_UNICODE(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_UNICODE; }

	/**
		ͬ��entity�����ݿ���
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		��ȡĳ�������е����ݷŵ�����
	*/
	void addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID);

	/**
		��ȡ��Ҫ�洢�ı����� �ֶ�����ת��Ϊsql�洢ʱ���ַ���ֵ
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context);
	virtual void getReadSqlItem(mysql::DBContext& context);
};

class EntityTableItemMysql_PYTHON : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_PYTHON(std::string itemDBType, 
		uint32 datalength, uint32 flags, enum_field_types mysqlItemtype):
	  EntityTableItemMysqlBase(itemDBType, datalength, flags, mysqlItemtype)
	  {
	  }

	virtual ~EntityTableItemMysql_PYTHON(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_PYTHON; }

	/**
		ͬ��entity�����ݿ���
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		��ȡĳ�������е����ݷŵ�����
	*/
	void addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID);

	/**
		��ȡ��Ҫ�洢�ı����� �ֶ�����ת��Ϊsql�洢ʱ���ַ���ֵ
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context);
	virtual void getReadSqlItem(mysql::DBContext& context);
};

class EntityTableItemMysql_BLOB : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_BLOB(std::string itemDBType, 
		uint32 datalength, uint32 flags, enum_field_types mysqlItemtype):
	  EntityTableItemMysqlBase(itemDBType, datalength, flags, mysqlItemtype)
	  {
	  }

	virtual ~EntityTableItemMysql_BLOB(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_BLOB; }

	/**
		ͬ��entity�����ݿ���
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		��ȡĳ�������е����ݷŵ�����
	*/
	void addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID);

	/**
		��ȡ��Ҫ�洢�ı����� �ֶ�����ת��Ϊsql�洢ʱ���ַ���ֵ
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context);
	virtual void getReadSqlItem(mysql::DBContext& context);
};

class EntityTableItemMysql_VECTOR2 : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_VECTOR2(std::string itemDBType, 
		uint32 datalength, uint32 flags, enum_field_types mysqlItemtype):
	  EntityTableItemMysqlBase(itemDBType, datalength, flags, mysqlItemtype)
	  {
	  }

	virtual ~EntityTableItemMysql_VECTOR2(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_VECTOR2; }
	
	virtual bool isSameKey(std::string key);

	/**
		ͬ��entity�����ݿ���
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		��ȡĳ�������е����ݷŵ�����
	*/
	void addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID);

	/**
		��ȡ��Ҫ�洢�ı����� �ֶ�����ת��Ϊsql�洢ʱ���ַ���ֵ
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context);
	virtual void getReadSqlItem(mysql::DBContext& context);

	virtual void init_db_item_name(const char* exstrFlag = "")
	{
		for(int i=0; i<2; ++i)
			kbe_snprintf(db_item_names_[i], MAX_BUF, TABLE_ITEM_PERFIX"_%d_%s%s", i, exstrFlag, itemName());
	}

protected:
	char db_item_names_[2][MAX_BUF];
};

class EntityTableItemMysql_VECTOR3 : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_VECTOR3(std::string itemDBType, 
		uint32 datalength, uint32 flags, enum_field_types mysqlItemtype):
	  EntityTableItemMysqlBase(itemDBType, datalength, flags, mysqlItemtype)
	  {
	  }

	virtual ~EntityTableItemMysql_VECTOR3(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_VECTOR3; }

	virtual bool isSameKey(std::string key);

	/**
		ͬ��entity�����ݿ���
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		��ȡĳ�������е����ݷŵ�����
	*/
	void addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID);

	/**
		��ȡ��Ҫ�洢�ı����� �ֶ�����ת��Ϊsql�洢ʱ���ַ���ֵ
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context);
	virtual void getReadSqlItem(mysql::DBContext& context);

	virtual void init_db_item_name(const char* exstrFlag = "")
	{
		for(int i=0; i<3; ++i)
			kbe_snprintf(db_item_names_[i], MAX_BUF, TABLE_ITEM_PERFIX"_%d_%s%s", i, exstrFlag, itemName());
	}

protected:
	char db_item_names_[3][MAX_BUF];
};

class EntityTableItemMysql_VECTOR4 : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_VECTOR4(std::string itemDBType, 
		uint32 datalength, uint32 flags, enum_field_types mysqlItemtype):
	  EntityTableItemMysqlBase(itemDBType, datalength, flags, mysqlItemtype)
	  {
	  }

	virtual ~EntityTableItemMysql_VECTOR4(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_VECTOR4; }

	virtual bool isSameKey(std::string key);

	/**
		ͬ��entity�����ݿ���
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		��ȡĳ�������е����ݷŵ�����
	*/
	void addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID);

	/**
		��ȡ��Ҫ�洢�ı����� �ֶ�����ת��Ϊsql�洢ʱ���ַ���ֵ
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context);
	virtual void getReadSqlItem(mysql::DBContext& context);

	virtual void init_db_item_name(const char* exstrFlag = "")
	{
		for(int i=0; i<4; ++i)
			kbe_snprintf(db_item_names_[i], MAX_BUF, TABLE_ITEM_PERFIX"_%d_%s%s", i, exstrFlag, itemName());
	}

protected:
	char db_item_names_[4][MAX_BUF];
};

class EntityTableItemMysql_MAILBOX : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_MAILBOX(std::string itemDBType, 
		uint32 datalength, uint32 flags, enum_field_types mysqlItemtype):
	  EntityTableItemMysqlBase(itemDBType, datalength, flags, mysqlItemtype)
	  {
	  }

	virtual ~EntityTableItemMysql_MAILBOX(){};

	uint8 type() const{ return TABLE_ITEM_TYPE_MAILBOX; }

	/**
		ͬ��entity�����ݿ���
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		��ȡĳ�������е����ݷŵ�����
	*/
	void addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID);

	/**
		��ȡ��Ҫ�洢�ı����� �ֶ�����ת��Ϊsql�洢ʱ���ַ���ֵ
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context);
	virtual void getReadSqlItem(mysql::DBContext& context);
};

class EntityTableItemMysql_ARRAY : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_ARRAY(std::string itemDBType, 
		uint32 datalength, uint32 flags, enum_field_types mysqlItemtype):
	  EntityTableItemMysqlBase(itemDBType, datalength, flags, mysqlItemtype),
	  pChildTable_(NULL)
	  {
	  }

	virtual ~EntityTableItemMysql_ARRAY(){};

	virtual bool isSameKey(std::string key);

	/**
		��ʼ��
	*/
	virtual bool initialize(const PropertyDescription* pPropertyDescription, 
		const DataType* pDataType, std::string name);

	uint8 type() const{ return TABLE_ITEM_TYPE_FIXEDARRAY; }

	/**
		ͬ��entity�����ݿ���
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		��ȡĳ�������е����ݷŵ�����
	*/
	void addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID);

	/**
		��ȡ��Ҫ�洢�ı����� �ֶ�����ת��Ϊsql�洢ʱ���ַ���ֵ
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context);
	virtual void getReadSqlItem(mysql::DBContext& context);

	virtual void init_db_item_name(const char* exstrFlag = "");

protected:
	EntityTable* pChildTable_;
};

class EntityTableItemMysql_FIXED_DICT : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_FIXED_DICT(std::string itemDBType, 
		uint32 datalength, uint32 flags, enum_field_types mysqlItemtype):
	  EntityTableItemMysqlBase(itemDBType, datalength, flags, mysqlItemtype)
	  {
	  }

	virtual ~EntityTableItemMysql_FIXED_DICT(){};

	typedef std::vector< std::pair< std::string, KBEShared_ptr<EntityTableItem> > > FIXEDDICT_KEYTYPES;

	uint8 type() const{ return TABLE_ITEM_TYPE_FIXEDDICT; }

	virtual bool isSameKey(std::string key);

	/**
		��ʼ��
	*/
	virtual bool initialize(const PropertyDescription* pPropertyDescription, 
		const DataType* pDataType, std::string name);

	/**
		ͬ��entity�����ݿ���
	*/
	virtual bool syncToDB(DBInterface* pdbi, void* pData = NULL);

	/**
		��ȡĳ�������е����ݷŵ�����
	*/
	void addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID);

	/**
		��ȡ��Ҫ�洢�ı����� �ֶ�����ת��Ϊsql�洢ʱ���ַ���ֵ
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context);
	virtual void getReadSqlItem(mysql::DBContext& context);

	virtual void init_db_item_name(const char* exstrFlag = "");

	uint32 getItemDatabaseLength(const std::string& name);

protected:
	EntityTableItemMysql_FIXED_DICT::FIXEDDICT_KEYTYPES			keyTypes_;		// ����̶��ֵ���ĸ���key������
};


/*
	ά��entity�����ݿ��еı�
*/
class EntityTableMysql : public EntityTable
{
public:
	EntityTableMysql(EntityTables* pEntityTables);
	virtual ~EntityTableMysql();
	
	/**
		��ʼ��
	*/
	virtual bool initialize(ScriptDefModule* sm, std::string name);

	/**
		ͬ��entity�����ݿ���
	*/
	virtual bool syncToDB(DBInterface* pdbi);

	/**
		ͬ��������
	*/
	virtual bool syncIndexToDB(DBInterface* pdbi);

	/** 
		����һ����item
	*/
	virtual EntityTableItem* createItem(std::string type, std::string defaultVal);

	DBID writeTable(DBInterface* pdbi, DBID dbid, int8 shouldAutoLoad, MemoryStream* s, ScriptDefModule* pModule);

	/**
		�����ݿ�ɾ��entity
	*/
	bool removeEntity(DBInterface* pdbi, DBID dbid, ScriptDefModule* pModule);

	/**
		��ȡ���е����ݷŵ�����
	*/
	virtual bool queryTable(DBInterface* pdbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		�����Ƿ��Զ�����
	*/
	virtual void entityShouldAutoLoad(DBInterface* pdbi, DBID dbid, bool shouldAutoLoad);

	/**
		��ѯ�Զ����ص�ʵ��
	*/
	virtual void queryAutoLoadEntities(DBInterface* pdbi, ScriptDefModule* pModule, 
		ENTITY_ID start, ENTITY_ID end, std::vector<DBID>& outs);

	/**
		��ȡĳ�������е����ݷŵ�����
	*/
	void addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID);

	/**
		��ȡ��Ҫ�洢�ı����� �ֶ�����ת��Ϊsql�洢ʱ���ַ���ֵ
	*/
	virtual void getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context);
	virtual void getReadSqlItem(mysql::DBContext& context);

	void init_db_item_name();

protected:
	
};


}

#ifdef CODE_INLINE
#include "entity_table_mysql.inl"
#endif
#endif // KBE_ENTITY_TABLE_MYSQL_H
