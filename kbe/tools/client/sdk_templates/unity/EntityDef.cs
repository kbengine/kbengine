namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;

	/*
		EntityDef模块
		管理了所有的实体定义的描述以及所有的数据类型描述
	*/
    public class EntityDef
    {
		// 所有的数据类型
		public static Dictionary<string, UInt16> datatype2id = new Dictionary<string, UInt16>();
		public static Dictionary<string, KBEDATATYPE_BASE> datatypes = new Dictionary<string, KBEDATATYPE_BASE>();
		public static Dictionary<UInt16, KBEDATATYPE_BASE> id2datatypes = new Dictionary<UInt16, KBEDATATYPE_BASE>();
		
		public static Dictionary<string, Int32> entityclass = new Dictionary<string, Int32>();
		
		public static Dictionary<string, ScriptModule> moduledefs = new Dictionary<string, ScriptModule>();
		public static Dictionary<UInt16, ScriptModule> idmoduledefs = new Dictionary<UInt16, ScriptModule>();

		public static void clear()
		{
			datatype2id.Clear();
			datatypes.Clear();
			id2datatypes.Clear();
			entityclass.Clear();
			moduledefs.Clear();
			idmoduledefs.Clear();
			
			initDataType();
			bindMessageDataType();
		}

		public EntityDef()
		{
			initDataType();
			bindMessageDataType();
		}
		
		public static void initDataType()
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
		
		public static void bindMessageDataType()
		{
			if(datatype2id.Count > 0)
				return;
			
			datatype2id["STRING"] = 1;
			datatype2id["STD::STRING"] = 1;

			id2datatypes[1] = datatypes["STRING"];
			
			datatype2id["UINT8"] = 2;
			datatype2id["BOOL"] = 2;
			datatype2id["DATATYPE"] = 2;
			datatype2id["CHAR"] = 2;
			datatype2id["DETAIL_TYPE"] = 2;
			datatype2id["MAIL_TYPE"] = 2;

			id2datatypes[2] = datatypes["UINT8"];
			
			datatype2id["UINT16"] = 3;
			datatype2id["UNSIGNED SHORT"] = 3;
			datatype2id["SERVER_ERROR_CODE"] = 3;
			datatype2id["ENTITY_TYPE"] = 3;
			datatype2id["ENTITY_PROPERTY_UID"] = 3;
			datatype2id["ENTITY_METHOD_UID"] = 3;
			datatype2id["ENTITY_SCRIPT_UID"] = 3;
			datatype2id["DATATYPE_UID"] = 3;

			id2datatypes[3] = datatypes["UINT16"];
			
			datatype2id["UINT32"] = 4;
			datatype2id["UINT"] = 4;
			datatype2id["UNSIGNED INT"] = 4;
			datatype2id["ARRAYSIZE"] = 4;
			datatype2id["SPACE_ID"] = 4;
			datatype2id["GAME_TIME"] = 4;
			datatype2id["TIMER_ID"] = 4;

			id2datatypes[4] = datatypes["UINT32"];
			
			datatype2id["UINT64"] = 5;
			datatype2id["DBID"] = 5;
			datatype2id["COMPONENT_ID"] = 5;

			id2datatypes[5] = datatypes["UINT64"];
			
			datatype2id["INT8"] = 6;
			datatype2id["COMPONENT_ORDER"] = 6;

			id2datatypes[6] = datatypes["INT8"];
			
			datatype2id["INT16"] = 7;
			datatype2id["SHORT"] = 7;

			id2datatypes[7] = datatypes["INT16"];
			
			datatype2id["INT32"] = 8;
			datatype2id["INT"] = 8;
			datatype2id["ENTITY_ID"] = 8;
			datatype2id["CALLBACK_ID"] = 8;
			datatype2id["COMPONENT_TYPE"] = 8;

			id2datatypes[8] = datatypes["INT32"];
			
			datatype2id["INT64"] = 9;

			id2datatypes[9] = datatypes["INT64"];
			
			datatype2id["PYTHON"] = 10;
			datatype2id["PY_DICT"] = 10;
			datatype2id["PY_TUPLE"] = 10;
			datatype2id["PY_LIST"] = 10;
			datatype2id["MAILBOX"] = 10;

			id2datatypes[10] = datatypes["PYTHON"];
			
			datatype2id["BLOB"] = 11;

			id2datatypes[11] = datatypes["BLOB"];
			
			datatype2id["UNICODE"] = 12;

			id2datatypes[12] = datatypes["UNICODE"];
			
			datatype2id["FLOAT"] = 13;

			id2datatypes[13] = datatypes["FLOAT"];
			
			datatype2id["DOUBLE"] = 14;

			id2datatypes[14] = datatypes["DOUBLE"];
			
			datatype2id["VECTOR2"] = 15;

			id2datatypes[15] = datatypes["VECTOR2"];
			
			datatype2id["VECTOR3"] = 16;

			id2datatypes[16] = datatypes["VECTOR3"];
			
			datatype2id["VECTOR4"] = 17;

			id2datatypes[17] = datatypes["VECTOR4"];
			
			datatype2id["FIXED_DICT"] = 18;
			// 这里不需要绑定，FIXED_DICT需要根据不同类型实例化动态得到id
			//id2datatypes[18] = datatypes["FIXED_DICT"];
			
			datatype2id["ARRAY"] = 19;
			// 这里不需要绑定，ARRAY需要根据不同类型实例化动态得到id
			//id2datatypes[19] = datatypes["ARRAY"];
		}
    }
    
} 
