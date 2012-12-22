/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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


#include "datatypes.hpp"
#include "resmgr/resmgr.hpp"

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
	DATATYPE_MAP::iterator iter = dataTypes_.begin();
	for (; iter != dataTypes_.end(); iter++) 
		iter->second->decRef();

	uid_dataTypes_.clear();
	dataTypesLowerName_.clear();
	dataTypes_.clear();
}

//-------------------------------------------------------------------------------------
bool DataTypes::initialize(std::string file)
{
	// 初始化一些基础类别
	addDateType("UINT8",	new IntType<uint8>);
	addDateType("UINT16",	new IntType<uint16>);
	addDateType("UINT64",	new UInt64Type);
	addDateType("UINT32",	new UInt32Type);

	addDateType("INT8",		new IntType<int8>);
	addDateType("INT16",	new IntType<int16>);
	addDateType("INT32",	new IntType<int32>);
	addDateType("INT64",	new Int64Type);

	addDateType("STRING",	new StringType);
	addDateType("UNICODE",	new UnicodeType);
	addDateType("FLOAT",	new FloatType);
	addDateType("DOUBLE",	new DoubleType);
	addDateType("PYTHON",	new PythonType);
	addDateType("PY_DICT",	new PyDictType);
	addDateType("PY_TUPLE",	new PyTupleType);
	addDateType("PY_LIST",	new PyListType);
	addDateType("MAILBOX",	new MailboxType);
	addDateType("BLOB",		new BlobType);

	addDateType("VECTOR2",	new VectorType(2));
	addDateType("VECTOR3",	new VectorType(3));
	addDateType("VECTOR4",	new VectorType(4));
	return loadAlias(file);
}

//-------------------------------------------------------------------------------------
bool DataTypes::loadAlias(std::string& file)
{
	TiXmlNode* node = NULL;
	XmlPlus* xml = new XmlPlus(Resmgr::getSingleton().matchRes(file).c_str());

	if(xml == NULL || !xml->isGood())
		return false;

	node = xml->getRootNode();

	if(node == NULL)
		return false;

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
				
				if(fixedDict->initialize(xml, childNode))
				{
					addDateType(aliasName, fixedDict);
				}
				else
				{
					ERROR_MSG(boost::format("DataTypes::loadAlias:parse FIXED_DICT [%1%] is error!\n") % 
						aliasName.c_str());
					
					delete fixedDict;
					return false;
				}
			}
			else if(type == "ARRAY")
			{
				FixedArrayType* fixedArray = new FixedArrayType;
				
				if(fixedArray->initialize(xml, childNode))
				{
					addDateType(aliasName, fixedArray);
				}
				else
				{
					ERROR_MSG(boost::format("DataTypes::loadAlias:parse ARRAY [%1%] is error!\n") % 
						aliasName.c_str());
					
					delete fixedArray;
					return false;
				}
			}
			else
			{
				DataType* dataType = getDataType(type);
				if(dataType == NULL)
				{
					ERROR_MSG(boost::format("DataTypes::loadAlias:can't fount type %1% by alias[%2%].\n") % 
						type.c_str() % aliasName.c_str());
					
					return false;
				}

				addDateType(aliasName, dataType);
			}
		}
	}
	XML_FOR_END(node);
	
	delete xml;
	return true;
}

//-------------------------------------------------------------------------------------
bool DataTypes::addDateType(std::string name, DataType* dataType)
{
	dataType->aliasName(name);
	std::string lowername = name;
	std::transform(lowername.begin(), lowername.end(), lowername.begin(), tolower);	
	DATATYPE_MAP::iterator iter = dataTypesLowerName_.find(lowername);
	if (iter != dataTypesLowerName_.end())
	{ 
		ERROR_MSG(boost::format("DataTypes::addDateType:exist a type %1%.\n") % name.c_str());
		return false;
	}

	dataTypes_[name] = dataType;
	dataTypesLowerName_[lowername] = dataType;
	uid_dataTypes_[dataType->id()] = dataType;

	//dataType->incRef();
	return true;
}

//-------------------------------------------------------------------------------------
bool DataTypes::addDateType(DATATYPE_UID uid, DataType* dataType)
{
	UID_DATATYPE_MAP::iterator iter = uid_dataTypes_.find(uid);
	if (iter != uid_dataTypes_.end())
	{
		return false;
	}

	uid_dataTypes_[uid] = dataType;
	return true;
}

//-------------------------------------------------------------------------------------
void DataTypes::delDataType(std::string name)
{
	DATATYPE_MAP::iterator iter = dataTypes_.find(name);
	if (iter == dataTypes_.end())
	{
		ERROR_MSG(boost::format("DataTypes::delDataType:not found type %1%.\n") % name.c_str());
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

	ERROR_MSG(boost::format("DataTypes::getDataType:not found type %1%.\n") % name.c_str());
	return NULL;
}

//-------------------------------------------------------------------------------------
DataType* DataTypes::getDataType(const char* name)
{
	DATATYPE_MAP::iterator iter = dataTypes_.find(name);
	if (iter != dataTypes_.end()) 
		return iter->second.get();

	ERROR_MSG(boost::format("DataTypes::getDataType:not found type %1%.\n") % name);
	return NULL;
}

//-------------------------------------------------------------------------------------
DataType* DataTypes::getDataType(DATATYPE_UID uid)
{
	UID_DATATYPE_MAP::iterator iter = uid_dataTypes_.find(uid);
	if (iter != uid_dataTypes_.end()) 
		return iter->second;

	ERROR_MSG(boost::format("DataTypes::getDataType:not found type %1%.\n") % uid);
	return NULL;
}

//-------------------------------------------------------------------------------------

}
