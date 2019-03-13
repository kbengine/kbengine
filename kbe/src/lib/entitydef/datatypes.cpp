// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "datatypes.h"
#include "resmgr/resmgr.h"

namespace KBEngine{

DataTypes::DATATYPE_MAP DataTypes::dataTypes_;
DataTypes::DATATYPE_MAP DataTypes::dataTypesLowerName_;
DataTypes::UID_DATATYPE_MAP DataTypes::uid_dataTypes_;
DataTypes::DATATYPE_ORDERS DataTypes::dataTypesOrders_;

static uint8 _g_baseTypeEndIndex = 0;

//-------------------------------------------------------------------------------------
DataTypes::DataTypes()
{
}

//-------------------------------------------------------------------------------------
DataTypes::~DataTypes()
{
	finalise();
}

//-------------------------------------------------------------------------------------
void DataTypes::finalise(void)
{
	//DATATYPE_MAP::iterator iter = dataTypes_.begin();
	//for (; iter != dataTypes_.end(); ++iter) 
	//	iter->second->decRef();

	uid_dataTypes_.clear();
	dataTypesLowerName_.clear();
	dataTypes_.clear();
	dataTypesOrders_.clear();

	_g_baseTypeEndIndex = 0;
}

//-------------------------------------------------------------------------------------
bool DataTypes::validTypeName(const std::string& typeName)
{
	// 不允许前面加_, 因为内部产生的一些临时结构前面使用了_, 避免误判
	if (typeName.size() > 0 && typeName[0] == '_')
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
bool DataTypes::initialize(std::string file)
{
	// 初始化一些基础类别
	addDataType("UINT8",		new IntType<uint8>);
	addDataType("UINT16",		new IntType<uint16>);
	addDataType("UINT64",		new UInt64Type);
	addDataType("UINT32",		new UInt32Type);

	addDataType("INT8",			new IntType<int8>);
	addDataType("INT16",		new IntType<int16>);
	addDataType("INT32",		new IntType<int32>);
	addDataType("INT64",		new Int64Type);

	addDataType("STRING",		new StringType);
	addDataType("UNICODE",		new UnicodeType);
	addDataType("FLOAT",		new FloatType);
	addDataType("DOUBLE",		new DoubleType);
	addDataType("PYTHON",		new PythonType);
	addDataType("PY_DICT",		new PyDictType);
	addDataType("PY_TUPLE",		new PyTupleType);
	addDataType("PY_LIST",		new PyListType);
	addDataType("ENTITYCALL",	new EntityCallType);
	addDataType("BLOB",			new BlobType);

	addDataType("VECTOR2",		new Vector2Type);
	addDataType("VECTOR3",		new Vector3Type);
	addDataType("VECTOR4",		new Vector4Type);

	_g_baseTypeEndIndex = dataTypesOrders_.size();
	return loadTypes(file);
}

//-------------------------------------------------------------------------------------
std::vector< std::string > DataTypes::getBaseTypeNames()
{
	std::vector< std::string > ret;
	ret.assign(dataTypesOrders_.begin(), dataTypesOrders_.begin() + _g_baseTypeEndIndex);
	return ret;
}

//-------------------------------------------------------------------------------------
bool DataTypes::loadTypes(std::string& file)
{
	// 允许纯脚本定义，则可能没有这个文件
	if (access(file.c_str(), 0) != 0)
		return true;

	SmartPointer<XML> xml(new XML(Resmgr::getSingleton().matchRes(file).c_str()));
	return loadTypes(xml);
}

//-------------------------------------------------------------------------------------
bool DataTypes::loadTypes(SmartPointer<XML>& xml)
{
	if (xml == NULL || !xml->isGood())
		return false;

	TiXmlNode* node = xml->getRootNode();

	if(node == NULL)
	{
		// root节点下没有子节点了
		return true;
	}

	XML_FOR_BEGIN(node)
	{
		std::string type = "";
		std::string aliasName = xml->getKey(node);
		TiXmlNode* childNode = node->FirstChild();

		if (!DataTypes::validTypeName(aliasName))
		{
			ERROR_MSG(fmt::format("DataTypes::loadTypes: Not allowed to use the prefix \"_\"! aliasName={}\n",
				aliasName.c_str()));

			return false;
		}

		if(childNode != NULL)
		{
			type = xml->getValStr(childNode);
			if(type == "FIXED_DICT")
			{
				FixedDictType* fixedDict = new FixedDictType;
				
				if(fixedDict->initialize(xml.get(), childNode, aliasName))
				{
					addDataType(aliasName, fixedDict);
				}
				else
				{
					ERROR_MSG(fmt::format("DataTypes::loadTypes: parse FIXED_DICT [{}] error!\n", 
						aliasName.c_str()));
					
					delete fixedDict;
					return false;
				}
			}
			else if(type == "ARRAY")
			{
				FixedArrayType* fixedArray = new FixedArrayType;
				
				if(fixedArray->initialize(xml.get(), childNode, aliasName))
				{
					addDataType(aliasName, fixedArray);
				}
				else
				{
					ERROR_MSG(fmt::format("DataTypes::loadTypes: parse ARRAY [{}] error!\n", 
						aliasName.c_str()));
					
					delete fixedArray;
					return false;
				}
			}
			else
			{
				DataType* dataType = getDataType(type);
				if(dataType == NULL)
				{
					ERROR_MSG(fmt::format("DataTypes::loadTypes: can't fount type {} by alias[{}].\n", 
						type.c_str(), aliasName.c_str()));
					
					return false;
				}

				addDataType(aliasName, dataType);
			}
		}
	}
	XML_FOR_END(node);
	
	return true;
}

//-------------------------------------------------------------------------------------
bool DataTypes::addDataType(std::string name, DataType* dataType)
{
	dataTypesOrders_.push_back(name);

	dataType->aliasName(name);
	std::string lowername = name;
	std::transform(lowername.begin(), lowername.end(), lowername.begin(), tolower);	

	DATATYPE_MAP::iterator iter = dataTypesLowerName_.find(lowername);
	if (iter != dataTypesLowerName_.end())
	{ 
		ERROR_MSG(fmt::format("DataTypes::addDataType(name): name {} exist.\n", name.c_str()));
		return false;
	}

	dataTypes_[name] = dataType;
	dataTypesLowerName_[lowername] = dataType;
	uid_dataTypes_[dataType->id()] = dataType;

	//dataType->incRef();

	if(g_debugEntity)
	{
		DEBUG_MSG(fmt::format("DataTypes::addDataType(name): {:p} name={}, aliasName={}, uid={}.\n", 
			(void*)dataType, name, dataType->aliasName(), dataType->id()));
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool DataTypes::addDataType(DATATYPE_UID uid, DataType* dataType)
{
	UID_DATATYPE_MAP::iterator iter = uid_dataTypes_.find(uid);
	if (iter != uid_dataTypes_.end())
	{
		ERROR_MSG(fmt::format("DataTypes(uid)::addDataType: utype {} exist.\n", uid));
		return false;
	}

	uid_dataTypes_[uid] = dataType;

	if(g_debugEntity)
	{
		DEBUG_MSG(fmt::format("DataTypes::addDataType(uid): {:p} aliasName={}, uid={}.\n", 
			(void*)dataType, dataType->aliasName(), uid));
	}

	return true;
}

//-------------------------------------------------------------------------------------
void DataTypes::delDataType(std::string name)
{
	DATATYPE_MAP::iterator iter = dataTypes_.find(name);
	if (iter == dataTypes_.end())
	{
		ERROR_MSG(fmt::format("DataTypes::delDataType:not found type {}.\n", name.c_str()));
	}
	else
	{
		uid_dataTypes_.erase(iter->second->id());
		iter->second->decRef();
		dataTypes_.erase(iter);

		std::string lowername = name;
		std::transform(lowername.begin(), lowername.end(), lowername.begin(), tolower);
		dataTypesLowerName_.erase(lowername);
	}
}

//-------------------------------------------------------------------------------------
DataType* DataTypes::getDataType(std::string name, bool notFoundOutError)
{
	DATATYPE_MAP::iterator iter = dataTypes_.find(name);
	if (iter != dataTypes_.end()) 
		return iter->second.get();

	if (notFoundOutError)
	{
		ERROR_MSG(fmt::format("DataTypes::getDataType:not found type {}.\n", name.c_str()));
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
DataType* DataTypes::getDataType(const char* name, bool notFoundOutError)
{
	DATATYPE_MAP::iterator iter = dataTypes_.find(name);
	if (iter != dataTypes_.end()) 
		return iter->second.get();

	if (notFoundOutError)
	{
		ERROR_MSG(fmt::format("DataTypes::getDataType:not found type {}.\n", name));
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
DataType* DataTypes::getDataType(DATATYPE_UID uid)
{
	UID_DATATYPE_MAP::iterator iter = uid_dataTypes_.find(uid);
	if (iter != uid_dataTypes_.end()) 
		return iter->second;

	ERROR_MSG(fmt::format("DataTypes::getDataType:not found type {}.\n", uid));
	return NULL;
}

//-------------------------------------------------------------------------------------

}
