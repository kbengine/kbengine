#include "entity_table_mongodb.h"
#include "kbe_table_mongodb.h"
#include "entitydef/scriptdef_module.h"
#include "entitydef/property.h"
#include "db_interface/db_interface.h"
#include "db_interface/entity_table.h"
#include "network/fixed_messages.h"

#ifndef CODE_INLINE
#include "entity_table_mongodb.inl"
#endif

namespace KBEngine {

	EntityTableMongodb::EntityTableMongodb(EntityTables* pEntityTables) :
		EntityTable(pEntityTables)
	{
	}

	EntityTableMongodb::~EntityTableMongodb()
	{

	}

	bool EntityTableMongodb::initialize(ScriptDefModule* sm, std::string name)
	{
		// 获取表名
		tableName(name);

		// 找到所有存储属性并且创建出所有的字段
		ScriptDefModule::PROPERTYDESCRIPTION_MAP& pdescrsMap = sm->getPersistentPropertyDescriptions();
		ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = pdescrsMap.begin();
		std::string hasUnique = "";

		for (; iter != pdescrsMap.end(); ++iter)
		{
			PropertyDescription* pdescrs = iter->second;

			EntityTableItem* pETItem = this->createItem(pdescrs->getDataType()->getName(), pdescrs->getDefaultValStr());

			pETItem->pParentTable(this);
			pETItem->utype(pdescrs->getUType());
			pETItem->tableName(this->tableName());

			bool ret = pETItem->initialize(pdescrs, pdescrs->getDataType(), pdescrs->getName());

			if (!ret)
			{
				delete pETItem;
				return false;
			}

			tableItems_[pETItem->utype()].reset(pETItem);
			tableFixedOrderItems_.push_back(pETItem);
		}

		// 特殊处理， 数据库保存方向和位置
		if (sm->hasCell())
		{
			ENTITY_PROPERTY_UID posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;
			ENTITY_PROPERTY_UID diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;

			Network::FixedMessages::MSGInfo* msgInfo =
				Network::FixedMessages::getSingleton().isFixed("Property::position");

			if (msgInfo != NULL)
			{
				posuid = msgInfo->msgid;
				msgInfo = NULL;
			}

			msgInfo = Network::FixedMessages::getSingleton().isFixed("Property::direction");
			if (msgInfo != NULL)
			{
				diruid = msgInfo->msgid;
				msgInfo = NULL;
			}

			EntityTableItem* pETItem = this->createItem("VECTOR3", "");
			pETItem->pParentTable(this);
			pETItem->utype(posuid);
			pETItem->tableName(this->tableName());
			pETItem->itemName("position");
			tableItems_[pETItem->utype()].reset(pETItem);
			tableFixedOrderItems_.push_back(pETItem);

			pETItem = this->createItem("VECTOR3", "");
			pETItem->pParentTable(this);
			pETItem->utype(diruid);
			pETItem->tableName(this->tableName());
			pETItem->itemName("direction");
			tableItems_[pETItem->utype()].reset(pETItem);
			tableFixedOrderItems_.push_back(pETItem);
		}

		init_db_item_name();

		return true;
	}

	//-------------------------------------------------------------------------------------
	void EntityTableMongodb::init_db_item_name()
	{
		EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
		for (; iter != tableItems_.end(); ++iter)
		{
			// 处理fixedDict字段名称的特例情况
			std::string exstrFlag = "";
			if (iter->second->type() == TABLE_ITEM_TYPE_FIXEDDICT)
			{
				exstrFlag = iter->second->itemName();
				if (exstrFlag.size() > 0)
					exstrFlag += "_";
			}

			static_cast<EntityTableItemMongodbBase*>(iter->second.get())->init_db_item_name(exstrFlag.c_str());
		}
	}

	bool EntityTableMongodb::syncToDB(DBInterface* pdbi)
	{
		//ERROR_MSG(fmt::format("EntityTableMysql::syncToDB(): {}.\n", tableName()));
		if (hasSync())
			return true;

		// DEBUG_MSG(fmt::format("EntityTableMysql::syncToDB(): {}.\n", tableName()));

		char name[MAX_BUF];
		kbe_snprintf(name, MAX_BUF, ENTITY_TABLE_PERFIX "_%s", tableName());

		if (!isChild_)
		{
			//在这里写执行命令，初始化数据库的表结构
			DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
			pdbiMongodb->createCollection(name);
		}
		return true;
	}

