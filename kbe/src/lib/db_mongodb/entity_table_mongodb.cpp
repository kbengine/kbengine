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
		return true;
	}



}