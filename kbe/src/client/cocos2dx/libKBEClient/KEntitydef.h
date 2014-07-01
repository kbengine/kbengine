#pragma once
#include "KDataTypes.h"
#include <map>
#include <string>

using namespace std;

namespace KBEngineClient
{
	//class Property;
	class ScriptModule;
	//class Method;

	typedef void*(*pMethod)();

#define  PropertyMap std::map<std::string,Property*>
#define  PropertyIdMap std::map<uint16,Property*>
#define  PropertyMapItr std::map<std::string,Property*>::iterator
#define  PropertyIdMapItr std::map<uint16,Property*>::iterator
    
    
#define  MethodMap std::map<std::string,Method*>
#define  MethodIdMap std::map<uint16,Method*>
#define  MethodMapItr std::map<std::string,Method*>::iterator
#define  MethodIdMapItr std::map<uint16,Method*>::iterator
    

	class EntityDef{
	public:
		static std::map<std::string,KBEDATATYPE_BASE> datatypes;
		static std::map<string,int32> datatype2id;
		static std::map<uint16,KBEDATATYPE_BASE> iddatatypes;
		static std::map<std::string,ScriptModule> moduledefs;
		static std::map<uint16,ScriptModule> idmoduledefs;
		static std::map<std::string,PropertyMap*> alldefpropertys;
		static std::map<std::string,PropertyMap*> defpropertys_;
		static std::map<uint16,PropertyMap*> iddefpropertys_;

		static bool __entityAliasID;												// 优化EntityID，aoi范围内小于255个EntityID, 传输到client时使用1字节伪ID 
		static bool __entitydefAliasID;												// 优化entity属性和方法广播时占用的带宽，entity客户端属性或者客户端不超过255个时， 方法uid和属性uid传输到client时使用1字节别名ID

		//
		static void EntityDef_Bind()
		{
			bindMessageDataType();
			initDataType();
		}
		
		static void initDataType()
		{
			datatypes["UINT8"] = new KBEDATATYPE_UINT8();
			datatypes["UINT16"] = new KBEDATATYPE_UINT16();
			datatypes["UINT32"] = new KBEDATATYPE_UINT32();
			datatypes["UINT64"] = new KBEDATATYPE_UINT64();
			
			datatypes["INT8"] = new KBEDATATYPE_INT8();
			datatypes["INT16"] = new KBEDATATYPE_INT16();
			datatypes["INT32"] = new KBEDATATYPE_INT32();
			datatypes["INT64"] = new KBEDATATYPE_INT64();
			
			datatypes["FLOAT"] = new KBEDATATYPE_FLOAT();
			datatypes["DOUBLE"] = new KBEDATATYPE_DOUBLE();
			
			datatypes["STRING"] = new KBEDATATYPE_STRING();
			datatypes["VECTOR2"] = new KBEDATATYPE_VECTOR2();
			datatypes["VECTOR3"] = new KBEDATATYPE_VECTOR3();
			datatypes["VECTOR4"] = new KBEDATATYPE_VECTOR4();
			datatypes["PYTHON"] = new KBEDATATYPE_PYTHON();
			datatypes["UNICODE"] = new KBEDATATYPE_UNICODE();
			datatypes["MAILBOX"] = new KBEDATATYPE_MAILBOX();
			datatypes["BLOB"] = new KBEDATATYPE_BLOB();
		}
		