	EntityTableItem* EntityTableMongodb::createItem(std::string type, std::string defaultVal)
	{
		if (type == "INT8")
		{
			return new EntityTableItemMongodb_DIGIT(type, "tinyint not null DEFAULT 0", 4, 0);
		}
		else if (type == "INT16")
		{
			return new EntityTableItemMongodb_DIGIT(type, "smallint not null DEFAULT 0", 6, 0);
		}
		else if (type == "INT32")
		{
			return new EntityTableItemMongodb_DIGIT(type, "int not null DEFAULT 0", 11, 0);
		}
		else if (type == "INT64")
		{
			return new EntityTableItemMongodb_DIGIT(type, "bigint not null DEFAULT 0", 20, 0);
		}
		else if (type == "UINT8")
		{
			return new EntityTableItemMongodb_DIGIT(type, "tinyint unsigned not null DEFAULT 0", 3, 0);
		}
		else if (type == "UINT16")
		{
			return new EntityTableItemMongodb_DIGIT(type, "smallint unsigned not null DEFAULT 0", 5, 0);
		}
		else if (type == "UINT32")
		{
			return new EntityTableItemMongodb_DIGIT(type, "int unsigned not null DEFAULT 0", 10, 0);
		}
		else if (type == "UINT64")
		{
			return new EntityTableItemMongodb_DIGIT(type, "bigint unsigned not null DEFAULT 0", 20, 0);
		}
		else if (type == "FLOAT")
		{
			return new EntityTableItemMongodb_DIGIT(type, "float not null DEFAULT 0", 0, 0);
		}
		else if (type == "DOUBLE")
		{
			return new EntityTableItemMongodb_DIGIT(type, "double not null DEFAULT 0", 0, 0);
		}
		else if (type == "STRING")
		{
			return new EntityTableItemMongodb_STRING("text", 0, 0);
		}
		else if (type == "UNICODE")
		{
			return new EntityTableItemMongodb_UNICODE("text", 0, 0);
		}
		else if (type == "PYTHON")
		{
			return new EntityTableItemMongodb_PYTHON("blob", 0, 0);
		}
		else if (type == "PY_DICT")
		{
			return new EntityTableItemMongodb_PYTHON("blob", 0, 0);
		}
		else if (type == "PY_TUPLE")
		{
			return new EntityTableItemMongodb_PYTHON("blob", 0, 0);
		}
		else if (type == "PY_LIST")
		{
			return new EntityTableItemMongodb_PYTHON("blob", 0, 0);
		}
		else if (type == "BLOB")
		{
			return new EntityTableItemMongodb_BLOB("blob", 0, 0);
		}
		else if (type == "ARRAY")
		{
			return new EntityTableItemMongodb_ARRAY("", 0, 0);
		}
		else if (type == "FIXED_DICT")
		{
			return new EntityTableItemMongodb_FIXED_DICT("", 0, 0);
		}
#ifdef CLIENT_NO_FLOAT
		else if (type == "VECTOR2")
		{
			return new EntityTableItemMongodb_VECTOR2("int not null DEFAULT 0", 0, 0);
		}
		else if (type == "VECTOR3")
		{
			return new EntityTableItemMongodb_VECTOR3("int not null DEFAULT 0", 0, 0);
		}
		else if (type == "VECTOR4")
		{
			return new EntityTableItemMongodb_VECTOR4("int not null DEFAULT 0", 0, 0);
		}
#else
		else if (type == "VECTOR2")
		{
			return new EntityTableItemMongodb_VECTOR2("float not null DEFAULT 0", 0, 0);
		}
		else if (type == "VECTOR3")
		{
			return new EntityTableItemMongodb_VECTOR3("float not null DEFAULT 0", 0, 0);
		}
		else if (type == "VECTOR4")
		{
			return new EntityTableItemMongodb_VECTOR4("float not null DEFAULT 0", 0, 0);
		}
#endif
		else if (type == "MAILBOX")
		{
			return new EntityTableItemMongodb_MAILBOX("blob", 0, 0);
		}

		KBE_ASSERT(false && "not found type.\n");
		return new EntityTableItemMongodb_STRING("", 0, 0);
	}

	DBID EntityTableMongodb::writeTable(DBInterface* pdbi, DBID dbid, int8 shouldAutoLoad, MemoryStream* s, ScriptDefModule* pModule)
	{
		mongodb::DBContext context;
		context.parentTableName = "";
		context.parentTableDBID = 0;
		context.dbid = dbid;
		context.tableName = pModule->getName();
		context.isEmpty = false;
		context.readresultIdx = 0;
		
		bson_t doc;
		bson_init(&doc);

		while (s->length() > 0)
		{
			ENTITY_PROPERTY_UID pid;
			(*s) >> pid;

			EntityTableItem* pTableItem = this->findItem(pid);
			if (pTableItem == NULL)
			{
				ERROR_MSG(fmt::format("EntityTable::writeTable: not found item[{}].\n", pid));
				return dbid;
			}

			static_cast<EntityTableItemMongodbBase*>(pTableItem)->getWriteSqlItem(pdbi, s, context, &doc);
		}

		//size_t len = 0;
		//char* json = bson_as_json(&doc, &len);

		if (context.dbid > 0) //更新
		{
			bson_t query;
			bson_init(&query);
			BSON_APPEND_INT64(&query, "id", context.dbid);

			DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
			char name[MAX_BUF];
			kbe_snprintf(name, MAX_BUF, ENTITY_TABLE_PERFIX "_%s", context.tableName.c_str());

			BSON_APPEND_INT64(&doc, "id", context.dbid);
			BSON_APPEND_INT32(&doc, TABLE_ITEM_PERFIX"_" TABLE_AUTOLOAD_CONST_STR, shouldAutoLoad);
			pdbiMongodb->updateCollection(name, &query, &doc);

			bson_destroy(&query);

		}
		else //插入
		{
			DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
			char name[MAX_BUF];
			kbe_snprintf(name, MAX_BUF, ENTITY_TABLE_PERFIX "_%s", context.tableName.c_str());

			context.dbid = time(NULL); //<todo:yelei>这里以后要修改
			BSON_APPEND_INT64(&doc, TABLE_ID_CONST_STR, context.dbid);
			BSON_APPEND_INT32(&doc, TABLE_ITEM_PERFIX"_" TABLE_AUTOLOAD_CONST_STR, shouldAutoLoad);
			pdbiMongodb->insertCollection(name, &doc);
		}

		bson_destroy(&doc);

		dbid = context.dbid;

		// 设置实体是否自动加载
		if (shouldAutoLoad > -1)
			entityShouldAutoLoad(pdbi, dbid, shouldAutoLoad > 0);

		//这里需要修改
		return dbid; 
	}

