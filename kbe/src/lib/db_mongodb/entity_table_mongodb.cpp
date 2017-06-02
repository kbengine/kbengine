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
		// ��ȡ����
		tableName(name);

		// �ҵ����д洢���Բ��Ҵ��������е��ֶ�
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

		// ���⴦�� ���ݿⱣ�淽���λ��
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
			// ����fixedDict�ֶ����Ƶ��������
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

	bool EntityTableMongodb::syncIndexToDB(DBInterface* pdbi)
	{
		//��ȷ����Ҫ�������ֶ�
		std::vector<EntityTableItem*> indexs;

		EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
		for (; iter != tableItems_.end(); ++iter)
		{
			if (strlen(iter->second->indexType()) == 0)
				continue;

			indexs.push_back(iter->second.get());
		}

		//��ȡĿǰ������
		char name[MAX_BUF];
		kbe_snprintf(name, MAX_BUF, ENTITY_TABLE_PERFIX "_%s", tableName());

		KBEUnordered_map<std::string, std::string> currDBKeys;
		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		mongoc_cursor_t * cursor = pdbiMongodb->collectionFindIndexes(name);

		const bson_t *indexinfo;
		bson_iter_t idx_spec_iter;
		while (mongoc_cursor_next(cursor, &indexinfo)) 
		{
			if (bson_iter_init_find(&idx_spec_iter, indexinfo, "name") &&
				BSON_ITER_HOLDS_UTF8(&idx_spec_iter))
			{
				std::string keyname = bson_iter_utf8(&idx_spec_iter, NULL);
				if (bson_iter_init_find(&idx_spec_iter, indexinfo, "unique")) //�Ƿ����Ψһ�Ա�ʶ
				{

					currDBKeys[keyname] = "UNIQUE";
				}
				else
				{
					currDBKeys[keyname] = "INDEX";
				}
			}
		}

		mongoc_cursor_destroy(cursor);

		//��ʼɾ������������������µ�����
		bson_t keys;
		bson_init(&keys);
		mongoc_index_opt_t opt;

		std::vector<EntityTableItem*>::iterator iiter = indexs.begin();
		for (; iiter != indexs.end();)
		{
			std::string itemName = fmt::format(TABLE_ITEM_PERFIX"_{}_1", (*iiter)->itemName()); //Ĭ�ϲ�����������
			std::string itemIndexsName = fmt::format(TABLE_ITEM_PERFIX"_{}", (*iiter)->itemName());
			KBEUnordered_map<std::string, std::string>::iterator fiter = currDBKeys.find(itemName);
			if (fiter != currDBKeys.end())
			{
				bool deleteIndex = fiter->second != (*iiter)->indexType();

				// ɾ���Ѿ�����ģ�ʣ�µľ���Ҫ�����ݿ�ɾ����index
				currDBKeys.erase(fiter);

				if (deleteIndex)
				{
					//��ɾ�������������ڴ�����Ҫ������
					pdbiMongodb->collectionDropIndex(name, itemName.c_str());
				}
				else
				{
					++iiter;
					continue;
				}
			}

			if ( std::string("UNIQUE") == (*iiter)->indexType())
			{
				//��Ҫ����Ψһ����
				bson_init(&keys);
				mongoc_index_opt_init(&opt);
				opt.unique = true;
				bson_append_int32(&keys, itemIndexsName.c_str(), -1, 1);
				pdbiMongodb->collectionCreateIndex(name, &keys, &opt);
			}
			else
			{
				bson_init(&keys);
				mongoc_index_opt_init(&opt);
				bson_append_int32(&keys, itemIndexsName.c_str(), -1, 1);
				pdbiMongodb->collectionCreateIndex(name, &keys, &opt);
			}

			++iiter;
		}
		bson_destroy(&keys);

		// ʣ�µľ���Ҫ�����ݿ�ɾ����index
		KBEUnordered_map<std::string, std::string>::iterator dbkey_iter = currDBKeys.begin();
		for (; dbkey_iter != currDBKeys.end(); ++dbkey_iter)
		{
			if (dbkey_iter->first == "_id_" || dbkey_iter->first == "id_1")
				continue;

			pdbiMongodb->collectionDropIndex(name, dbkey_iter->first.c_str());
		}

		return true;
	}

	bool EntityTableMongodb::syncToDB(DBInterface* pdbi)
	{
		//ERROR_MSG(fmt::format("EntityTableMysql::syncToDB(): {}.\n", tableName()));
		if (hasSync())
			return true;
		
		sync_ = true;

		// DEBUG_MSG(fmt::format("EntityTableMysql::syncToDB(): {}.\n", tableName()));

		char name[MAX_BUF];
		kbe_snprintf(name, MAX_BUF, ENTITY_TABLE_PERFIX "_%s", tableName());

		if (!isChild_)
		{
			//������дִ�������ʼ�����ݿ�ı�ṹ
			DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
			if (pdbiMongodb->createCollection(name))
			{
				//��һ�δ�������Ҫ������ID������
				//��Ҫ����Ψһ����
				bson_t keys;
				mongoc_index_opt_t opt;
				bson_init(&keys);
				mongoc_index_opt_init(&opt);
				opt.unique = true;
				bson_append_int32(&keys, "id", -1, 1);
				pdbiMongodb->collectionCreateIndex(name, &keys, &opt);
				bson_destroy(&keys);
			}

			//��Ϊmongodbȱ�ٶ�����������޸ģ������޷���mysql������def�ֶε����Ӻ�ɾ�ġ�Ϊ�˽��������⣬Ҫ����mongodb���ļ������ݿ�ĵ��ص�
			//�������ݱ�ṹ�����и��£�ͨ�������Դﵽ�����ȷʹ�á�
			//bson_t query;
			//bson_init(&query);
			//mongoc_cursor_t * cursor = pdbiMongodb->collectionFind(name, MONGOC_QUERY_NONE, 0, 0, 0, &query, NULL, NULL);

			//const bson_t *doc = NULL;
			//if (mongoc_cursor_more(cursor))
			//{
			//	mongoc_cursor_next(cursor, &doc);
			//}

			//if (doc == NULL)
			//{
			//	mongoc_cursor_destroy(cursor);
			//	bson_destroy(&query);
			//	return true;
			//}

			//����������ݣ�����ͨ���������ݣ��Ϳ���֪��Ŀǰ�е�ֵ,ɾ���������
			//bson_iter_t iter;
			//bson_iter_init(&iter, doc);
			//
			//while (bson_iter_next(&iter))
			//{
			//	const char *ikey = bson_iter_key(&iter);
			//	std::string tname(ikey);

			//	if (tname == "_id" || tname == TABLE_ID_CONST_STR ||
			//		tname == TABLE_ITEM_PERFIX"_" TABLE_AUTOLOAD_CONST_STR ||
			//		tname == TABLE_PARENTID_CONST_STR)
			//	{
			//		continue;
			//	}

			//	EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
			//	bool found = false;

			//	for (; iter != tableItems_.end(); ++iter)
			//	{
			//		if (iter->second->isSameKey(tname))
			//		{
			//			found = true;
			//			break;
			//		}
			//	}

			//	
			//	if (!found)
			//	{
			//		//���µ�def��û�з��֣�����Ҫִ��ɾ������
			//		bson_t query;
			//		bson_init(&query);

			//		bson_error_t  error;					
			//		char command[MAX_BUF];
			//		kbe_snprintf(command, MAX_BUF, "{\"$set\":{\"%s\":10}}", tname.c_str());
			//		std::string t_command(command);
			//		bson_t * bsons = bson_new_from_json((const uint8_t *)t_command.c_str(), t_command.length(), &error);
			//		

			//		if (bsons == NULL)
			//		{
			//			ERROR_MSG(fmt::format("{}\n", error.message));
			//		}

			//		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
			//		pdbiMongodb->updateCollection(name, MONGOC_UPDATE_MULTI_UPDATE, &query, bsons, NULL);
			//	}
			//}

			//mongoc_cursor_destroy(cursor);
			//bson_destroy(&query);
			
			//ֻ�Ա�ĵ�һ������������ڶ���mongodb�޷�����������Ҳû�б�Ҫ����
			if (!syncIndexToDB(pdbi))
				return false;
		}

		return true;
	}

	EntityTableItem* EntityTableMongodb::createItem(std::string type, std::string defaultVal)
	{
		if (type == "INT8")
		{
			int8 v = 0;
			
			try
			{				
				StringConv::str2value(v, defaultVal.c_str());
			}
			catch (...)
			{
				v = 0;
			}

			return new EntityTableItemMongodb_DIGIT<int8>(type, v, 1, 0);
		}
		else if (type == "INT16")
		{
			int16 v = 0;
			try
			{
				StringConv::str2value(v, defaultVal.c_str());
			}
			catch (...)
			{
				v = 0;
			}

			return new EntityTableItemMongodb_DIGIT<int16>(type, v, 2, 0);
		}
		else if (type == "INT32")
		{
			int32 v = 0;
			try
			{
				StringConv::str2value(v, defaultVal.c_str());
			}
			catch (...)
			{
				v = 0;
			}

			return new EntityTableItemMongodb_DIGIT<int32>(type, v, 4, 0);
		}
		else if (type == "INT64")
		{
			int64 v = 0;
			try
			{
				StringConv::str2value(v, defaultVal.c_str());
			}
			catch (...)
			{
				v = 0;
			}
			return new EntityTableItemMongodb_DIGIT<int64>(type, v, 8, 0);
		}
		else if (type == "UINT8")
		{
			uint8 v = 0;
			try
			{
				StringConv::str2value(v, defaultVal.c_str());
			}
			catch (...)
			{
				v = 0;
			}

			return new EntityTableItemMongodb_DIGIT<uint8>(type, v, 1, 0);
		}
		else if (type == "UINT16")
		{
			uint16 v = 0;
			try
			{
				StringConv::str2value(v, defaultVal.c_str());
			}
			catch (...)
			{
				v = 0;
			}

			return new EntityTableItemMongodb_DIGIT<uint16>(type, v, 2, 0);
		}
		else if (type == "UINT32")
		{
			uint32 v = 0;
			try
			{
				StringConv::str2value(v, defaultVal.c_str());
			}
			catch (...)
			{
				v = 0;
			}
			return new EntityTableItemMongodb_DIGIT<uint32>(type, v, 4, 0);
		}
		else if (type == "UINT64")
		{
			uint64 v = 0;
			try
			{
				StringConv::str2value(v, defaultVal.c_str());
			}
			catch (...)
			{
				v = 0;
			}

			return new EntityTableItemMongodb_DIGIT<uint64>(type, v, 8, 0);
		}
		else if (type == "FLOAT")
		{
			float v = 0;
			try
			{
				StringConv::str2value(v, defaultVal.c_str());
			}
			catch (...)
			{
				v = 0;
			}

			return new EntityTableItemMongodb_DIGIT<float>(type, v, 0, 0);
		}
		else if (type == "DOUBLE")
		{
			double v = 0;
			try
			{
				StringConv::str2value(v, defaultVal.c_str());
			}
			catch (...)
			{
				v = 0;
			}

			return new EntityTableItemMongodb_DIGIT<double>(type, v, 0, 0);
		}
		else if (type == "STRING")
		{
			return new EntityTableItemMongodb_STRING(defaultVal, 0, 0);
		}
		else if (type == "UNICODE")
		{
			return new EntityTableItemMongodb_UNICODE(defaultVal, 0, 0);
		}
		else if (type == "PYTHON")
		{
			return new EntityTableItemMongodb_PYTHON(defaultVal, 0, 0);
		}
		else if (type == "PY_DICT")
		{
			return new EntityTableItemMongodb_PYTHON(defaultVal, 0, 0);
		}
		else if (type == "PY_TUPLE")
		{
			return new EntityTableItemMongodb_PYTHON(defaultVal, 0, 0);
		}
		else if (type == "PY_LIST")
		{
			return new EntityTableItemMongodb_PYTHON(defaultVal, 0, 0);
		}
		else if (type == "BLOB")
		{
			return new EntityTableItemMongodb_BLOB(defaultVal, 0, 0);
		}
		else if (type == "ARRAY")
		{
			return new EntityTableItemMongodb_ARRAY("", 0, 0);
		}
		else if (type == "FIXED_DICT")
		{
			return new EntityTableItemMongodb_FIXED_DICT("", 0, 0);
		}
		else if (type == "VECTOR2")
		{
			return new EntityTableItemMongodb_VECTOR2(0, 0, 0);
		}
		else if (type == "VECTOR3")
		{
			return new EntityTableItemMongodb_VECTOR3(0, 0, 0);
		}
		else if (type == "VECTOR4")
		{
			return new EntityTableItemMongodb_VECTOR4(0, 0, 0);
		}
		else if (type == "MAILBOX")
		{
			return new EntityTableItemMongodb_MAILBOX("", 0, 0);
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

		if (context.dbid > 0) //����
		{
			bson_t query;
			bson_init(&query);
			BSON_APPEND_INT64(&query, "id", context.dbid);

			DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
			char name[MAX_BUF];
			kbe_snprintf(name, MAX_BUF, ENTITY_TABLE_PERFIX "_%s", context.tableName.c_str());

			BSON_APPEND_INT64(&doc, "id", context.dbid);
			BSON_APPEND_INT32(&doc, TABLE_ITEM_PERFIX"_" TABLE_AUTOLOAD_CONST_STR, shouldAutoLoad);
			pdbiMongodb->updateCollection(name, MONGOC_UPDATE_NONE, &query, &doc, NULL);

			bson_destroy(&query);

		}
		else //����
		{
			DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
			char name[MAX_BUF];
			kbe_snprintf(name, MAX_BUF, ENTITY_TABLE_PERFIX "_%s", context.tableName.c_str());

			context.dbid = genUUID64();
			BSON_APPEND_INT64(&doc, TABLE_ID_CONST_STR, context.dbid);
			BSON_APPEND_INT32(&doc, TABLE_ITEM_PERFIX"_" TABLE_AUTOLOAD_CONST_STR, shouldAutoLoad);
			pdbiMongodb->insertCollection(name, MONGOC_INSERT_NONE, &doc, NULL);
		}

		bson_destroy(&doc);

		dbid = context.dbid;

		// ����ʵ���Ƿ��Զ�����
		if (shouldAutoLoad > -1)
			entityShouldAutoLoad(pdbi, dbid, shouldAutoLoad > 0);

		//������Ҫ�޸�
		return dbid; 
	}

	/**
	��ȡ���е����ݷŵ�����
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

		//�Ȱ����ݲ�ѯ����
		bson_t query;
		bson_init(&query);
		BSON_APPEND_INT64(&query, "id", dbid);

		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		char name[MAX_BUF];
		kbe_snprintf(name, MAX_BUF, ENTITY_TABLE_PERFIX "_%s", context.tableName.c_str());

		mongoc_cursor_t * cursor = pdbiMongodb->collectionFind(name, MONGOC_QUERY_NONE, 0, 0, 0, &query, NULL, NULL);
				
		const bson_t *doc = NULL;
		bson_error_t  error;
		while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &doc)) {
			break;
		}

		if (mongoc_cursor_error(cursor, &error)) {
			ERROR_MSG(fmt::format("An error occurred: {}\n", error.message));
		}

		if (doc == NULL)
		{
			mongoc_cursor_destroy(cursor);
			return false;
		}
		
		
		iter = tableFixedOrderItems_.begin();
		for (; iter != tableFixedOrderItems_.end(); ++iter)
		{
			static_cast<EntityTableItemMongodbBase*>((*iter))->addToStream(s, context, dbid, doc);
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
			{
				//���û���ҵ����ݣ���Ҫ�������Դ���
#ifdef CLIENT_NO_FLOAT
				(*s) << (int32)0;
#else
				(*s) << (float)0;
#endif
				continue;
			}
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
	bool EntityTableItemMongodb_VECTOR3::isSameKey(std::string key)
	{
		for (int i = 0; i<3; ++i)
		{
			if (key == db_item_names_[i])
				return true;
		}

		return false;
	}

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
			{
				//���û���ҵ����ݣ���Ҫ�������Դ���
#ifdef CLIENT_NO_FLOAT
				(*s) << (int32)0;
#else
				(*s) << (float)0;
#endif
				continue;
			}

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
	bool EntityTableItemMongodb_VECTOR4::isSameKey(std::string key)
	{
		for (int i = 0; i<4; ++i)
		{
			if (key == db_item_names_[i])
				return true;
		}

		return false;
	}

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
			{
				//���û���ҵ����ݣ���Ҫ�������Դ���
#ifdef CLIENT_NO_FLOAT
				(*s) << (int32)0;
#else
				(*s) << (float)0;
#endif
				continue;
			}

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

		// �����ӱ�
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

	bool EntityTableItemMongodb_ARRAY::isSameKey(std::string key)
	{
		//�����mysql������һ������Ϊmongodb��array���ǵ�������һ�ű�
		return pChildTable_->tableName() == key;
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
			if (!bson_iter_init_find(&biter, doc, pChildTable_->tableName()) || !BSON_ITER_HOLDS_ARRAY(&biter))
			{
				//���û���ҵ����ݣ���Ҫ�������Դ���
				(*s) << (ArraySize)0;
				return;
			}

			bson_iter_recurse(&biter, &array_iter);
			
			mongodb::DBContext::DB_RW_CONTEXTS::iterator iter = context.optable.begin();
			for (; iter != context.optable.end(); ++iter)
			{
				if (pChildTable_->tableName() == iter->first)
				{
					ArraySize size = 0;
					std::list<bson_t *> bsonlist;

					//��ȡ����
					while (true)
					{
						char str[4];
						sprintf(str, "%d", size);
						//���ҵ�i������
						if (!bson_iter_find(&array_iter, str))
							break;
						
						//��ȡ������ݵ�bson_t
						const uint8_t *buf;
						uint32_t len;
						bson_iter_document(&array_iter, &len, &buf);

						bson_t * rec = new bson_t();
						bson_init_static(rec, buf, len);
						bsonlist.push_back(rec);

						size++;
					}

					//ArraySize size = (ArraySize)iter->second->dbids[resultDBID].size();
					(*s) << size;

					while (!bsonlist.empty())
					{
						static_cast<EntityTableMongodb*>(pChildTable_)->addToStream(s, *iter->second.get(), 0, bsonlist.front());

						bson_destroy(bsonlist.front());
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
	
	bool EntityTableItemMongodb_FIXED_DICT::isSameKey(std::string key)
	{
		FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();
		bool tmpfound = false;

		for (; fditer != keyTypes_.end(); ++fditer)
		{
			if (fditer->second->isSameKey(key))
			{
				tmpfound = true;
				break;
			}
		}

		return tmpfound;
	}
	
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
	template<class T>
	void EntityTableItemMongodb_DIGIT<T>::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mongodb::DBContext& context, bson_t * doc)
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

	template<class T>
	void EntityTableItemMongodb_DIGIT<T>::getReadSqlItem(mongodb::DBContext& context)
	{
		mongodb::DBContext::DB_ITEM_DATA* pSotvs = new mongodb::DBContext::DB_ITEM_DATA();
		pSotvs->sqlkey = db_item_name();
		memset(pSotvs->sqlval, 0, MAX_BUF);
		context.items.push_back(KBEShared_ptr<mongodb::DBContext::DB_ITEM_DATA>(pSotvs));
	}

	template<class T>
	void EntityTableItemMongodb_DIGIT<T>::addToStream(MemoryStream* s, mongodb::DBContext& context, DBID resultDBID, const bson_t * doc)
	{

		bson_iter_t iter;
		bool isdefault = false;
		if (!bson_iter_init_find(&iter, doc, db_item_name()))
		{
			isdefault = true;
		}

		if (dataSType_ == "INT8")
		{
			if (isdefault || !BSON_ITER_HOLDS_INT32(&iter))
			{
				(*s) << (int8)defaultValue_;
				return;
			}

			int32 v = bson_iter_int32(&iter);
			int8 vv = static_cast<int8>(v);
			(*s) << vv;
		}
		else if (dataSType_ == "INT16")
		{
			if (isdefault || !BSON_ITER_HOLDS_INT32(&iter))
			{
				(*s) << (int16)defaultValue_;
				return;
			}

			int32 v = bson_iter_int32(&iter);
			int16 vv = static_cast<int16>(v);
			(*s) << vv;
		}
		else if (dataSType_ == "INT32")
		{
			if (isdefault || !BSON_ITER_HOLDS_INT32(&iter))
			{
				(*s) << (int32)defaultValue_;
				return;
			}

			int32 v = bson_iter_int32(&iter);
			(*s) << v;
		}
		else if (dataSType_ == "INT64")
		{
			if (isdefault || !BSON_ITER_HOLDS_INT64(&iter))
			{
				(*s) << (int64)defaultValue_;
				return;
			}

			int64 v = bson_iter_int64(&iter);
			(*s) << v;
		}
		else if (dataSType_ == "UINT8")
		{
			if (isdefault || !BSON_ITER_HOLDS_INT32(&iter))
			{
				(*s) << (uint8)defaultValue_;
				return;
			}

			int32 v = bson_iter_int32(&iter);
			uint8 vv = static_cast<uint8>(v);
			(*s) << vv;
		}
		else if (dataSType_ == "UINT16")
		{
			if (isdefault || !BSON_ITER_HOLDS_INT32(&iter))
			{
				(*s) << (uint16)defaultValue_;
				return;
			}

			int32 v = bson_iter_int32(&iter);
			uint16 vv = static_cast<uint16>(v);
			(*s) << vv;
		}
		else if (dataSType_ == "UINT32")
		{
			if (isdefault || !BSON_ITER_HOLDS_INT32(&iter))
			{
				(*s) << (uint32)defaultValue_;
				return;
			}

			uint32 v = bson_iter_int32(&iter);
			(*s) << v;
		}
		else if (dataSType_ == "UINT64")
		{
			if (isdefault || !BSON_ITER_HOLDS_INT64(&iter))
			{
				(*s) << (uint64)defaultValue_;
				return;
			}

			uint64 v = bson_iter_int64(&iter);
			(*s) << v;
		}
		else if (dataSType_ == "FLOAT")
		{
			if (isdefault || !BSON_ITER_HOLDS_DOUBLE(&iter))
			{
				(*s) << (float)defaultValue_;
				return;
			}

			double v = bson_iter_double(&iter);
			float vv = static_cast<float>(v);
			(*s) << vv;
		}
		else if (dataSType_ == "DOUBLE")
		{
			if (isdefault || !BSON_ITER_HOLDS_DOUBLE(&iter))
			{
				(*s) << (double)defaultValue_;
				return;
			}

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

		bson_append_utf8(doc, pSotvs->sqlkey, (int)strlen(pSotvs->sqlkey), val.c_str(), val.length());
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
		if (!bson_iter_init_find(&iter, doc, db_item_name()) || !BSON_ITER_HOLDS_UTF8(&iter))
		{
			//���û���ҵ����ݣ���Ҫ�������Դ���
			(*s) << defaultValue_;
			return;
		}

		uint32_t len = 0;
		const char * value = bson_iter_utf8(&iter, &len);
		std::string datas(value, len);
		(*s) << datas;
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

		bson_append_utf8(doc, pSotvs->sqlkey, (int)strlen(pSotvs->sqlkey), val.c_str(), val.length());
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
		if (!bson_iter_init_find(&iter, doc, db_item_name()) || !BSON_ITER_HOLDS_UTF8(&iter))
		{
			//���û���ҵ����ݣ���Ҫ�������Դ���
			(*s).appendBlob(defaultValue_.data(), defaultValue_.size());
			return;
		}

		uint32_t len = 0;
		const char * value = bson_iter_utf8(&iter, &len);
		std::string datas(value, len);
		(*s).appendBlob(datas.data(), datas.size());
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

		bson_append_utf8(doc, pSotvs->sqlkey, (int)strlen(pSotvs->sqlkey), val.c_str(), val.length());
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
		if (!bson_iter_init_find(&iter, doc, db_item_name()) || !BSON_ITER_HOLDS_UTF8(&iter))
		{
			//���û���ҵ����ݣ���Ҫ�������Դ���
			(*s).appendBlob(defaultValue_.data(), defaultValue_.size());
			return;
		}

		uint32_t len = 0;
		const char * value = bson_iter_utf8(&iter, &len);
		std::string datas(value, len);
		(*s).appendBlob(datas.data(), datas.size());

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
		if (!bson_iter_init_find(&iter, doc, db_item_name()) || !BSON_ITER_HOLDS_BINARY(&iter))
		{
			//���û���ҵ����ݣ���Ҫ�������Դ���
			(*s).appendBlob(defaultValue_.data(), defaultValue_.size());
			return;
		}

		bson_subtype_t btype;
		uint32_t len = 0;
		const char * value;
		bson_iter_binary(&iter, &btype, &len, (const uint8_t**)&value);
		std::string datas(value, len);
		(*s).appendBlob(datas);
	}
}