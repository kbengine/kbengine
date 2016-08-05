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


		//在这里写执行命令，初始化数据库的表结构
		DBInterfaceMongodb *pdbiMongodb = static_cast<DBInterfaceMongodb *>(pdbi);
		pdbiMongodb->createCollection(name);
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

	//-------------------------------------------------------------------------------------
	void EntityTableItemMongodbBase::init_db_item_name(const char* exstrFlag)
	{
		kbe_snprintf(db_item_name_, MAX_BUF, TABLE_ITEM_PERFIX"_%s%s", exstrFlag, itemName());
	}

	//-------------------------------------------------------------------------------------

}