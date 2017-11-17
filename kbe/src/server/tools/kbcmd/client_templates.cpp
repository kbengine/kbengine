/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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

#include "client_templates.h"
#include "client_templates_unity.h"	
#include "client_templates_ue4.h"
#include "entitydef/entitydef.h"
#include "entitydef/scriptdef_module.h"
#include "entitydef/property.h"
#include "entitydef/method.h"
#include "entitydef/datatypes.h"
#include "entitydef/datatype.h"

#ifdef _WIN32  
#include <direct.h>  
#include <io.h>  
#elif _LINUX  
#include <stdarg.h>  
#include <sys/stat.h>  
#endif  

#if KBE_PLATFORM == PLATFORM_WIN32
#define KBE_ACCESS _access  
#define KBE_MKDIR(a) _mkdir((a))  
#else
#define KBE_ACCESS access  
#define KBE_MKDIR(a) mkdir((a),0755)  
#endif  

namespace KBEngine {	

int CreatDir(const char *pDir)
{
	int i = 0;
	int iRet;
	int iLen;
	char* pszDir;

	if (NULL == pDir)
	{
		return 0;
	}

	pszDir = strdup(pDir);
	iLen = strlen(pszDir);

	// 创建中间目录  
	for (i = 0; i < iLen; i++)
	{
		if (pszDir[i] == '\\' || pszDir[i] == '/')
		{
			pszDir[i] = '\0';

			//如果不存在,创建  
			iRet = KBE_ACCESS(pszDir, 0);
			if (iRet != 0)
			{
				iRet = KBE_MKDIR(pszDir);
				if (iRet != 0)
				{
					free(pszDir);
					return -1;
				}
			}

			//支持linux,将所有\换成/  
			pszDir[i] = '/';
		}
	}

	if (iLen > 0 && KBE_ACCESS(pszDir, 0) != 0)
	{
		iRet = KBE_MKDIR(pszDir);
	}
	
	free(pszDir);
	return iRet;
}

//-------------------------------------------------------------------------------------
ClientTemplates::ClientTemplates():
	path_(),
	sourcefileBody_(),
	sourcefileName_()
{

}

//-------------------------------------------------------------------------------------
ClientTemplates::~ClientTemplates()
{

}

//-------------------------------------------------------------------------------------
ClientTemplates* ClientTemplates::createClientTemplates(const std::string& type)
{
	std::string lowerType = type;
	std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), tolower);

	if (lowerType == "unity")
	{
		return new ClientTemplatesUnity();
	}
	else if(lowerType == "ue4")
	{
		return new ClientTemplatesUE4();
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::good() const
{
	return true;
}

//-------------------------------------------------------------------------------------
void ClientTemplates::onCreateModuleFileName(const std::string& moduleName)
{
	sourcefileName_ = moduleName + ".unknown";
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::saveFile()
{
	if (CreatDir(path_.c_str()) == -1)
	{
		ERROR_MSG(fmt::format("creating directory error! path={}\n", path_));
		return false;
	}

	std::string path = path_ + sourcefileName_;
	
	DEBUG_MSG(fmt::format("ClientTemplates::saveFile(): {}\n",
		path));

	FILE *fp = fopen(path.c_str(), "w");

	if (NULL == fp)
	{
		ERROR_MSG(fmt::format("ClientTemplates::saveFile(): fopen error! {}\n",
			path));

		return false;
	}

	int written = fwrite(sourcefileBody_.c_str(), 1, sourcefileBody_.size(), fp);
	if (written != (int)sourcefileBody_.size())
	{
		ERROR_MSG(fmt::format("ClientTemplates::saveFile(): fwrite error! {}\n",
			path));

		return false;
	}

	if(fclose(fp))
	{
		ERROR_MSG(fmt::format("ClientTemplates::saveFile(): fclose error! {}\n",
			path));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::create(const std::string& path)
{
	path_ = path;

	if (path_[path_.size() - 1] != '\\' && path_[path_.size() - 1] != '/')
#ifdef _WIN32  
		path_ += "/";
#else
		path_ += "\\";
#endif

	if (!writeTypes())
		return false;

	const EntityDef::SCRIPT_MODULES& scriptModules = EntityDef::getScriptModules();
	EntityDef::SCRIPT_MODULES::const_iterator moduleIter = scriptModules.begin();
	for (; moduleIter != scriptModules.end(); ++moduleIter)
	{
		ScriptDefModule* pScriptDefModule = (*moduleIter).get();

		if (!writeModule(pScriptDefModule))
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
void ClientTemplates::onCreateTypeFileName()
{
	sourcefileName_ = "kbe_types.unknown";
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::writeTypes()
{
	sourcefileName_ = sourcefileBody_ = "";
	onCreateTypeFileName();

	if (!writeTypesBegin())
		return false;

	const DataTypes::DATATYPE_MAP& dataTypes = DataTypes::dataTypes();
	

	const DataTypes::DATATYPE_ORDERS& dataTypesOrders = DataTypes::dataTypesOrders();
	DataTypes::DATATYPE_ORDERS::const_iterator oiter = dataTypesOrders.begin();

	for (; oiter != dataTypesOrders.end(); ++oiter)
	{
		DataTypes::DATATYPE_MAP::const_iterator iter = dataTypes.find((*oiter));

		std::string typeName = iter->first;

		if (typeName[0] == '_')
			continue;

		DataType* pDataType = iter->second.get();

		if (pDataType->type() == DATA_TYPE_FIXEDDICT)
		{
			FixedDictType* pFixedDictType = static_cast<FixedDictType*>(pDataType);

			if (!writeTypeBegin(typeName, pFixedDictType))
				return false;

			FixedDictType::FIXEDDICT_KEYTYPE_MAP& keyTypes = pFixedDictType->getKeyTypes();
			FixedDictType::FIXEDDICT_KEYTYPE_MAP::iterator itemIter = keyTypes.begin();
			for(; itemIter != keyTypes.end(); ++itemIter)
			{
				std::string type = itemIter->second->dataType->getName();
				std::string itemTypeName = itemIter->first;
				std::string itemTypeAliasName = itemIter->second->dataType->aliasName();

				if (type == "INT8")
				{
					if (!writeTypeItemType_INT8(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "INT16")
				{
					if (!writeTypeItemType_INT16(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "INT32")
				{
					if (!writeTypeItemType_INT32(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "INT64")
				{
					if (!writeTypeItemType_INT64(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "UINT8")
				{
					if (!writeTypeItemType_UINT8(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "UINT16")
				{
					if (!writeTypeItemType_UINT16(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "UINT32")
				{
					if (!writeTypeItemType_UINT32(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "UINT64")
				{
					if (!writeTypeItemType_UINT64(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "FLOAT")
				{
					if (!writeTypeItemType_FLOAT(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "DOUBLE")
				{
					if (!writeTypeItemType_DOUBLE(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "STRING")
				{
					if (!writeTypeItemType_STRING(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "UNICODE")
				{
					if (!writeTypeItemType_UNICODE(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "PYTHON")
				{
					if (!writeTypeItemType_PYTHON(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "PY_DICT")
				{
					if (!writeTypeItemType_PY_DICT(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "PY_TUPLE")
				{
					if (!writeTypeItemType_PY_TUPLE(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "PY_LIST")
				{
					if (!writeTypeItemType_PY_LIST(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "BLOB")
				{
					if (!writeTypeItemType_BLOB(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "ARRAY")
				{
					if (!writeTypeItemType_ARRAY(itemTypeName, itemTypeAliasName, itemIter->second->dataType))
						return false;
				}
				else if (type == "FIXED_DICT")
				{
					if (!writeTypeItemType_FIXED_DICT(itemTypeName, itemTypeAliasName, itemIter->second->dataType))
						return false;
				}
#ifdef CLIENT_NO_FLOAT
				else if (type == "VECTOR2")
				{
					if (!writeTypeItemType_VECTOR2(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "VECTOR3")
				{
					if (!writeTypeItemType_VECTOR3(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "VECTOR4")
				{
					if (!writeTypeItemType_VECTOR4(itemTypeName, itemTypeAliasName))
						return false;
				}
#else
				else if (type == "VECTOR2")
				{
					if (!writeTypeItemType_VECTOR2(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "VECTOR3")
				{
					if (!writeTypeItemType_VECTOR3(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "VECTOR4")
				{
					if (!writeTypeItemType_VECTOR4(itemTypeName, itemTypeAliasName))
						return false;
				}
#endif
				else if (type == "MAILBOX")
				{
					if (!writeTypeItemType_MAILBOX(itemTypeName, itemTypeAliasName))
						return false;
				}
			}

			if (!writeTypeEnd(typeName, pFixedDictType))
				return false;
		}
		else if (pDataType->type() == DATA_TYPE_FIXEDARRAY)
		{
			FixedArrayType* pFixedArrayType = static_cast<FixedArrayType*>(pDataType);

			if (!writeTypeBegin(typeName, pFixedArrayType, fmt::format("{}<#REPLACE#>", typeToType("ARRAY"))))
				return false;

			std::string type = pFixedArrayType->getDataType()->getName();
			std::string itemTypeAliasName = pFixedArrayType->getDataType()->aliasName();

			if (type != "ARRAY" && type != "FIXED_DICT")
			{
				std::string newType = typeToType(type);
				strutil::kbe_replace(sourcefileBody_, "#REPLACE#", newType);

				std::string::size_type fpos = sourcefileBody_.find("#REPLACE#");
				KBE_ASSERT(fpos == std::string::npos);
			}

			if (type == "ARRAY")
			{
				if (!writeTypeItemType_ARRAY(typeName, itemTypeAliasName, pFixedArrayType->getDataType()))
					return false;
			}
			else if (type == "FIXED_DICT")
			{
				if (!writeTypeItemType_FIXED_DICT(typeName, itemTypeAliasName, pFixedArrayType->getDataType()))
					return false;
			}

			strutil::kbe_replace(sourcefileBody_, "#REPLACE#", "object");

			if (!writeTypeEnd(typeName, pFixedArrayType))
				return false;
		}
		else
		{
		}
	}

	if (!writeTypesEnd())
		return false; 

	return saveFile();
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::writeTypesBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::writeTypesEnd()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::writeModule(ScriptDefModule* pEntityScriptDefModule)
{
	DEBUG_MSG(fmt::format("ClientTemplates::writeModule(): {}/{}\n",
		path_, pEntityScriptDefModule->getName()));

	sourcefileName_ = sourcefileBody_ = "";
	onCreateModuleFileName(pEntityScriptDefModule->getName());

	if (!writeModuleBegin(pEntityScriptDefModule))
		return false;

	if (!writePropertys(pEntityScriptDefModule, pEntityScriptDefModule))
		return false;

	if (!writeMethods(pEntityScriptDefModule, pEntityScriptDefModule))
		return false;

	if (!writeModuleEnd(pEntityScriptDefModule))
		return false;

	return saveFile();
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::writeModuleBegin(ScriptDefModule* pEntityScriptDefModule)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::writeModuleEnd(ScriptDefModule* pEntityScriptDefModule)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::writePropertys(ScriptDefModule* pEntityScriptDefModule,
	ScriptDefModule* pCurrScriptDefModule)
{
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& clientPropertys = pCurrScriptDefModule->getClientPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator propIter = clientPropertys.begin();
	for (; propIter != clientPropertys.end(); ++propIter)
	{
		PropertyDescription* pPropertyDescription = propIter->second;
		if (!writeProperty(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription))
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::writeProperty(ScriptDefModule* pEntityScriptDefModule,
	ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription)
{
	std::string type = pPropertyDescription->getDataType()->getName();

	if (type == "INT8")
	{
		return writeProperty_INT8(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "INT16")
	{
		return writeProperty_INT16(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "INT32")
	{
		return writeProperty_INT32(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "INT64")
	{
		return writeProperty_INT64(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "UINT8")
	{
		return writeProperty_UINT8(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "UINT16")
	{
		return writeProperty_UINT16(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "UINT32")
	{
		return writeProperty_UINT32(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "UINT64")
	{
		return writeProperty_UINT64(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "FLOAT")
	{
		return writeProperty_FLOAT(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "DOUBLE")
	{
		return writeProperty_DOUBLE(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "STRING")
	{
		return writeProperty_STRING(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "UNICODE")
	{
		return writeProperty_UNICODE(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "PYTHON")
	{
		return writeProperty_PYTHON(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "PY_DICT")
	{
		return writeProperty_PY_DICT(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "PY_TUPLE")
	{
		return writeProperty_PY_TUPLE(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "PY_LIST")
	{
		return writeProperty_PY_LIST(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "BLOB")
	{
		return writeProperty_BLOB(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "ARRAY")
	{
		return writeProperty_ARRAY(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "FIXED_DICT")
	{
		return writeProperty_FIXED_DICT(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
#ifdef CLIENT_NO_FLOAT
	else if (type == "VECTOR2")
	{
		return writeProperty_VECTOR2(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "VECTOR3")
	{
		return writeProperty_VECTOR3(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "VECTOR4")
	{
		return writeProperty_VECTOR4(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
#else
	else if (type == "VECTOR2")
	{
		return writeProperty_VECTOR2(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "VECTOR3")
	{
		return writeProperty_VECTOR3(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "VECTOR4")
	{
		return writeProperty_VECTOR4(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
#endif
	else if (type == "MAILBOX")
	{
		return writeProperty_MAILBOX(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}

	assert(false);
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::writeMethods(ScriptDefModule* pEntityScriptDefModule,
	ScriptDefModule* pCurrScriptDefModule)
{
	sourcefileBody_ += "\n\n";

	ScriptDefModule::METHODDESCRIPTION_MAP& clientMethods = pCurrScriptDefModule->getClientMethodDescriptions();
	ScriptDefModule::METHODDESCRIPTION_MAP::iterator methodIter = clientMethods.begin();
	for (; methodIter != clientMethods.end(); ++methodIter)
	{
		MethodDescription* pMethodDescription = methodIter->second;
		if (!writeMethod(pEntityScriptDefModule, pCurrScriptDefModule, pMethodDescription, "#REPLACE#"))
			return false;

		std::string::size_type fpos = sourcefileBody_.find("#REPLACE#");
		KBE_ASSERT(fpos != std::string::npos);

		std::string argsBody = "";

		std::vector<DataType*>& argTypes = pMethodDescription->getArgTypes();
		std::vector<DataType*>::iterator iter = argTypes.begin();

		int i = 1;

		for (; iter != argTypes.end(); ++iter)
		{
			DataType* pDataType = (*iter);

			if (pDataType->type() == DATA_TYPE_FIXEDARRAY)
			{
				FixedArrayType* pFixedArrayType = static_cast<FixedArrayType*>(pDataType);
				
				std::string argsTypeBody;
				if (!writeMethodArgs_ARRAY(pFixedArrayType, argsTypeBody, pFixedArrayType->aliasName()))
				{
					return false;
				}

				argsBody += fmt::format("{} param{}, ", argsTypeBody, i++);
			}
			else if (pDataType->type() == DATA_TYPE_FIXEDDICT)
			{
				FixedDictType* pFixedDictType = static_cast<FixedDictType*>(pDataType);

				std::string argsTypeBody = typeToType(pFixedDictType->aliasName());
				if (!writeMethodArgs_Const_Ref(pDataType, argsTypeBody))
				{
					return false;
				}

				argsBody += fmt::format("{} param{}, ", argsTypeBody, i++);
			}
			else if (pDataType->type() != DATA_TYPE_DIGIT)
			{
				std::string argsTypeBody = typeToType(pDataType->getName());
				if (!writeMethodArgs_Const_Ref(pDataType, argsTypeBody))
				{
					return false;
				}

				argsBody += fmt::format("{} param{}, ", argsTypeBody, i++);
			}
			else
			{
				argsBody += fmt::format("{} param{}, ", typeToType(pDataType->getName()), i++);
			}
		}

		if (argsBody.size() > 0)
		{
			argsBody.erase(argsBody.size() - 2, 1);
		}

		strutil::kbe_replace(sourcefileBody_, "#REPLACE#", argsBody);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::writeMethodArgs_ARRAY(FixedArrayType* pFixedArrayType, std::string& stackArgsTypeBody, const std::string& childItemName)
{
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::writeMethodArgs_Const_Ref(DataType* pDataType, std::string& stackArgsTypeBody)
{
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientTemplates::writeMethod(ScriptDefModule* pEntityScriptDefModule,
	ScriptDefModule* pCurrScriptDefModule, MethodDescription* pMethodDescription, const char* fillString)
{
	return false;
}

//-------------------------------------------------------------------------------------
}