	/**
	获取所有的数据放到流中
	*/
	bool EntityTableMongodb::queryTable(DBInterface* pdbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
	{
		KBE_ASSERT(pModule && s && dbid > 0);

		mongodb::DBContext context;
		context.parentTableName = "";
		context.parentTableDBID = 0;
		context.dbid = dbid;
		context.tableName = pModule->getName();
		context.isEmpty = false;
		context.readresultIdx = 0;

		std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();
		for (; iter != tableFixedOrderItems_.end(); ++iter)
		{
			static_cast<EntityTableItemMongodbBase*>((*iter))->getReadSqlItem(context);
		}

		//先把数据查询到手
		bson_t query;
		bson_init(&query);
		BSON_APPEND_INT64(&query, "id", dbid);

		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		char name[MAX_BUF];
		kbe_snprintf(name, MAX_BUF, ENTITY_TABLE_PERFIX "_%s", context.tableName.c_str());

		mongoc_cursor_t * cursor = pdbiMongodb->collectionFind(name, &query);
		
		std::list<const bson_t *> value;
		const bson_t *doc;
		bson_error_t  error;
		while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &doc)) {
			value.push_back(doc);
		}

		if (mongoc_cursor_error(cursor, &error)) {
			ERROR_MSG("An error occurred: %s\n", error.message);
		}

		if (value.size() == 0)
		{
			mongoc_cursor_destroy(cursor);
			return false;
		}
		
		
		iter = tableFixedOrderItems_.begin();
		for (; iter != tableFixedOrderItems_.end(); ++iter)
		{
			static_cast<EntityTableItemMongodbBase*>((*iter))->addToStream(s, context, dbid, value.front());
		}

		mongoc_cursor_destroy(cursor);