		static void bindMessageDataType()
		{
			if(datatype2id.size() > 0)
				return;
			
			datatype2id["STRING"] = 1;
			datatype2id["STD::STRING"] = 1;
			
			datatype2id["UINT8"] = 2;
			datatype2id["BOOL"] = 2;
			datatype2id["DATATYPE"] = 2;
			datatype2id["CHAR"] = 2;
			datatype2id["DETAIL_TYPE"] = 2;
			datatype2id["MAIL_TYPE"] = 2;
			
			datatype2id["UINT16"] = 3;
			datatype2id["UNSIGNED SHORT"] = 3;
			datatype2id["SERVER_ERROR_CODE"] = 3;
			datatype2id["ENTITY_TYPE"] = 3;
			datatype2id["ENTITY_PROPERTY_UID"] = 3;
			datatype2id["ENTITY_METHOD_UID"] = 3;
			datatype2id["ENTITY_SCRIPT_UID"] = 3;
			datatype2id["DATATYPE_UID"] = 3;
			
			datatype2id["UINT32"] = 4;
			datatype2id["UINT"] = 4;
			datatype2id["UNSIGNED INT"] = 4;
			datatype2id["ARRAYSIZE"] = 4;
			datatype2id["SPACE_ID"] = 4;
			datatype2id["GAME_TIME"] = 4;
			datatype2id["TIMER_ID"] = 4;
			
			datatype2id["UINT64"] = 5;
			datatype2id["DBID"] = 5;
			datatype2id["COMPONENT_ID"] = 5;
			
			datatype2id["INT8"] = 6;
			datatype2id["COMPONENT_ORDER"] = 6;
			
			datatype2id["INT16"] = 7;
			datatype2id["SHORT"] = 7;
			
			datatype2id["INT32"] = 8;
			datatype2id["INT"] = 8;
			datatype2id["ENTITY_ID"] = 8;
			datatype2id["CALLBACK_ID"] = 8;
			datatype2id["COMPONENT_TYPE"] = 8;
			
			datatype2id["INT64"] = 9;
			
			datatype2id["PYTHON"] = 10;
			datatype2id["PY_DICT"] = 10;
			datatype2id["PY_TUPLE"] = 10;
			datatype2id["PY_LIST"] = 10;
			datatype2id["MAILBOX"] = 10;
			
			datatype2id["BLOB"] = 11;
			
			datatype2id["UNICODE"] = 12;
			
			datatype2id["FLOAT"] = 13;
			
			datatype2id["DOUBLE"] = 14;
			
			datatype2id["VECTOR2"] = 15;
			
			datatype2id["VECTOR3"] = 16;
			
			datatype2id["VECTOR4"] = 17;
			
			datatype2id["FIXED_DICT"] = 18;
			
			datatype2id["ARRAY"] = 19;
		}

		//
		static void entitydefAliasID(bool v)
		{ 
			__entitydefAliasID = v; 
		}

		static bool entitydefAliasID()
		{ 
			return __entitydefAliasID; 
		}

		static void entityAliasID(bool v)
		{ 
			__entityAliasID = v; 
		}

		static bool entityAliasID()
		{ 
			return __entityAliasID; 
		}

		static bool scriptModuleAliasID()
		{ 
			return __entitydefAliasID && moduledefs.size() <= 255; 
		}
 	};


	class ScriptModule
	{
	public:
		string name;
		bool usePropertyDescrAlias;
		bool useMethodDescrAlias;
		
		ScriptModule(){ };
		//
		ScriptModule(std::string& name){	this->name = name; this->useMethodDescrAlias =false; this->usePropertyDescrAlias = false;};
		PropertyMap propertys;
		std::map<uint16,Property*> idpropertys;

		std::map<std::string,Method*> methods;
		std::map<uint16,Method*> idmethods;
		std::map<std::string,Method*> base_methods;
		std::map<uint16,Method*> idbase_methods;
		std::map<std::string,Method*> cell_methods;
		std::map<uint16,Method*> idcell_methods;
	};

	class Method
	{
	public:
		Method(){};
		Method(std::string name_){	name = name_; }
        Method(Method* other_){ Method& other= *other_; name = other.name ; aliasID = other.aliasID; methodUtype = other.methodUtype; args = other.args; }
		Method(Method& other) { name = other.name ; aliasID = other.aliasID; methodUtype = other.methodUtype; args = other.args; }
		string name;
		int16 aliasID;
		std::vector<KBEDATATYPE_BASE*> args;
		uint16 methodUtype;
	};//

	class Property
	{
	public:
        Property(){};
        Property(Property* other) {};
        Property(Property& other) {};
		string name;
		uint16 properUtype;
		//void* setmethod;
		//pMethod setmethod;
		std::string setmethod;
		KBEDATATYPE_BASE utype;
		string defaultValStr;
		int16 aliasID;
		object val;
	};
}
//end namespace