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


#include "datatypes.h"
#include "resmgr/resmgr.h"

namespace KBEngine{

DataTypes::DATATYPE_MAP DataTypes::dataTypes_;
DataTypes::DATATYPE_MAP DataTypes::dataTypesLowerName_;
DataTypes::UID_DATATYPE_MAP DataTypes::uid_dataTypes_;

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
}

//-------------------------------------------------------------------------------------
bool DataTypes::initialize(std::string file)
{
	// 初始化一些基础类别
	addDataType("UINT8",	new IntType<uint8>);
	addDataType("UINT16",	new IntType<uint16>);
	addDataType("UINT64",	new UInt64Type);
	addDataType("UINT32",	new UInt32Type);

	addDataType("INT8",		new IntType<int8>);
	addDataType("INT16",	new IntType<int16>);
	addDataType("INT32",	new IntType<int32>);
	addDataType("INT64",	new Int64Type);

	addDataType("STRING",	new StringType);
	addDataType("UNICODE",	new UnicodeType);
	addDataType("FLOAT",	new FloatType);
	addDataType("DOUBLE",	new DoubleType);
	addDataType("PYTHON",	new PythonType);
	addDataType("PY_DICT",	new PyDictType);
	addDataType("PY_TUPLE",	new PyTupleType);
	addDataType("PY_LIST",	new PyListType);
	addDataType("MAILBOX",	new MailboxType);
	addDataType("BLOB",		new BlobType);

	addDataType("VECTOR2",	new VectorType(2));
	addDataType("VECTOR3",	new VectorType(3));
	addDataType("VECTOR4",	new VectorType(4));
	return loadAlias(file);
}

//-------------------------------------------------------------------------------------
bool DataTypes::loadAlias(std::string& file)
{
	TiXmlNode* node = NULL;
	SmartPointer<XML> xml(new XML(Resmgr::getSingleton().matchRes(file).c_str()));

	if(xml == NULL || !xml->isGood())
		return false;

	node = xml->getRootNode();

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

		if(childNode != NULL)
		{
			type = xml->getValStr(childNode);
			if(type == "FIXED_DICT")
			{
				FixedDictType* fixedDict = new FixedDictType;
				
				if(fixedDict->initialize(xml.get(), childNode))
				{
					addDataType(aliasName, fixedDict);
				}
				else
				{
					ERROR_MSG(fmt::format("DataTypes::loadAlias: parse FIXED_DICT [{}] error!\n", 
						aliasName.c_str()));
					
					delete fixedDict;
					return false;
				}
			}
			else if(type == "ARRAY")
			{
				FixedArrayType* fixedArray = new FixedArrayType;
				
				if(fixedArray->initialize(xml.get(), childNode))
				{
					addDataType(aliasName, fixedArray);
				}
				else
				{
					ERROR_MSG(fmt::format("DataTypes::loadAlias: parse ARRAY [{}] error!\n", 
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
					ERROR_MSG(fmt::format("DataTypes::loadAlias:can't fount type {} by alias[{}].\n", 
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
		dataTypesLowerName_.erase(iter);
	}
}

//-------------------------------------------------------------------------------------
DataType* DataTypes::getDataType(std::string name)
{
	DATATYPE_MAP::iterator iter = dataTypes_.find(name);
	if (iter != dataTypes_.end()) 
		return iter->second.get();

	ERROR_MSG(fmt::format("DataTypes::getDataType:not found type {}.\n", name.c_str()));
	return NULL;
}

//-------------------------------------------------------------------------------------
DataType* DataTypes::getDataType(const char* name)
{
	DATATYPE_MAP::iterator iter = dataTypes_.find(name);
	if (iter != dataTypes_.end()) 
		return iter->second.get();

	ERROR_MSG(fmt::format("DataTypes::getDataType:not found type {}.\n", name));
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