		return true;
	}


	//-------------------------------------------------------------------------------------
	void EntityTableItemMongodbBase::init_db_item_name(const char* exstrFlag)
	{
		kbe_snprintf(db_item_name_, MAX_BUF, TABLE_ITEM_PERFIX"_%s%s", exstrFlag, itemName());
	}

	bool EntityTableItemMongodbBase::initialize(const PropertyDescription* pPropertyDescription,
		const DataType* pDataType, std::string name)
	{
		itemName(name);

		pDataType_ = pDataType;
		pPropertyDescription_ = pPropertyDescription;
		indexType_ = pPropertyDescription->indexType();
		return true;
	}

	//-------------------------------------------------------------------------------------

	//-------------------------------------------------------------------------------------
	void EntityTableMongodb::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context, bson_t * doc)
	{
		if (tableFixedOrderItems_.size() == 0)
			return;

		std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();

		mongodb::DBContext* context1 = new mongodb::DBContext();
		context1->parentTableName = (*iter)->pParentTable()->tableName();
		context1->tableName = (*iter)->tableName();
		context1->parentTableDBID = 0;
		context1->dbid = 0;
		context1->isEmpty = (s == NULL);
		context1->readresultIdx = 0;

		KBEShared_ptr< mongodb::DBContext > opTableValBox1Ptr(context1);
		context.optable.push_back(std::pair<std::string/*tableName*/, KBEShared_ptr< mongodb::DBContext > >
			((*iter)->tableName(), opTableValBox1Ptr));	

		for (; iter != tableFixedOrderItems_.end(); ++iter)
		{
			static_cast<EntityTableItemMongodbBase*>((*iter))->getWriteSqlItem(pdbi, s, *context1, doc);
		}
	}

	void EntityTableMongodb::getReadSqlItem(mongodb::DBContext& context)
	{
		if (tableFixedOrderItems_.size() == 0)
			return;

		std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();

		mongodb::DBContext* context1 = new mongodb::DBContext();
		context1->parentTableName = (*iter)->pParentTable()->tableName();
		context1->tableName = (*iter)->tableName();
		context1->parentTableDBID = 0;
		context1->dbid = 0;
		context1->isEmpty = true;
		context1->readresultIdx = 0;

		KBEShared_ptr< mongodb::DBContext > opTableValBox1Ptr(context1);
		context.optable.push_back(std::pair<std::string/*tableName*/, KBEShared_ptr< mongodb::DBContext > >
			((*iter)->tableName(), opTableValBox1Ptr));

		for (; iter != tableFixedOrderItems_.end(); ++iter)
		{
			static_cast<EntityTableItemMongodbBase*>((*iter))->getReadSqlItem(*context1);
		}
	}

	void EntityTableMongodb::addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID, const bson_t * doc)
	{
		std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();
		for (; iter != tableFixedOrderItems_.end(); ++iter)
		{
			static_cast<EntityTableItemMongodbBase*>((*iter))->addToStream(s, context, resultDBID, doc);
		}
	}

	//-------------------------------------------------------------------------------------
	bool EntityTableItemMongodb_VECTOR2::isSameKey(std::string key)
	{
		for (int i = 0; i<2; ++i)
		{
			if (key == db_item_names_[i])
				return true;
		}

		return false;
	}
	
	void EntityTableItemMongodb_VECTOR2::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context, bson_t * doc)
	{
		if (s == NULL)
			return;

#ifdef CLIENT_NO_FLOAT
		int32 v;
#else
		float v;
#endif

		ArraySize asize;

		(*s) >> asize;
		KBE_ASSERT(asize == 2);

		for (ArraySize i = 0; i<asize; ++i)
		{
			(*s) >> v;
			mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();
			pSotvs->sqlkey = db_item_names_[i];

#ifdef CLIENT_NO_FLOAT
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);
#else
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%f", v);
#endif
			
			context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));

			//bson_t
			BSON_APPEND_DOUBLE(doc, pSotvs->sqlkey, v);
		}
	}

	void EntityTableItemMongodb_VECTOR2::getReadSqlItem(mongodb::DBContext& context)
	{
		ArraySize asize = 2;
		for (ArraySize i = 0; i<asize; ++i)
		{
			mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();
			pSotvs->sqlkey = db_item_names_[i];
			memset(pSotvs->sqlval, 0, MAX_BUF);
			context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));
		}
	}

	void EntityTableItemMongodb_VECTOR2::addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID, const bson_t * doc)
	{
		ArraySize asize = 2;
		(*s) << asize;

		for (ArraySize i = 0; i < asize; ++i)
		{
			bson_iter_t iter;
			if (!bson_iter_init_find(&iter, doc, db_item_names_[i]))
				continue;

#ifdef CLIENT_NO_FLOAT
			int32 v = bson_iter_int32(&iter);
#else
			double vv = bson_iter_double(&iter);
			float v = static_cast<float>(vv);
#endif
			(*s) << v;
		}
	}

	//-------------------------------------------------------------------------------------
	void EntityTableItemMongodb_VECTOR3::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context, bson_t * doc)
	{
		if (s == NULL)
			return;

#ifdef CLIENT_NO_FLOAT
		int32 v;
#else
		float v;
#endif

		ArraySize asize;

		(*s) >> asize;
		KBE_ASSERT(asize == 3);

		for (ArraySize i = 0; i<asize; ++i)
		{
			(*s) >> v;
			mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();
			pSotvs->sqlkey = db_item_names_[i];

#ifdef CLIENT_NO_FLOAT
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);
#else
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%f", v);
#endif
			context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));

			//bson_t
			BSON_APPEND_DOUBLE(doc, pSotvs->sqlkey, v);
		}
	}

	void EntityTableItemMongodb_VECTOR3::getReadSqlItem(mongodb::DBContext& context)
	{
		ArraySize asize = 3;
		for (ArraySize i = 0; i<asize; ++i)
		{
			mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();
			pSotvs->sqlkey = db_item_names_[i];
			memset(pSotvs->sqlval, 0, MAX_BUF);
			context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));
		}
	}

	void EntityTableItemMongodb_VECTOR3::addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID, const bson_t * doc)
	{
		ArraySize asize = 3;
		(*s) << asize;

		for (ArraySize i = 0; i < asize; ++i)
		{
			bson_iter_t iter;
			if (!bson_iter_init_find(&iter, doc, db_item_names_[i]))
				continue;

#ifdef CLIENT_NO_FLOAT
			int32 v = bson_iter_int32(&iter);
#else
			double vv = bson_iter_double(&iter);
			float v = static_cast<float>(vv);
#endif
			(*s) << v;
		}
	}

	//-------------------------------------------------------------------------------------
	void EntityTableItemMongodb_VECTOR4::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context, bson_t * doc)
	{
		if (s == NULL)
			return;

#ifdef CLIENT_NO_FLOAT
		int32 v;
#else
		float v;
#endif

		ArraySize asize;

		(*s) >> asize;
		KBE_ASSERT(asize == 4);

		for (ArraySize i = 0; i<asize; ++i)
		{
			(*s) >> v;
			mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();
			pSotvs->sqlkey = db_item_names_[i];

#ifdef CLIENT_NO_FLOAT
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);
#else
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%f", v);
#endif

			context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));

			//bson_t
			BSON_APPEND_DOUBLE(doc, pSotvs->sqlkey, v);
		}
	}

	void EntityTableItemMongodb_VECTOR4::getReadSqlItem(mongodb::DBContext& context)
	{
		ArraySize asize = 4;
		for (ArraySize i = 0; i<asize; ++i)
		{
			mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();
			pSotvs->sqlkey = db_item_names_[i];
			memset(pSotvs->sqlval, 0, MAX_BUF);
			context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));
		}
	}

	void EntityTableItemMongodb_VECTOR4::addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID, const bson_t * doc)
	{
		ArraySize asize = 4;
		(*s) << asize;

		for (ArraySize i = 0; i < asize; ++i)
		{
			bson_iter_t iter;
			if (!bson_iter_init_find(&iter, doc, db_item_names_[i]))
				continue;

#ifdef CLIENT_NO_FLOAT
			int32 v = bson_iter_int32(&iter);
#else
			double vv = bson_iter_double(&iter);
			float v = static_cast<float>(vv);
#endif			
			(*s) << v;
		}
	}

	void EntityTableItemMongodb_MAILBOX::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context, bson_t * doc)
	{
	}

	void EntityTableItemMongodb_MAILBOX::getReadSqlItem(mongodb::DBContext& context)
	{
	}

	void EntityTableItemMongodb_MAILBOX::addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID, const bson_t * doc)
	{
	}

	//-------------------------------------------------------------------------------------
	bool EntityTableItemMongodb_ARRAY::initialize(const PropertyDescription* pPropertyDescription,
		const DataType* pDataType, std::string name)
	{
		bool ret = EntityTableItemMongodbBase::initialize(pPropertyDescription, pDataType, name);
		if (!ret)
			return false;

		// 创建子表
		EntityTableMongodb* pTable = new EntityTableMongodb(this->pParentTable()->pEntityTables());

		std::string tname = this->pParentTable()->tableName();
		std::vector<std::string> qname;
		EntityTableItem* pparentItem = this->pParentTableItem();
		while (pparentItem != NULL)
		{
			if (strlen(pparentItem->itemName()) > 0)
				qname.push_back(pparentItem->itemName());
			pparentItem = pparentItem->pParentTableItem();
		}

		if (qname.size() > 0)
		{
			for (int i = (int)qname.size() - 1; i >= 0; i--)
			{
				tname += "_";
				tname += qname[i];
			}
		}

		std::string tableName = tname + "_";
		std::string itemName = "";

		if (name.size() > 0)
		{
			tableName += name;
		}
		else
		{
			tableName += TABLE_ARRAY_ITEM_VALUES_CONST_STR;
		}

		if (itemName.size() == 0)
		{
			if (static_cast<FixedArrayType*>(const_cast<DataType*>(pDataType))->getDataType()->type() != DATA_TYPE_FIXEDDICT)
				itemName = TABLE_ARRAY_ITEM_VALUE_CONST_STR;
		}

		pTable->tableName(tableName);
		pTable->isChild(true);

		EntityTableItem* pArrayTableItem;
		pArrayTableItem = pParentTable_->createItem(static_cast<FixedArrayType*>(const_cast<DataType*>(pDataType))->getDataType()->getName(), pPropertyDescription->getDefaultValStr());
		pArrayTableItem->utype(-pPropertyDescription->getUType());
		pArrayTableItem->pParentTable(this->pParentTable());
		pArrayTableItem->pParentTableItem(this);
		pArrayTableItem->tableName(pTable->tableName());

		ret = pArrayTableItem->initialize(pPropertyDescription,
			static_cast<FixedArrayType*>(const_cast<DataType*>(pDataType))->getDataType(), itemName.c_str());

		if (!ret)
		{
			delete pTable;
			return ret;
		}

		pTable->addItem(pArrayTableItem);
		pChildTable_ = pTable;

		pTable->pEntityTables()->addTable(pTable);
		return true;
	}

	void EntityTableItemMongodb_ARRAY::init_db_item_name(const char* exstrFlag)
	{
		if (pChildTable_)
		{
			static_cast<EntityTableMongodb*>(pChildTable_)->init_db_item_name();
		}
	}

	void EntityTableItemMongodb_ARRAY::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context, bson_t * doc)
	{
		ArraySize size = 0;
		if (s)
			(*s) >> size;

		if (pChildTable_)
		{

			bson_t child;

			bson_append_array_begin(doc, pChildTable_->tableName(), -1, &child);

			if (size > 0)
			{
				for (ArraySize i = 0; i < size; ++i)
				{
					bson_t item;
					bson_init(&item);
					static_cast<EntityTableMongodb*>(pChildTable_)->getWriteSqlItem(pdbi, s, context, &item);
					char str[4];
					sprintf(str, "%d", i);
					bson_append_document(&child, str, -1, &item);
				}
			}
			else
			{
				//bson_t item;
				//bson_init(&item);
				//static_cast<EntityTableMongodb*>(pChildTable_)->getWriteSqlItem(pdbi, NULL, context, &item);
				//bson_append_document(&child, "0", -1, &item);
			}

			bson_append_array_end(doc, &child);
		}
	}

	void EntityTableItemMongodb_ARRAY::getReadSqlItem(mongodb::DBContext& context)
	{
		if (pChildTable_)
		{
			static_cast<EntityTableMongodb*>(pChildTable_)->getReadSqlItem(context);
		}
	}

	void EntityTableItemMongodb_ARRAY::addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID, const bson_t * doc)
	{
		if (pChildTable_)
		{
			bson_iter_t biter;
			bson_iter_t array_iter;
			if (!bson_iter_init_find(&biter, doc, pChildTable_->tableName()))
				return;
			bson_iter_recurse(&biter, &array_iter);
			
			mongodb::DBContext::DB_RW_CONTEXTS::iterator iter = context.optable.begin();
			for (; iter != context.optable.end(); ++iter)
			{
				if (pChildTable_->tableName() == iter->first)
				{
					ArraySize size = 0;
					std::list<bson_t> bsonlist;

					//先取数据
					while (true)
					{
						char str[4];
						sprintf(str, "%d", size);
						//查找第i组数据
						if (!bson_iter_find(&array_iter, str))
							break;
						
						//获取这个数据的bson_t
						const uint8_t *buf;
						uint32_t len;
						bson_iter_document(&array_iter, &len, &buf);

						bson_t rec;
						bson_init_static(&rec, buf, len);
						bsonlist.push_back(rec);

						size++;
					}

					//ArraySize size = (ArraySize)iter->second->dbids[resultDBID].size();
					(*s) << size;

					while (!bsonlist.empty())
					{
						static_cast<EntityTableMongodb*>(pChildTable_)->addToStream(s, *iter->second.get(), 0, &bsonlist.front());

						bson_destroy(&bsonlist.front());
						bsonlist.pop_front();
					}

					return;
				}
			}
		}

		ArraySize size = 0;
		(*s) << size;
	}

	//-------------------------------------------------------------------------------------
	void EntityTableItemMongodb_FIXED_DICT::init_db_item_name(const char* exstrFlag)
	{
		FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();

		for (; fditer != keyTypes_.end(); ++fditer)
		{
			std::string new_exstrFlag = exstrFlag;
			if (fditer->second->type() == TABLE_ITEM_TYPE_FIXEDDICT)
				new_exstrFlag += fditer->first + "_";

			static_cast<EntityTableItemMongodbBase*>(fditer->second.get())->init_db_item_name(new_exstrFlag.c_str());
		}
	}

	bool EntityTableItemMongodb_FIXED_DICT::initialize(const PropertyDescription* pPropertyDescription,
		const DataType* pDataType, std::string name)
	{
		bool ret = EntityTableItemMongodbBase::initialize(pPropertyDescription, pDataType, name);
		if (!ret)
			return false;

		KBEngine::FixedDictType* fdatatype = static_cast<KBEngine::FixedDictType*>(const_cast<DataType*>(pDataType));

		FixedDictType::FIXEDDICT_KEYTYPE_MAP& keyTypes = fdatatype->getKeyTypes();
		FixedDictType::FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes.begin();

		for (; iter != keyTypes.end(); ++iter)
		{
			if (!iter->second->persistent)
				continue;

			EntityTableItem* tableItem = pParentTable_->createItem(iter->second->dataType->getName(), pPropertyDescription->getDefaultValStr());

			tableItem->pParentTable(this->pParentTable());
			tableItem->pParentTableItem(this);
			tableItem->utype(-pPropertyDescription->getUType());
			tableItem->tableName(this->tableName());
			if (!tableItem->initialize(pPropertyDescription, iter->second->dataType, iter->first))
				return false;

			std::pair< std::string, KBEShared_ptr<EntityTableItem> > itemVal;
			itemVal.first = iter->first;
			itemVal.second.reset(tableItem);

			keyTypes_.push_back(itemVal);
		}

		return true;
	}
	
	void EntityTableItemMongodb_FIXED_DICT::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context, bson_t * doc)
	{
		FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();


		for (; fditer != keyTypes_.end(); ++fditer)
		{
			static_cast<EntityTableItemMongodbBase*>(fditer->second.get())->getWriteSqlItem(pdbi, s, context, doc);
		}

	}

	void EntityTableItemMongodb_FIXED_DICT::getReadSqlItem(mongodb::DBContext& context)
	{
		FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();

		for (; fditer != keyTypes_.end(); ++fditer)
		{
			static_cast<EntityTableItemMongodbBase*>(fditer->second.get())->getReadSqlItem(context);
		}
	}

	void EntityTableItemMongodb_FIXED_DICT::addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID, const bson_t * doc)
	{
		FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();

		for (; fditer != keyTypes_.end(); ++fditer)
		{
			static_cast<EntityTableItemMongodbBase*>(fditer->second.get())->addToStream(s, context, resultDBID, doc);
		}
	}

	//-------------------------------------------------------------------------------------
	void EntityTableItemMongodb_DIGIT::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context, bson_t * doc)
	{
		if (s == NULL)
			return;

		mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();


		if (dataSType_ == "INT8")
		{
			int8 v;
			(*s) >> v;
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);

			BSON_APPEND_INT32(doc, db_item_name(), v);
		}
		else if (dataSType_ == "INT16")
		{
			int16 v;
			(*s) >> v;
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);

			BSON_APPEND_INT32(doc, db_item_name(), v);
		}
		else if (dataSType_ == "INT32")
		{
			int32 v;
			(*s) >> v;
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);

			BSON_APPEND_INT32(doc, db_item_name(), v);
		}
		else if (dataSType_ == "INT64")
		{
			int64 v;
			(*s) >> v;
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%" PRI64, v);

			BSON_APPEND_INT64(doc, db_item_name(), v);
		}
		else if (dataSType_ == "UINT8")
		{
			uint8 v;
			(*s) >> v;
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%u", v);

			BSON_APPEND_INT32(doc, db_item_name(), v);
		}
		else if (dataSType_ == "UINT16")
		{
			uint16 v;
			(*s) >> v;
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%u", v);

			BSON_APPEND_INT32(doc, db_item_name(), v);
		}
		else if (dataSType_ == "UINT32")
		{
			uint32 v;
			(*s) >> v;
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%u", v);

			BSON_APPEND_INT32(doc, db_item_name(), v);
		}
		else if (dataSType_ == "UINT64")
		{
			uint64 v;
			(*s) >> v;
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%" PRIu64, v);

			BSON_APPEND_INT64(doc, db_item_name(), v);
		}
		else if (dataSType_ == "FLOAT")
		{
			float v;
			(*s) >> v;
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%f", v);

			BSON_APPEND_DOUBLE(doc, db_item_name(), v);
		}
		else if (dataSType_ == "DOUBLE")
		{
			double v;
			(*s) >> v;
			kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%lf", v);

			BSON_APPEND_DOUBLE(doc, db_item_name(), v);
		}

		pSotvs->sqlkey = db_item_name();
		context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));
	}

	void EntityTableItemMongodb_DIGIT::getReadSqlItem(mongodb::DBContext& context)
	{
		mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();
		pSotvs->sqlkey = db_item_name();
		memset(pSotvs->sqlval, 0, MAX_BUF);
		context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));
	}

	void EntityTableItemMongodb_DIGIT::addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID, const bson_t * doc)
	{

		bson_iter_t iter;
		if (!bson_iter_init_find(&iter, doc, db_item_name()))
			return;	

		if (dataSType_ == "INT8")
		{
			int32 v = bson_iter_int32(&iter);
			int8 vv = static_cast<int8>(v);
			(*s) << vv;
		}
		else if (dataSType_ == "INT16")
		{
			int32 v = bson_iter_int32(&iter);
			int16 vv = static_cast<int16>(v);
			(*s) << vv;
		}
		else if (dataSType_ == "INT32")
		{
			int32 v = bson_iter_int32(&iter);
			(*s) << v;
		}
		else if (dataSType_ == "INT64")
		{
			int64 v = bson_iter_int64(&iter);
			(*s) << v;
		}
		else if (dataSType_ == "UINT8")
		{
			int32 v = bson_iter_int32(&iter);
			uint8 vv = static_cast<uint8>(v);
			(*s) << vv;
		}
		else if (dataSType_ == "UINT16")
		{
			int32 v = bson_iter_int32(&iter);
			uint16 vv = static_cast<uint16>(v);
			(*s) << vv;
		}
		else if (dataSType_ == "UINT32")
		{
			uint32 v = bson_iter_int32(&iter);
			(*s) << v;
		}
		else if (dataSType_ == "UINT64")
		{
			uint64 v = bson_iter_int64(&iter);
			(*s) << v;
		}
		else if (dataSType_ == "FLOAT")
		{
			double v = bson_iter_double(&iter);
			float vv = static_cast<float>(v);
			(*s) << vv;
		}
		else if (dataSType_ == "DOUBLE")
		{
			double v = bson_iter_double(&iter);
			(*s) << v;
		}
	}


	//-------------------------------------------------------------------------------------
	void EntityTableItemMongodb_STRING::getWriteSqlItem(DBInterface* pdbi,
		MemoryStream* s, mongodb::DBContext& context, bson_t * doc)
	{
		if (s == NULL)
			return;

		mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();

		std::string val;
		(*s) >> val;

		pSotvs->extraDatas = "\"";

		char* tbuf = new char[val.size() * 2 + 1];
		memset(tbuf, 0, val.size() * 2 + 1);

		//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(),
		//	tbuf, val.c_str(), val.size());

		pSotvs->extraDatas += tbuf;
		pSotvs->extraDatas += "\"";

		SAFE_RELEASE_ARRAY(tbuf);

		memset(pSotvs, 0, sizeof(pSotvs->sqlval));
		pSotvs->sqlkey = db_item_name();
		context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));

		BSON_APPEND_UTF8(doc, pSotvs->sqlkey, val.c_str(), val.length());
	}

	void EntityTableItemMongodb_STRING::getReadSqlItem(mongodb::DBContext& context)
	{
		mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();
		pSotvs->sqlkey = db_item_name();
		memset(pSotvs->sqlval, 0, MAX_BUF);
		context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));
	}

	void EntityTableItemMongodb_STRING::addToStream(MemoryStream* s,
		mongodb::DBContext& context, DBID resultDBID, const bson_t * doc)
	{
		bson_iter_t iter;
		if (bson_iter_init_find(&iter, doc, db_item_name()))
		{
			uint32_t len = 0;
			const char * value = bson_iter_utf8(&iter, &len);
			std::string datas(value, len);
			(*s) << datas;
		}
	}

	//-------------------------------------------------------------------------------------
	void EntityTableItemMongodb_UNICODE::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s,
		mongodb::DBContext& context, bson_t * doc)
	{
		if (s == NULL)
			return;

		mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();

		std::string val;
		s->readBlob(val);

		char* tbuf = new char[val.size() * 2 + 1];
		memset(tbuf, 0, val.size() * 2 + 1);

		//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(),
		//	tbuf, val.c_str(), val.size());

		pSotvs->extraDatas = "\"";
		pSotvs->extraDatas += tbuf;
		pSotvs->extraDatas += "\"";
		SAFE_RELEASE_ARRAY(tbuf);

		memset(pSotvs, 0, sizeof(pSotvs->sqlval));
		pSotvs->sqlkey = db_item_name();
		context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));

		BSON_APPEND_UTF8(doc, pSotvs->sqlkey, val.c_str(), val.length());
	}

	void EntityTableItemMongodb_UNICODE::getReadSqlItem(mongodb::DBContext& context)
	{
		mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();
		pSotvs->sqlkey = db_item_name();
		memset(pSotvs->sqlval, 0, MAX_BUF);
		context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));
	}

	void EntityTableItemMongodb_UNICODE::addToStream(MemoryStream* s,
		mongodb::DBContext& context, DBID resultDBID, const bson_t * doc)
	{
		bson_iter_t iter;
		if (bson_iter_init_find(&iter, doc, db_item_name()))
		{
			uint32_t len = 0;
			const char * value = bson_iter_utf8(&iter, &len);
			std::string datas(value, len);
			(*s).appendBlob(datas);
		}
	}

	//-------------------------------------------------------------------------------------
	void EntityTableItemMongodb_BLOB::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context, bson_t * doc)
	{
		if (s == NULL)
			return;

		mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();

		std::string val;
		s->readBlob(val);

		char* tbuf = new char[val.size() * 2 + 1];
		memset(tbuf, 0, val.size() * 2 + 1);

		//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(),
		//	tbuf, val.data(), val.size());

		pSotvs->extraDatas = "\"";
		pSotvs->extraDatas += tbuf;
		pSotvs->extraDatas += "\"";
		SAFE_RELEASE_ARRAY(tbuf);

		memset(pSotvs, 0, sizeof(pSotvs->sqlval));
		pSotvs->sqlkey = db_item_name();
		context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));

		BSON_APPEND_UTF8(doc, pSotvs->sqlkey, val.c_str(), val.length());
	}

	void EntityTableItemMongodb_BLOB::getReadSqlItem(mongodb::DBContext& context)
	{
		mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();
		pSotvs->sqlkey = db_item_name();
		memset(pSotvs->sqlval, 0, MAX_BUF);
		context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));
	}

	void EntityTableItemMongodb_BLOB::addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID, const bson_t * doc)
	{
		bson_iter_t iter;
		if (bson_iter_init_find(&iter, doc, db_item_name()))
		{
			uint32_t len = 0;
			const char * value = bson_iter_utf8(&iter, &len);
			std::string datas(value, len);
			(*s).appendBlob(datas.data(), datas.size());
		}

	}

	//-------------------------------------------------------------------------------------
	void EntityTableItemMongodb_PYTHON::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context, bson_t * doc)
	{
		if (s == NULL)
			return;

		mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();

		std::string val;
		s->readBlob(val);

		char* tbuf = new char[val.size() * 2 + 1];
		memset(tbuf, 0, val.size() * 2 + 1);

		//mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(),
		//	tbuf, val.c_str(), val.size());

		pSotvs->extraDatas = "\"";
		pSotvs->extraDatas += tbuf;
		pSotvs->extraDatas += "\"";
		SAFE_RELEASE_ARRAY(tbuf);

		memset(pSotvs, 0, sizeof(pSotvs->sqlval));
		pSotvs->sqlkey = db_item_name();
		context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));

		BSON_APPEND_BINARY(doc, pSotvs->sqlkey, BSON_SUBTYPE_BINARY, (const uint8_t *)val.c_str(), val.length());
	}

	void EntityTableItemMongodb_PYTHON::getReadSqlItem(mongodb::DBContext& context)
	{
		mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();
		pSotvs->sqlkey = db_item_name();
		memset(pSotvs->sqlval, 0, MAX_BUF);
		context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));
	}

	void EntityTableItemMongodb_PYTHON::addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID, const bson_t * doc)
	{
		bson_iter_t iter;
		if (bson_iter_init_find(&iter, doc ,db_item_name()))
		{
			bson_subtype_t btype;
			uint32_t len = 0;
			const char * value;
			bson_iter_binary(&iter, &btype, &len, (const uint8_t**)&value);
			std::string datas(value, len);
			(*s).appendBlob(datas);
		}
	}
}