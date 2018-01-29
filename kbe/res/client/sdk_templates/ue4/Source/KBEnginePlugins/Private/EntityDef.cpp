#include "EntityDef.h"
#include "ScriptModule.h"
#include "DataTypes.h"
#include "Entity.h"
#include "KBDebug.h"

TMap<FString, uint16> EntityDef::datatype2id;
TMap<FString, KBEDATATYPE_BASE*> EntityDef::datatypes;
TMap<uint16, KBEDATATYPE_BASE*> EntityDef::id2datatypes;

TMap<FString, int32> EntityDef::entityclass;

TMap<FString, ScriptModule*> EntityDef::moduledefs;
TMap<uint16, ScriptModule*> EntityDef::idmoduledefs;

void EntityDef::initialize()
{
	EntityFactory::initialize();
	initDataType();
	bindMessageDataType();
}

void EntityDef::clear()
{
	for (auto& item : EntityDef::datatypes)
		delete item.Value;

	for (auto& item : EntityDef::moduledefs)
		delete item.Value;

	datatype2id.Empty();
	datatypes.Empty();
	id2datatypes.Empty();
	entityclass.Empty();
	moduledefs.Empty();
	idmoduledefs.Empty();

	initDataType();
	bindMessageDataType();
}

void EntityDef::initDataType()
{
	if (datatypes.Num() > 0)
		return;

	datatypes.Add(TEXT("UINT8"), new KBEDATATYPE_UINT8());
	datatypes.Add(TEXT("UINT16"), new KBEDATATYPE_UINT16());
	datatypes.Add(TEXT("UINT32"), new KBEDATATYPE_UINT32());
	datatypes.Add(TEXT("UINT64"), new KBEDATATYPE_UINT64());

	datatypes.Add(TEXT("INT8"), new KBEDATATYPE_INT8());
	datatypes.Add(TEXT("INT16"), new KBEDATATYPE_INT16());
	datatypes.Add(TEXT("INT32"), new KBEDATATYPE_INT32());
	datatypes.Add(TEXT("INT64"), new KBEDATATYPE_INT64());

	datatypes.Add(TEXT("FLOAT"), new KBEDATATYPE_FLOAT());
	datatypes.Add(TEXT("DOUBLE"), new KBEDATATYPE_DOUBLE());

	datatypes.Add(TEXT("STRING"), new KBEDATATYPE_STRING());
	datatypes.Add(TEXT("VECTOR2"), new KBEDATATYPE_VECTOR2());
	datatypes.Add(TEXT("VECTOR3"), new KBEDATATYPE_VECTOR3());
	datatypes.Add(TEXT("VECTOR4"), new KBEDATATYPE_VECTOR4());
	datatypes.Add(TEXT("PYTHON"), new KBEDATATYPE_PYTHON());
	datatypes.Add(TEXT("PY_DICT"), new KBEDATATYPE_PYTHON());
	datatypes.Add(TEXT("PY_TUPLE"), new KBEDATATYPE_PYTHON());
	datatypes.Add(TEXT("PY_LIST"), new KBEDATATYPE_PYTHON());
	datatypes.Add(TEXT("UNICODE"), new KBEDATATYPE_UNICODE());
	datatypes.Add(TEXT("ENTITYCALL"), new KBEDATATYPE_ENTITYCALL());
	datatypes.Add(TEXT("BLOB"), new KBEDATATYPE_BLOB());
}

void EntityDef::bindMessageDataType()
{
	if (datatype2id.Num() > 0)
		return;

	datatype2id.Add(TEXT("STRING"), 1);
	datatype2id.Add(TEXT("STD::STRING"), 1);

	id2datatypes.Add(1, datatypes["STRING"]);

	datatype2id.Add(TEXT("UINT8"), 2);
	datatype2id.Add(TEXT("BOOL"), 2);
	datatype2id.Add(TEXT("DATATYPE"), 2);
	datatype2id.Add(TEXT("CHAR"), 2);
	datatype2id.Add(TEXT("DETAIL_TYPE"), 2);
	datatype2id.Add(TEXT("ENTITYCALL_CALL_TYPE"), 2);

	id2datatypes.Add(2, datatypes["UINT8"]);

	datatype2id.Add(TEXT("UINT16"), 3);
	datatype2id.Add(TEXT("UNSIGNED SHORT"), 3);
	datatype2id.Add(TEXT("SERVER_ERROR_CODE"), 3);
	datatype2id.Add(TEXT("ENTITY_TYPE"), 3);
	datatype2id.Add(TEXT("ENTITY_PROPERTY_UID"), 3);
	datatype2id.Add(TEXT("ENTITY_METHOD_UID"), 3);
	datatype2id.Add(TEXT("ENTITY_SCRIPT_UID"), 3);
	datatype2id.Add(TEXT("DATATYPE_UID"), 3);

	id2datatypes.Add(3, datatypes["UINT16"]);

	datatype2id.Add(TEXT("UINT32"), 4);
	datatype2id.Add(TEXT("UINT"), 4);
	datatype2id.Add(TEXT("UNSIGNED INT"), 4);
	datatype2id.Add(TEXT("ARRAYSIZE"), 4);
	datatype2id.Add(TEXT("SPACE_ID"), 4);
	datatype2id.Add(TEXT("GAME_TIME"), 4);
	datatype2id.Add(TEXT("TIMER_ID"), 4);

	id2datatypes.Add(4, datatypes["UINT32"]);

	datatype2id.Add(TEXT("UINT64"), 5);
	datatype2id.Add(TEXT("DBID"), 5);
	datatype2id.Add(TEXT("COMPONENT_ID"), 5);

	id2datatypes.Add(5, datatypes["UINT64"]);

	datatype2id.Add(TEXT("INT8"), 6);
	datatype2id.Add(TEXT("COMPONENT_ORDER"), 6);

	id2datatypes.Add(6, datatypes["INT8"]);

	datatype2id.Add(TEXT("INT16"), 7);
	datatype2id.Add(TEXT("SHORT"), 7);

	id2datatypes.Add(7, datatypes["INT16"]);

	datatype2id.Add(TEXT("INT32"), 8);
	datatype2id.Add(TEXT("INT"), 8);
	datatype2id.Add(TEXT("ENTITY_ID"), 8);
	datatype2id.Add(TEXT("CALLBACK_ID"), 8);
	datatype2id.Add(TEXT("COMPONENT_TYPE"), 8);

	id2datatypes.Add(8, datatypes["INT32"]);

	datatype2id.Add(TEXT("INT64"), 9);

	id2datatypes.Add(9, datatypes["INT64"]);

	datatype2id.Add(TEXT("PYTHON"), 10);
	datatype2id.Add(TEXT("PY_DICT"), 10);
	datatype2id.Add(TEXT("PY_TUPLE"), 10);
	datatype2id.Add(TEXT("PY_LIST"), 10);
	datatype2id.Add(TEXT("ENTITYCALL"), 10);

	id2datatypes.Add(10, datatypes["PYTHON"]);

	datatype2id.Add(TEXT("BLOB"), 11);

	id2datatypes.Add(11, datatypes["BLOB"]);

	datatype2id.Add(TEXT("UNICODE"), 12);

	id2datatypes.Add(12, datatypes["UNICODE"]);

	datatype2id.Add(TEXT("FLOAT"), 13);

	id2datatypes.Add(13, datatypes["FLOAT"]);

	datatype2id.Add(TEXT("DOUBLE"), 14);

	id2datatypes.Add(14, datatypes["DOUBLE"]);

	datatype2id.Add(TEXT("VECTOR2"), 15);

	id2datatypes.Add(15, datatypes["VECTOR2"]);

	datatype2id.Add(TEXT("VECTOR3"), 16);

	id2datatypes.Add(16, datatypes["VECTOR3"]);

	datatype2id.Add(TEXT("VECTOR4"), 17);

	id2datatypes.Add(17, datatypes["VECTOR4"]);

	datatype2id.Add(TEXT("FIXED_DICT"), 18);
	// 这里不需要绑定，FIXED_DICT需要根据不同类型实例化动态得到id
	//id2datatypes.Add(18, datatypes["FIXED_DICT"]);

	datatype2id.Add(TEXT("ARRAY"), 19);
	// 这里不需要绑定，ARRAY需要根据不同类型实例化动态得到id
	//id2datatypes.Add(19, datatypes["ARRAY"]);
}